//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4024
//	7-Stage Ripple-Carry Binary Counter/Divider
//	Copyright (C) 2021 Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4024

#define MAX_COUNT 128
#define NUM_BITS 7
#define NUM_BITS_PLUS_1 8

struct CD4024 : Module {
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
	
	bool prevQ[NUM_BITS] = {};
	
	const int bitmap[NUM_BITS_PLUS_1] = { 0, 1, 2, 4, 8, 16, 32, 64};

	int count = 0;
	int prevCount = -1;
	
	CD4024() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
		prevCount = -1;
	}
	
	void onReset() override {
		clockInput.reset();
		resetInput.reset();

		count = 0;
		prevCount = -1;
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
		}
		else {
			// process the clock
			bool prevClock = clockInput.isHigh();
			bool clock = clockInput.process(inputs[CLOCK_INPUT].getVoltage());
			
			// note - negative edge!
			if (prevClock && !clock) {
				if (++count >= MAX_COUNT)
					count = 0;
			}
		}
		
		// decode the outputs
		if (count != prevCount) {
			int g = 0;
			for (int i = 1; i < NUM_BITS_PLUS_1; i++) {
				bool q = (count & bitmap[i]) > 0;

				if (q) {
					outputs[DIVIDE_OUTPUTS + g].setVoltage(gateVoltage);
					lights[DIVIDE_LIGHTS + g].setBrightness(1.0f);
				}
				else {
					outputs[DIVIDE_OUTPUTS + g].setVoltage(0.0f);
					lights[DIVIDE_LIGHTS + g].setBrightness(0.0f);
				}

				g++;
			}
		}
		
		prevCount = count;
	}
};

struct CD4024Widget : ModuleWidget {
	CD4024Widget(CD4024 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4024.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// clock/inhibit and reset inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW1]), module, CD4024::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW2]), module, CD4024::RESET_INPUT));

		// gates
		int i = 0;
		for (int r = 0; r < 7; r++) {
			// decoded outputs
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW1 + r]), module, CD4024::DIVIDE_OUTPUTS + i));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS7[STD_ROW1 + r] - 19), module, CD4024::DIVIDE_LIGHTS + i));
			
			i++;
		}

	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4024 *module = dynamic_cast<CD4024*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4024 = createModel<CD4024, CD4024Widget>("CD4024");
