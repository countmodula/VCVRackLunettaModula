//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4042
//	Quad Clocked "D" Latch
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4042

#define NUM_GATES 4

struct CD4042 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(D_INPUTS, NUM_GATES),
		CLOCK_INPUT,
		POLARITY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_GATES),
		ENUMS(NQ_OUTPUTS, NUM_GATES),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_GATES),
		ENUMS(NQ_LIGHTS, NUM_GATES),
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	bool qValues[NUM_GATES] = {};
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput clockInput;
	CMOSInput polarityInput;
	
	CD4042() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			qValues[g] = false;
		}
		
		clockInput.reset();
		polarityInput.reset();
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
		}
		
		clockInput.setMode(mode);
		polarityInput.setMode(mode);
		
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

		// process polarity function
		bool clock = clockInput.process(inputs[CLOCK_INPUT].getVoltage());
		bool polarity = polarityInput.process(inputs[POLARITY_INPUT].getVoltage());
		
		bool latch = (polarity != clock);
		
		// process gates
		for (int g = 0; g < NUM_GATES; g++) {
			
			bool q = qValues[g] = (latch ? qValues[g] : aInputs[g].process(inputs[D_INPUTS + g].getVoltage()));

			outputs[Q_OUTPUTS + g].setVoltage(boolToGate(q));
			outputs[NQ_OUTPUTS + g].setVoltage(boolToGateInverted(q));
			lights[Q_LIGHTS + g].setBrightness(boolToLight(q));
			lights[NQ_LIGHTS + g].setBrightness(boolToLightInverted(q));
		}
	}
};

struct CD4042Widget : ModuleWidget {
	CD4042Widget(CD4042 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4042.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
		
			// D inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW1 + g]), module, CD4042::D_INPUTS + g));
			
			// Q/NQ outputs
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW1 + g]), module, CD4042::Q_OUTPUTS + g));
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW1 + g]), module, CD4042::NQ_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS5[STD_ROW1 + g] - 19), module, CD4042::Q_LIGHTS + g));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5] + 12, STD_ROWS5[STD_ROW1 + g] - 19), module, CD4042::NQ_LIGHTS + g));
		}
		
		// select inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL2], STD_ROWS5[STD_ROW5]), module, CD4042::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL4], STD_ROWS5[STD_ROW5]), module, CD4042::POLARITY_INPUT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4042 *module = dynamic_cast<CD4042*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4042 = createModel<CD4042, CD4042Widget>("CD4042");
