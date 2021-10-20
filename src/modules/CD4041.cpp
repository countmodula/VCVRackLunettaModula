//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4041
//	Quad True/Complement Buffer
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4041

#define NUM_GATES 4

struct CD4041 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
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
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	
	CD4041() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		char c = 'A';
		for (int g = 0; g < NUM_GATES; g++) {
			configInput(A_INPUTS + g, rack::string::f("%c", c));
			configOutput(Q_OUTPUTS + g, rack::string::f("%c", c));
			configOutput(NQ_OUTPUTS + g, rack::string::f("Not %c", c++));
		}
		
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
		}
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
		}
		
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
		
		int in = A_INPUTS;
		int qOut = Q_OUTPUTS;
		int nqOut = NQ_OUTPUTS;
		int qLed = Q_LIGHTS;
		int nqLed = NQ_LIGHTS;
		for (int g = 0; g < NUM_GATES; g++) {
			bool q = aInputs[g].process(inputs[in++].getVoltage());

			if (q) {
				outputs[qOut++].setVoltage(gateVoltage);
				lights[qLed++].setBrightness(1.0f);

				outputs[nqOut++].setVoltage(0.0f);
				lights[nqLed++].setBrightness(0.0f);
			}
			else {
				outputs[qOut++].setVoltage(0.0f);
				lights[qLed++].setBrightness(0.0f);

				outputs[nqOut++].setVoltage(gateVoltage);
				lights[nqLed++].setBrightness(1.0f);
			}
		}
	}
};

struct CD4041Widget : ModuleWidget {
	CD4041Widget(CD4041 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4041.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		int cols[NUM_GATES] = {STD_COL1, STD_COL3, STD_COL1, STD_COL3};
		int rows[NUM_GATES] = {STD_ROW1, STD_ROW1, STD_ROW4, STD_ROW4};
		for (int g = 0; g < NUM_GATES; g++) {
		
			// inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS6[rows[g]]), module, CD4041::A_INPUTS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS6[rows[g] + 1]), module, CD4041::Q_OUTPUTS + g));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[g]] + 12, STD_ROWS6[rows[g] + 1] - 19), module, CD4041::Q_LIGHTS + g));

			// NQ output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS6[rows[g] + 2]), module, CD4041::NQ_OUTPUTS + g));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[g]] + 12, STD_ROWS6[rows[g] + 2] - 19), module, CD4041::NQ_LIGHTS + g));
		}
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4041 *module = dynamic_cast<CD4041*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4041 = createModel<CD4041, CD4041Widget>("CD4041");
