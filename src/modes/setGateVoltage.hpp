//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
// 	common code for setting the output gate voltage based on the mode
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------

gateVoltage = (mode == VCVRACK_STANDARD ? 10.0f : VDD);