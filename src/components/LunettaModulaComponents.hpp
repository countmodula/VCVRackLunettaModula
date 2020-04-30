//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
//	Custom components
//  Copyright (C) 2020  Adam Verspaget 
//----------------------------------------------------------------------------
#include "componentlibrary.hpp"

using namespace rack;


//-------------------------------------------------------------------
// screws
//-------------------------------------------------------------------
struct LunettaModulaScrew : SVGScrew {
	LunettaModulaScrew() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/ScrewHex.svg")));
		box.size = sw->box.size;
	}
};


//-------------------------------------------------------------------
// Ports
//-------------------------------------------------------------------
struct LunettaModulaLogicInputJack : SVGPort {
	LunettaModulaLogicInputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicInputJack.svg")));
	}
};

struct LunettaModulaLogicOutputJack : SVGPort {
	LunettaModulaLogicOutputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicOutputJack.svg")));
	}
};

struct LunettaModulaLogicConstantHighJack : SVGPort {
	LunettaModulaLogicConstantHighJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicConstantHighJack.svg")));
	}
};

struct LunettaModulaLogicConstantLowJack : SVGPort {
	LunettaModulaLogicConstantLowJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicConstantLowJack.svg")));
	}
};

//-------------------------------------------------------------------
// on-off toggle switch
//-------------------------------------------------------------------
struct LunettaModulaToggle2P : SvgSwitch {
	int pos;
	int neg;

	LunettaModulaToggle2P() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_2.svg")));

		// no shadow for switches
		shadow->opacity = 0.0f;

		neg = pos = 0;
	}
	
	// handle the manually entered values
	void onChange(const event::Change &e) override {
		
		SvgSwitch::onChange(e);
		
		if (paramQuantity->getValue() > 0.5f)
			paramQuantity->setValue(1.0f);
		else
			paramQuantity->setValue(0.0f);
	}
	
	// override the base randomizer as it sets switches to invalid values.
	void randomize() override {
		SvgSwitch::randomize();

		if (paramQuantity->getValue() > 0.5f)
			paramQuantity->setValue(1.0f);
		else
			paramQuantity->setValue(0.0f);
	}	
};

//-------------------------------------------------------------------
// on-off-on toggle switch
//-------------------------------------------------------------------
struct LunettaModulaToggle3P : SvgSwitch {
	int pos;
	int neg;
	
	LunettaModulaToggle3P() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_2.svg")));
		
		// no shadow for switches
		shadow->opacity = 0.0f;
		
		neg = pos = 0;
	}

	// handle the manually entered values
	void onChange(const event::Change &e) override {
		
		SvgSwitch::onChange(e);
		
		if (paramQuantity->getValue() > 1.33f)
			paramQuantity->setValue(2.0f);
		else if (paramQuantity->getValue() > 0.67f)
			paramQuantity->setValue(1.0f);
		else
			paramQuantity->setValue(0.0f);
	}
	
	// override the base randomizer as it sets switches to invalid values.
	void randomize() override {
		SvgSwitch::randomize();
		
		if (paramQuantity->getValue() > 1.33f)
			paramQuantity->setValue(2.0f);
		else if (paramQuantity->getValue() > 0.67f)
			paramQuantity->setValue(1.0f);
		else
			paramQuantity->setValue(0.0f);
	}
};

//-------------------------------------------------------------------
// push button base
//-------------------------------------------------------------------
struct LunettaModulaPB :  SvgSwitch {
	LunettaModulaPB() {
		// no shadow for switches or buttons
		shadow->opacity = 0.0f;
	}

	// override the base randomizer as it sets switches to invalid values.
	void randomize() override {
		SvgSwitch::randomize();
		
		if (paramQuantity->getValue() > 0.5f)
			paramQuantity->setValue(1.0f);
		else
			paramQuantity->setValue(0.0f);
	}
};

//-------------------------------------------------------------------
// push button
//-------------------------------------------------------------------
struct LunettaModulaPBSwitch : LunettaModulaPB {
    LunettaModulaPBSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton_1.svg")));
    }
};

struct LunettaModulaPBSwitchMomentary : LunettaModulaPB {
    LunettaModulaPBSwitchMomentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton_1.svg")));

		momentary = true;
    }
};
 
struct LunettaModulaPBSwitchMomentaryUnlit : LunettaModulaPB {
    LunettaModulaPBSwitchMomentaryUnlit() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButton_0.svg")));

		momentary = true;
    }
}; 
 
//-------------------------------------------------------------------
// small square push button
//-------------------------------------------------------------------
struct LunettaModulaPBSwitchMini : LunettaModulaPB {
    LunettaModulaPBSwitchMini() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButtonMini_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButtonMini_1.svg")));
    }
};

struct LunettaModulaPBSwitchMiniMomentary : LunettaModulaPB {
    LunettaModulaPBSwitchMiniMomentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButtonMini_0.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/PushButtonMini_1.svg")));
		
 		momentary = true;
    }
};
