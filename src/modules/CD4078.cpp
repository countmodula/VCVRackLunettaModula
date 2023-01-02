//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4078
//	8 Input OR/NOR Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4078

struct CD4078 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, 8),
		NUM_INPUTS
	};
	enum OutputIds {
		J_OUTPUT,
		K_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		J_LIGHT,
		K_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[8];
	
	CD4078() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		char c = 'A';
		for (int i = 0; i < 8; i ++)
			configInput(A_INPUTS + i, rack::string::f("%c", c++));
		
		configOutput(J_OUTPUT, "J");
		configOutput(K_OUTPUT, "K");
		
		outputInfos[J_OUTPUT]->description = "NOR";
		outputInfos[K_OUTPUT]->description = "OR";		
		
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < 8; g++) {
			aInputs[g].reset();
		}
	}
	
	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < 8; g++) {
			aInputs[g].setMode(mode);
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
		
		// process gates
		bool q = false;
		for (int i = 0; i < 8; i++) {
			q |= aInputs[i].process(inputs[A_INPUTS + i].getVoltage());
		}

		if (q) {
			outputs[J_OUTPUT].setVoltage(0.0f);
			lights[J_LIGHT].setBrightness(0.0f);
			
			outputs[K_OUTPUT].setVoltage(gateVoltage);
			lights[K_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[J_OUTPUT].setVoltage(gateVoltage);
			lights[J_LIGHT].setBrightness(1.0f);
			
			outputs[K_OUTPUT].setVoltage(0.0f);
			lights[K_LIGHT].setBrightness(0.0f);
		}
	}
};

struct CD4078Widget : ModuleWidget {
	CD4078Widget(CD4078 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4078.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int i = 0; i < 8; i++) {
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[i < 4 ? STD_COL1 : STD_COL3], STD_ROWS5[STD_ROW1 + (i % 4)]), module, CD4078::A_INPUTS + i));
		}
		
		// Q output
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW5]), module, CD4078::J_OUTPUT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW5]), module, CD4078::K_OUTPUT));
		
		// lights
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4078::J_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4078::K_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4078 *module = dynamic_cast<CD4078*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4078 = createModel<CD4078, CD4078Widget>("CD4078");
