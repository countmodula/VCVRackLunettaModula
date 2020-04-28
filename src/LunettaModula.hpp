//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula 
//	Header.
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------
#include "rack.hpp"
using namespace rack;

// Forward-declare the Plugin
extern Plugin *pluginInstance;

// Forward-declare each Model, defined in each module source file
#include "DeclareModels.hpp"

// theme functions
int readDefaultTheme();
void saveDefaultTheme(int theme);

#include "components/LunettaModulaComponents.hpp"
#include "components/StdComponentPositions.hpp"


