//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4019
//	Quad AND-OR Select Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

#define NUM_GATES 4

struct CD4019 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		KA_INPUT,
		KB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_GATES),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_GATES),
		ENUMS(STATUS_LIGHTS, 3),
		NUM_LIGHTS
	};
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput kaInput;
	CMOSInput kbInput;
	
	CD4019() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
		}
		
		kaInput.reset();
		kbInput.reset();
	}

	json_t *dataToJson() override {
		json_t *root = json_object();

		json_object_set_new(root, "moduleVersion", json_integer(1));
		
		return root;
	}
	
	void dataFromJson(json_t *root) override {

	}	

	void process(const ProcessArgs &args) override {
		
		// process select function
		int select = (kaInput.process(inputs[KA_INPUT].getVoltage()) ? 0x01 : 0x00);
		select += (kbInput.process(inputs[KB_INPUT].getVoltage()) ? 0x02: 0x00);
		
		// process gates
		for (int g = 0; g < NUM_GATES; g++) {
			
			bool a = aInputs[g].process(inputs[A_INPUTS + g].getVoltage());
			bool b = bInputs[g].process(inputs[B_INPUTS + g].getVoltage()); 

			bool q = false;
			switch (select) {
				case 0: // Always low
					break;
				case 1: // A input
					q = a;
					break;
				case 2: // B Input
					q = b;
					break;
				case 3: // OR function
					q = a || b;
					break;
			}

			outputs[Q_OUTPUTS + g].setVoltage(boolToGate(q));
			lights[Q_LIGHTS + g].setBrightness(boolToLight(q));
		}

		// show status
		int j = 1;
		for (int i = 0; i < 3; i++)
			lights[STATUS_LIGHTS + i].setBrightness(boolToLight(j++ == select));

	}
};

struct CD4019Widget : ModuleWidget {
	CD4019Widget(CD4019 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4019.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
		
			// A/B inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW1 + g]), module, CD4019::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW1 + g]), module, CD4019::B_INPUTS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW1 + g]), module, CD4019::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5] + 12, STD_ROWS5[STD_ROW1 + g] - 19), module, CD4019::Q_LIGHTS + g));
		}
		
		// select inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW5]), module, CD4019::KA_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW5]), module, CD4019::KB_INPUT));
		
		// status lights
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW5] - 10), module, CD4019::STATUS_LIGHTS));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW5]), module, CD4019::STATUS_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW5] + 10), module, CD4019::STATUS_LIGHTS + 2));
	}
};

Model *modelCD4019 = createModel<CD4019, CD4019Widget>("CD4019");
