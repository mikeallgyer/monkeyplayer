// AlbumTextureManager.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A class for dynamically loading/unloading texture
#include "d3dUtil.h"

#include <vector>
#include <string>

#include "AlbumTextureManager.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "Logger.h"
#include "MetadataReader.h"

using namespace std;

using namespace MonkeyPlayer;

//#define TEXTURE_MGR_DBG 1

const int AlbumTextureManager::NUM_CACHEABLE_TEXTURES = 30;
const int AlbumTextureManager::MAX_WEIGHT = 5;
AlbumTextureManager* AlbumTextureManager::mInstance = NULL;

// used for synchronization
CCriticalSection AlbumTextureManager::mCritSection;

AlbumTextureManager::AlbumTextureManager()
{
	mCurrIndex = 0;
	mStopping = false;
	mThreadStopped = false;
	string albumCoverFile = FileManager::getContentAsset(std::string("Textures\\UnknownAlbum.jpg"));
	D3DXCreateTextureFromFile(gDevice, albumCoverFile.c_str(), &mDefaultTexture);
	AfxBeginThread(loaderThread, this);
}
AlbumTextureManager::~AlbumTextureManager()
{
	for (unsigned int i = 0; i < mTextures.size(); i++)
	{
		ReleaseCOM(mTextures[i]->mTexture);
		delete mTextures[i];
	}
	mTextures.clear();
}
/*static*/ AlbumTextureManager* AlbumTextureManager::instance()
{
	if (mInstance == NULL)
	{
		mInstance = snew AlbumTextureManager();
	}
	return mInstance;
}
/*static*/ void AlbumTextureManager::destroy()
{
	if (mInstance != NULL)
	{
		mInstance->mStopping = true;
		int i = 6; // max 3 seconds
		while (!mInstance->mThreadStopped && i > 0)
		{
			Sleep(500);
			i--;
		}
		delete mInstance;
		mInstance = NULL;
	}
}
bool AlbumTextureManager::isTextureBad(int albumId)
{
	if (mBadAlbums.find(albumId) != mBadAlbums.end())
	{
		return mBadAlbums[albumId];
	}
	return false;
}

IDirect3DTexture9* AlbumTextureManager::getTexture(int albumId)
{
	bool junk;
	return getTexture(albumId, junk);
}
IDirect3DTexture9* AlbumTextureManager::getTexture(int albumId, bool &usedDefault)
{
	IDirect3DTexture9* tex = mDefaultTexture;
	usedDefault = true;
	if (albumId >= 0)
	{
		CSingleLock lock(&mCritSection);
		lock.Lock();
#ifdef TEXTURE_MGR_DBG
		Logger::instance()->write(string("AlbumTextureManager::getTexture() LOCK"));
#endif
		if (mBadAlbums.find(albumId) == mBadAlbums.end())
		{
			for (unsigned int i = 0; i < mTextures.size(); i++)
			{
				if (mTextures[i]->mAlbumId == albumId)
				{
					tex = mTextures[i]->mTexture;
					mTextures[i]->mWeight= min(MAX_WEIGHT, mTextures[i]->mWeight+ 1);
					usedDefault = false;
					break;
				}
			}
			if (tex == mDefaultTexture)
			{
				mTexturesToLoad.insert(albumId);
			}
		}
#ifdef TEXTURE_MGR_DBG
		Logger::instance()->write(string("AlbumTextureManager::getTexture() UNLOCK"));
#endif
		lock.Unlock();
	}
	return tex;
}
void AlbumTextureManager::update(float dt)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
#ifdef TEXTURE_MGR_DBG
		Logger::instance()->write(string("AlbumTextureManager::Update() LOCK"));
#endif

	for (unsigned int i = 0; i < mTextures.size(); i++)
	{
		mTextures[i]->mWeight= max(0, mTextures[i]->mWeight- 1);
	}

#ifdef TEXTURE_MGR_DBG
		Logger::instance()->write(string("AlbumTextureManager::Update() UNLOCK"));
#endif
	lock.Unlock();

}
int AlbumTextureManager::getIndexToReplace()
{
	int index = mCurrIndex;
	int minWeight = MAX_WEIGHT;
	for (unsigned int i = 0; i < mTextures.size(); i++)
	{
		int index = (i + mCurrIndex) % mTextures.size();
		if (mTextures[index]->mWeight < minWeight)
		{
			index = index;
			minWeight = mTextures[index]->mWeight;
		}
	}
	return index;
}

/*static*/ UINT AlbumTextureManager::loaderThread(LPVOID pParam)
{
	AlbumTextureManager* mgr = (AlbumTextureManager*)pParam;
	do
	{
		while (mgr->mTexturesToLoad.size() > 0)
		{
			CSingleLock lock(&mCritSection);
			lock.Lock();
#ifdef TEXTURE_MGR_DBG
			Logger::instance()->write(string("AlbumTextureManager::loaderThread() LOCK"));
#endif
			set<int>::iterator iter = mgr->mTexturesToLoad.begin();
			IDirect3DTexture9* tex = loadTexture(*iter);
			if (tex != NULL)
			{
				if (mgr->mTextures.size() < NUM_CACHEABLE_TEXTURES)
				{
					AlbumTextureManager::AlbumTexture* a = snew AlbumTextureManager::AlbumTexture();
					a->mAlbumId = (*iter);
					a->mWeight= 0;
					a->mTexture = tex;
					mgr->mTextures.push_back(a);
					mgr->mCurrIndex = mgr->mTextures.size() - 1;
				}
				else
				{
					int index = mgr->getIndexToReplace();
					mgr->mCurrIndex = index + 2;

					if (mgr->mCurrIndex >= mgr->mTextures.size()) 
					{
						mgr->mCurrIndex = 1;
					}
					ReleaseCOM(mgr->mTextures[index]->mTexture);
					mgr->mTextures[index]->mAlbumId = (*iter);
					mgr->mTextures[index]->mWeight= MAX_WEIGHT;
					mgr->mTextures[index]->mTexture = tex;
				}
			}
			else
			{
				mgr->mBadAlbums[*iter] = true;
			}
			mgr->mTexturesToLoad.erase(iter);
			//mgr->mTexturesToLoad.clear();
#ifdef TEXTURE_MGR_DBG
		Logger::instance()->write(string("AlbumTextureManager::loaderThread() UNLOCK"));
#endif
			lock.Unlock();
			Sleep(100);
		}
		Sleep(1000);

	} while(!mgr->mStopping);

	mgr->mThreadStopped = false;

	return 0;
}

/*static*/ IDirect3DTexture9* AlbumTextureManager::loadTexture(int albumId)
{
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(albumId);

	IDirect3DTexture9* tex = NULL;
	AlbumArt *art = NULL;
	bool found = false;
	for (unsigned int i = 0; i < tracks.size(); i++)
	{
		if (!found) 
		{
			art = MetadataReader::getAlbumArt(tracks[i]->Filename.c_str());
			if (art != NULL)
			{
				found = true;
			}
		}
		delete tracks[i];
	}
	if (art != NULL)
	{
		HRESULT hr = D3DXCreateTextureFromFileInMemory(gDevice, art->data, art->length, &tex);
		if (hr != D3D_OK)
		{
			ReleaseCOM(tex);
			tex = NULL;
		}
		delete art;
	}
	return tex;
}
