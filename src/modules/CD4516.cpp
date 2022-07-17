//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4516
//	Presettable Binary Up/Down Counter
//	Copyright (C) 2022 Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4516

#define MAX_COUNT 16
#define MAX_COUNT_MINUS_1 15
#define NUM_BITS 4
#define NUM_BITS_PLUS_1 5

struct CD4516 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		CLOCK_INPUT,
		RESET_INPUT,
		countUp_INPUT,
		ENUMS(P_INPUTS, NUM_BITS),
		PRESET_ENABLE_INPUT,
		CARRY_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_BITS),
		CARRY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_BITS),
		CARRY_LIGHT,
		UP_LIGHT,
		DOWN_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput clockInput;
	CMOSInput resetInput;
	CMOSInput updDownInput;
	CMOSInput carryInput;
	CMOSInput presetEnableInput;
	CMOSInput presetInputs[NUM_BITS];
	
	const int bitmap[NUM_BITS_PLUS_1] = { 0, 1, 2, 4, 8};
	const int outputLabels[NUM_BITS] = {2, 4, 8, 16 };

	int count = 0;
	bool update = true;
	bool reset = false;
	bool countUp = false;

	CD4516() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
		
		configInput(CLOCK_INPUT, "Clock");
		configInput(RESET_INPUT, "Reset");
		configInput(PRESET_ENABLE_INPUT, "Preset enable");
		configInput(CARRY_INPUT, "Carry");
		configInput(countUp_INPUT, "Up/Down");
		
		inputInfos[CLOCK_INPUT]->description = "Negative edge triggered";
		inputInfos[RESET_INPUT]->description = "Resets count to 0, holds all outputs low when high";
		
		for (int b = 0; b < NUM_BITS; b++) {
			configOutput(Q_OUTPUTS + b, rack::string::f("Q%d", b + 1));
			//outputInfos[Q_OUTPUTS + b]->description = rack::string::f("Divide by %d", outputLabels[b]);
			configInput(P_INPUTS + b, rack::string::f("P%d", b + 1));
		}

		configOutput(CARRY_OUTPUT, "Carry");

		count = 0;
		update = true;
		reset = false;
		countUp = false;
	}
	
	void onReset() override {
		clockInput.reset();
		resetInput.reset();
		updDownInput.reset();
		presetEnableInput.reset();

		for (int b = 0; b < NUM_BITS; b++) {
			presetInputs[b].reset();
		}
		
		count = 0;
		update = true;
		reset = true;
		countUp = false;
	}

	void setIOMode (int mode) {
		clockInput.setMode(mode);
		resetInput.setMode(mode);
		updDownInput.setMode(mode);
		presetEnableInput.setMode(mode);
		
		for (int b = 0; b < NUM_BITS; b++) {
			presetInputs[b].setMode(mode);
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

		bool prevcountUp = updDownInput.isHigh();
		countUp = updDownInput.process(inputs[countUp_INPUT].getVoltage());
		
		if (prevcountUp != countUp) {
			update = true;
		}
		
		// are we reset?
		if (resetInput.process(inputs[RESET_INPUT].getVoltage())) {
			count = 0;

			if (!reset)
				update = true;
				
			reset = true;
		}
		else {
			
			// process the carry input
			bool prevCarryIn = carryInput.isHigh();
			bool carryIn = carryInput.process(inputs[CARRY_INPUT].getVoltage());
			if (prevCarryIn != carryIn)
				update = true;
			
			// process the clock inputs - low carry in halts the clock.
			bool prevClock = clockInput.isHigh();
			bool clock = clockInput.process(carryIn ? 0.0f : inputs[CLOCK_INPUT].getVoltage());

			// need to be sure we don't double trigger on the rising clock when transitioning from carry state in the same cycle
			if (!prevClock && prevCarryIn && !carryIn)
				clock = false;

			// are we presetting?
			if (presetEnableInput.process(inputs[PRESET_ENABLE_INPUT].getVoltage())) {
				int c = 0;
				for (int p = 0, b = 1; p < NUM_BITS; p++, b++) {
					float v = inputs[P_INPUTS + p].getVoltage();

					if (presetInputs[p].process(v)) {
						c += bitmap[b];
					}
				}

				if (c != count) {
					count = c;
					update = true;
				}
			}
			else {
				// process the clock edge if required
				if (!prevClock && clock) {
					
					if (countUp) {
						if (++count > MAX_COUNT_MINUS_1)
							count = 0;
					}
					else {
						if (--count < 0)
							count = MAX_COUNT_MINUS_1;
					}

					update = true;
					reset = false;
				}
			}
		}
		
		// decode the outputs
		int g = 0;
		for (int i = 1; i < NUM_BITS_PLUS_1; i++) {
			bool q = (count & bitmap[i]) > 0;

			if (q) {
				outputs[Q_OUTPUTS + g].setVoltage(gateVoltage);
				if (update)
					lights[Q_LIGHTS + g].setBrightness(1.0f);
			}
			else {
				outputs[Q_OUTPUTS + g].setVoltage(0.0f);
				if (update)
					lights[Q_LIGHTS + g].setBrightness(0.0f);
			}

			g++;
		}
		
		// set carry output - watch out, it's active low.
		bool carry = (count != (countUp ? MAX_COUNT_MINUS_1 : 0));
		if (carry) {
			outputs[CARRY_OUTPUT].setVoltage(gateVoltage);
			if (update)
				lights[CARRY_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[CARRY_OUTPUT].setVoltage(0.0f);
			if (update)
				lights[CARRY_LIGHT].setBrightness(0.0f);
		}
		
		// set direction leds
		if (update) {
			if (countUp) {
				lights[UP_LIGHT].setBrightness(1.0f);
				lights[DOWN_LIGHT].setBrightness(0.0f);
			}
			else {
				lights[UP_LIGHT].setBrightness(0.0f);
				lights[DOWN_LIGHT].setBrightness(1.0f);
			}
		}
		
		update = false;
	}
};

struct CD4516Widget : ModuleWidget {
	CD4516Widget(CD4516 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4516.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// clock/reset/carry and direction inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW2]), module, CD4516::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW4]), module, CD4516::RESET_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW6]), module, CD4516::countUp_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS6[STD_ROW1]), module, CD4516::CARRY_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1]), module, CD4516::PRESET_ENABLE_INPUT));

		// bit inputs/outputs/lights
		int i = 0;
		int row = STD_ROW2;
		for (int r = 0; r < NUM_BITS; r++) {
			// inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[row]), module, CD4516::P_INPUTS + i));
			
			// Q outputs
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS6[row]), module, CD4516::Q_OUTPUTS + i));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5] + 12, STD_ROWS6[row] - 19), module, CD4516::Q_LIGHTS + i));
			
			i++;
			row++;
		}
		
		// carry out
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL5], STD_ROWS6[STD_ROW6]), module, CD4516::CARRY_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL5] + 12, STD_ROWS6[STD_ROW6] - 19), module, CD4516::CARRY_LIGHT));

		// status lights
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW6] - 10), module, CD4516::UP_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW6] + 13), module, CD4516::DOWN_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4516 *module = dynamic_cast<CD4516*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4516 = createModel<CD4516, CD4516Widget>("CD4516");
