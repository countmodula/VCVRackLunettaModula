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
struct LunettaModulaScrew : SvgScrew {
	LunettaModulaScrew() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/ScrewHex.svg")));
		box.size = sw->box.size;
	}
};

//-------------------------------------------------------------------
// Ports
//-------------------------------------------------------------------
struct LunettaModulaLogicInputJack : SvgPort {
	LunettaModulaLogicInputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicInputJack.svg")));
	}
};

struct LunettaModulaLogicOutputJack : SvgPort {
	LunettaModulaLogicOutputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicOutputJack.svg")));
	}
};

struct LunettaModulaLogicConstantHighJack : SvgPort {
	LunettaModulaLogicConstantHighJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicConstantHighJack.svg")));
	}
};

struct LunettaModulaLogicConstantLowJack : SvgPort {
	LunettaModulaLogicConstantLowJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/LogicConstantLowJack.svg")));
	}
};

struct LunettaModulaAnalogInputJack : SvgPort {
	LunettaModulaAnalogInputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/AnalogInputJack.svg")));
	}
};

struct LunettaModulaAnalogOutputJack : SvgPort {
	LunettaModulaAnalogOutputJack() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/AnalogOutputJack.svg")));
	}
};

//-------------------------------------------------------------------
// Knobs
//-------------------------------------------------------------------

// base knob
struct LunettaModulaKnob : SvgKnob {
	LunettaModulaKnob() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
	}
};

// coloured knobs
struct LunettaModulaKnobRed : LunettaModulaKnob {
	LunettaModulaKnobRed() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/KnobRed.svg")));
	}
};

//-------------------------------------------------------------------
// rotary switches
//-------------------------------------------------------------------
// TODO: parameterise the colour

struct LunettaModulaRotarySwitch : SvgKnob {
	LunettaModulaRotarySwitch() {
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;
		snap = true;
		smooth = false;
	}
	
	// handle the manually entered values
	void onChange(const event::Change &e) override {
		
		SvgKnob::onChange(e);
		
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}
	
	
	// override the base randomizer as it sets switches to invalid values.
	void randomize() override {
		SvgKnob::randomize();
		
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}	
	
};

struct LunettaModulaRotarySwitchRed : LunettaModulaRotarySwitch {
	LunettaModulaRotarySwitchRed() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/KnobRed.svg")));
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

