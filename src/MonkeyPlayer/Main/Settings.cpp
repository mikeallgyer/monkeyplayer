//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Holds all options from a config file
// Note, this is a singleton class

#include <d3dUtil.h>
#include "Settings.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

using namespace std;
using namespace MonkeyPlayer;

//helper
void trimString(string& str);

Settings* Settings::mInstance = NULL;
const string Settings::WHITESPACE = "\t\n\r ";
const string Settings::CONTENT_DIR = "ContentDir";

const string Settings::OPT_DISPLAY_TYPE = "DisplayType";
const string Settings::OPT_RANDOM_ON = "RandomOn";
const string Settings::OPT_ORDER_BY = "OrderBy";
const string Settings::OPT_STOP_AFTER = "StopAfter";
const string Settings::OPT_STOP_AFTER_ON = "StopAfterOn";
const string Settings::OPT_GO_TO_SONG = "GoToSong";
const string Settings::OPT_AUDIO_TYPES = "AudioTypes";
const string Settings::LAST_ALBUM_VIEWED = "LastAlbumIndex";

Settings::Settings()
{
#ifdef _DEBUG
	mFilename = "settings.cfg";
#else
	TCHAR appPath[1024];
	getAppDataPath(appPath, "settings.cfg"); 
	mFilename = appPath;
#endif

	loadValues();
}
Settings::~Settings()
{
}

Settings* Settings::instance()
{
	if (mInstance== NULL)
	{
		// use "unsafe" new, so it doesn't show up in the memory dump
		mInstance = snew Settings();
		Logger::instance()->write("Using settings file: " + mInstance->mFilename);
	}
	return mInstance;
}
void Settings::destroy()
{
	if (mInstance != NULL)
	{
		delete mInstance;
		mInstance = NULL;
	}
}
// get value with given name.
// if name doesn't exist, default value is
// saved and returned
string Settings::getStringValue(string name, string defaultValue)
{
	if (mValues.find(name) == mValues.end())
	{
		setValue(name, defaultValue);
	}
	return mValues[name];
}
// set (and save) value
void Settings::setValue(string name, string value)
{
	mValues[name] = value;
	writeValues();
}

// get value with given name.
// if name doesn't exist, default value is
// saved and returned
int Settings::getIntValue(string name, int defaultValue)
{
	if (mValues.find(name) == mValues.end())
	{
		setValue(name, defaultValue);
	}
	int numValue;
	try 
	{
		istringstream stream(mValues[name]);
		stream >> numValue;
	}
	catch (...)
	{
		setValue(name, defaultValue);
		numValue = defaultValue;
	}
	return numValue;
}
// set (and save) value
void Settings::setValue(string name, int value)
{
	stringstream sstream;
	sstream << value;
	mValues[name] = sstream.str();
	writeValues();
}
// get value with given name.
// if name doesn't exist, default value is
// saved and returned
bool Settings::getBoolValue(string name, bool defaultValue)
{
	return getIntValue(name, defaultValue ? 1 : 0) != 0;
}
// set (and save) value
void Settings::setValue(string name, bool value)
{
	setValue(name, value ? 1 : 0);
}

// get value with given name.
// if name doesn't exist, default value is
// saved and returned
float Settings::getFloatValue(string name, float defaultValue)
{
	if (mValues.find(name) == mValues.end())
	{
		setValue(name, defaultValue);
	}
	float numValue;
	try 
	{
		istringstream stream(mValues[name]);
		stream >> numValue;
	}
	catch (...)
	{
		setValue(name, defaultValue);
		numValue = defaultValue;
	}
	return numValue;
}
// set (and save) value
void Settings::setValue(string name, float value)
{
	stringstream sstream;
	sstream << value;
	mValues[name] = sstream.str();
	writeValues();
}

void Settings::loadValues()
{
	mValues.clear();

	try 
	{
		ifstream infile(mFilename.c_str());

		if (infile.good())
		{
			string currLine = "";
			
			while (infile.good())
			{
				getline(infile, currLine);
				trimString(currLine);

				// comments
				if (currLine.length() > 1 && currLine[0] != '#')
				{
					int sepPos = currLine.find('=');
					if (sepPos != string::npos)
					{
						string name = currLine.substr(0, sepPos);
						string value = currLine.substr(sepPos + 1);
						trimString(name);
						trimString(value);
						mValues[name] = value;
					}
				}
			}

		}
		infile.close();
	}
	catch (...) {}
}
void Settings::writeValues()
{
	try 
	{
		ofstream outfile(mFilename.c_str());
		map<string, string>::iterator iter;
		for (iter = mValues.begin(); iter != mValues.end(); iter++)
		{
			outfile << (*iter).first << " = " << (*iter).second << endl;
		}
		outfile.close();
	}
	catch (...) {}
}

void Settings::getAppDataPath(TCHAR* outPath, TCHAR* filename)
{
	// Get path for each computer, non-user specific and non-roaming data.
   if ( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, 
                                    NULL, 0, outPath ) ) )
   {
      // Append product-specific path - this path needs to already exist
      // for GetTempFileName to succeed.
      PathAppend( outPath, "\\MonkeyPlayer\\" );
		CreateDirectory(outPath, 0);
      PathAppend( outPath, filename );
	}
}
void trimString(string& str)
{
	if (str.length() < 1)
	{
		return;
	}
   unsigned int start = 0;
   while (start < str.length() && Settings::WHITESPACE.find(str[start]) != string::npos)
   {
      ++start;
   }
   unsigned int end = str.length() - 1;
	while (end >= 0 && Settings::WHITESPACE.find(str[end]) != string::npos)
   {
      --end;
   }

   ++end;
   int length = end - start;
   if (length >= 0)
   {
      str = str.substr(start, length);
   }
}