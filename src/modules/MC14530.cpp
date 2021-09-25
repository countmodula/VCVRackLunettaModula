//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - MC14530
//	Dual 5 Input Majority Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME MC14530

#define NUM_GATES 5

struct MC14530 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		WA_INPUT,
		WB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ZA_OUTPUT,
		ZB_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ZA_LIGHT,
		ZB_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput waInput;
	CMOSInput wbInput;
	
	MC14530() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
		}
		
		waInput.reset();
		wbInput.reset();
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
		}
		
		waInput.setMode(mode);
		wbInput.setMode(mode);
		
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
		
		int a = 0, b = 0;
		for (int g = 0; g < NUM_GATES; g++) {
			
			if (aInputs[g].process(inputs[A_INPUTS + g].getVoltage()))
				a++;

			if (bInputs[g].process(inputs[B_INPUTS + g].getVoltage()))
				b++;
		}
	
		bool za = waInput.process(inputs[WA_INPUT].getNormalVoltage(gateVoltage)) == (a > 2);
		bool zb = wbInput.process(inputs[WB_INPUT].getNormalVoltage(gateVoltage)) == (b > 2);
	
		if (za) {
			outputs[ZA_OUTPUT].setVoltage(gateVoltage);
			lights[ZA_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[ZA_OUTPUT].setVoltage(0.0f);
			lights[ZA_LIGHT].setBrightness(0.0f);
		}

		if (zb) {
			outputs[ZB_OUTPUT].setVoltage(gateVoltage);
			lights[ZB_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[ZB_OUTPUT].setVoltage(0.0f);
			lights[ZB_LIGHT].setBrightness(0.0f);
		}
	}
};

struct MC14530Widget : ModuleWidget {
	MC14530Widget(MC14530 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MC14530.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
			// A/B inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW1 + g]), module, MC14530::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW1 + g]), module, MC14530::B_INPUTS + g));
		}
		
		// w inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW6]), module, MC14530::WA_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW6]), module, MC14530::WB_INPUT));

		// outputs
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW7]), module, MC14530::ZA_OUTPUT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW7]), module, MC14530::ZB_OUTPUT));
			
		// lights
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] + 12, STD_ROWS7[STD_ROW7] - 19), module, MC14530::ZA_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS7[STD_ROW7] - 19), module, MC14530::ZB_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		MC14530 *module = dynamic_cast<MC14530*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelMC14530 = createModel<MC14530, MC14530Widget>("MC14530");
