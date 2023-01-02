//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4022
//	Decade Counter/Divider
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4022

#define MAX_COUNT 8
#define CARRY_COUNT 4
 
struct CD4022 : Module {
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
	bool reset = false;
	bool carry = false;
	bool update = false;
	
	CD4022() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configInput(CLOCK_INPUT, "Clock");
		configInput(INHIBIT_INPUT, "Inhibit");
		configInput(RESET_INPUT, "Reset");
		inputInfos[INHIBIT_INPUT]->description = "Disables the clock and inhibits the count";
		inputInfos[RESET_INPUT]->description = "Resets count to 0";
		
		for (int c = 0; c < MAX_COUNT; c++)
			configOutput(DECODED_OUTPUTS + c, rack::string::f("Decoded %d", c));
			
		configOutput(CARRY_OUTPUT, "Carry");		
		
		setIOMode(VCVRACK_STANDARD);
		reset = false;
		carry = false;
		update = true;
	}
	
	void onReset() override {
		clockInput.reset();
		inhibitInput.reset();
		resetInput.reset();
		
		count = 0;
		carry = false;
		reset = false;
		update = true;
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
			carry = false;
			
			if (!reset)
				update = true;
				
			reset = true;
		}
		else {
			// process the clock
			bool prevClock = clockInput.isHigh();
			bool enable = !inhibitInput.process(inputs[INHIBIT_INPUT].getVoltage());
			bool clock = enable && clockInput.process(inputs[CLOCK_INPUT].getVoltage());
			
			if (!prevClock && clock) {
				if (++count >= MAX_COUNT) {
					carry = true;
					count = 0;
				}
				else if (count >= CARRY_COUNT)
					carry = false;
					
				update = true;
				reset = false;
			}
		}
		
		// decode the outputs
		int out = DECODED_OUTPUTS;
		int led = DECODED_LIGHTS;
		
		for (int i = 0; i < MAX_COUNT; i++) {
			bool q = (i == count);
			if (q) {
				outputs[out++].setVoltage(gateVoltage);
				if (update)
					lights[led++].setBrightness(1.0f);
			}
			else {
				outputs[out++].setVoltage(0.0f);
				if (update)
					lights[led++].setBrightness(0.0f);
			}
		}
		
		if(carry) {
			outputs[CARRY_OUTPUT].setVoltage(gateVoltage);
			if (update)
				lights[CARRY_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[CARRY_OUTPUT].setVoltage(0.0f);
			if (update)
				lights[CARRY_LIGHT].setBrightness(0.0f);
		}
		
		update = false;
	}
};

struct CD4022Widget : ModuleWidget {
	CD4022Widget(CD4022 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4022.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// clock/inhibit and reset inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW1]), module, CD4022::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW2]), module, CD4022::INHIBIT_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS5(STD_ROW3)), module, CD4022::RESET_INPUT));

		// gates
		int cols[2] = {STD_COL3, STD_COL5};
		int i = 0;
		for (int c = 0; c < 2; c++) {
			for (int r = 0; r < CARRY_COUNT; r++) {
				// decoded outputs
				addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[c]], STD_ROWS5[STD_ROW1 + r]), module, CD4022::DECODED_OUTPUTS + i));
				addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[c]] + 12, STD_ROWS5[STD_ROW1 + r] - 19), module, CD4022::DECODED_LIGHTS + i));
				
				i++;
			}
		}
		
		// carry out
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW5]), module, CD4022::CARRY_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4022::CARRY_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4022 *module = dynamic_cast<CD4022*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4022 = createModel<CD4022, CD4022Widget>("CD4022");
