//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
//	Utilities - handy little bits and bobs to make life easier
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#pragma once

// supply voltage for CMOS
#define VDD 12.0f

// handy macros to convert a bool to an appropriate value for output and display
#define boolToGate(x) x ? gateVoltage : 0.0f
#define boolToLight(x) x ? 1.0f : 0.0f 
#define boolToGateInverted(x) x ? 0.0f : gateVoltage
#define boolToLightInverted(x) x ? 0.0f : 1.0f 

enum CMOSModes {
	VCVRACK_STANDARD,
	CMOS_NON_SCHMITT,
	CMOS_SCHMITT	
};


