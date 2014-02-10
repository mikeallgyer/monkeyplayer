// SmallAlbumItem.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays a single album and its tracks

#include <sstream>

#include "AlbumTextureManager.h"
#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "SmallAlbumItem.h"
#include "Settings.h"
#include "SoundManager.h"
#include "WindowManager.h"

using namespace MonkeyPlayer;

// used for synchronization
CCriticalSection SmallAlbumItem::mCritSection;

SmallAlbumItem::SmallAlbumItem(float x, float y, Album album, vector<Track*> tracks)
{
	mAlbumDimension = 150;

	mAlbumTitleX = mAlbumDimension + 50;
	mAlbumTitleY = SmallAlbumTile::TEXT_MARGIN_TOP;
	mTrackTitleX = mAlbumTitleX;
	mTrackTitleY = 30 + SmallAlbumTile::TEXT_MARGIN_TOP;

	mTrackTitleWidth = 300  + SmallAlbumTile::TEXT_MARGIN_TOP;;
	mTrackTimeWidth = 70;
	mTrackRatingWidth = 50;

	mX = floor(x);
	mY = floor(y);

	setAlbum(album, tracks);
	mCurrSelection = -1;
	mAlbumSelected = false;

}
SmallAlbumItem::~SmallAlbumItem()
{
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		delete mTracks[i];
	}
}

void SmallAlbumItem::selectNext()
{
	mCurrSelection = min((int)mTracks.size() - 1, mCurrSelection + 1);
}
void SmallAlbumItem::selectPrevious()
{
	mCurrSelection = max(0, mCurrSelection - 1);
}
void SmallAlbumItem::selectNone()
{
	mCurrSelection = -1;
	mAlbumSelected = false;
}
void SmallAlbumItem::selectFirst()
{
	mCurrSelection = 0;
}
void SmallAlbumItem::selectLast()
{
	mCurrSelection = mTracks.size() - 1;
}
bool SmallAlbumItem::isFirstSelected()
{
	return mCurrSelection == 0;
}
bool SmallAlbumItem::isLastSelected()
{
	return mCurrSelection == (mTracks.size() - 1);
}
int SmallAlbumItem::getSelectedIndex()
{
	return mCurrSelection;
}
void SmallAlbumItem::getSelectionTopLeft(int &x, int &y)
{
	x = mTrackTitleX;
	y = mTrackTitleY;

	if (mCurrSelection > -1)
	{
		y += SmallAlbumTile::TRACK_FONT_HEIGHT * mCurrSelection;
	}
}
void SmallAlbumItem::getSelectionBottomLeft(int &x, int &y)
{
	x = mTrackTitleX;
	y = mTrackTitleY;

	if (mCurrSelection > -1)
	{
		y += SmallAlbumTile::TRACK_FONT_HEIGHT * (mCurrSelection + 1);
	}
}

void SmallAlbumItem::setPos(float x, float y)
{
	mX = floor(x);
	mY = floor(y);

}

bool SmallAlbumItem::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x2 = mX + mWidth;
	float y2 = mY + mHeight;

	return !(xPoint < mX || yPoint < mY || xPoint > x2 || yPoint > y2);
}

int SmallAlbumItem::getItemAtPos(int x, int y)
{
	int itemIndex = -1;
	int relX = x - (int)mX;
	int relY = y - (int)mY;


	if (relX >= mTrackTitleX && relY >= mTrackTitleY)
	{
		int selTrack = (relY - mTrackTitleY) / SmallAlbumTile::TRACK_FONT_HEIGHT;
		if (selTrack < (int)mTracks.size())
		{
			itemIndex = selTrack;
		}
	}
	return itemIndex;
}

void SmallAlbumItem::setAlbum(Album album)
{
	setAlbum(album, DatabaseManager::instance()->getTracks(mAlbum));
}
void SmallAlbumItem::setAlbum(Album album, vector<Track*> tracks)
{
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		delete mTracks[i];
	}
	mGenreString = "";
	mYearString = "";
	mTracks.clear();
	mAlbum = album;
	mTracks = tracks;
	getAlbumInfo();
}

void SmallAlbumItem::getAlbumInfo()
{
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
		IDirect3DTexture9* tex;
		HRESULT hr = D3DXCreateTextureFromFileInMemory(gDevice, art->data, art->length, &tex);
		if (hr != D3D_OK)
		{
			mAlbumCoverSprite->setTextureIndex(0);
		}
		else
		{
			mAlbumCoverSprite->addTexture(1, tex, true, true);
		}
		delete art;
	}
	else
	{
		mAlbumCoverSprite->setTextureIndex(0);
	}
*/
	// get year and genre
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		if (mGenreString == "" && mTracks[i]->Genre != DatabaseStructs::INVALID_ID)
		{
			Genre g;
			DatabaseManager::instance()->getGenre(mTracks[i]->Genre, &g);
			if (g.Id != DatabaseStructs::INVALID_ID)
			{
				mGenreString = g.Title;
			}
		}
	}
	if (mAlbum.Year != DatabaseStructs::DEF_EMPTY_YEAR)
	{
		std::stringstream sstr;
		sstr << mAlbum.Year;
		mYearString = sstr.str();
	}
	recalculateSize();
}
void SmallAlbumItem::recalculateSize()
{
	mWidth = 600.0f;
	mHeight = (float)max((float)mAlbumDimension, mTrackTitleY + (float)mTracks.size() * SmallAlbumTile::TRACK_FONT_HEIGHT);

}
Track* SmallAlbumItem::getSelectedTrack()
{
	if (mCurrSelection >= 0 && mCurrSelection < (int)mTracks.size())
	{
		return mTracks[mCurrSelection];
	}
	return NULL;
}
bool SmallAlbumItem::getAlbumSelected()
{
	return mAlbumSelected;
}
void SmallAlbumItem::setAlbumSelected(bool sel)
{
	mAlbumSelected = sel;
}
void SmallAlbumItem::addTrack(Track *track)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();

	int insertIndex = 0;
	vector<Track*>::iterator iter = mTracks.begin();
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		if (mTracks[i]->Id == track->Id)
		{
			insertIndex = -1;
			break;
		}
		else if (mTracks[i]->TrackNumber > track->TrackNumber)
		{
			insertIndex = i;
			break;
		}
		iter++;
	}

	if (insertIndex >= 0)
	{
		mTracks.insert(iter, snew Track(*track));
		if (mTracks.size() == 1)
		{
			getAlbumInfo();
		}
		else
		{
			recalculateSize();
		}
	}


	lock.Unlock();
}