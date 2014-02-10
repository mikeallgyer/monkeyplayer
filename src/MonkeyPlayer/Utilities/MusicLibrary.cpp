// MusicLibrary.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Handles library of audio files

#include <vector>
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MusicLibrary.h"
#include "SoundManager.h"

using namespace MonkeyPlayer;

MusicLibrary* MusicLibrary::mInstance = NULL;
CONST string MusicLibrary::CURRENT_SONG = "CURRENT";

MusicLibrary::MusicLibrary()
{
	mPlaylistWindow = NULL;
	mPlaybackOptionsWindow = NULL;

	SoundManager::instance()->addCallback(soundEventCB, this);
	mNextFromCallback = false;
	mPlayNextSong = false;
	mPlayPreviousSong = false;
	gInput->registerGlobalKey(VK_MEDIA_PLAY_PAUSE);
	gInput->registerGlobalKey(VK_MEDIA_STOP);
	gInput->registerGlobalKey(VK_MEDIA_NEXT_TRACK);
	gInput->registerGlobalKey(VK_MEDIA_PREV_TRACK);
	mLastPlayedId = -1;
	mLastPlayedQueue = false;
	mPlayingPrevious = false;
}
void MusicLibrary::update(float dt)
{
	if (gInput->keyPressed(VK_MEDIA_PLAY_PAUSE))
	{
		SoundManager::instance()->setPaused(!SoundManager::instance()->isPaused());
	}
	if (gInput->keyPressed(VK_MEDIA_STOP))
	{
		SoundManager::instance()->stop();
	}
	bool nextBtnPushed = false;
	if (gInput->keyPressed(VK_MEDIA_NEXT_TRACK))
	{
		mPlayNextSong = true;
		mNextSong = "";
		nextBtnPushed = true;
	}
	if (gInput->keyPressed(VK_MEDIA_PREV_TRACK))
	{
		mPlayPreviousSong = true;
		mNextSong = "";
	}

	if (mPlayNextSong)
	{
		if (mNextSong == CURRENT_SONG)
		{
			mLastPlayedQueue = true;
			if (!mPlaylistWindow->playCurrentSong())
			{
				mLastPlayedQueue = false;
				mPlaybackOptionsWindow->playCurrentSong();
			}
		}
		else if (mNextSong != "")
		{
			SoundManager::instance()->playFile(mNextSong.c_str());
		}
		else 
		{
			bool played = false;
			mLastPlayedQueue = true;

			// pick from album or artist
			if (mNextFromCallback && mPlaybackOptionsWindow->isRepeatOn() && mPlaybackOptionsWindow->getRepeat() == PlaybackOptionsWindow::SONG)
			{
				Track lastPlayed;
				DatabaseManager::instance()->getTrack(mLastPlayedId, &lastPlayed);
				if (lastPlayed.Id != DatabaseStructs::INVALID_ID)
				{
					SoundManager::instance()->playFile(lastPlayed.Filename.c_str());
					mLastPlayedQueue = false;
					played = true;
				}
			}

			if (!played)
			{
				bool usePlaylist = mNextFromCallback && mPlaybackOptionsWindow->isRepeatOn() && 
					mPlaybackOptionsWindow->getRepeat() != PlaybackOptionsWindow::QUEUE;
				// stop playing?
				bool stopPlaying = false;
				if (!nextBtnPushed)
				{
					stopPlaying = getStopPlaying(usePlaylist);
				}
				if (!stopPlaying)
				{
					if ((usePlaylist || !mPlaylistWindow->playNextSong(mPlaybackOptionsWindow->isRepeatOn())))
					{
						mPlaybackOptionsWindow->playNextSong();
						mLastPlayedQueue = true;
					}
				}
			}
		}
		mPlayNextSong = false;
		mNextSong = "";
		mNextFromCallback = false;
	}
	else if (mPlayPreviousSong)
	{
		bool usePlaylist = false;//!mPlaybackOptionsWindow->isRepeatOn();
/*		if ((usePlaylist || !mPlaylistWindow->playPreviousSong()))
		{
			mPlaybackOptionsWindow->playPreviousSong();
		}
		*/
		if (mHistory.size() > 0)
		{
			mHistory.pop_back(); // currently playing
		}
		if (mHistory.size() > 0)
		{
			mPlayingPrevious = true;
			HistoryRecord r = mHistory[mHistory.size() - 1];
			
			if (r.fromQueue)
			{
				Track* t = mPlaylistWindow->getPreviousSong();
				if (t->Filename == r.file)
				{
					mHistory.pop_back();
				}
				mPlaylistWindow->playPreviousSong();
			}
			else
			{
				mHistory.pop_back();
				SoundManager::instance()->playFile(r.file.c_str());
			}
		}
		mPlayPreviousSong = false;
	}
}
bool MusicLibrary::getStopPlaying(bool usePlaylist)
{
	bool stopPlaying = false;
	int trash;
	if (mPlaybackOptionsWindow->isStopAfterOn() && mNextFromCallback)
	{
		if (mPlaybackOptionsWindow->getStopAfter() == PlaybackOptionsWindow::SONG)
		{
			stopPlaying = true;
		}
		else if (mPlaybackOptionsWindow->getStopAfter() == PlaybackOptionsWindow::QUEUE && mPlaylistWindow->getNextSong(false, trash) == NULL)
		{
			stopPlaying = true;
		}
		else if (mLastPlayedId != -1) // can't determine if we should stop if last song is unknown
		{
			Track lastPlayed;
			DatabaseManager::instance()->getTrack(mLastPlayedId, &lastPlayed);
			Track *nextTrack;
			if (usePlaylist || 
				(nextTrack = mPlaylistWindow->getNextSong(mPlaybackOptionsWindow->isRepeatOn(), trash)) == NULL)
			{
				nextTrack = mPlaybackOptionsWindow->getNextSong(trash);
			}
			if (lastPlayed.Id != -1)
			{
				if (mPlaybackOptionsWindow->getStopAfter() == PlaybackOptionsWindow::ALBUM && 
					lastPlayed.AlbumId != nextTrack->AlbumId)
				{
					stopPlaying = true;
				}
				else if (mPlaybackOptionsWindow->getStopAfter() == PlaybackOptionsWindow::ARTIST && 
					(lastPlayed.AlbumId != nextTrack->AlbumId && lastPlayed.Artist != nextTrack->Artist))
				{
					stopPlaying = true;
				}
			}
		}
	}
	return stopPlaying;
}
void MusicLibrary::playSong(string song)
{
	mPlayNextSong = true;
	mNextSong = song;
}
void MusicLibrary::playCurrentSong()
{
	mPlayNextSong = true;
	mNextSong = CURRENT_SONG;
}

void MusicLibrary::playNextSong()
{
	mPlayNextSong = true;
	mNextSong = "";
}
void MusicLibrary::playPreviousSong()
{
	mPlayPreviousSong = true;
	mNextSong = "";
}
void MusicLibrary::setPlaylistWindow(PlaylistWindow* win)
{
	mPlaylistWindow = win;

	vector<ListItem*> items = mPlaylistWindow->getItems();

	for (int i = 0; i < mPlaylistWindow->getHighlightedIndex(); i++)
	{
		mHistory.push_back(HistoryRecord(((TrackListItem*)items[i])->getTrack()->Filename, true));
	}
}

PlaylistWindow* MusicLibrary::getPlaylistWindow()
{
	return mPlaylistWindow;
}
void MusicLibrary::setPlaybackOptionsWindow(PlaybackOptionsWindow* win)
{
	mPlaybackOptionsWindow = win;
}

PlaybackOptionsWindow* MusicLibrary::getPlaybackOptionsWindow()
{
	return mPlaybackOptionsWindow;
}

void MusicLibrary::destroy()
{
	if (mInstance != NULL)
	{
		SoundManager::instance()->removeCallback(mInstance);
		delete mInstance;
		mInstance = NULL;
	}
}

void MusicLibrary::onSoundEvent(SoundManager::SoundEvent ev)
{
	if (ev == SoundManager::SOUND_FINISHED_EVENT)
	{
		// don't do it in the callback or SM gets confused
		mPlayNextSong = true;
		mNextFromCallback = true;
	}
	else if (ev == SoundManager::START_EVENT)
	{
		string f = SoundManager::instance()->getCurrFile();
		mLastPlayedId = -1;
		if (f != "")
		{
			Track t;
			DatabaseManager::instance()->getTrack(f, &t);
			if (t.Id != DatabaseStructs::INVALID_ID)
			{
				mLastPlayedId = t.Id;
			}
			if (!mPlayingPrevious && (mHistory.size() <= 0 || f != mHistory[mHistory.size() - 1].file))
			{
				mHistory.push_back(HistoryRecord(f, mLastPlayedQueue));
				mLastPlayedQueue = false;
			}
			mPlayingPrevious = false;
		}
	}
}

MusicLibrary* MusicLibrary::instance()
{
	if (mInstance == NULL)
	{
		mInstance = snew MusicLibrary();
	}
	return mInstance;
}

