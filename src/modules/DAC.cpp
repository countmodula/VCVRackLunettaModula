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
	
	
	DAC() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configParam(BITS_PARAM, 2.0f, 8.0f, 8.0f, "Bits");
		configParam(SCALE_PARAM, 0.0f, 10.0f, 10.0f, "Output scale", " Volts");
		configParam(OFFSET_PARAM, -5.0f, 5.0f, 0.0f, "Output offset", " Volts");
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

	// table of full scale bits
	float maxBits[9] = {0, 0, 3, 7, 15, 31, 63, 127, 255};

	void process(const ProcessArgs &args) override {

		// determine bit depth and bit voltage to use
		int bits = clamp((int)(params[BITS_PARAM].getValue()), 2, 8);
		float vRef = clamp(params[SCALE_PARAM].getValue(), 0.0f, 10.0f);
		float bitSize = vRef / maxBits[bits];
		
		// process the input bits
		float analogeValue = 0.0f;
		float displayValue = 0.0f;
 	
		if (vRef > 0.0f) {
			int x = 0x01;
			for (int b = 0; b < bits; b++) {
				if (digitalInputs[b].process(inputs[DIGITAL_INPUTS + b].getVoltage()))
					analogeValue += (bitSize * (float)x);
				
				x = x << 1;
			}

			displayValue = analogeValue / vRef;
		}
		
		outputs[ANALOGUE_OUPUT].setVoltage(clamp(analogeValue + params[OFFSET_PARAM].getValue(), 0.0f, 12.0f));
		lights[ANALOGUE_LIGHT].setBrightness(displayValue);
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
		addParam(createParamCentered<LunettaModulaRotarySwitchRed>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_HALF_ROWS8(STD_ROW2)), module, DAC::BITS_PARAM));
		addParam(createParamCentered<LunettaModulaKnobRed>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS8[STD_ROW4]), module, DAC::SCALE_PARAM));
		addParam(createParamCentered<LunettaModulaKnobRed>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_HALF_ROWS8(STD_ROW5)), module, DAC::OFFSET_PARAM));
		
		// analogue output
		addOutput(createOutputCentered<LunettaModulaAnalogOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS8[STD_ROW7]), module, DAC::ANALOGUE_OUPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS8[STD_ROW7] - 19), module, DAC::ANALOGUE_LIGHT));
	}
	
	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"
	
	void appendContextMenu(Menu *menu) override {
		DAC *module = dynamic_cast<DAC*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
	
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
};

Model *modelDAC = createModel<DAC, DACWidget>("DAC");
