// LargeAlbumItem.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// large album
#include <string>
#include <vector>
#include <queue>

#include "DatabaseStructs.h"
#include "IDrawable.h"

#ifndef LARGE_ALBUM_ITEM_H
#define LARGE_ALBUM_ITEM_H

namespace MonkeyPlayer
{
	struct AlbumPos
	{
		AlbumPos() : visible(true), xPos(0), yRotation(0), time(.5f) {}
		AlbumPos(bool vis, float x, float yRot, float t) : visible(vis), xPos(x), yRotation(yRot), time(t) {}
		bool visible;
		float xPos;
		float yRotation;
		float time;
	};

	class LargeAlbumItem : public IDrawable
	{
	public:
		LargeAlbumItem(Album album);
		~LargeAlbumItem();
		void onDeviceLost();
		void onDeviceReset();

		void update(float dt);

		void preRender();
		void display();
		int getNumTriangles() { return 0; }

		void setVisible(bool vis);
		bool getVisible();
		Album getAlbum();
		vector<Track*> getTracks();
		D3DXMATRIX getWorld();
		bool isMoving();
		void setPosition(float x, float yRot);
		void setDest(float x, float yRot, bool vis, float time);
		void addDest(float x, float yRot, bool vis, float time);
		IDirect3DTexture9* getTexture();
		void setTracksDirty();

	private:

		void calculateMatrix(float x, float yRot);

		Album mAlbum;
		vector<Track*> mTracks;

		D3DXMATRIX mWorld;
		float mX, mYRot;
		float mStartX, mStartYRot;
		float mCurrTime;
		bool mVisible;
		bool mMoving;
		bool mTracksDirty;
		queue<AlbumPos> mPositions;
		AlbumPos mCurrDestination;

		IDirect3DTexture9* mTexture;
	};
}
#endif