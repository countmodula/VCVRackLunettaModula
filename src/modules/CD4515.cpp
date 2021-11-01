//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4515
//	4-Bit Latched/4-to-16 Line Decoder
//  Copyright (C) 2021  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4515

struct CD4515 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		A_INPUT,
		B_INPUT,
		C_INPUT,
		D_INPUT,
		STROBE_INPUT,
		INHIBIT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(S_OUTPUTS, 16),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(S_LIGHTS, 16),
		NUM_LIGHTS
	};

	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput aInput;
	CMOSInput bInput;
	CMOSInput cInput;
	CMOSInput dInput;
	CMOSInput strobeInput;
	CMOSInput inhibitInput;
	
	bool prevClock = false;
	int count = 0;
	int prevCount = -2;
	
	bool inh = false;
	bool prevInh =false;
	
	CD4515() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configInput(A_INPUT, "A");
		configInput(B_INPUT, "B");
		configInput(C_INPUT, "C");
		configInput(D_INPUT, "B");
		configInput(STROBE_INPUT, "Strobe");
		configInput(INHIBIT_INPUT, "Inhibit");

		inputInfos[A_INPUT]->description = "Least significant bit";
		inputInfos[B_INPUT]->description = "Most significant bit";
		inputInfos[STROBE_INPUT]->description = "Loads the decoder with the logic values present at the A-D inputs and sets the appropriate output low";
		inputInfos[INHIBIT_INPUT]->description = "Forces all outputs low";

		for (int b = 0; b < 16; b++) {
			configOutput(S_OUTPUTS + b, rack::string::f("S%d", b));
			outputInfos[S_OUTPUTS + b]->description = "Active lolw (high when inhibited)";
		}
		
		setIOMode(VCVRACK_STANDARD);
			
		count = 1;
		prevCount = 0;
		prevInh = false;
		
		onReset(); //make sure all outputs are set high
	}
	
	void onReset() override {
	
		aInput.reset();
		bInput.reset();
		cInput.reset();
		dInput.reset();
		strobeInput.reset();
		inhibitInput.reset();
		
		for(int i = 0; i < 16; i++) {
			outputs[S_OUTPUTS + i].setVoltage(gateVoltage);
			lights[S_LIGHTS + i].setBrightness(1.0f);
		}
		
		count = 0;
		prevCount = 0;
		prevInh = false;
	}
	
	void setIOMode (int mode) {
		// set CMOS input properties
		aInput.setMode(mode);
		bInput.setMode(mode);
		cInput.setMode(mode);
		dInput.setMode(mode);
		strobeInput.setMode(mode);
		inhibitInput.setMode(mode);

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

		// are we inhibited?
		inh = inhibitInput.process(inputs[INHIBIT_INPUT].getVoltage());

		// process the strobe
		bool clock = strobeInput.process(inputs[STROBE_INPUT].getVoltage());
		
		if (clock && !prevClock) {

			// decode the data inputs
			count = 0;
			if (aInput.process(inputs[A_INPUT].getVoltage()))
				count++;
				
			if (bInput.process(inputs[B_INPUT].getVoltage()))
				count += 2;
				
			if (cInput.process(inputs[C_INPUT].getVoltage()))
				count += 4;
				
			if (dInput.process(inputs[D_INPUT].getVoltage()))
				count += 8;
		}

		int countToUse = inh ? 0 : count;
		
		// data outputs
		if (prevCount != countToUse || prevInh != inh) {
			if (inh) {
				outputs[S_OUTPUTS + countToUse].setVoltage(gateVoltage);
				lights[S_LIGHTS + countToUse].setBrightness(1.0f);
			}
			else {
				outputs[S_OUTPUTS + countToUse].setVoltage(0.0f);
				lights[S_LIGHTS + countToUse].setBrightness(0.0f);
			}

			if (prevCount >=0) {
				outputs[S_OUTPUTS + prevCount].setVoltage(gateVoltage);
				lights[S_LIGHTS + prevCount].setBrightness(1.0f);
			}
			prevCount = countToUse;
		}

		prevInh = inh;
		prevClock = clock;
	}
};

struct CD4515Widget : ModuleWidget {
	CD4515Widget(CD4515 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4515.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW1)), module, CD4515::A_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW2)), module, CD4515::B_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW3)), module, CD4515::C_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW4)), module, CD4515::D_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW6)), module, CD4515::STROBE_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW7)), module, CD4515::INHIBIT_INPUT));

		// decoded outputs
		int outputId = CD4515::S_OUTPUTS;
		int lightId = CD4515::S_LIGHTS;
		int col = STD_COL3;
		for (int i = 0; i < 2; i++) {
			for (int b = 0; b < 8; b++) {
				addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[col], STD_ROWS8[STD_ROW1 + b]), module, outputId++));
				addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[col] + 15, STD_ROWS8[STD_ROW1 + b] - 12), module, lightId++));
			}
			col += 2;
		}
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4515 *module = dynamic_cast<CD4515*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4515 = createModel<CD4515, CD4515Widget>("CD4515");
