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
#include "PlaylistWindow.h"


#ifndef MUSIC__LOADER__H
#define MUSIC__LOADER__H

class MusicLibrary
{
public:
	static MusicLibrary* instance();
	static void destroy();

	void update(float dt);
	void playNextSong();
	void playPreviousSong();
	void playSong(string song);
	void setPlaylistWindow(PlaylistWindow* win);
	PlaylistWindow* getPlaylistWindow();

	static void soundEventCB(void *obj, SoundManager::SoundEvent ev)
	{
		MusicLibrary* win = static_cast<MusicLibrary*>(obj);
		if (win)
		{
			win->onSoundEvent(ev);
		}
	}

private:

	MusicLibrary();

	void onSoundEvent(SoundManager::SoundEvent ev);

	static MusicLibrary* mInstance;
	bool mPlayNextSong;
	bool mPlayPreviousSong;
	string mNextSong;
	PlaylistWindow* mPlaylistWindow;

	// synchronization
	static CCriticalSection mCritSection;
};

#endif