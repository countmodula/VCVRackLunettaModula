//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - DAC
//	Analogue to digital converter
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME DAC

struct DAC : Module {
	enum ParamIds {
		BITS_PARAM,
		SCALE_PARAM,
		OFFSET_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(DIGITAL_INPUTS, 8),
		NUM_INPUTS
	};
	enum OutputIds {
		ANALOGUE_OUPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ANALOGUE_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	CMOSInput digitalInputs[8];
	
	int prevDigitalValue = -1;
	int bits = 8, prevBits = 2;
	float vRef = 10.0f, prevVRef = 0.0f;
	float offset = 0.0f, prevOffset = 0.0f;
	
	int processCount = 8;
	float analogeValue = 0.0f;
	float displayValue = 0.0f;
	
	
	const int bitmap[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

	// table of full scale bits
	const float maxBits[9] = {0, 0, 3, 7, 15, 31, 63, 127, 255};
	
	DAC() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configParam(BITS_PARAM, 2.0f, 8.0f, 8.0f, "Bits");
		configParam(SCALE_PARAM, 0.0f, 10.0f, 10.0f, "Output scale", " Volts");
		configParam(OFFSET_PARAM, -5.0f, 5.0f, 0.0f, "Output offset", " Volts");

		for (int b = 0; b < 8; b++)
			configInput(DIGITAL_INPUTS + b, rack::string::f("Bit %d", b + 1));
	
		inputInfos[DIGITAL_INPUTS]->description = "Least significant bit";
		inputInfos[DIGITAL_INPUTS + 7]->description = "Most significant bit";
		
		configOutput(ANALOGUE_OUPUT, "Analogue");

		setIOMode(VCVRACK_STANDARD);
		
		analogeValue = 0.0f;
		displayValue = 0.0f;
	}
	
	void onReset() override {
		for (int b = 0; b < 8; b++)
			digitalInputs[b].reset();
	}
	
	void setIOMode (int mode) {
		
		for (int b = 0; b < 8; b++)
			digitalInputs[b].setMode(mode);

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

		// determine bit depth and bit voltage to use - no need to do this at audio rate
		if (++processCount > 8) {
			bits = clamp((int)(params[BITS_PARAM].getValue()), 2, 8);
			vRef = clamp(params[SCALE_PARAM].getValue(), 0.0f, 10.0f);
			offset = params[OFFSET_PARAM].getValue();
			
			// force update of output on change of parameter
			if (prevBits != bits || vRef != prevVRef || offset != prevOffset) {
				prevDigitalValue = -1;
				
				prevOffset = offset;
				prevVRef = vRef;
				prevBits = bits;
			}
		}
		
		// process the input bits
		if (vRef > 0.0f) {
			int digitalValue = 0;
			for (int b = 0; b < bits; b++) {
				if (digitalInputs[b].process(inputs[DIGITAL_INPUTS + b].getVoltage()))
					digitalValue += bitmap[b];
			}
			
			if (digitalValue != prevDigitalValue) {
				prevDigitalValue = digitalValue;
				analogeValue = (vRef / maxBits[bits] * (float)digitalValue);
				displayValue = analogeValue / vRef;
				lights[ANALOGUE_LIGHT].setBrightness(displayValue);
			}
			
			outputs[ANALOGUE_OUPUT].setVoltage(clamp(analogeValue + offset, 0.0f, 12.0f));
		}
		else {
			if (processCount == 0) {
				outputs[ANALOGUE_OUPUT].setVoltage(0.0f);
				lights[ANALOGUE_LIGHT].setBrightness(0.0f);
			}
		}
	}
};

struct DACWidget : ModuleWidget {
	DACWidget(DAC *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DAC.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// digital inputs
		for (int b = 0; b < 8; b++)
			addInput(createInputCentered<LunettaModulaLogicInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS8[STD_ROW1 + b]), module, DAC::DIGITAL_INPUTS + b));

		// converter section
		addParam(createParamCentered<RotarySwitch<RedKnob>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_HALF_ROWS8(STD_ROW2)), module, DAC::BITS_PARAM));
		addParam(createParamCentered<Potentiometer<RedKnob>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS8[STD_ROW4]), module, DAC::SCALE_PARAM));
		addParam(createParamCentered<Potentiometer<RedKnob>>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_HALF_ROWS8(STD_ROW5)), module, DAC::OFFSET_PARAM));
		
		// analogue output
		addOutput(createOutputCentered<LunettaModulaAnalogOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS8[STD_ROW7]), module, DAC::ANALOGUE_OUPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS8[STD_ROW7] - 19), module, DAC::ANALOGUE_LIGHT));
	}
	
	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"
	
#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		DAC *module = dynamic_cast<DAC*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
	
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelDAC = createModel<DAC, DACWidget>("DAC");
