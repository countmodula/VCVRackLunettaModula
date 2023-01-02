//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - Truth2
//	2-Bit Logic Truth table
//  Copyright (C) 2023  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME Truth2

#define NUM_BITS 4

struct Truth2 : Module {
	enum ParamIds {
		ENUMS(STATE_PARAMS, NUM_BITS),
		NUM_PARAMS
	};
	enum InputIds {
		A_INPUT,
		B_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		Q_OUTPUT,
		NQ_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		Q_LIGHT,
		NQ_LIGHT,
		ENUMS(STATE_PARAM_LIGHTS, NUM_BITS),
		ENUMS(CURRENT_STATE_LIGHTS, NUM_BITS),
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"

	int processCount = 8;
	int moduleVersion = 1;

	CMOSInput aInput;
	CMOSInput bInput;

	bool states[NUM_BITS] = {};
	
	Truth2() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		for (int g = 0; g < NUM_BITS; g++) {
			configSwitch(STATE_PARAMS + g, 0.0f, 1.0f, 0.0f, rack::string::f("Q state %d", g + 1), {"Off", "On"});
		}
			
		configInput(A_INPUT, "A");
		configInput(B_INPUT, "B");
		
		configOutput(Q_OUTPUT, "Q");
		configOutput(NQ_OUTPUT, "Not Q");
		
		moduleVersion = 1;
	}
	
	void onReset() override {
		aInput.reset();
		bInput.reset();	
		
		for (int g = 0; g < NUM_BITS; g++) {
			states[g] = false;
		}
		
		processCount = 8;
	}
	
	void setIOMode (int mode) {
		
		// set CMOS input properties
		aInput.setMode(mode);
		bInput.setMode(mode);
		
		// set gate voltage
		#include "../modes/setGateVoltage.hpp"
	}	
	
	json_t *dataToJson() override {
		json_t *root = json_object();
		json_object_set_new(root, "moduleVersion", json_integer(2));

		// add the I/O mode details
		#include "../modes/dataToJson.hpp"
		
		return root;
	}
	
	void dataFromJson(json_t *root) override {
		
		json_t *version = json_object_get(root, "moduleVersion");
		if (version)
			moduleVersion = json_number_value(version);
		
		// grab the I/O mode details
		#include "../modes/dataFromJson.hpp"
		
		processCount = 8;
	}	

	void process(const ProcessArgs &args) override {

		// process inputs
		int q = 0;
		if (aInput.process(inputs[A_INPUT].getVoltage())) {
			q += 2;
		}
		
		if (bInput.process(inputs[B_INPUT].getVoltage())) {
			q += 1;
		}
		
		// process buttons and state lights - no need to do this at audio rates
		if (++processCount > 8) {
			processCount = 0;
			
			for (int g = 0; g < NUM_BITS; g++) {
				states[g] = params[STATE_PARAMS + g].getValue() > 0.5f;
				
				lights[CURRENT_STATE_LIGHTS +g].setBrightness(boolToLight(g == q));
			}			
		}

		// set outputs and lights
		if (states[q]) {
			outputs[Q_OUTPUT].setVoltage(gateVoltage);
			lights[Q_LIGHT].setBrightness(1.0f);
			
			outputs[NQ_OUTPUT].setVoltage(0.0f);
			lights[NQ_LIGHT].setBrightness(0.0f);
		}
		else {
			outputs[Q_OUTPUT].setVoltage(0.0f);
			lights[Q_LIGHT].setBrightness(0.0f);
			
			outputs[NQ_OUTPUT].setVoltage(gateVoltage);
			lights[NQ_LIGHT].setBrightness(1.0f);
		}
		
	}
};

struct Truth2Widget : ModuleWidget {
	Truth2Widget(Truth2 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Truth2.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// A/B inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW1]), module, Truth2::A_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1]), module, Truth2::B_INPUT));

		// buttons and state lights
		for (int g = 0; g < NUM_BITS; g++) {
			addParam(createParamCentered<LunettaModulaLEDPushButton<LunettaModulaPBLight<RedLight>>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW2 + g]), module, Truth2::STATE_PARAMS + g, Truth2::STATE_PARAM_LIGHTS + g));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] - 15, STD_ROWS6[STD_ROW2 + g]), module, Truth2::CURRENT_STATE_LIGHTS + g));
		}
		
		// Q output
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW6]), module, Truth2::Q_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] + 12, STD_ROWS6[STD_ROW6] - 19), module, Truth2::Q_LIGHT));
		
		// NQ output
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW6]), module, Truth2::NQ_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW6] - 19), module, Truth2::NQ_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"
	
	void appendContextMenu(Menu *menu) override {
		Truth2 *module = dynamic_cast<Truth2*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
};

Model *modelTruth2 = createModel<Truth2, Truth2Widget>("Truth2");
