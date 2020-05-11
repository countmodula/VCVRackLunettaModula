//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4023
//	Triple 3 Input NAND Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4023

#define NUM_GATES 3

struct CD4023 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		ENUMS(C_INPUTS, NUM_GATES),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_GATES),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_GATES),
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"

	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput cInputs[NUM_GATES];
	
	CD4023() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
			cInputs[g].reset();
		}
	}
	
	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
			cInputs[g].setMode(mode);
		}
		
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
		
		for (int g = 0; g < NUM_GATES; g++) {
			bool q = aInputs[g].process(inputs[A_INPUTS + g].getVoltage());
			q &= bInputs[g].process(inputs[B_INPUTS + g].getVoltage());
			q &= cInputs[g].process(inputs[C_INPUTS + g].getVoltage());

			outputs[Q_OUTPUTS + g].setVoltage(boolToGateInverted(q));
			lights[Q_LIGHTS + g].setBrightness(boolToLightInverted(q));
		}		
	}
};

struct CD4023Widget : ModuleWidget {
	CD4023Widget(CD4023 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4023.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		int offset = 0;
		for (int g = 0; g < NUM_GATES; g++) {
		
			// A/B/C inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW1 + offset]), module, CD4023::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1 + offset]), module, CD4023::B_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW2 + offset]), module, CD4023::C_INPUTS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW2 + offset]), module, CD4023::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW2 + offset] - 19), module, CD4023::Q_LIGHTS + g));
			
			offset += 2;
		}
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4023 *module = dynamic_cast<CD4023*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4023 = createModel<CD4023, CD4023Widget>("CD4023");
