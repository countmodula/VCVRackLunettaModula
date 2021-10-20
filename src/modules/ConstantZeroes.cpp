//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - ConstantZeroes
//	Hex inverter
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"
#include "../inc/CMOSInput.hpp"

// used by mode management includes
#define MODULE_NAME ConstantZeroes

struct ConstantZeroes : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, 12),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, 12),
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"

	int count = 100;

	ConstantZeroes() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		setIOMode(VCVRACK_STANDARD);
		
		for (int g = 0; g < 12; g ++) {
			configOutput(Q_OUTPUTS + g, "Logical zero");
		}
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

	void process(const ProcessArgs &args) override {

		if (++count > 100) {
			for (int g = 0; g < 12; g++) {	
				outputs[Q_OUTPUTS + g].setVoltage(0.0f);
				lights[Q_LIGHTS + g].setBrightness(0.0f);
			}
			
			count = 0;
		}
	}
};

struct ConstantZeroesWidget : ModuleWidget {
	ConstantZeroesWidget(ConstantZeroes *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ConstantZeroes.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < 12; g++) {

			addOutput(createOutputCentered<LunettaModulaLogicConstantLowJack>(Vec(STD_COLUMN_POSITIONS[g < 6 ? STD_COL1 : STD_COL3], STD_ROWS6[STD_ROW1 + (g % 6)]), module, ConstantZeroes::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[g < 6 ? STD_COL1 : STD_COL3] + 12, STD_ROWS6[STD_ROW1 + (g % 6)] - 19), module, ConstantZeroes::Q_LIGHTS + g));
		}
	}	

	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"

#ifdef CMOS_MODEL_ENABLED
	void appendContextMenu(Menu *menu) override {
		ConstantZeroes *module = dynamic_cast<ConstantZeroes*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}
#endif	
};

Model *modelConstantZeroes = createModel<ConstantZeroes, ConstantZeroesWidget>("ConstantZeroes");
