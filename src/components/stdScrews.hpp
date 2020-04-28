//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula
//  common screw functionality
//  Copyright (C) 2020  Adam Verspaget
//------------------------------------------------------------------------

// add the left hand screws
addChild(createWidget<LunettaModulaScrew>(Vec(RACK_GRID_WIDTH, 0)));
addChild(createWidget<LunettaModulaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

// add the right hand screws for modules wider than 8HP
if (box.size.x / RACK_GRID_WIDTH > 8.0f) {
	addChild(createWidget<LunettaModulaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<LunettaModulaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}
