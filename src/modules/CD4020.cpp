//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4020
//	14-Stage Ripple-Carry Binary Counter/Divider
//	Copyright (C) 2021 Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4020

#define MAX_COUNT 16384
#define NUM_BITS 14
#define NUM_BITS_PLUS_1 15

struct CD4020 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(DIVIDE_OUTPUTS, NUM_BITS),
		CARRY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(DIVIDE_LIGHTS, NUM_BITS),
		CARRY_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput clockInput;
	CMOSInput resetInput;

	const int bitmap[NUM_BITS_PLUS_1] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192 };

	int count = 0;
	bool update = true;
	bool reset = false;
	
	CD4020() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
		update = true;
		reset = false;
	}
	
	void onReset() override {
		clockInput.reset();
		resetInput.reset();
		count = 0;
		update = true;
		reset = true;
	}

	void setIOMode (int mode) {
		clockInput.setMode(mode);
		resetInput.setMode(mode);
		
		// set gate voltage
		#include "../modes/setGateVoltage.hpp"
	}

	json_t *dataToJson() override {
		json_t *root = json_object();

		json_object_set_new(root, "moduleVersion", json_integer(1));
		
		// add the I/O mode details
		#include "../modes/dataToJson.hpp"		

		return root;
	}
	
	void dataFromJson(json_t *root) override {

		// grab the I/O mode details
		#include "../modes/dataFromJson.hpp"
	}	

	void process(const ProcessArgs &args) override {

		// are we reset?
		if (resetInput.process(inputs[RESET_INPUT].getVoltage())) {
			count = 0;
			
			if (!reset)
				update = true;
				
			reset = true;
		}
		else {
			// process the clock
			bool prevClock = clockInput.isHigh();
			bool clock = clockInput.process(inputs[CLOCK_INPUT].getVoltage());
			
			// note - negative edge!
			if (prevClock && !clock) {
				if (++count >= MAX_COUNT)
					count = 0;

				update = true;
				reset = false;
			}
		}
		
		// decode the outputs
		int g = 0;
		for (int i = 1; i < NUM_BITS_PLUS_1; i++) {
			bool q = (count & bitmap[i]) > 0;

			if (q) {
				outputs[DIVIDE_OUTPUTS + g].setVoltage(gateVoltage);
				if (update)
					lights[DIVIDE_LIGHTS + g].setBrightness(1.0f);
			}
			else {
				outputs[DIVIDE_OUTPUTS + g].setVoltage(0.0f);
				if (update)
					lights[DIVIDE_LIGHTS + g].setBrightness(0.0f);
			}

			g++;
		}
		
		update = false;
	}
};

struct CD4020Widget : ModuleWidget {
	CD4020Widget(CD4020 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4020.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// clock/inhibit and reset inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW1]), module, CD4020::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW2]), module, CD4020::RESET_INPUT));

		// gates
		int cols[2] = {STD_COL3, STD_COL5};
		int i = 0;
		for (int c = 0; c < 2; c++) {
			for (int r = 0; r < 7; r++) {
				// decoded outputs
				addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[c]], STD_ROWS7[STD_ROW1 + r]), module, CD4020::DIVIDE_OUTPUTS + i));
				addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[c]] + 12, STD_ROWS7[STD_ROW1 + r] - 19), module, CD4020::DIVIDE_LIGHTS + i));
				
				i++;
			}
		}
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4020 *module = dynamic_cast<CD4020*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4020 = createModel<CD4020, CD4020Widget>("CD4020");
