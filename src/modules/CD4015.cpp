//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4015
//	Dual 4-Stage Static Shift Register
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4015

#define NUM_GATES 2




struct CD4015 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(DATA_INPUTS, NUM_GATES),
		ENUMS(RESET_INPUTS, NUM_GATES),
		ENUMS(CLOCK_INPUTS, NUM_GATES),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_GATES * 4),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_GATES * 4),
		NUM_LIGHTS
	};
	
	struct TappedShiftRegister {
		bool bits[4] = {};
		
		void process(bool newValue) {
			bits[3] = bits[2];
			bits[2] = bits[1];
			bits[1] = bits[0];
			bits[0] = newValue;
		}
		
		void reset () {
			for (int i = 0; i < 4; i++)
				bits[i] = false;
		}
	};	
		
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput dataInputs[NUM_GATES];
	CMOSInput resetInputs[NUM_GATES];
	CMOSInput clockInputs[NUM_GATES];
	
	TappedShiftRegister shiftReg[NUM_INPUTS];
	
	bool prevQ[NUM_GATES][4] = {};
	
	CD4015() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		for (int g = 0; g < NUM_GATES; g++) {
			dataInputs[g].reset();
			resetInputs[g].reset();
			clockInputs[g].reset();
			
			shiftReg[g].reset();
		}
	}

	void setIOMode (int mode) {
		
		// set CMOS input properties
		for (int g = 0; g < NUM_GATES; g++) {
			dataInputs[g].setMode(mode);
			resetInputs[g].setMode(mode);
			clockInputs[g].setMode(mode);
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
	
		int qOffset = 0;
		
		for (int g = 0; g < NUM_GATES; g++) {
			bool update = false;

			// process the current shift register
			if (resetInputs[g].process(inputs[RESET_INPUTS + g].getVoltage())) {
				// reset holds all outputs low
				shiftReg[g].reset();
				update = true;
			}
			else {
				// process clock
				bool prevClock = clockInputs[g].isHigh();
				bool clock = clockInputs[g].process(inputs[CLOCK_INPUTS + g].getVoltage());
				
				// leading clock edge shifts the data and insert the new data
				if (clock & !prevClock) {
					shiftReg[g].process(dataInputs[g].process(inputs[DATA_INPUTS + g].getVoltage()));
					update = true;
				}
			}
			
			// process the outputs
			if (update) {
				update = false;
					
				for (int i = 0; i < 4; i++) {
					bool q = shiftReg[g].bits[i];
					if (q != prevQ[g][i]) {
						prevQ[g][i] = q;
						
						outputs[Q_OUTPUTS + qOffset + i].setVoltage(boolToGate(q));
						lights[Q_LIGHTS + qOffset + i].setBrightness(boolToLight(q));
					}
				}
			}
			
			qOffset += 4;
		}
	}
};

struct CD4015Widget : ModuleWidget {
	CD4015Widget(CD4015 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4015.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		int cols[NUM_GATES] = {STD_COL1, STD_COL3};
		for (int g = 0; g < NUM_GATES; g++) {
			// inputs
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS7[STD_ROW1]), module, CD4015::CLOCK_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS7[STD_ROW2]), module, CD4015::RESET_INPUTS + g));
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS7[STD_ROW3]), module, CD4015::DATA_INPUTS + g));
			
			// outputs
			for (int i = 0; i < 4; i ++) {
				addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[cols[g]], STD_ROWS7[STD_ROW4 + i]), module, CD4015::Q_OUTPUTS + (g * 4) + i));
				addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[cols[g]] + 12, STD_ROWS7[STD_ROW4 + i] - 19), module, CD4015::Q_LIGHTS + (g * 4) + i));
			}
		}
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4015 *module = dynamic_cast<CD4015*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelCD4015 = createModel<CD4015, CD4015Widget>("CD4015");
