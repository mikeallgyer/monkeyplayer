// LargeAlbumWidget.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Window of large albums
#include <string>
#include <vector>

#include "IWidget.h"
#include "LargeAlbumItem.h"
#include "RenderTarget.h"
#include "Sprite.h"

#ifndef LARGE_ALBUM_WIDGET_H
#define LARGE_ALBUM_WIDGET_H


class LargeAlbumWidget : public IWidget
{
public:
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
	int getNumTriangles();
	virtual bool onMouseEvent(MouseEvent e);
	virtual void refresh();
	void goToChar(char c);

	static const float ALBUM_WIDTH;
	static const float ALBUM_HEIGHT;
	static const float ALBUM_DEPTH;

	void addAlbum(Album* album);
	void addTrack(Track* track);
protected:

	void doAddAlbum(Album* album);
	void doAddTrack(Track* track);

	vector<AlbumPos> mPositions;
	std::vector<Sprite*> mSprites; 
	Sprite* mTargetSprite;
	vector<LargeAlbumItem*> mLargeAlbums;
	std::vector<Album*> mAlbumsToAdd;
	std::vector<Track*> mTracksToAdd;
	int mAlbumIndex;

	float mTimeInterval;
	float mX, mY, mWidth, mHeight, mX2, mY2;
	bool mResized;
	bool mStartedOnTop;
	float mNumTriangles;

	void createBuffers();

	bool isPointInside(int x, int y);

	ID3DXEffectPool* mPool;
	ID3DXEffect* mEffect;
	D3DXHANDLE mTechnique;
	D3DXHANDLE mTextureHandle;
	D3DXHANDLE mViewHandle;
	D3DXHANDLE mWorldHandle;
	D3DXHANDLE mDiffuseHandle;
	D3DXHANDLE mProjectionHandle;
	D3DXHANDLE mLightColorHandle;
	D3DXHANDLE mAmbientHandle;
	D3DXHANDLE mCamPosHandle;
	D3DXHANDLE mSpecularPowerHandle;
	D3DXHANDLE mSpecularColorHandle;
	D3DXHANDLE mLightPosHandle;
	D3DXHANDLE mFogColorHandle;
	D3DXHANDLE mFogEnabledHandle;

	D3DXMATRIX mView;
	D3DXMATRIX mProjection;
	D3DXMATRIX mWorld;
	D3DXVECTOR4 mDiffuseColor;
	D3DXVECTOR4 mLightColor;
	D3DXVECTOR4 mAmbient;
	D3DXVECTOR3 mCamPos;
	float mSpecularPower;
	D3DXVECTOR3 mSpecularColor;
	D3DXVECTOR3 mLightPos;
	D3DXVECTOR4 mFogColor;
	bool mFogEnabled;

	RenderTarget* mTarget;
	IDirect3DVertexBuffer9* mVertexBuffer;
	IDirect3DIndexBuffer9*  mIndexBuffer;

	// synchronization
	static CCriticalSection mCritSection;
};
#endif