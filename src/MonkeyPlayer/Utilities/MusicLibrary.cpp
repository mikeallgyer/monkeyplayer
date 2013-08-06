// MusicLibrary.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Handles library of audio files

#include <vector>
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MusicLibrary.h"

MusicLibrary* MusicLibrary::mInstance = NULL;

MusicLibrary::MusicLibrary()
{
}

MusicLibrary* MusicLibrary::instance()
{
	if (mInstance == NULL)
	{
		mInstance = new MusicLibrary();
	}
	return mInstance;
}