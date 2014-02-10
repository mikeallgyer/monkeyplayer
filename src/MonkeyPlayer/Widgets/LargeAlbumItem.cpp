// LargeAlbumItem.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// large album
#include <string>
#include <vector>

#include "AlbumTextureManager.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "LargeAlbumItem.h"
#include "MetadataReader.h"

using namespace MonkeyPlayer;

LargeAlbumItem::LargeAlbumItem(Album album) : mCurrDestination(0, 0, true, .5f)
{
	mAlbum = album;
	mX = 0;
	mYRot = 0;
	mStartX = 0;
	mStartYRot = 0;
	mCurrTime = 0;
	mVisible = false;
	calculateMatrix(mStartX, mStartYRot);
	mMoving = false;
	
	mTracks = DatabaseManager::instance()->getTracks(mAlbum);
	mTracksDirty = false;
	// get cover art 
/*	AlbumArt *art = NULL;
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		art = MetadataReader::getAlbumArt(mTracks[i]->Filename.c_str());
		if (art != NULL)
		{
			break;
		}
	}
	if (art != NULL)
	{
		HRESULT hr = D3DXCreateTextureFromFileInMemory(gDevice, art->data, art->length, &mTexture);
		delete art;
	}
	else
	{
		string albumCoverFile = FileManager::getContentAsset(std::string("Textures\\UnknownAlbum.jpg"));
		D3DXCreateTextureFromFile(gDevice, albumCoverFile.c_str(), &mTexture);
	}
	*/
}
LargeAlbumItem::LargeAlbumItem(Album album, vector<Track*> tracks) : mCurrDestination(0, 0, true, .5f)
{
	mAlbum = album;
	mX = 0;
	mYRot = 0;
	mStartX = 0;
	mStartYRot = 0;
	mCurrTime = 0;
	mVisible = false;
	calculateMatrix(mStartX, mStartYRot);
	mMoving = false;
	
	mTracks = tracks;
	mTracksDirty = false;
	// get cover art 
/*	AlbumArt *art = NULL;
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		art = MetadataReader::getAlbumArt(mTracks[i]->Filename.c_str());
		if (art != NULL)
		{
			break;
		}
	}
	if (art != NULL)
	{
		HRESULT hr = D3DXCreateTextureFromFileInMemory(gDevice, art->data, art->length, &mTexture);
		delete art;
	}
	else
	{
		string albumCoverFile = FileManager::getContentAsset(std::string("Textures\\UnknownAlbum.jpg"));
		D3DXCreateTextureFromFile(gDevice, albumCoverFile.c_str(), &mTexture);
	}
	*/
}
LargeAlbumItem::~LargeAlbumItem()
{
//	ReleaseCOM(mTexture);
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		delete mTracks[i];
	}
}
void LargeAlbumItem::onDeviceLost()
{
}
void LargeAlbumItem::onDeviceReset()
{
}

void LargeAlbumItem::update(float dt)
{
	// handled externally now
	if (false && isMoving())
	{
		mCurrTime += dt;
		if (mCurrTime > mCurrDestination.time)
		{
			mCurrTime = mCurrDestination.time;
			float percent = mCurrTime / mCurrDestination.time;
			mX = mStartX + (mCurrDestination.xPos - mStartX) * percent;
			mYRot = mStartYRot + (mCurrDestination.yRotation - mStartYRot) * percent;
			calculateMatrix(mX, mYRot);
			mMoving = false;

			if (mPositions.size() > 0)
			{
				AlbumPos p = mPositions.front();
				mPositions.pop();
				setDest(p.xPos, p.yRotation, p.visible, p.time);
				mCurrTime = 0;
				mMoving = true;
			}
			else if (!mCurrDestination.visible)
			{
//				mVisible = false;
			}
		}
		else 
		{
			float percent = mCurrTime / mCurrDestination.time;
			mX = mStartX + (mCurrDestination.xPos - mStartX) * percent;
			mYRot = mStartYRot + (mCurrDestination.yRotation - mStartYRot) * percent;
			calculateMatrix(mX, mYRot);
		}
	}
}

void LargeAlbumItem::preRender()
{
}
void LargeAlbumItem::display()
{
}
void LargeAlbumItem::calculateMatrix(float x, float yRot)
{
	D3DXMATRIX xMat, yRotMat;

	D3DXMatrixTranslation(&xMat, x, -2.0f, 0);
	D3DXMatrixRotationY(&yRotMat, yRot);

	mWorld = yRotMat * xMat;
}
void LargeAlbumItem::setVisible(bool vis)
{
	mVisible = vis;
}

bool LargeAlbumItem::getVisible()
{
	return mVisible;
}
Album LargeAlbumItem::getAlbum()
{
	return mAlbum;
}
vector<Track*> LargeAlbumItem::getTracks()
{
	if (mTracksDirty)
	{
		for (unsigned int i = 0; i < mTracks.size(); i++)
		{
			delete mTracks[i];
		}
		mTracks = DatabaseManager::instance()->getTracks(mAlbum);
		mTracksDirty = false;
	}
	return mTracks;
}
D3DXMATRIX LargeAlbumItem::getWorld()
{
	return mWorld;
}
bool LargeAlbumItem::isMoving()
{
	return mMoving;
}
void LargeAlbumItem::setPosition(float x, float yRot)
{
	mX = x;
	mYRot = yRot;
	calculateMatrix(mX, mYRot);
}
void LargeAlbumItem::setDest(float x, float yRot, bool vis, float time)
{
	mCurrDestination.xPos = x;
	mCurrDestination.yRotation = yRot;
	mCurrDestination.visible = vis;
	mCurrDestination.time = time;
	if (mCurrDestination.visible)
	{
		mVisible = true;
	}
	mStartX = mX;
	mStartYRot = mYRot;
	mCurrTime = 0;
	mMoving = true;
}
void LargeAlbumItem::addDest(float x, float yRot, bool vis, float time)
{
	if (!mMoving)
	{
		mCurrTime = 0;
		setDest(x, yRot, vis, time);
	}
	else
	{
		mPositions.push(AlbumPos(vis, x, yRot, time));
	}
}

IDirect3DTexture9* LargeAlbumItem::getTexture()
{
	return AlbumTextureManager::instance()->getTexture(mAlbum.Id);
}

void LargeAlbumItem::setTracksDirty()
{
	mTracksDirty = true;
};
