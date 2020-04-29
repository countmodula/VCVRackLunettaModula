//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4000
//	Dual 4 Input NOR Gate with Inverter
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4000

#define NUM_GATES 2

struct CD4000 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		ENUMS(C_INPUTS, NUM_GATES),
		G_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_GATES),
		L_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_GATES),
		L_LIGHT,
		NUM_LIGHTS
	};

	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput cInputs[NUM_GATES];
	CMOSInput gInput;
	
	CD4000() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
			cInputs[g].reset();
		}
		
		gInput.reset();
	}
	
	void setIOMode (int mode) {
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
			cInputs[g].setMode(mode);
		}
		
		gInput.setMode(mode);
		
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
		for (int g = 0; g < NUM_GATES; g++) {
			bool q = aInputs[g].process(inputs[A_INPUTS + g].getVoltage());
			q |= bInputs[g].process(inputs[B_INPUTS + g].getVoltage()); 
			q |= cInputs[g].process(inputs[C_INPUTS + g].getVoltage());

			outputs[Q_OUTPUTS + g].setVoltage(boolToGateInverted(q));
			lights[Q_LIGHTS + g].setBrightness(boolToLightInverted(q));
		}		
		
		// inverter
		bool inv = gInput.process(inputs[G_INPUT].getVoltage());
		outputs[L_OUTPUT].setVoltage(boolToGateInverted(inv));
		lights[L_LIGHT].setBrightness(boolToLightInverted(inv));
	}
};

struct CD4000Widget : ModuleWidget {
	CD4000Widget(CD4000 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4000.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		int cols[NUM_GATES] = {STD_COL1, STD_COL3};
		for (int g = 0; g < NUM_GATES; g++) {
		
			// A/B/C inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS6[STD_ROW1] + 14), module, CD4000::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS6[STD_ROW2] + 14), module, CD4000::B_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS6[STD_ROW3] + 14), module, CD4000::C_INPUTS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS6[STD_ROW4] + 14), module, CD4000::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[g]] + 12, STD_ROWS6[STD_ROW4] - 5), module, CD4000::Q_LIGHTS + g));
		}
		
		// inverter
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW6] - 14), module, CD4000::G_INPUT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW6] - 14), module, CD4000::L_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW6] - 33), module, CD4000::L_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

	void appendContextMenu(Menu *menu) override {
		CD4000 *module = dynamic_cast<CD4000*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}		
};

Model *modelCD4000 = createModel<CD4000, CD4000Widget>("CD4000");
