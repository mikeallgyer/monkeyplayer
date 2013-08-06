//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Helper functions for file operations

#include <iostream>
#include <string>
#include <vector>

#include "FileManager.h"

using namespace std;

const string FileManager::FILE_TYPE_MUSIC = ".MP3.WAV";
const string FileManager::FILE_TYPE_IMAGE = ".JPG.JPEG.GIF.GIFF.PNG.DDS";

std::string toUpper(const std::string & s)
{
    std::string ret(s.size(), char());
    for(unsigned int i = 0; i < s.size(); ++i)
        ret[i] = (s[i] <= 'z' && s[i] >= 'a') ? s[i]-('a'-'A') : s[i];
    return ret;
}

int FileManager::getAllFiles(vector<string> &matchingFiles, string &rootDir, const string &extensions, 
									  bool recursive)
{
	int count = 0;

	std::string     strFilePath;          // Filepath
	std::string     strPattern;           // Pattern
	std::string     strExtension;         // Extension
	HANDLE          hFile;                // Handle to file
	WIN32_FIND_DATA FileInformation;      // File information

	if (rootDir[rootDir.length() - 1] != '\\')
	{
		rootDir = rootDir + "\\";
	}

	strPattern = rootDir + "*.*";
	hFile = ::FindFirstFile(strPattern.c_str(), &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != '.')
			{
				strFilePath.erase();
				strFilePath = rootDir + FileInformation.cFileName;

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(recursive)
					{
						// Search subdirectory
						count += getAllFiles(matchingFiles, strFilePath, extensions, recursive);
					}
				}
				else
				{
					// Check extension
					strExtension = FileInformation.cFileName;
					
					size_t dotPos = strExtension.rfind(".");
					if (dotPos != string::npos)
					{
						strExtension = strExtension.substr(dotPos);
						strExtension = toUpper(strExtension);
						if (extensions.find("*") != string::npos ||
							(extensions.find(strExtension)!= string::npos))
						{
							// Increase counter
							++count;
							matchingFiles.push_back(rootDir + FileInformation.cFileName);
						}
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}

	return count;
}

std::string FileManager::getFileName(std::string path)
{
	if (path.length() > 0)
	{
		unsigned int slashPos = path.find_last_of('\\');
		unsigned int backslashPos = path.find_last_of('/');
		unsigned int startPos = std::string::npos;

		if (slashPos != std::string::npos && backslashPos != std::string::npos)
		{
			startPos = max(slashPos, backslashPos);
		}
		else if (slashPos != std::string::npos)
		{
			startPos = slashPos;
		}
		else if (backslashPos != std::string::npos)
		{
			startPos = backslashPos;
		}

		if (startPos != std::string::npos)
		{
			if ( startPos < (path.length() - 1))
			{
				startPos++;
			}
			path = path.substr(startPos);
		}
	}
	return path;
}

std::string FileManager::getContainingDirectory(std::string path)
{
	if (path.length() > 0)
	{
		unsigned int slashPos = path.find_last_of('\\');
		unsigned int backslashPos = path.find_last_of('/');
		unsigned int startPos = std::string::npos;

		if (slashPos != std::string::npos && backslashPos != std::string::npos)
		{
			startPos = max(slashPos, backslashPos);
		}
		else if (slashPos != std::string::npos)
		{
			startPos = slashPos;
		}
		else if (backslashPos != std::string::npos)
		{
			startPos = backslashPos;
		}

		if (startPos != std::string::npos)
		{
			if ( startPos < (path.length() - 1))
			{
				startPos++;
			}
			path = path.substr(0, startPos);
		}
		else 
		{
			path = ".";
		}
	}
	else 
	{
		path = ".";
	}
	return path;
}

std::string FileManager::getContentAsset(std::string &relPath)
{
	return Settings::instance()->getStringValue(Settings::CONTENT_DIR, "") + "\\" + relPath;

}