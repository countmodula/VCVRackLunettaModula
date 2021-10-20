//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4019
//	Quad AND-OR Select Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4019

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
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput kaInput;
	CMOSInput kbInput;
	
	int prevSelect = -1;
	
	CD4019() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		for (int i = 0; i < NUM_GATES; i ++) {
			configInput(A_INPUTS + i, rack::string::f("A%d", i + 1));
			configInput(B_INPUTS + i, rack::string::f("B%d", i + 1));
			configOutput(Q_OUTPUTS + i, rack::string::f("D%d", i + 1));
		}
		
		configInput(KA_INPUT, "KA");
		configInput(KB_INPUT, "KB");

		
		
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
		}
		
		kaInput.reset();
		kbInput.reset();
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
		}
		
		kaInput.setMode(mode);
		kbInput.setMode(mode);
		
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
		
		// process select function
		int select = (kaInput.process(inputs[KA_INPUT].getVoltage()) ? 0x01 : 0x00);
		select += (kbInput.process(inputs[KB_INPUT].getVoltage()) ? 0x02: 0x00);
		
		// process gates
		int out = Q_OUTPUTS;
		int led = Q_LIGHTS;
		int aIn = A_INPUTS;
		int bIn = B_INPUTS;
		for (int g = 0; g < NUM_GATES; g++) {
			bool a = aInputs[g].process(inputs[aIn++].getVoltage());
			bool b = bInputs[g].process(inputs[bIn++].getVoltage()); 

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

			if (q) {
				outputs[out++].setVoltage(gateVoltage);
				lights[led++].setBrightness(1.0f);
			}
			else {
				outputs[out++].setVoltage(0.0f);
				lights[led++].setBrightness(0.0f);
			}
		}

		// show status
		if (select != prevSelect) {
			int j = 1;
			
			prevSelect = select;
			
			for (int i = 0; i < 3; i++)
				lights[STATUS_LIGHTS + i].setBrightness(boolToLight(j++ == select));
		}
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

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4019 *module = dynamic_cast<CD4019*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4019 = createModel<CD4019, CD4019Widget>("CD4019");
