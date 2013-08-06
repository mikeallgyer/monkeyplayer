// MusicLibrary.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Handles library of audio files

#include <vector>
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "MusicLoader.h"


#ifndef MUSIC__LOADER__H
#define MUSIC__LOADER__H

class MusicLibrary
{
	static MusicLibrary* instance();

private:

	MusicLibrary();
	static MusicLibrary* mInstance;

	// synchronization
	static CCriticalSection mCritSection;
};

#endif