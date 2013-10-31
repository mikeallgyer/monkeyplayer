//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Holds all options from a config file
// Note, this is a singleton class

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>

using namespace std;

namespace MonkeyPlayer
{
	class Logger
	{
	public:
		static Logger* instance();
		static void destroy();
		
		void write(string message);

	private:
		Logger();
		static string getCurrTime();

		static Logger* mInstance;
		string mFilename;
		bool mDisableLogging;
	};
}
#endif