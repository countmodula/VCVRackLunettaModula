//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4008
//	4-Bit Full Adder With Parallel Carry Out
//	Copyright (C) 2021  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4008

#define NUM_GATES 4

struct CD4008 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		CARRY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(SUM_OUTPUTS, NUM_GATES),
		CARRY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(SUM_LIGHTS, NUM_GATES),
		CO_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput carryInput;

	const bool sumMap[8] = { false, true, true, false, true, false, false , true };
	const bool carryMap[8] = {false, false, false, true, false, true, true, true };

	CD4008() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
		}
		
		carryInput.reset();
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
		}
		
		carryInput.setMode(mode);

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

		bool carry = carryInput.process(inputs[CARRY_INPUT].getVoltage());

		// process gates
		for (int g = 0; g < NUM_GATES; g++) {
			int i = aInputs[g].process(inputs[A_INPUTS + g].getVoltage()) ? 1 : 0;
			i += bInputs[g].process(inputs[B_INPUTS + g].getVoltage()) ? 2 : 0;
			i += carry ? 4 : 0;
			
			// process the addition for this bit using truth tables
			bool q = sumMap[i];
			carry = carryMap [i];
			
			if (q) {
				outputs[SUM_OUTPUTS + g].setVoltage(gateVoltage);
				lights[SUM_LIGHTS + g].setBrightness(1.0f);
			}
			else {
				outputs[SUM_OUTPUTS + g].setVoltage(0.0f);
				lights[SUM_LIGHTS + g].setBrightness(0.0f);
			}
		}
		
		// process carry out
		if (carry) {
			outputs[CARRY_OUTPUT].setVoltage(gateVoltage);
			lights[CO_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[CARRY_OUTPUT].setVoltage(0.0f);
			lights[CO_LIGHT].setBrightness(0.0f);
		}
	}
};

struct CD4008Widget : ModuleWidget {
	CD4008Widget(CD4008 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4008.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
		
			// A/B inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW1 + g]), module, CD4008::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW1 + g]), module, CD4008::B_INPUTS + g));
			
			// sum outputs
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW1 + g]), module, CD4008::SUM_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5] + 12, STD_ROWS5[STD_ROW1 + g] - 19), module, CD4008::SUM_LIGHTS + g));
		}
		
		// carry
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW5]), module, CD4008::CARRY_INPUT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW5]), module, CD4008::CARRY_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4008::CO_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4008 *module = dynamic_cast<CD4008*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4008 = createModel<CD4008, CD4008Widget>("CD4008");
