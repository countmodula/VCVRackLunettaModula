//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4086
//	Expandable 4-Wide 2-Input AND-OR-INVERT Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4086

#define NUM_GATES 4

struct CD4086 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		INH_INPUT,
		EN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		J_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		J_LIGHT,
		INH_LIGHT,
		EN_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput inhInput;
	CMOSInput enInput;
	
	CD4086() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void onReset() override {
		
		// gate inputs
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
		}
		
		// control inputs
		inhInput.reset();
		enInput.reset();
	}
	
	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
		}
		
		inhInput.setMode(mode);
		enInput.setMode(mode);
		
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
		for (int g = 0; g < NUM_GATES; g++) {
			bool a = aInputs[g].process(inputs[A_INPUTS + g].getVoltage());
			bool b = bInputs[g].process(inputs[B_INPUTS + g].getVoltage());
			q |= (a && b);
		}		

		bool inhibit = inhInput.process(inputs[INH_INPUT].getVoltage());
		bool enable = enInput.process(inputs[EN_INPUT].getNormalVoltage(gateVoltage));
		
		lights[INH_LIGHT].setBrightness(boolToLight(inhibit));
		lights[EN_LIGHT].setBrightness(boolToLight(enable));
		
		bool j = (inhibit || !enable || q);
		
		outputs[J_OUTPUT].setVoltage(boolToGateInverted(j));
		lights[J_LIGHT].setBrightness(boolToLightInverted(j));
	}
};

struct CD4086Widget : ModuleWidget {
	CD4086Widget(CD4086 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4086.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int i = 0; i < NUM_GATES; i++) {
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW1 + i]), module, CD4086::A_INPUTS + i));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1 + i]), module, CD4086::B_INPUTS + i));
		}
		
		// inhibit and enable inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW5]), module, CD4086::EN_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW5]), module, CD4086::INH_INPUT));
		
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW6] - 10), module, CD4086::EN_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW6] + 13), module, CD4086::INH_LIGHT));
		
		// J output/light
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW6]), module, CD4086::J_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] + 12, STD_ROWS6[STD_ROW6] - 19), module, CD4086::J_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4086 *module = dynamic_cast<CD4086*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4086 = createModel<CD4086, CD4086Widget>("CD4086");
