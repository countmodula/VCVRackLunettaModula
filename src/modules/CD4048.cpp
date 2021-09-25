//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4048
//	Multifunction Expandable 8-Input Gate
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4048

#define NUM_GATES 4

struct CD4048 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(ABCD_INPUTS, NUM_GATES),
		ENUMS(EFGH_INPUTS, NUM_GATES),
		EXP_INPUT,
		KA_INPUT,
		KB_INPUT,
		KC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		J_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		J_LIGHT,
		ENUMS(STATUS_LIGHTS, 8),
		NUM_LIGHTS
	};
	
	enum FUNCTIONS {
		NOR_FUNCTION,
		OR_FUNCTION,
		OR_AND_FUNCTION,
		OR_NAND_FUNCTION,
		AND_FUNCTION,
		NAND_FUNCTION,
		AND_NOR_FUNCTION,
		AND_OR_FUNCTION,
		NUM_FUNCTIONS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput abcdInputs[NUM_GATES];
	CMOSInput efghInputs[NUM_GATES];
	CMOSInput kaInput;
	CMOSInput kbInput;
	CMOSInput kcInput;
	CMOSInput expInput;
	
	int processCount = 8;
	
	CD4048() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
		
		processCount = 8;
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			abcdInputs[g].reset();
			efghInputs[g].reset();
		}
		
		kaInput.reset();
		kbInput.reset();
		kcInput.reset();
		expInput.reset();
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			abcdInputs[g].setMode(mode);
			efghInputs[g].setMode(mode);
		}
		
		kaInput.setMode(mode);
		kbInput.setMode(mode);
		kcInput.setMode(mode);
		expInput.setMode(mode);
		
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
		
		// process the function inputs
		int function = (kaInput.process(inputs[KA_INPUT].getVoltage()) ? 0x04 : 0x00) +	
						(kbInput.process(inputs[KB_INPUT].getVoltage()) ? 0x02: 0x00) +
						(kcInput.process(inputs[KC_INPUT].getVoltage()) ? 0x01: 0x00);

		// expansion input
		bool exp = expInput.process(inputs[EXP_INPUT].getVoltage());

		// process primary gate logic
		bool abcd = false, efgh = false;
		switch (function) {
			case OR_FUNCTION:
			case NOR_FUNCTION:
			case OR_AND_FUNCTION:
			case OR_NAND_FUNCTION:
				// (A | B | C | D) / (E | F | G | H)
				for (int g = 0; g < NUM_GATES; g++) {
					abcd |= abcdInputs[g].process(inputs[ABCD_INPUTS + g].getVoltage());
					efgh |= efghInputs[g].process(inputs[EFGH_INPUTS + g].getVoltage()); 
				}
				
				break;
				
			case AND_FUNCTION:
			case NAND_FUNCTION:
			case AND_OR_FUNCTION:
			case AND_NOR_FUNCTION:
				// (A & B & C & D) / (E & F & G & H)
				abcd = efgh = true;
				for (int g = 0; g < NUM_GATES; g++) {
					abcd &= abcdInputs[g].process(inputs[ABCD_INPUTS + g].getVoltage());
					efgh &= efghInputs[g].process(inputs[EFGH_INPUTS + g].getVoltage()); 
				}
				break;
		}

		// now determine the secondary logic
		bool j = false;
		switch (function) {
			case OR_FUNCTION:		// J = (A | B | C | D) | (E | F | G | H) | EXP)
			case AND_OR_FUNCTION:	// J = (A & B & C & D) | (E & F & G & H) | EXP
				j = abcd | efgh | exp;
				break;
			case NOR_FUNCTION:		// J = !((A | B | C | D) | (E | F | G | H) | EXP)
			case AND_NOR_FUNCTION:	// J = !((A & B & C & D) | (E & F & G & H) | EXP)
				j = !(abcd | efgh | exp);
				break;
			case AND_FUNCTION:		// J = (A & B & C & D) & (E & F & G & H) & !EXP)
			case OR_AND_FUNCTION:	// J = (A | B | C | D) & (E | F | G | H) & !EXP
				j = abcd & efgh & !exp;
				break;
			case NAND_FUNCTION:		// J = !((A & B & C & D) & (E & F & G & H) & !EXP)
			case OR_NAND_FUNCTION:	// J = !((A | B | C | D) & (E | F | G | H) & !EXP)
				j = !(abcd & efgh & !exp);
				j = !(abcd & efgh & !exp);
				break;
		}
		
		if (j) {
			outputs[J_OUTPUT].setVoltage(gateVoltage);
			lights[J_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[J_OUTPUT].setVoltage(0.0f);
			lights[J_LIGHT].setBrightness(0.0f);
		}
		
		// show status
		if (++processCount > 8) {
			processCount = 0;
			
			for (int i = 0; i < NUM_FUNCTIONS; i++)
				lights[STATUS_LIGHTS + i].setBrightness(boolToLight(i == function));
		}
	}
};

struct CD4048Widget : ModuleWidget {
	CD4048Widget(CD4048 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4048.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
		
			// A/B inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW1 + g]), module, CD4048::ABCD_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW1 + g]), module, CD4048::EFGH_INPUTS + g));
		}
		
		// function/expand inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW1]), module, CD4048::EXP_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW2]), module, CD4048::KA_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW3]), module, CD4048::KB_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW4]), module, CD4048::KC_INPUT));

		// J output/light
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS5[STD_ROW5]), module, CD4048::J_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4048::J_LIGHT));

		
		// status lights
		int offset = -22;
		for (int i = 0; i < CD4048::NUM_FUNCTIONS; i++) {
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] - 15, STD_ROWS5[STD_ROW5] + offset), module, CD4048::STATUS_LIGHTS + i));
			
			offset += 8;
		}
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4048 *module = dynamic_cast<CD4048*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4048 = createModel<CD4048, CD4048Widget>("CD4048");
