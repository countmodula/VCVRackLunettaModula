//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4012
//	Dual 4 Input NAND Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

#define NUM_GATES 2

struct CD4012 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		ENUMS(C_INPUTS, NUM_GATES),
		ENUMS(D_INPUTS, NUM_GATES),
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
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput cInputs[NUM_GATES];
	CMOSInput dInputs[NUM_GATES];
	
	CD4012() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
			cInputs[g].reset();
			dInputs[g].reset();
		}
	}

	json_t *dataToJson() override {
		json_t *root = json_object();

		json_object_set_new(root, "moduleVersion", json_integer(1));
		
		return root;
	}
	
	void dataFromJson(json_t *root) override {

	}	

	void process(const ProcessArgs &args) override {
		
		// process gates
		for (int g = 0; g < NUM_GATES; g++) {
			bool q = aInputs[g].process(inputs[A_INPUTS + g].getVoltage());
			q &= bInputs[g].process(inputs[B_INPUTS + g].getVoltage());
			q &= cInputs[g].process(inputs[C_INPUTS + g].getVoltage());
			q &= dInputs[g].process(inputs[D_INPUTS + g].getVoltage());

			outputs[Q_OUTPUTS + g].setVoltage(boolToGateInverted(q));
			lights[Q_LIGHTS + g].setBrightness(boolToLightInverted(q));
		}		
	}
};

struct CD4012Widget : ModuleWidget {
	CD4012Widget(CD4012 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4012.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		int cols[NUM_GATES] = {STD_COL1, STD_COL3};
		for (int g = 0; g < NUM_GATES; g++) {
		
			// A/B/C/D inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS5[STD_ROW1]), module, CD4012::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS5[STD_ROW2]), module, CD4012::B_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS5[STD_ROW3]), module, CD4012::C_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS5[STD_ROW4]), module, CD4012::D_INPUTS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS5[STD_ROW5]), module, CD4012::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[g]] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4012::Q_LIGHTS + g));
		}
	}
};

Model *modelCD4012 = createModel<CD4012, CD4012Widget>("CD4012");
