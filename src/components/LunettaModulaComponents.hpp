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
struct LunettaModulaKnob : app::SvgKnob {
	widget::SvgWidget* bg;
	widget::SvgWidget* fg;
	std::string svgFile = "";
	float orientation = 0.0;
	
	LunettaModulaKnob() {
		svgFile = "";
		orientation = 0.0;
		minAngle = -0.83*M_PI;
		maxAngle = 0.83*M_PI;

		bg = new widget::SvgWidget;
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/Components/Knob-bg.svg")));
		fb->addChildBelow(bg, tw);
		
		fg = new widget::SvgWidget;
		fb->addChildBelow(fg, tw);

		this->setSvg(Svg::load(asset::plugin(pluginInstance, "res/Components/KnobPointer.svg")));
	}
};

// red knob
template <typename TBase = LunettaModulaKnob>
struct TRedKnob : TBase {
	TRedKnob() {
		this->svgFile = "Red";
		this->fg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/Components/Knob-" + this->svgFile + "-fg.svg")));		
	}
};
typedef TRedKnob<> RedKnob;

//-------------------------------------------------------------------
// Rotary controls
//-------------------------------------------------------------------
// Rotary switch base - values are limted to whole numbers
template <typename TBase>
struct RotarySwitch : TBase {
	RotarySwitch() {
		this->snap = true;
		this->smooth = false;
	}
	
	// handle the manually entered values
	void onChange(const event::Change &e) override {
		
		SvgKnob::onChange(e);
		
		this->getParamQuantity()->setValue(roundf(this->getParamQuantity()->getValue()));
	}
};

// standard rotary potentiometer base
template <typename TBase>
struct Potentiometer : TBase {
	Potentiometer() {
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

		if (getParamQuantity()->getValue() > 0.5f)
			getParamQuantity()->setValue(1.0f);
		else
			getParamQuantity()->setValue(0.0f);
	}
};

//-------------------------------------------------------------------
// on-off-on toggle switch
//-------------------------------------------------------------------
struct LunettaModulaToggle3P : SvgSwitch {
	LunettaModulaToggle3P() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Components/SW_Toggle_2.svg")));
		
		// no shadow for switches
		shadow->opacity = 0.0f;
	}

	// handle the manually entered values
	void onChange(const event::Change &e) override {
		
		SvgSwitch::onChange(e);
		float v = getParamQuantity()->getValue();
		
		if (v > 1.33f)
			getParamQuantity()->setValue(2.0f);
		else if (v > 0.67f)
			getParamQuantity()->setValue(1.0f);
		else
			getParamQuantity()->setValue(0.0f);
	}
};

