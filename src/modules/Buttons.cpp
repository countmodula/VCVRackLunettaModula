//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula - Buttons
//	Hex manual logic buttons
//  Copyright (C) 2021  Adam Verspaget
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
		ENUMS(LATCH_LIGHTS, NUM_GATES),
		ENUMS(BTN_PARAM_LIGHTS, NUM_GATES),
		NUM_LIGHTS
	};
	
	// add the variables we'll use when managing modes
	#include "../modes/modeVariables.hpp"

	int processCount = 8;
	int moduleVersion = 2;
	bool setButtonModes = false;
	
	// for backward compatibility with v1 modules
	bool latched[NUM_GATES] = {};
	bool buttonValue[NUM_GATES] = {};
	
	Buttons() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		
		char c = 'A';
		for (int g = 0; g < NUM_GATES; g++) {
			configButton(BTN_PARAMS + g, rack::string::f("Button %c", c));
			configOutput(BTN_PARAMS + g, rack::string::f("Button %c", c++));
		}
			
		moduleVersion = 2;
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

		json_object_set_new(root, "moduleVersion", json_integer(2));

		json_t *mod = json_array();
		json_t *val = json_array();
		
		for (int g = 0; g < NUM_GATES; g++) {
			json_array_insert_new(mod, g, json_boolean(latched[g]));
			json_array_insert_new(val, g, json_boolean(params[BTN_PARAMS + g].getValue() > 0.5f));
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
					buttonValue[g] = json_boolean_value(v);
			}
		}
		
		// grab the I/O mode details
		#include "../modes/dataFromJson.hpp"
		
		setButtonModes = true;
	}	

	void process(const ProcessArgs &args) override {
		
		// process buttons
		if (++processCount > 8) {
			processCount = 0;
			
			for (int g = 0; g < NUM_GATES; g++) {
				bool q = params[BTN_PARAMS + g].getValue() > 0.5f;
				outputs[Q_OUTPUTS + g].setVoltage(boolToGate(q));
				lights[Q_LIGHTS + g].setBrightness(boolToLight(q));
				lights[LATCH_LIGHTS + g].setBrightness(boolToLight(latched[g]));
			}
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
			addParam(createParamCentered<LunettaModulaLEDPushButtonMomentary<LunettaModulaPBLight<RedLight>>>(Vec(STD_COLUMN_POSITIONS[STD_COL1], STD_ROWS6[STD_ROW1 + g]), module, Buttons::BTN_PARAMS + g, Buttons::BTN_PARAM_LIGHTS + g));
			
			// Q output
			addOutput(createOutputCentered<LunettaModulaLogicOutputJack>(Vec(STD_COLUMN_POSITIONS[STD_COL3], STD_ROWS6[STD_ROW1 + g]), module, Buttons::Q_OUTPUTS + g));
			
			// lights
			addChild(createLightCentered<SmallLight<GreenLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL2] - 8, STD_ROWS6[STD_ROW1 + g] - 19), module, Buttons::LATCH_LIGHTS + g));
			addChild(createLightCentered<SmallLight<RedLight>>(Vec(STD_COLUMN_POSITIONS[STD_COL3] + 12, STD_ROWS6[STD_ROW1 + g] - 19), module, Buttons::Q_LIGHTS + g));
		}
	}
	
	// all button mode menu item
	struct ButtonModeAllMenuItem : MenuItem {
		ButtonsWidget *widget;
		Buttons *module;
		bool latch;
	
		void onAction(const event::Action &e) override {
			if (latch) {
				for (int i = 0; i < NUM_GATES; i++) {
					LunettaModulaLitPB *pB = (LunettaModulaLitPB *)(widget->getParam(Buttons::BTN_PARAMS + i));
					pB->setLatchMode(false); 
					module->latched[i] = true;
				}
			}
			else {
				for (int i = 0; i < NUM_GATES; i++) {
					LunettaModulaLitPB *pB = (LunettaModulaLitPB *)(widget->getParam(Buttons::BTN_PARAMS + i));
					pB->setMomentaryMode(); 
					module->latched[i] = false;
				}			
			}
		}
	};		
	
	// button mode menu item
	struct ButtonModeMenuItem : MenuItem {
		ButtonsWidget *widget;
		Buttons *module;
		int id;
		
		void onAction(const event::Action &e) override {
			LunettaModulaLitPB *pB = (LunettaModulaLitPB *)(widget->getParam(Buttons::BTN_PARAMS + id));
			pB->toggleMode();
			module->latched[id] ^= true;
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
				LunettaModulaLitPB *pB = (LunettaModulaLitPB *)(widget->getParam(Buttons::BTN_PARAMS + i));

				ButtonModeMenuItem *bModeMenuItem = createMenuItem<ButtonModeMenuItem>(labels[i], CHECKMARK(pB->momentary == false));
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
	
	void step() override {
		if (module) {
			Buttons *m = dynamic_cast<Buttons*>(module);

			if (m->setButtonModes) {
				m->setButtonModes = false;
				
				for (int i = 0; i < NUM_GATES; i ++) {
					if (m->latched[i]) {
						LunettaModulaLitPB *pB = (LunettaModulaLitPB *)(getParam(Buttons::BTN_PARAMS + i));
						pB->setLatchMode(m->buttonValue[i]);
					}
				}
			}
		}

		Widget::step();
	}	
};

Model *modelButtons = createModel<Buttons, ButtonsWidget>("Buttons");
