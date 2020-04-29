//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - Buttons
//	Hex manual logic buttons
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "../LunettaModula.hpp"
#include "../inc/Utility.hpp"


// used by mode management includes
#define MODULE_NAME Buttons

#define NUM_GATES 6

struct Buttons : Module {
	enum ParamIds {
		ENUMS(BTN_PARAMS, NUM_GATES),
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(Q_OUTPUTS, NUM_GATES),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(Q_LIGHTS, NUM_GATES),
		ENUMS(MOM_LIGHTS, NUM_GATES),
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"
	
	bool latched[NUM_GATES] = {};
	bool buttonValue[NUM_GATES] = {};
	
	bool outValue[NUM_GATES] = {};
	
	Buttons() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		for (int g = 0; g < NUM_GATES; g++) 
			configParam(BTN_PARAMS + g, 0.0f, 1.0f, 0.0f, "High/low");
	}
	
	void onReset() override {
		
		for(int i = 0; i < NUM_GATES; i++)
			buttonValue[i] = outValue[i] = latched[i] = false;
	}
	
	void setIOMode (int mode) {
		
		// set gate voltage
		#include "../modes/setGateVoltage.hpp"
	}	
	
	json_t *dataToJson() override {
		json_t *root = json_object();

		json_object_set_new(root, "moduleVersion", json_integer(1));

		json_t *mod = json_array();
		json_t *val = json_array();
		
		for (int g = 0; g < NUM_GATES; g++) {
			json_array_insert_new(mod, g, json_boolean(latched[g]));
			json_array_insert_new(val, g, json_boolean(outValue[g]));
		}

		json_object_set_new(root, "modes", mod);
		json_object_set_new(root, "states", val);

		// add the I/O mode details
		#include "../modes/dataToJson.hpp"		
		
		return root;
	}
	
	void dataFromJson(json_t *root) override {

		json_t *mod = json_object_get(root, "modes");
		json_t *val = json_object_get(root, "states");

		for (int g = 0; g < NUM_GATES; g++) {
			if (mod) {
				json_t *v = json_array_get(mod, g);
				if (v)
					latched[g] = json_boolean_value(v);
			}
			
			if (val) {
				json_t *v = json_array_get(val, g);
				if (v)
					outValue[g] = json_boolean_value(v);
			}
		}
		
		// grab the I/O mode details
		#include "../modes/dataFromJson.hpp"
	}	

	void process(const ProcessArgs &args) override {
		
		// process buttons
		for (int g = 0; g < NUM_GATES; g++) {
			bool q = params[BTN_PARAMS + g].getValue() > 0.5f;
			
			if (latched[g]) {
				// we toggle the outputs on click in latched mode
				if (q && !buttonValue[g])
					outValue[g] = !outValue[g];
			}
			else {
				// output follows the button press in momentary mode
				outValue[g] = q;
			}
		
			outputs[Q_OUTPUTS + g].setVoltage(boolToGate(outValue[g]));
			lights[Q_LIGHTS + g].setBrightness(boolToLight(outValue[g]));

			lights[MOM_LIGHTS + g].setBrightness(boolToLight(latched[g]));
			
			// for identification of the button click
			buttonValue[g] = q;
		}	
	}
};

struct ButtonsWidget : ModuleWidget {
	ButtonsWidget(Buttons *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Buttons.svg")));

		// screws
		#include "../components/stdScrews.hpp"	

		// gates
		for (int g = 0; g < NUM_GATES; g++) {
		
			// buttons
			addParam(createParamCentered<LunettaModulaPBSwitchMomentaryUnlit>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW1 + g]), module, Buttons::BTN_PARAMS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1 + g]), module, Buttons::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<GreenLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL2] - 8, STD_ROWS6[STD_ROW1 + g] - 19), module, Buttons::MOM_LIGHTS + g));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW1 + g] - 19), module, Buttons::Q_LIGHTS + g));
		}
	}
	
	// all button mode menu item
	struct ButtonModeAllMenuItem : MenuItem {
		ButtonsWidget *widget;
		Buttons *module;
		bool latch;
	
		void onAction(const event::Action &e) override {
		
			// history - current settings
			history::ModuleChange *h = new history::ModuleChange;
			h->name = "all button mode";
			
			h->moduleId = widget->module->id;
			h->oldModuleJ = widget->toJson();

			for (int i = 0; i < NUM_GATES; i++) {
				module->latched[i] = latch;
				if (!latch)
					module->outValue[i] = false;
			}	
			
			// history - new settings
			h->newModuleJ = widget->toJson();
			APP->history->push(h);	
		}
	};		
	
	// button mode menu item
	struct ButtonModeMenuItem : MenuItem {
		ButtonsWidget *widget;
		Buttons *module;
		int id;
		
		void onAction(const event::Action &e) override {
		
			// history - current settings
			history::ModuleChange *h = new history::ModuleChange;
			h->name = "button mode";
			
			h->moduleId = widget->module->id;
			h->oldModuleJ = widget->toJson();

			module->latched[id] = !module->latched[id];
			if (!module->latched[id])
				module->outValue[id] = false;
			
			// history - new settings
			h->newModuleJ = widget->toJson();
			APP->history->push(h);	
		}
	};		
	
	// button mode menu 
	struct ButtonModeMenu : MenuItem {
		ButtonsWidget *widget;
		Buttons *module;
		
		const std::string labels[NUM_GATES] = {	"Button A Latched",
												"Button B Latched",
												"Button C Latched",
												"Button D Latched",
												"Button E Latched",
												"Button F Latched"};
											
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			ButtonModeAllMenuItem *bModeMenuItemAllLatched = createMenuItem<ButtonModeAllMenuItem>("All Latched");
			bModeMenuItemAllLatched->widget = widget;
			bModeMenuItemAllLatched->module = module;
			bModeMenuItemAllLatched->latch = true;
			menu->addChild(bModeMenuItemAllLatched);

			ButtonModeAllMenuItem *nModeMenuItemAllMom = createMenuItem<ButtonModeAllMenuItem>("All Momentary");
			nModeMenuItemAllMom->widget = widget;
			nModeMenuItemAllMom->module = module;
			nModeMenuItemAllMom->latch = false;
			menu->addChild(nModeMenuItemAllMom);	
			
			for (int i = 0; i < NUM_GATES; i++) {
				ButtonModeMenuItem *bModeMenuItem = createMenuItem<ButtonModeMenuItem>(labels[i], CHECKMARK(module->latched[i]));
				bModeMenuItem->widget = widget;
				bModeMenuItem->module = module;
				bModeMenuItem->id = i;
				menu->addChild(bModeMenuItem);
			}
			
			return menu;	
		}
	};	
	
	// include the I/O mode menu item struct we'll need when we add the theme menu items
	#include "../modes/modeMenuItem.hpp"
	
	void appendContextMenu(Menu *menu) override {
		Buttons *module = dynamic_cast<Buttons*>(this->module);
		assert(module);

		// blank separator
		menu->addChild(new MenuSeparator());

		// add mode menu item
		ButtonModeMenu *bModeMenuItem = createMenuItem<ButtonModeMenu>("Button Modes", RIGHT_ARROW);
		bModeMenuItem->widget = this;
		bModeMenuItem->module = module;
		menu->addChild(bModeMenuItem);
		
		// add the I/O mode menu items
		#include "../modes/modeMenus.hpp"
	}	
};

Model *modelButtons = createModel<Buttons, ButtonsWidget>("Buttons");
