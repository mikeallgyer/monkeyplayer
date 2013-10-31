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

MusicLibrary::MusicLibrary()
{
	mPlaylistWindow = NULL;

	SoundManager::instance()->addCallback(soundEventCB, this);
	mPlayNextSong = false;
	mPlayPreviousSong = false;
	gInput->registerGlobalKey(VK_MEDIA_PLAY_PAUSE);
	gInput->registerGlobalKey(VK_MEDIA_STOP);
	gInput->registerGlobalKey(VK_MEDIA_NEXT_TRACK);
	gInput->registerGlobalKey(VK_MEDIA_PREV_TRACK);
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
	if (gInput->keyPressed(VK_MEDIA_NEXT_TRACK))
	{
		mPlayNextSong = true;
		mNextSong = "";
	}
	if (gInput->keyPressed(VK_MEDIA_PREV_TRACK))
	{
		mPlayPreviousSong = true;
		mNextSong = "";
	}

	if (mPlayNextSong)
	{
		if (mNextSong != "")
		{
			SoundManager::instance()->playFile(mNextSong.c_str());
		}
		else 
		{
			mPlaylistWindow->playNextSong();
		}
		mPlayNextSong = false;
		mNextSong = "";
	}
	else if (mPlayPreviousSong)
	{
		mPlaylistWindow->playPreviousSong();
		mPlayPreviousSong = false;

/*		if (mNextSongPath == "")
		{
			std::string currFile = SoundManager::instance()->getCurrFile();
			int currTrackIndex = mListBox->findItem(currFile);
			if (currTrackIndex > -1)
			{
				currTrackIndex++;
				ListItem* nextItem = mListBox->setHighlightedItem(currTrackIndex);
				if (nextItem != NULL)
				{
					SoundManager::instance()->playFile(
						((TrackListItem*)nextItem)->getTrack()->Filename.c_str());
				}
			}
		}
		else 
		{
			int currTrackIndex = mListBox->findItem(mNextSongPath);
			if (currTrackIndex > -1)
			{
				ListItem* nextItem = mListBox->setHighlightedItem(currTrackIndex);
				if (nextItem != NULL)
				{
					SoundManager::instance()->playFile(
						((TrackListItem*)nextItem)->getTrack()->Filename.c_str());
				}
			}
			mNextSongPath = "";
		}
	*/
	}
}
void MusicLibrary::playSong(string song)
{
	mPlayNextSong = true;
	mNextSong = song;
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
}

PlaylistWindow* MusicLibrary::getPlaylistWindow()
{
	return mPlaylistWindow;
}

void MusicLibrary::destroy()
{
	if (mInstance == NULL)
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
	}
}

MusicLibrary* MusicLibrary::instance()
{
	if (mInstance == NULL)
	{
		mInstance = new MusicLibrary();
	}
	return mInstance;
}