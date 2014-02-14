// MusicLoader.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Handles loading of audio files

#include <string>
#include <vector>
#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "Logger.h"
#include "MetadataReader.h"
#include "MusicLoader.h"
#include "ProgressBar.h"

using namespace MonkeyPlayer;

MusicLoader* MusicLoader::mInstance = NULL;
bool MusicLoader::mTerminating = false;

MusicLoader::MusicLoader()
{
	mTerminating = false;
	mPlaylistWindow = NULL;
	mCollectionWindow = NULL;
}
MusicLoader::~MusicLoader()
{
	for (unsigned int i = 0; i < mThreads.size(); i++)
	{
		if (!mThreads[i]->mFinished)
		{
			mThreads[i]->mTerminateRequested = true;
			do
			{
				Sleep(500);
			} while (!mThreads[i]->mFinished);
		}
		delete mThreads[i];
	}
}

void MusicLoader::terminate()
{
	if (mInstance != NULL)
	{
		mTerminating = true;
		delete mInstance;
		mInstance = NULL;
	}
}

void MusicLoader::loadDirectory(std::string dirPath)
{
	std::vector<std::string> files;
	int count = FileManager::getAllFiles(files, dirPath, 
		Settings::instance()->getStringValue(Settings::OPT_AUDIO_TYPES, FileManager::FILE_TYPE_MUSIC));

	MusicLoaderThreadObj* obj = snew MusicLoaderThreadObj(files, mPlaylistWindow, mCollectionWindow);
	mThreads.push_back(obj);
	AfxBeginThread(loaderThread, obj);
}
void MusicLoader::loadFiles(std::vector<std::string> filenames)
{
	MusicLoaderThreadObj* obj = snew MusicLoaderThreadObj(filenames, mPlaylistWindow, mCollectionWindow);
	mThreads.push_back(obj);
	AfxBeginThread(loaderThread, obj);
}

void MusicLoader::setPlaylistWindow(PlaylistWindow *win)
{
	mPlaylistWindow = win;
}

void MusicLoader::setCollectionWindow(CollectionWindow *win)
{
	mCollectionWindow = win;
}

MusicLoader* MusicLoader::instance()
{
	if (mInstance == NULL)
	{
		mInstance = snew MusicLoader();
	}
	return mInstance;
}
UINT MusicLoader::loaderThread(LPVOID pParam)
{
	MusicLoaderThreadObj* loader = (MusicLoaderThreadObj*)pParam;
	if (loader->mPlaylistWindow != NULL)
	{
		char buf[35];
		sprintf_s(buf, 35, "Begin loading %i files.", loader->mFiles.size());
		Logger::instance()->write(buf);
		// get filenames and music info already stored in db
		std::map<std::string, Track*> existing = DatabaseManager::instance()->getAllTracks();

		gWindowMgr->getProgressBar()->setVisible(true);
		gWindowMgr->getProgressBar()->setMaxValue((float)(loader->mFiles.size()) * 2.0f);
		gWindowMgr->getProgressBar()->setCurrentValue(0);

		std::vector<TrackExtended*> tracks;
		std::vector<bool> trackIsNew;
		DatabaseManager::instance()->beginTransaction();
		for (unsigned int i = 0; i < loader->mFiles.size() && !loader->mTerminateRequested; i++)
		{
			Track* currTrack = snew Track(-1, loader->mFiles[i], "", "", "", -1, -1, -1, -1, false, -1, 0, 0);
			
			Album* currAlbum = snew Album();
			Genre* currGenre = snew Genre();
			MetadataReader::getTrackInfo(loader->mFiles[i].c_str(), currTrack, currAlbum, currGenre);
			DatabaseManager::instance()->addAlbum(*currAlbum);
			DatabaseManager::instance()->addGenre(*currGenre);

			trackIsNew.push_back(false);

			TrackExtended* trackE = snew TrackExtended();
			trackE->t = currTrack;
			trackE->a = currAlbum;
			trackE->g = currGenre;
			tracks.push_back(trackE);

			gWindowMgr->getProgressBar()->setCurrentValue(gWindowMgr->getProgressBar()->getCurrentValue() + 1.0f);
			
		}

		DatabaseManager::instance()->endTransaction();

		vector<Album*> allAlbums = DatabaseManager::instance()->getAllAlbums();
		vector<Genre*> allGenres = DatabaseManager::instance()->getAllGenres();
		DatabaseManager::instance()->beginTransaction();

		for (unsigned int i = 0; i < tracks.size() && !loader->mTerminateRequested; i++)
		{
			// if we wait til the end it becomes unresponsive
			if (i > 0 && 0 == i % 100)
			{
				DatabaseManager::instance()->endTransaction();
				DatabaseManager::instance()->beginTransaction();
			}		
			for (unsigned int j = 0; j < allAlbums.size(); j++)
			{
				if (allAlbums[j]->Year == tracks[i]->a->Year && allAlbums[j]->Title == tracks[i]->a->Title)
				{
					tracks[i]->t->AlbumId = allAlbums[j]->Id;
					break;
				}
			}
			for (unsigned int j = 0; j < allGenres.size(); j++)
			{
				if (allGenres[j]->Title == tracks[i]->g->Title)
				{
					tracks[i]->t->Genre = allGenres[j]->Id;
					break;
				}
			}

			DatabaseManager::instance()->addTrack(*tracks[i]->t);
			gWindowMgr->getProgressBar()->setCurrentValue(gWindowMgr->getProgressBar()->getCurrentValue() + 1.0f);
		}
		DatabaseManager::instance()->endTransaction();
		for (unsigned int i = 0; i < tracks.size() && !loader->mTerminateRequested; i++)
		{
			delete tracks[i]->t;
			delete tracks[i];
		}
			loader->mCollectionWindow->reloadAll();
		for (unsigned int j = 0; j < allAlbums.size(); j++)
		{
			//loader->mCollectionWindow->addAlbum(allAlbums[j]);
		}
		for (unsigned int j = 0; j < allAlbums.size(); j++)
		{
			delete allAlbums[j];
		}
		for (unsigned int j = 0; j < allGenres.size(); j++)
		{
			delete allGenres[j];
		}

		gWindowMgr->getProgressBar()->setVisible(false);
		sprintf_s(buf, 35, "Finished loading %i files.", tracks.size());
		Logger::instance()->write(buf);
	}

	loader->mFinished = true;
	return 0;
}
