//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - ADC
//	Analogue to digital converter
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"

// used by mode management includes
#define MODULE_NAME ADC

struct ADC : Module {
	enum ParamIds {
		BITS_PARAM,
		LEVEL_PARAM,
		REFERENCE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ANALOGUE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(BIT_OUTPUTS, 8),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(BIT_LIGHTS, 8),
		OL_LIGHT,
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	int prevValue = 0;
	
	float outs[8] = {};
	
	ADC() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configParam(BITS_PARAM, 2.0f, 8.0f, 8.0f, "Bits");
		configParam(LEVEL_PARAM, 0.0f, 1.0f, 1.0f, "Input level", " ", 0.0f, 10.0f, 0.0f );
		configParam(REFERENCE_PARAM, 1.0f, 10.0f, 10.0f, "Reference voltage", " Volts");
		
		setIOMode(VCVRACK_STANDARD);
		
		for (int b = 0; b < 8; b++)
			outs[b] = 0.0f;
	}
	
	void onReset() override {

	}
	
	void setIOMode (int mode) {
		
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

		// determine bit depth and reference voltage to use
		int bits = clamp((int)(params[BITS_PARAM].getValue()), 2, 8);
		float vRef = clamp(params[REFERENCE_PARAM].getValue(), 1.0f, 10.0f);
		float bitSize = vRef / maxBits[bits];
		
		// process incoming analogue value
		float analogueIn = inputs[ANALOGUE_INPUT].getVoltage() * params[LEVEL_PARAM].getValue();
		float vIn = clamp(analogueIn, 0.0f, vRef);
		lights[OL_LIGHT].setSmoothBrightness(analogueIn < 0.0f || analogueIn > (vRef + bitSize/ 2.0f), args.sampleTime);
		
		// digital value = (2^bits -1) * input/reference
		int digitalValue = (int)(maxBits[bits] * (vIn/vRef));

		// now set the outputs/lights accordingly
		if(prevValue != digitalValue) {
			prevValue = digitalValue;
			
			int x = 0x01;
			for (int b = 0; b < 8; b++) {
				if (b < bits) {
					bool q = (x == (digitalValue & x));
					outs[b] = boolToGate(q);
					lights[BIT_LIGHTS + b].setBrightness(boolToLight(q));
					x = x << 1;
				}
				else {
					outs[b] = 0.0f;
					lights[BIT_LIGHTS + b].setBrightness(0.0f);
				}
			}
		}

		for (int b = 0; b < 8; b++)
			outputs[BIT_OUTPUTS + b].setVoltage(outs[b]);
	}
};

struct ADCWidget : ModuleWidget {
	ADCWidget(ADC *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ADC.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// input section
		addInput(createInputCentered<LunettaModulaAnalogInputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW1)), module, ADC::ANALOGUE_INPUT));
		addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW2)), module, ADC::OL_LIGHT));
		addParam(createParamCentered<LunettaModulaKnobRed>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW3)), module, ADC::LEVEL_PARAM));

		// TODO: add a clock input ?

		// converter section
		addParam(createParamCentered<LunettaModulaKnobRed>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW5)), module, ADC::REFERENCE_PARAM));
		addParam(createParamCentered<LunettaModulaRotarySwitchRed>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_HALF_ROWS8(STD_ROW7)), module, ADC::BITS_PARAM));
		
		// outputs
		for (int b = 0; b < 8; b++) {
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS8[STD_ROW1 + b]), module, ADC::BIT_OUTPUTS + b));
			
			// lights
			addChild(createLightCentered<SmallLight<GreenLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 15, STD_ROWS8[STD_ROW1 + b] - 12), module, ADC::BIT_LIGHTS + b));
		}
		
	}
	
	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"
	
#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		ADC *module = dynamic_cast<ADC*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
	
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif
};

Model *modelADC = createModel<ADC, ADCWidget>("ADC");
