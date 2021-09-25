//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4562
//	128 Stage Static Shift Register
//  Copyright (C) 2021  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4562
#define NUM_BITS 128
#define NUM_BITS_LESS_1 127

struct CD4562 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		DATA_INPUT,
		CLOCK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, 8),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, 8),
		NUM_LIGHTS
	};

	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput dataInput;
	CMOSInput clockInput;
	bool prevClock = false;
	
	bool shiftRegister[NUM_BITS] = {};
	unsigned int in = NUM_BITS-1;
	unsigned int out[8] = { 111, 95, 79, 63, 47, 31, 15, 127 };

	CD4562() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		dataInput.reset();
		clockInput.reset();
	
		for (int i = 0; i < NUM_BITS; i++)
			shiftRegister[i] = false;
			
		for (int b = 0; b < 8; b++) {
			outputs[Q_OUTPUTS + b].setVoltage(0.0f);
			lights[Q_LIGHTS + b].setBrightness(0.0f);
		}
	}
	
	void setIOMode (int mode) {
		// set CMOS input properties
		dataInput.setMode(mode);
		clockInput.setMode(mode);

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

		// process the clock
		bool clock = clockInput.process(inputs[CLOCK_INPUT].getVoltage());
		
		if (clock && !prevClock) {

			// grab the data input we want
			bool data = dataInput.process(inputs[DATA_INPUT].getVoltage());

			// process the shift register here
			shiftRegister[in++]= data;
			if (in > NUM_BITS_LESS_1)
				in = 0;
			
			for (int b = 0; b < 8; b++) {
				if (++out[b] > NUM_BITS_LESS_1)
					out[b] = 0;
			}
		}

		// data outputs
		for (int b = 0; b < 8; b++) {
			if (shiftRegister[out[b]]) {
				outputs[Q_OUTPUTS + b].setVoltage(gateVoltage);
				lights[Q_LIGHTS + b].setBrightness(1.0f);
			}
			else {
				outputs[Q_OUTPUTS + b].setVoltage(0.0f);
				lights[Q_LIGHTS + b].setBrightness(0.0f);
			}
		}

		prevClock = clock;
	}
};

struct CD4562Widget : ModuleWidget {
	CD4562Widget(CD4562 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4562.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS8[STD_ROW3]), module, CD4562::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS8[STD_ROW6]), module, CD4562::DATA_INPUT));

		// Q outputs
		for (int b = 0; b < 8; b++) {
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS8[STD_ROW1 + b]), module, CD4562::Q_OUTPUTS + b));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 15, STD_ROWS8[STD_ROW1 + b] - 12), module, CD4562::Q_LIGHTS + b));
		}
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4562 *module = dynamic_cast<CD4562*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4562 = createModel<CD4562, CD4562Widget>("CD4562");
