//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Helper functions for file operations

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <iostream>
#include <string>
#include <vector>
//#include <windows.h>
#include "d3dUtil.h"
#include "Settings.h"

using namespace std;

class FileManager
{
public:

	static const string FILE_TYPE_MUSIC;
	static const string FILE_TYPE_IMAGE;

	static int getAllFiles(vector<string> &matchingFiles, string &rootDir, const string &extensions,
		bool recursive = true);
	static std::string getContainingDirectory(std::string path);
	static std::string getFileName(std::string path);
	static std::string getContentAsset(std::string &relPath);

};
#endif
