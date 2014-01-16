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
#include "PlaybackOptionsWindow.h"


#ifndef MUSIC__LOADER__H
#define MUSIC__LOADER__H

namespace MonkeyPlayer
{
	class MusicLibrary
	{
	public:
		static MusicLibrary* instance();
		static void destroy();

		void update(float dt);
		void playNextSong();
		void playPreviousSong();
		void playCurrentSong();
		void playSong(string song);
		void setPlaylistWindow(PlaylistWindow* win);
		PlaylistWindow* getPlaylistWindow();
		void setPlaybackOptionsWindow(PlaybackOptionsWindow* win);
		PlaybackOptionsWindow* getPlaybackOptionsWindow();

		static void soundEventCB(void *obj, SoundManager::SoundEvent ev)
		{
			MusicLibrary* win = static_cast<MusicLibrary*>(obj);
			if (win)
			{
				win->onSoundEvent(ev);
			}
		}

	private:

		struct HistoryRecord 
		{
			string file;
			bool fromQueue;
			HistoryRecord(string f, bool queue) : file(f), fromQueue(queue) {}
		};
		MusicLibrary();

		void onSoundEvent(SoundManager::SoundEvent ev);
		bool getStopPlaying(bool usePlaylist);

		static MusicLibrary* mInstance;
		static const string CURRENT_SONG;
		bool mPlayNextSong;
		bool mNextFromCallback;
		bool mPlayPreviousSong;
		string mNextSong;
		int mLastPlayedId;
		vector<HistoryRecord> mHistory;
		bool mLastPlayedQueue;
		bool mPlayingPrevious;

		PlaylistWindow* mPlaylistWindow;
		PlaybackOptionsWindow* mPlaybackOptionsWindow;

		// synchronization
		static CCriticalSection mCritSection;
	};
}
#endif