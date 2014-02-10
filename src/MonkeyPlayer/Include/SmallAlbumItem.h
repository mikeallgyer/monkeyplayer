// SmallAlbumItem.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays a single album and its tracks
#include <map>
#include <vector>

#include "DatabaseStructs.h"
#include "IWidget.h"
#include "RenderTarget.h"
#include "Sprite.h"

#ifndef SMALL_ALBUM_ITEM_H
#define SMALL_ALBUM_ITEM_H

namespace MonkeyPlayer
{
	class SmallAlbumItem
	{
	public:
		SmallAlbumItem(float x, float y, Album album, vector<Track*> tracks);
		~SmallAlbumItem();

		void setPos(float x, float y);
		float getHeight() { return mHeight; }

		void selectNext();
		void selectPrevious();
		void selectNone();
		void selectFirst();
		void selectLast();
		bool isFirstSelected();
		bool isLastSelected();
		int getSelectedIndex();
		void getSelectionTopLeft(int &x, int &y);
		void getSelectionBottomLeft(int &x, int &y);

		Album getAlbum() { return mAlbum; }
		std::vector<Track*> getTracks() { return mTracks; }
		Track* getSelectedTrack();
		bool getAlbumSelected();
		void setAlbumSelected(bool sel);
		void addTrack(Track* track);

		bool isPointInside(int x, int y);
		int getItemAtPos(int x, int y);

		void getAlbumInfo();
		void recalculateSize();
	protected:
		static bool mFontLost;
		static bool mFontReset;
		string mAlbumCoverFile;

		float mX, mY, mWidth, mHeight;
		std::vector<int> mSelectedIndices;
		int mCurrSelection;

		int mAlbumTitleX, mAlbumTitleY; 
		int mTrackTitleX, mTrackTitleY; 
		
		int mTrackTitleWidth;
		int mTrackTimeWidth;
		int mTrackRatingWidth;

		int mAlbumDimension;

		void setAlbum(Album album);
		void setAlbum(Album album, vector<Track*> tracks);

		Album mAlbum;
		std::vector<Track*> mTracks;
		std::string mGenreString;
		std::string mYearString;

		bool mAlbumSelected;
	protected:
		// synchronization
		static CCriticalSection mCritSection;

	};
}
#endif