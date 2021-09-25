//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4063
//	4-Bit Magnitude Comparator
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4063

#define NUM_GATES 4

struct CD4063 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		LT_INPUT,
		EQ_INPUT,
		GT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_GATES),
		LT_OUTPUT,
		EQ_OUTPUT,
		GT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_GATES),
		LT_LIGHT,
		EQ_LIGHT,
		GT_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput ltInput;
	CMOSInput eqInput;
	CMOSInput gtInput;
	
	CD4063() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
		}
		
		ltInput.reset();
		eqInput.reset();
		gtInput.reset();
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
		}
		
		ltInput.setMode(mode);
		eqInput.setMode(mode);
		gtInput.setMode(mode);
		
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
		int a = 0, b = 0, v = 1;
		for (int g = 0; g < NUM_GATES; g++) {
			
			if (aInputs[g].process(inputs[A_INPUTS + g].getVoltage()))
				a += v;

			if (bInputs[g].process(inputs[B_INPUTS + g].getVoltage()))
				b += v;

			v *= 2;
		}
	
		if (a == b) {
			// input bits are equal, output the cascade iputs
			bool ltIn = ltInput.process(inputs[LT_INPUT].getVoltage());
			bool eqIn = gtInput.process(inputs[EQ_INPUT].getNormalVoltage(gateVoltage));
			bool gtIn = gtInput.process(inputs[GT_INPUT].getVoltage());

			if (ltIn) {
				outputs[LT_OUTPUT].setVoltage(gateVoltage);
				lights[LT_LIGHT].setBrightness(1.0f);
			}
			else {
				outputs[LT_OUTPUT].setVoltage(0.0f);
				lights[LT_LIGHT].setBrightness(0.0f);
			}
			
			if (eqIn) {
				outputs[EQ_OUTPUT].setVoltage(gateVoltage);
				lights[EQ_LIGHT].setBrightness(1.0f);
			}
			else {
				outputs[EQ_OUTPUT].setVoltage(0.0f);
				lights[EQ_LIGHT].setBrightness(0.0f);
			}
			
			if (gtIn) {
				outputs[GT_OUTPUT].setVoltage(gateVoltage);
				lights[GT_LIGHT].setBrightness(1.0f);
			}
			else {
				outputs[GT_OUTPUT].setVoltage(0.0f);
				lights[GT_LIGHT].setBrightness(0.0f);
			}
		}
		else {
			// not equal, process lt/gt
			bool lt = (a < b);
			if (lt) {
				outputs[LT_OUTPUT].setVoltage(gateVoltage);
				lights[LT_LIGHT].setBrightness(1.0f);
				
				outputs[GT_OUTPUT].setVoltage(0.0f);
				lights[GT_LIGHT].setBrightness(0.0f);
			}
			else {
				outputs[LT_OUTPUT].setVoltage(0.0f);
				lights[LT_LIGHT].setBrightness(0.0f);
				
				outputs[GT_OUTPUT].setVoltage(gateVoltage);
				lights[GT_LIGHT].setBrightness(1.0f);
			}
			
			outputs[EQ_OUTPUT].setVoltage(0.0f);
			lights[EQ_LIGHT].setBrightness(0.0f);
		}
	}
};

struct CD4063Widget : ModuleWidget {
	CD4063Widget(CD4063 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4063.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
			// A/B inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW1 + g]), module, CD4063::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW1 + g]), module, CD4063::B_INPUTS + g));
		}
		
		// cascade inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW5]), module, CD4063::LT_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW6]), module, CD4063::EQ_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW7]), module, CD4063::GT_INPUT));

		// outputs
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW5]), module, CD4063::LT_OUTPUT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW6]), module, CD4063::EQ_OUTPUT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW7]), module, CD4063::GT_OUTPUT));
			
		// lights
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS7[STD_ROW5] - 19), module, CD4063::LT_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS7[STD_ROW6] - 19), module, CD4063::EQ_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS7[STD_ROW7] - 19), module, CD4063::GT_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4063 *module = dynamic_cast<CD4063*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4063 = createModel<CD4063, CD4063Widget>("CD4063");
