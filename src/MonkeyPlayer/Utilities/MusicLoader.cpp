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
	int count = FileManager::getAllFiles(files, dirPath, FileManager::FILE_TYPE_MUSIC);

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
		gWindowMgr->getProgressBar()->setMaxValue((float)(loader->mFiles.size() * 3));
		gWindowMgr->getProgressBar()->setCurrentValue(0);

		std::vector<Track*> tracks;
		std::vector<bool> trackIsNew;
		for (unsigned int i = 0; i < loader->mFiles.size() && !loader->mTerminateRequested; i++)
		{
			Track* currTrack;
			
			// already in db
			if (existing.find(loader->mFiles[i]) != existing.end())
			{
				currTrack = snew Track(*existing[loader->mFiles[i]]);
			}
			else // new file
			{
				currTrack = snew Track(-1, loader->mFiles[i], "", "", -1, -1, -1, -1, false, -1, 0, 0);
//				DatabaseManager::instance()->addTrack(*currTrack); // do this later
			}

			trackIsNew.push_back(currTrack->Length == -1);

			tracks.push_back(currTrack);

			gWindowMgr->getProgressBar()->setCurrentValue(gWindowMgr->getProgressBar()->getCurrentValue() + 1.0f);
			
//			loader->mPlaylistWindow->addItem(tracks[i]);
		}
		for (std::map<std::string, Track*>::iterator i = existing.begin(); i != existing.end(); ++i)
		{
			delete (*i).second;
		}
		existing.clear();

		// get album and genre...THIS is the bottleneck!
		for (unsigned int i = 0; i < tracks.size() && !loader->mTerminateRequested; i++)
		{
			if (trackIsNew[i])
			{
				Album currAlbum;
				Genre currGenre;
				MetadataReader::getTrackInfo(tracks[i]->Filename.c_str(), tracks[i], &currAlbum, &currGenre);
				DatabaseManager::instance()->addAlbum(currAlbum); // this will add or get existing
				DatabaseManager::instance()->addGenre(currGenre); // this will add or get existing
				tracks[i]->AlbumId = currAlbum.Id;
				tracks[i]->Genre = currGenre.Id;

				loader->mCollectionWindow->addAlbum(&currAlbum);
			}
			gWindowMgr->getProgressBar()->setCurrentValue(gWindowMgr->getProgressBar()->getCurrentValue() + 1.0f);
		}
		DatabaseManager::instance()->beginTransaction();

		// now get song info...stop if requested
		int newCount = 0;
		for (unsigned int i = 0; i < tracks.size() && !loader->mTerminateRequested; i++)
		{
			if (trackIsNew[i])
			{
//				loader->mPlaylistWindow->modifyItem(tracks[i]);
				DatabaseManager::instance()->addTrack(*tracks[i]);
				loader->mCollectionWindow->addTrack(tracks[i]);
				newCount++;
			}
			gWindowMgr->getProgressBar()->setCurrentValue(gWindowMgr->getProgressBar()->getCurrentValue() + 1.0f);
		}
		gWindowMgr->getProgressBar()->setVisible(false);
		sprintf_s(buf, 35, "Finished loading %i files.", newCount);
		Logger::instance()->write(buf);
		DatabaseManager::instance()->endTransaction();
	}

	loader->mFinished = true;
	return 0;
}
