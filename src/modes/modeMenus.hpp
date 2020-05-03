//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
// 	common menus for selecting I/O Mode
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#ifdef CMOS_MODEL_ENABLED
// add main I/O mode menu item
ModeMenu *modeMenuItem = createMenuItem<ModeMenu>("I/O Mode", RIGHT_ARROW);
modeMenuItem->module = module;
menu->addChild(modeMenuItem);
#endif
