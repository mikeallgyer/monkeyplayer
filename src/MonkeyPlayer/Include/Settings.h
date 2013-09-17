//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Holds all options from a config file
// Note, this is a singleton class

#ifndef SETTINGS_H
#define SETTINGS_H

#include <fstream>
#include <map>
#include <string>
#include <shlwapi.h>

using namespace std;

class Settings
{

public:
	static Settings* instance();
	static void destroy();

	// get value with given name.
	// if name doesn't exist, default value is
	// saved and returned
	string getStringValue(string name, string defaultValue);
	// set (and save) value
	void setValue(string name, string value);

	// get value with given name.
	// if name doesn't exist, default value is
	// saved and returned
	int getIntValue(string name, int defaultValue);
	// set (and save) value
	void setValue(string name, int value);

	// get value with given name.
	// if name doesn't exist, default value is
	// saved and returned
	bool getBoolValue(string name, bool defaultValue);
	// set (and save) value
	void setValue(string name, bool value);

	// get value with given name.
	// if name doesn't exist, default value is
	// saved and returned
	float getFloatValue(string name, float defaultValue);
	// set (and save) value
	void setValue(string name, float value);

	static void getAppDataPath(TCHAR* outPath, TCHAR* filename);
	static const string WHITESPACE;
	static const string CONTENT_DIR;
	static const string OPT_DISPLAY_TYPE;
	static const string OPT_RANDOM_ON;
	static const string OPT_ORDER_BY;
	static const string OPT_STOP_AFTER;
	static const string OPT_STOP_AFTER_ON;
	static const string OPT_GO_TO_SONG;
private:
	Settings();
	~Settings();

	void loadValues();
	void writeValues();

	// members
	static Settings *mInstance;
	string mFilename;
	map<string, string> mValues;
};

#endif