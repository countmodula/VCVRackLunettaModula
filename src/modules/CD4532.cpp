//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4532
//	8-Bit Priority Encoder
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4532

struct CD4532 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(DIGITAL_INPUTS, 8),
		E_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(BIN_OUPUTS, 3),
		E_OUTPUT,
		GS_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(BIN_LIGHTS, 3),
		E_LIGHT,
		GS_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput digitalInputs[8];
	CMOSInput eInput;
	
	// maps 0-7 to binary values - should be quicker than converting
	const bool outputMmap[8][3] = { {false, false, false},
									{true, false, false},
									{false, true, false},
									{true, true, false},
									{false, false, true},
									{true, false, true},
									{false, true, true},
									{true, true, true}};

	CD4532() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int b = 0; b < 8; b++)
			digitalInputs[b].reset();
		
		eInput.reset();	
	}
	
	void setIOMode (int mode) {
		
		for (int b = 0; b < 8; b++)
			digitalInputs[b].setMode(mode);

		eInput.setMode(mode);

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

		if (eInput.process(inputs[E_INPUT].getVoltage())) {
			bool groupSelect = false;
			int priority = 0;
			for (int b = 7; b >= 0; b--) {
				if (digitalInputs[b].process(inputs[DIGITAL_INPUTS + b].getVoltage())) {
					priority = b;
					groupSelect = true;
					break;
				}
			}

			for (int i = 0; i < 3; i++) {
				bool out = groupSelect ? outputMmap[priority][i] : false;
				
				if (out) {
					outputs[BIN_OUPUTS + i].setVoltage(gateVoltage);
					lights[BIN_LIGHTS + i].setBrightness(1.0f);
				}
				else {
					outputs[BIN_OUPUTS + i].setVoltage(0.0f);
					lights[BIN_LIGHTS + i].setBrightness(0.0f);
				}
			}
			
			if (groupSelect) {
				outputs[E_OUTPUT].setVoltage(0.0f);
				lights[E_LIGHT].setBrightness(0.0f);
				
				outputs[GS_OUTPUT].setVoltage(gateVoltage);
				lights[GS_LIGHT].setBrightness(1.0f);
			}
			else {
				outputs[E_OUTPUT].setVoltage(gateVoltage);
				lights[E_LIGHT].setBrightness(1.0f);
				
				outputs[GS_OUTPUT].setVoltage(0.0f);
				lights[GS_LIGHT].setBrightness(0.0f);
			}
			
		}
		else {
			outputs[E_OUTPUT].setVoltage(0.0f);
			lights[E_LIGHT].setBrightness(0.0f);
			outputs[GS_OUTPUT].setVoltage(0.0f);
			lights[GS_LIGHT].setBrightness(0.0f);
			
			for (int i = 0; i < 3; i++) {
				outputs[BIN_OUPUTS + i].setVoltage(0.0f);
				lights[BIN_LIGHTS + i].setBrightness(0.0f);
			}
		}
	}
};

struct CD4532Widget : ModuleWidget {
	CD4532Widget(CD4532 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4532.svg")));

		// screws
		#include "../components/stdScrews.hpp"	


		// digital inputs
		for (int b = 0; b < 8; b++)
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS8[STD_ROW1 + b]), module, CD4532::DIGITAL_INPUTS + b));

		// enable input
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1]), module, CD4532::E_INPUT));
			
		// enable and group select outputs
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW2]), module, CD4532::E_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW2] - 19), module, CD4532::E_LIGHT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW3]), module, CD4532::GS_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW3] - 19), module, CD4532::GS_LIGHT));
		
		// bit outputs
		for (int b = 0; b < 3; b++) {
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW4 + b]), module, CD4532::BIN_OUPUTS + b));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW4 + b] - 19), module, CD4532::BIN_LIGHTS + b));
		}		
	}
	
	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"
	
#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4532 *module = dynamic_cast<CD4532*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
	
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4532 = createModel<CD4532, CD4532Widget>("CD4532");
