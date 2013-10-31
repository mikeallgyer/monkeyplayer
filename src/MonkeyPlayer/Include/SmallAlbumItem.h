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
	class SmallAlbumItem : public IWidget
	{
	public:
		static const int TRACK_FONT_HEIGHT;

		SmallAlbumItem(float x, float y, Album album,
			void (*selectedtItemCB)(void* ptrObj, Track* selItem) = NULL, void* callbackObj = NULL);
		~SmallAlbumItem();

		void onDeviceLost();
		void onDeviceReset();
		void recreateTargets();

		void update(float dt);

		virtual void preRender();
		void display() {}
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

		std::vector<Sprite*> getSprites();
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent e);

		Album getAlbum() { return mAlbum; }
		std::vector<Track*> getTracks() { return mTracks; }
		Track* getSelectedTrack();
		bool getAlbumSelected();
		void setAlbumSelected(bool sel);
		void addTrack(Track* track);

		bool isPointInside(int x, int y);
		bool isPointInsideAlbum(int x, int y);
		int getItemAtPos(int x, int y);

	protected:
		std::vector<Sprite*> mSprites; 

		static ID3DXFont* mArtistFont;
		static ID3DXFont* mAlbumFont;
		static ID3DXFont* mTrackFont;
		static bool mFontLost;
		static bool mFontReset;
		string mAlbumCoverFile;

		float mX, mY, mWidth, mHeight;
		std::vector<int> mSelectedIndices;
		int mCurrSelection;

		// main list area
		RenderTarget* mTarget;
		Sprite* mTargetSprite;
		Sprite* mHighlightedSprite;
		Sprite* mAlbumCoverSprite;
		Sprite* mSelectionSprite;
		Sprite* mCDSelectionSprite;

		int mAlbumTitleX, mAlbumTitleY; 
		int mTrackTitleX, mTrackTitleY; 
		
		int mTrackTitleWidth;
		int mTrackTimeWidth;
		int mTrackRatingWidth;

		int mAlbumDimension;

		void setAlbum(Album album);
			
		// callback
		void (*mCallback)(void* ptrObj, Track* selItem);
		void *mCallbackObj;

		Album mAlbum;
		std::vector<Track*> mTracks;
		std::string mGenreString;
		std::string mYearString;

		bool mRedraw;

		bool mAlbumSelected;
	protected:
		static int TEXT_MARGIN_LEFT;
		static int TEXT_MARGIN_RIGHT;
		static int TEXT_MARGIN_TOP;
		static int TEXT_MARGIN_BOTTOM;
		static float SELECTION_BOX_MARGIN;
		static float TRACK_TIME_START;
		// synchronization
		static CCriticalSection mCritSection;

		void getAlbumInfo();
		void recalculateSize();
	};
}
#endif