//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - CD4031
//	64 Stage Static Shift Register
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME CD4031

struct CD4031 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		DATA_INPUT,
		CLOCK_INPUT,
		RECIRC_INPUT,
		MODE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		DEL_CLOCK_OUTPUT,
		Q_OUTPUT,
		NQ_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		DEL_CLOCK_LIGHT,
		Q_LIGHT,
		NQ_LIGHT,
		MODE_DATA_LIGHT,
		MODE_RECIRC_LIGHT,
		NUM_LIGHTS
	};

	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput dataInput;
	CMOSInput clockInput;
	CMOSInput recircInput;
	CMOSInput modeInput;
	
	bool delayedClock[2] = {};
	
	bool shiftRegister[64] = {};
	unsigned int in = 63;
	unsigned int out = 0;
	
	CD4031() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configInput(DATA_INPUT, "Data");
		configInput(CLOCK_INPUT, "Clock");
		configInput(RECIRC_INPUT, "Recirculate");
		configInput(MODE_INPUT, "Mode");
		inputInfos[MODE_INPUT]->description = "Low to select data input, high to select recirculate input";
		
		configOutput(DEL_CLOCK_OUTPUT, "Delayed clock");
		outputInfos[DEL_CLOCK_OUTPUT]->description = "Clock delayed by 1 sample";
		configOutput(Q_OUTPUT, "Q");
		configOutput(NQ_OUTPUT, "Not Q");
		
		setIOMode(VCVRACK_STANDARD);
	}
	
	void onReset() override {
		dataInput.reset();
		clockInput.reset();
		recircInput.reset();
		modeInput.reset();
		
		delayedClock[0] = delayedClock[1] = false;
		
		for (int i = 0; i < 64; i++)
			shiftRegister[i] = false;
	}
	
	void setIOMode (int mode) {
		// set CMOS input properties
		dataInput.setMode(mode);
		clockInput.setMode(mode);
		recircInput.setMode(mode);
		modeInput.setMode(mode);
		
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
		
		// what mode are we in?
		bool recircMode = modeInput.process(inputs[MODE_INPUT].getVoltage());
	
		// grab the data input we want
		bool data = false;
		if (recircMode) {
			data = recircInput.process(inputs[RECIRC_INPUT].getVoltage());
			
			lights[MODE_DATA_LIGHT].setBrightness(0.0f);
			lights[MODE_RECIRC_LIGHT].setBrightness(1.0f);
		}
		else {
			data = dataInput.process(inputs[DATA_INPUT].getVoltage());

			lights[MODE_DATA_LIGHT].setBrightness(1.0f);
			lights[MODE_RECIRC_LIGHT].setBrightness(0.0f);
		}
	
		// process the clock
		bool clock = clockInput.process(inputs[CLOCK_INPUT].getVoltage());
		delayedClock[1] = delayedClock[0];
		bool edge = (clock & ! delayedClock[0]);
		delayedClock[0] = clock;
		
		// delayed clock
		if (delayedClock[1]) {
			outputs[DEL_CLOCK_OUTPUT].setVoltage(gateVoltage);
			lights[DEL_CLOCK_LIGHT].setBrightness(1.0f);
		}
		else {
			outputs[DEL_CLOCK_OUTPUT].setVoltage(0.0f);
			lights[DEL_CLOCK_LIGHT].setBrightness(0.0f);
		}
		
		// process the shift register here
		if (edge) {
			shiftRegister[in++]= data;
			if (in > 63)
				in = 0;
			
			if (++out > 63)
					out = 0;
		}

		// data outputs
		if (shiftRegister[out]) {
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

struct CD4031Widget : ModuleWidget {
	CD4031Widget(CD4031 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CD4031.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// inputs
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW1]), module, CD4031::CLOCK_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW2]), module, CD4031::MODE_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL2], STD_ROWS5[STD_ROW3]), module, CD4031::DATA_INPUT));
		addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL2], STD_ROWS5[STD_ROW4]), module, CD4031::RECIRC_INPUT));

		// mode lights
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL2] - 6, STD_ROWS5[STD_ROW2] - 8), module, CD4031::MODE_DATA_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL2] - 6, STD_ROWS5[STD_ROW2] + 8), module, CD4031::MODE_RECIRC_LIGHT));

			
		// delayed clock
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW1]), module, CD4031::DEL_CLOCK_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS5[STD_ROW1] - 19), module, CD4031::DEL_CLOCK_LIGHT));

		// Q/NQ output
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS5[STD_ROW5]), module, CD4031::Q_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4031::Q_LIGHT));
		addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS5[STD_ROW5]), module, CD4031::NQ_OUTPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS5[STD_ROW5] - 19), module, CD4031::NQ_LIGHT));
	}

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		CD4031 *module = dynamic_cast<CD4031*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
#endif	
};

Model *modelCD4031 = createModel<CD4031, CD4031Widget>("CD4031");
