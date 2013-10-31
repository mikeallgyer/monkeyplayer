//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Holds all options from a config file
// Note, this is a singleton class

#include <d3dUtil.h>
#include "d3dApp.h"
#include "Logger.h"
#include "Settings.h"

#include <fstream>
#include <sstream>
#include <string>
#include <time.h>

using namespace std;
using namespace MonkeyPlayer;

Logger* Logger::mInstance = NULL;

Logger::Logger()
{
#if _DEBUG
	mFilename = "MonkeyPlayer.log";
#else
	TCHAR path[1024];
	Settings::getAppDataPath(path, "MonkeyPlayer.log");
	mFilename = path;
#endif
	mDisableLogging = false;
}

Logger* Logger::instance()
{
	if (mInstance == NULL)
	{
		mInstance = snew Logger();
		try
		{
			ofstream outfile(mInstance->mFilename.c_str());

			outfile << "****" << getCurrTime() << ": Logging Begin****\n";

			outfile.close();
		}
		catch (...)
		{
			MessageBox(0, "Cannot open logfile.", "Logger", 0);
			mInstance->mDisableLogging = true;
		}
	}

	return mInstance;
}

void Logger::destroy()
{
	if (mInstance != NULL)
	{
		mInstance->write("****Logging End****");
		delete mInstance;
	}
}

void Logger::write(string message)
{
	if (!mDisableLogging)
	{
		try
		{
			ofstream outfile(mFilename.c_str(), ios::app);

			char buf[2048];
			sprintf_s(buf, 2048, "%s: %s\n", getCurrTime().c_str(), message.c_str());
			outfile << buf;

			outfile.close();
		}
		catch (...)
		{
			MessageBox(0, "Cannot open logfile.", "Logger", 0);
			mDisableLogging = true;
		}
	}
}

string Logger::getCurrTime()
{
	time_t currTime;
	struct tm timeInfo;

	time(&currTime);
	localtime_s(&timeInfo, &currTime);
	char timeBuf[32];
	asctime_s(timeBuf, 32, &timeInfo);

	string timeStr(timeBuf);
	timeStr = timeStr.substr(0, 24);
	return timeStr;
}