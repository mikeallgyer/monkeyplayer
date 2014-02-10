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

#ifndef SMALL_ALBUM_TILE_H
#define SMALL_ALBUM_TILE_H

namespace MonkeyPlayer
{
	class SmallAlbumItem;

	class SmallAlbumTile : public IWidget
	{
	public:
		static int TEXT_MARGIN_LEFT;
		static int TEXT_MARGIN_RIGHT;
		static int TEXT_MARGIN_TOP;
		static int TEXT_MARGIN_BOTTOM;
		static float SELECTION_BOX_MARGIN;
		static float TRACK_TIME_START;
		static float MIN_HEIGHT;
		static float MAX_HEIGHT;
		static float STD_WIDTH;
		static const int TRACK_FONT_HEIGHT;

		SmallAlbumTile();
		SmallAlbumTile(float x, float y,
			void (*selectedtItemCB)(void* ptrObj, Track* selItem) = NULL, void* callbackObj = NULL);
		void init(float x, float y,
			void (*selectedtItemCB)(void* ptrObj, Track* selItem) = NULL, void* callbackObj = NULL);
		~SmallAlbumTile();

		void onDeviceLost();
		void onDeviceReset();
		void recreateTargets();

		void update(float dt);

		virtual void preRender();
		void display() {}
		void setPos(float x, float y);
		float getHeight() { return mAlbumItem == NULL ? MIN_HEIGHT : mAlbumItem->getHeight(); }

		void selectNext();
		void selectPrevious();
		void selectNone();
		void selectFirst();
		void selectLast();
		void setSelectedIndex(int index);
		bool isFirstSelected();
		bool isLastSelected();
		int getSelectedIndex();
		void getSelectionTopLeft(int &x, int &y);
		void getSelectionBottomLeft(int &x, int &y);

		std::vector<Sprite*> getSprites();
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent e);

		void setAlbumItem(SmallAlbumItem* albumItem);
		SmallAlbumItem *getAlbumItem();

		Album getAlbum() { return mAlbumItem == NULL ? mEmptyAlbum : mAlbumItem->getAlbum(); }
		vector<Track*> getTracks() { return mAlbumItem == NULL ? mEmptyTracks : mAlbumItem->getTracks(); }
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

		float mX, mY, mWidth;
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
	
		// callback
		void (*mCallback)(void* ptrObj, Track* selItem);
		void *mCallbackObj;

		SmallAlbumItem* mAlbumItem;

		std::string mGenreString;
		std::string mYearString;

		bool mRedraw;
		bool mTryTexture;
		bool mTextureFailed;

		bool mAlbumSelected;
	protected:

		// synchronization
		static CCriticalSection mCritSection;

		void recalculateSize();
		static Album mEmptyAlbum;
		static vector<Track*> mEmptyTracks;
	};
}
#endif