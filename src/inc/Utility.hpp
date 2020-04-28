//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
//	Utilities - handy little bits and bobs to make life easier
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#pragma once

// handy macros to convert a bool to an appropriate value for output and display
// CMOS can output Vdd
#define boolToGate(x) x ? 12.0f : 0.0f
#define boolToLight(x) x ? 1.0f : 0.0f 
#define boolToGateInverted(x) x ? 0.0f : 12.0f
#define boolToLightInverted(x) x ? 0.0f : 1.0f 

