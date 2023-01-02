//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4539
//	Dual 4 Input Multiplexer
//  Copyright (C) 2022  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4539

#define NUM_GATES 4

struct CD4539 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(A_INPUTS, NUM_GATES),
		ENUMS(B_INPUTS, NUM_GATES),
		EA_INPUT,
		EB_INPUT,
		S0_INPUT,
		S1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		A_OUTPUT,
		B_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		A_LIGHT,
		B_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInputs[NUM_GATES];
	CMOSInput bInputs[NUM_GATES];
	CMOSInput eaInput;
	CMOSInput ebInput;
	CMOSInput s0Input;
	CMOSInput s1Input;
	
	CD4539() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		char c = '1';
		for (int i = 0; i < NUM_GATES; i++) {
			configInput(A_INPUTS + i, rack::string::f("Multiplexer A%c", c));
			configInput(B_INPUTS + i, rack::string::f("Multiplexer B%c", c++));
		}
		
		configInput(EA_INPUT, "Enable multiplexer A");
		configInput(EB_INPUT, "Enable multiplexer B");
		
		inputInfos[EA_INPUT]->description = "Active low";
		inputInfos[EB_INPUT]->description = "Active low";
		
		configOutput(A_OUTPUT, "Multiplexer A");
		configOutput(B_OUTPUT, "Multiplexer B");
		
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].reset();
			bInputs[g].reset();
		}
		
		s0Input.reset();
		s1Input.reset();
		
		eaInput.reset();
		ebInput.reset();
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			aInputs[g].setMode(mode);
			bInputs[g].setMode(mode);
		}
		
		eaInput.setMode(mode);
		ebInput.setMode(mode);
	
		s0Input.setMode(mode);
		s1Input.setMode(mode);
	
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
		
		// determine selected input
		int s = 0;
		if (s0Input.process(inputs[S0_INPUT].getVoltage()))
			s += 1;

		if (s1Input.process(inputs[S1_INPUT].getVoltage()))
			s += 2;
		
		// are we enabled? watch out - enable is active low
		bool a = !eaInput.process(inputs[EA_INPUT].getVoltage());
		bool b = !ebInput.process(inputs[EB_INPUT].getVoltage());
	
		// grab the input value for channel a if enabled
		if (a) {
			a &= aInputs[s].process(inputs[A_INPUTS + s].getVoltage());
		}
		
		// grab the input value for channel b if enabled
		if (b) {
			b &= bInputs[s].process(inputs[B_INPUTS + s].getVoltage());
		}
	
		// output channel a
		if (a) {
			outputs[A_OUTPUT].setVoltage(gateVoltage);
			lights[A_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[A_OUTPUT].setVoltage(0.0f);
			lights[A_LIGHT].setBrightness(0.0f);
		}

		// output channel b
		if (b) {
			outputs[B_OUTPUT].setVoltage(gateVoltage);
			lights[B_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[B_OUTPUT].setVoltage(0.0f);
			lights[B_LIGHT].setBrightness(0.0f);
		}
	}
};

struct CD4539Widget : ModuleWidget {
	CD4539Widget(CD4539 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4539.svg")));

		// screws
		#include "../components/stdScrews.hpp"
		
		// select inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW1]), module, CD4539::S0_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW1]), module, CD4539::S1_INPUT));

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
			// A/B inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW2 + g]), module, CD4539::A_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW2 + g]), module, CD4539::B_INPUTS + g));
		}

		// enable inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW6]), module, CD4539::EA_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW6]), module, CD4539::EB_INPUT));		

		// outputs
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS7[STD_ROW7]), module, CD4539::A_OUTPUT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS7[STD_ROW7]), module, CD4539::B_OUTPUT));
			
		// lights
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] + 12, STD_ROWS7[STD_ROW7] - 19), module, CD4539::A_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS7[STD_ROW7] - 19), module, CD4539::B_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4539 *module = dynamic_cast<CD4539*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4539 = createModel<CD4539, CD4539Widget>("CD4539");
