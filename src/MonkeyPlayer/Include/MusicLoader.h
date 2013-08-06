// MusicLoader.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Handles loading of audio files

#include <string>
#include <vector>
#include "CollectionWindow.h"
#include "DatabaseManager.h"
#include "DatabaseStructs.h"
#include "ListItem.h"
#include "PlaylistWindow.h"

#ifndef MUSIC_LOADER_H
#define MUSIC_LOADER_H

struct MusicLoaderThreadObj
{
public:
	MusicLoaderThreadObj(std::vector<std::string> files, PlaylistWindow* playlistWindow, CollectionWindow* collWindow)
	{
		mFiles = files;
		mPlaylistWindow = playlistWindow;
		mCollectionWindow = collWindow;
		mFinished = false;
		mTerminateRequested = false;
	}
	~MusicLoaderThreadObj()
	{
	}

	std::vector<std::string> mFiles;
	PlaylistWindow* mPlaylistWindow;
	CollectionWindow* mCollectionWindow;

	bool mFinished;
	bool mTerminateRequested;
};

class MusicLoader
{
public:
	static void terminate();

	void loadDirectory(std::string dirPath);
	void loadFiles(std::vector<std::string> filenames);
	void setPlaylistWindow(PlaylistWindow *win);
	void setCollectionWindow(CollectionWindow *win);

	static MusicLoader* instance();

private:
	MusicLoader();
	~MusicLoader();

	PlaylistWindow* mPlaylistWindow;
	CollectionWindow* mCollectionWindow;
	std::vector<MusicLoaderThreadObj*> mThreads;
	static UINT loaderThread(LPVOID pParam);
	static MusicLoader* mInstance;
	static bool mTerminating;
//	static CCriticalSection mMutex;

};

#endif