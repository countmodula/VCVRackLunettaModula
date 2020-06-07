//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4017
//	Decade Counter/Divider
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4017

#define MAX_COUNT 10
#define CARRY_COUNT 5
 
struct CD4017 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		INHIBIT_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(DECODED_OUTPUTS, MAX_COUNT),
		CARRY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(DECODED_LIGHTS, MAX_COUNT),
		CARRY_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput clockInput;
	CMOSInput inhibitInput;
	CMOSInput resetInput;
	
	int count = 0;
	
	CD4017() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void onReset() override {
		clockInput.reset();
		inhibitInput.reset();
		resetInput.reset();
		
		count = 0;
	}

	void setIOMode (int mode) {
		clockInput.setMode(mode);
		inhibitInput.setMode(mode);
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
			bool enable = !inhibitInput.process(inputs[INHIBIT_INPUT].getVoltage());
			bool clock = enable && clockInput.process(inputs[CLOCK_INPUT].getVoltage());
			
			if (!prevClock && clock) {
				if (++count >= MAX_COUNT)
					count = 0;
			}
		}
		
		// decode the outputs
		for (int i = 0; i < MAX_COUNT; i++) {
			bool q = (i == count);
			outputs[DECODED_OUTPUTS + i].setVoltage(boolToGate(q));
			lights[DECODED_LIGHTS + i].setBrightness(boolToLight(q));
		}
		
		if(count < CARRY_COUNT) {
			outputs[CARRY_OUTPUT].setVoltage(gateVoltage);
			lights[CARRY_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[CARRY_OUTPUT].setVoltage(0.0f);
			lights[CARRY_LIGHT].setBrightness(0.0f);
		}		
	}
};

struct CD4017Widget : ModuleWidget {
	CD4017Widget(CD4017 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4017.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// clock/inhibit and reset inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW1]), module, CD4017::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW2]), module, CD4017::INHIBIT_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS5(STD_ROW3)), module, CD4017::RESET_INPUT));

		// gates
		int cols[2] = {STD_COL3, STD_COL5};
		int i = 0;
		for (int c = 0; c < 2; c++) {
			for (int r = 0; r < CARRY_COUNT; r++) {
				// decoded outputs
				addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[c]], STD_ROWS5[STD_ROW1 + r]), module, CD4017::DECODED_OUTPUTS + i));
				addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[c]] + 12, STD_ROWS5[STD_ROW1 + r] - 19), module, CD4017::DECODED_LIGHTS + i));
				
				i++;
			}
		}
		
		// carry out
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW5]), module, CD4017::CARRY_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4017::CARRY_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4017 *module = dynamic_cast<CD4017*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4017 = createModel<CD4017, CD4017Widget>("CD4017");
