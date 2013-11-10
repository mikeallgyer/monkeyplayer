// LargeAlbumWidget.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Window of large albums
#include <string>
#include <vector>

#include "IWidget.h"
#include "Label.h"
#include "LargeAlbumItem.h"
#include "RenderTarget.h"
#include "Sprite.h"
#include "TrackListBox.h"

#ifndef LARGE_ALBUM_WIDGET_H
#define LARGE_ALBUM_WIDGET_H

#include "CollectionWindow.h"

namespace MonkeyPlayer
{
	class LargeAlbumWidget : public IWidget
	{
	public:
		static const float ALBUM_WIDTH;
		static const float ALBUM_HEIGHT;
		static const float ALBUM_DEPTH;
		
		static const float SLOW_SCROLL_SPEED;
		static const float FAST_SCROLL_SPEED;
		static const float REALLY_FAST_SCROLL_SPEED;
		static const float FAST_SCROLL_DELAY;

		LargeAlbumWidget(float x, float y, float width, float height);
		~LargeAlbumWidget();

		void onDeviceLost();
		void onDeviceReset();
		void recreateTargets();
		void update(float dt);

		virtual void preRender();
		
		void display() {}
		void setPos(float x, float y, float width = -999.0f, float height = -999.0f);
		float getX() { return mX; }
		float getY() { return mY; }
		float getWidth() { return mWidth; }
		float getHeight() { return mHeight; }

		std::vector<Sprite*> getSprites();
		std::vector<IWidget*> getWidgets();
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent ev);
		void focus();
		void blur();
		virtual void refresh();
		void goToChar(char c);
		void goToSong(Album a, Track t, bool doHighlight = true);
		int getCurrentAlbum();

		void onContextMenuSelected(ItemListBox* menu);

		void addAlbum(Album* album);
		void addTrack(Track* track);

		static void listBox_callback(void* obj, ItemListBox* listBox)
		{
			LargeAlbumWidget* win = static_cast<LargeAlbumWidget*>(obj);
			if (win)
			{
				win->onItemSelected(listBox->getSelectedItem(), ((TrackListBox*)listBox)->getSelectedIndex());
			}
		}
		static void btn_callback(void* obj, Button* btn)
		{
			LargeAlbumWidget* win = static_cast<LargeAlbumWidget*>(obj);
			if (win)
			{
				win->onBtnClicked(btn);
			}
		}

	protected:
		void goToAlbum(int index);
		void queueThing(CollectionWindow::SELECTED_THING thing);

		void doAddAlbum(Album* album);
		void doAddTrack(Track* track);

		vector<AlbumPos> mPositions;
		std::vector<Sprite*> mSprites; 
		vector<IWidget*> mWidgets;
		Sprite* mTargetSprite;
		TrackListBox* mTrackBox;
		SimpleLabel* mLargeAlbumLbl;
		SimpleLabel* mArtistLbl;
		Sprite* mSelectionSprite;
		Button* mSearchBtn;

		vector<LargeAlbumItem*> mLargeAlbums;
		std::vector<Album*> mAlbumsToAdd;
		std::vector<Track*> mTracksToAdd;
		int mAlbumIndex;
		float mDisplayingAlbum;
		float mScrollingDuration;
		float mTotScrollingDuration;

		float mTimeInterval;
		float mX, mY, mWidth, mHeight, mX2, mY2;
		bool mResized;
		bool mStartedOnTop;
		int mNumTriangles;
		bool mHasFocus;
		int mPlayingTrack;
		int mPlayingAlbum;
		CollectionWindow::SELECTED_THING mSelectedThing;

		CollectionWindow::RIGHT_CLICKED_ITEM mRightClicked;
		int mRightClickedTrack;

		void createBuffers();

		bool isPointInside(int x, int y);
		void setTracks();
		void onItemSelected(ListItem* item, int index);
		void onBtnClicked(Button* btn);

		ID3DXEffectPool* mPool;
		ID3DXEffect* mEffect;
		D3DXHANDLE mTechnique;
		D3DXHANDLE mTextureHandle;
		D3DXHANDLE mViewHandle;
		D3DXHANDLE mWorldHandle;
		D3DXHANDLE mDiffuseHandle;
		D3DXHANDLE mProjectionHandle;
		D3DXHANDLE mLightColorHandle;
		D3DXHANDLE mLight2ColorHandle;
		D3DXHANDLE mAmbientHandle;
		D3DXHANDLE mCamPosHandle;
		D3DXHANDLE mSpecularPowerHandle;
		D3DXHANDLE mSpecularColorHandle;
		D3DXHANDLE mLightPosHandle;
		D3DXHANDLE mLight2PosHandle;
		D3DXHANDLE mFogColorHandle;
		D3DXHANDLE mFogEnabledHandle;

		D3DXMATRIX mView;
		D3DXMATRIX mProjection;
		D3DXMATRIX mWorld;
		D3DXVECTOR4 mDiffuseColor;
		D3DXVECTOR4 mLightColor;
		D3DXVECTOR4 mLight2Color;
		D3DXVECTOR4 mAmbient;
		D3DXVECTOR3 mCamPos;
		D3DXVECTOR3 mLookAt;
		float mSpecularPower;
		D3DXVECTOR3 mSpecularColor;
		D3DXVECTOR3 mLightPos;
		D3DXVECTOR3 mLight2Pos;
		D3DXVECTOR4 mFogColor;
		bool mFogEnabled;

		RenderTarget* mTarget;
		IDirect3DVertexBuffer9* mVertexBuffer;
		IDirect3DIndexBuffer9*  mIndexBuffer;

		// synchronization
		static CCriticalSection mCritSection;
	};
}
#endif