//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - Binary8
//	8 Bit Binary Control
//  Copyright (C) 2022  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../components/LunettaModulaLEDDisplay.hpp"
#include "../inc/Utility.hpp"

// used by mode management includes
#define MODULE_NAME Binary8

struct Binary8 : Module {
	enum ParamIds {
		VALUE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(BIT_OUTPUTS, 8),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(BIT_LIGHTS, 8),
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	int prevValue = 0;
	int value = 0;
	int processCount = 8;
	
	float outs[8] = {};
	
	Binary8() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		configParam(VALUE_PARAM, 0.0f, 255.0f, 0.0f, "Value");
		
		setIOMode(VCVRACK_STANDARD);
		
		for (int b = 0; b < 8; b++) {
			outs[b] = 0.0f;
			configOutput(BIT_OUTPUTS + b, rack::string::f("Bit %d", b + 1));
		}
		
		outputInfos[BIT_OUTPUTS]->description = "Least significant bit";
		outputInfos[BIT_OUTPUTS + 7]->description = "Most significant bit";
		processCount = 8;
	}
	
	void onReset() override {
		processCount = 8;
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

	void process(const ProcessArgs &args) override {

		// no point processing controls at audio rate
		if (++processCount > 8) {
			processCount = 0;
			// determine bit depth and reference voltage to use
			value = (int)(params[VALUE_PARAM].getValue());
		}

		// now set the outputs/lights accordingly
		if(prevValue != value) {
			prevValue = value;

			int x = 0x01;
			for (int b = 0; b < 8; b++) {
				bool q = (x == (value & x));
				outs[b] = boolToGate(q);
				lights[BIT_LIGHTS + b].setBrightness(boolToLight(q));
				x = x << 1;
			}
		}

		for (int b = 0; b < 8; b++)
			outputs[BIT_OUTPUTS + b].setVoltage(outs[b]);

	}
};

struct Binary8Widget : ModuleWidget {
	
	LunettaModulaLEDDisplayMedium *valueDisplay;
	int displayValue = 0;
	
	Binary8Widget(Binary8 *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Binary8.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// value selector
		addParam(createParamCentered<RotarySwitch<RedKnob>>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS8[STD_ROW3]), module, Binary8::VALUE_PARAM));
		
		// outputs
		for (int b = 0; b < 8; b++) {
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS8[STD_ROW1 + b]), module, Binary8::BIT_OUTPUTS + b));
			
			// lights
			addChild(createLightCentered<SmallLight<GreenLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 15, STD_ROWS8[STD_ROW1 + b] - 12), module, Binary8::BIT_LIGHTS + b));
		}
		
		displayValue = 0;
		valueDisplay = new LunettaModulaLEDDisplayMedium(2, true);
		valueDisplay->setCentredPos(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS8[STD_ROW2]));
		valueDisplay->setText(displayValue);
		addChild(valueDisplay);
		
	}
	
	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"
	
#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		Binary8 *module = dynamic_cast<Binary8*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
	
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif
	void step() override {
		if (module) {
			Binary8 *m = dynamic_cast<Binary8*>(this->module);

			if (displayValue != m->value) {
				displayValue = m->value;
				valueDisplay->setText(displayValue);
			}
		}
		
		Widget::step();
	}

};

Model *modelBinary8 = createModel<Binary8, Binary8Widget>("Binary8");
