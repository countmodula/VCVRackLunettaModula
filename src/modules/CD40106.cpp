//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD40106
//	 Hex Schmitt-Trigger Inverters
//  Copyright (C) 2021  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD40106

#define NUM_GATES 6

struct CD40106 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(I_INPUTS, NUM_GATES),
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
	
	CMOSInput iInputs[NUM_GATES];
	
	CD40106() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(CD40106_SCHMITT); // custom IO mode for this module based on the CD40106 datasheet
		
		char c = 'A';
		for (int g = 0; g < NUM_GATES; g++) {
			configInput(I_INPUTS + g, rack::string::f("Gate %d", g + 1));
			inputInfos[I_INPUTS + g]->description = "Schmitt trigger input with thresholds at approx. 4.6 and 7 volts";
			
			configOutput(Q_OUTPUTS + g, rack::string::f("Gate %d %c (inverted)", g + 1, c++));
		}
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			iInputs[g].reset();
		}
	}
	
	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			iInputs[g].setMode(mode);
		}
		
		// set gate voltage
		#include "../modes/setGateVoltage.hpp"
	}	

	json_t *dataToJson() override {
		json_t *root = json_object();

		json_object_set_new(root, "moduleVersion", json_integer(1));

		return root;
	}
	
	// void dataFromJson(json_t *root) override {
	// }	

	void process(const ProcessArgs &args) override {
		// process gates
		for (int g = 0; g < NUM_GATES; g++) {
			bool q = !iInputs[g].process(inputs[I_INPUTS + g].getVoltage());

			if (q) {
				outputs[Q_OUTPUTS + g].setVoltage(gateVoltage);
				lights[Q_LIGHTS + g].setBrightness(1.0f);
			}
			else {
				outputs[Q_OUTPUTS + g].setVoltage(0.0f);
				lights[Q_LIGHTS + g].setBrightness(0.0f);
			}
		}	
	}
};

struct CD40106Widget : ModuleWidget {
	CD40106Widget(CD40106 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD40106.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
		
			// inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW1 + g]), module, CD40106::I_INPUTS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1 + g]), module, CD40106::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW1 + g] - 19), module, CD40106::Q_LIGHTS + g));
		}
	}	
};

Model *modelCD40106 = createModel<CD40106, CD40106Widget>("CD40106");
