//----------------------------------------------------------------------------
//	Lunetta Modula Plugin for VCV Rack by Count Modula 
//  Copyright (C) 2020  Adam Verspaget
//----------------------------------------------------------------------------

#include "LunettaModula.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

#include "InitialiseModels.hpp"

}


// save the given global count modula lunetta settings`
void saveSettings(json_t *rootJ) {
	std::string settingsFilename = asset::user("LunettaModula.json");
	
	FILE *file = fopen(settingsFilename.c_str(), "w");
	
	if (file) {
		json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
		fclose(file);
	}
}

// read the global count modula settings
json_t * readSettings() {
	std::string settingsFilename = asset::user("LunettaModula.json");
	FILE *file = fopen(settingsFilename.c_str(), "r");
	
	if (!file) {
		return json_object();
	}
	
	json_error_t error;
	json_t *rootJ = json_loadf(file, 0, &error);
	
	fclose(file);
	return rootJ;
}

// read the default theme value from the global count modula lunetta settings file
int readDefaultTheme() {
	int theme = 0; // default to the standard theme
	
	// read the settings file
	json_t *rootJ = readSettings();
	
	// get the default theme value
	json_t* jsonTheme = json_object_get(rootJ, "DefaultTheme");
	if (jsonTheme)
		theme = json_integer_value(jsonTheme);

	// houskeeping
	json_decref(rootJ);
	
	return theme;
}

// save the given theme value in the global count modula lunetta settings file
void saveDefaultTheme(int theme) {
	// read the settings file
	json_t *rootJ = readSettings();
	
	// set the default theme value
	json_object_set_new(rootJ, "DefaultTheme", json_integer(theme));

	// save the updated data
	saveSettings(rootJ);
	
	// houskeeping
	json_decref(rootJ);
}




