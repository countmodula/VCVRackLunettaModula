//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
// 	common menu item structs for handling selecting I/O modes
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#ifdef CMOS_MODEL_ENABLED
// theme selection menu item
struct ModeMenuItem : MenuItem {
	MODULE_NAME *module;
	int modeToUse = 0;
	
	void onAction(const event::Action &e) override {
		module->setIOMode(modeToUse);
		module->ioMode = modeToUse;
	}
};

// I/O mode menu
struct ModeMenu : MenuItem {
	MODULE_NAME *module;
	
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		// VCV Rack Standard I/O levels: 10V gates, Schmitt trigger set at 0.1 and 2 volts
		ModeMenuItem *standardMenuItem = createMenuItem<ModeMenuItem>("VCV Rack Standard", CHECKMARK(module->ioMode == VCVRACK_STANDARD));
		standardMenuItem->module = module;
		standardMenuItem->modeToUse = VCVRACK_STANDARD;
		menu->addChild(standardMenuItem);
		
#ifdef CMOS_IO_MODEL
		// CMOS standard I/O levels: 12V gates, no Schmitt trigger and unstable at the high/low trip point
		ModeMenuItem *cmosNSTMenuItem = createMenuItem<ModeMenuItem>("CMOS Non-Schmitt Trigger", CHECKMARK(module->ioMode == CMOS_NON_SCHMITT));
		cmosNSTMenuItem->module = module;
		cmosNSTMenuItem->modeToUse = CMOS_NON_SCHMITT;
		menu->addChild(cmosNSTMenuItem);
#endif

		// CMOS Standard I/O levels: 12V gates, Schmitt trigger inputs set at approx 1/3 and 2/3 Vdd (Vdd = 12 Volts here)
		ModeMenuItem *csmosSTMenuItem = createMenuItem<ModeMenuItem>("CMOS Schmitt Trigger", CHECKMARK(module->ioMode == CMOS_SCHMITT));
		csmosSTMenuItem->module = module;
		csmosSTMenuItem->modeToUse = CMOS_SCHMITT;
		menu->addChild(csmosSTMenuItem);
		
		return menu;	
	}
};
#endif