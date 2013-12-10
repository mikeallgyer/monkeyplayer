// AlbumTextureManager.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A class for dynamically loading/unloading texture
#include "d3dUtil.h"

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

#ifndef ALBUM_TEXTURE_MANAGER_H
#define ALBUM_TEXTURE_MANAGER_H

namespace MonkeyPlayer
{
	class AlbumTextureManager
	{
	public:
		static AlbumTextureManager* instance();
		static void destroy();

		~AlbumTextureManager();
		IDirect3DTexture9* getTexture(int albumId);
		IDirect3DTexture9* getTexture(int albumId, bool &usedDefault);
		bool isTextureBad(int albumId);
		void update(float dt);

	private:
		struct AlbumTexture
		{
			IDirect3DTexture9* mTexture;
			int mTrackId;
			int mAlbumId;
			int mWeight; // higher means it should stick around

			AlbumTexture() : mTexture(NULL), mWeight(0), mTrackId(-1), mAlbumId(-1) {}
			~AlbumTexture() 
			{
				if (mTexture) ReleaseCOM(mTexture);
			}
		};

		static const int NUM_CACHEABLE_TEXTURES;
		static const int MAX_WEIGHT;
		static AlbumTextureManager* mInstance;

		vector<AlbumTexture*> mTextures;
		map<int, bool> mBadAlbums; // can't load textures for these
		set<int> mTexturesToLoad; // album id
		unsigned int mCurrIndex; // forms a circular array

		bool mStopping;
		bool mThreadStopped;

		IDirect3DTexture9* mDefaultTexture;
		AlbumTextureManager();
		int getIndexToReplace();
	protected:
		// synchronization
		static CCriticalSection mCritSection;

		static UINT loaderThread(LPVOID pParam);
		static IDirect3DTexture9* loadTexture(int albumId);
	};
}
#endif