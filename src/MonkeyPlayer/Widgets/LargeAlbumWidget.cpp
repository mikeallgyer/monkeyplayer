// LargeAlbumWidget.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Window of large albums
#include "../Winforms/SearchForm.h"

#include "LargeAlbumWidget.h"
#include "d3dApp.h"
#include "DatabaseManager.h"
#include "MusicLibrary.h"
#include "Settings.h"
#include "Vertex.h"


#include <MonkeyInput.h>

using namespace MonkeyPlayer;

const float LargeAlbumWidget::ALBUM_WIDTH = 4.0f;
const float LargeAlbumWidget::ALBUM_HEIGHT = 3.88f;
const float LargeAlbumWidget::ALBUM_DEPTH = .2f;
const float LargeAlbumWidget::SLOW_SCROLL_SPEED = 2.0f;
const float LargeAlbumWidget::FAST_SCROLL_SPEED = 6.0f;
const float LargeAlbumWidget::REALLY_FAST_SCROLL_SPEED = 60.0f;
const float LargeAlbumWidget::FAST_SCROLL_DELAY = 1.0f;

// used for synchronization
CCriticalSection LargeAlbumWidget::mCritSection;

LargeAlbumWidget::LargeAlbumWidget(float x, float y, float width, float height)
{
	std::string fxPath = Settings::instance()->getStringValue(Settings::CONTENT_DIR, "") + "\\Effects\\default.fx";

	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectPool(&mPool));
	(D3DXCreateEffectFromFile(gDevice, fxPath.c_str(), 
		0, 0, D3DXSHADER_DEBUG, 0, &mEffect, &errors));

	if (errors)
	{
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);
		PostQuitMessage(1);
	}

	mTechnique = mEffect->GetTechniqueByName("Phong");
	mTextureHandle = mEffect->GetParameterByName(0, "Texture");
	mViewHandle = mEffect->GetParameterByName(0, "view");
	mWorldHandle = mEffect->GetParameterByName(0, "world");
	mDiffuseHandle = mEffect->GetParameterByName(0, "diffuseColor");
	mProjectionHandle = mEffect->GetParameterByName(0, "projection");
	mLightColorHandle = mEffect->GetParameterByName(0, "lightColor");
	mLight2ColorHandle = mEffect->GetParameterByName(0, "lightColor2");
	mAmbientHandle = mEffect->GetParameterByName(0, "ambient");
	mCamPosHandle = mEffect->GetParameterByName(0, "camPos");
	mSpecularPowerHandle = mEffect->GetParameterByName(0, "specularPower");
	mSpecularColorHandle = mEffect->GetParameterByName(0, "specularColor");
	mLightPosHandle = mEffect->GetParameterByName(0, "lightPos");
	mLight2PosHandle = mEffect->GetParameterByName(0, "lightPos2");
	mFogColorHandle = mEffect->GetParameterByName(0, "fogColor");
	mFogEnabledHandle = mEffect->GetParameterByName(0, "fogEnabled");

	D3DXMatrixIdentity(&mWorld);
	mDiffuseColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	mCamPos = D3DXVECTOR3(0, -1.0f, -10.0f);
	mLookAt = D3DXVECTOR3(0, -0.5f, 0.0f);
	mLightColor = D3DXVECTOR4(.5f, .5f, .5f, 1.0f);
	mLight2Color = D3DXVECTOR4(.5f, .5f, .5f, 1.0f);
	mAmbient = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpecularPower = 33.0f;
	mSpecularColor = D3DXVECTOR3(0.8f, 0.8f, 0.8f);
	mLightPos = D3DXVECTOR3(0.0f, 2.0f, -20.0f);
	mLight2Pos = D3DXVECTOR3(0.0f, 2.0f, 5.0f);
	mFogColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	mFogEnabled = false;

	mTimeInterval = .5f;

	mPositions.push_back(AlbumPos(false, -7.0f, -D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(false, -6.0f, -D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, -5.f, -D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, -4.0f, -D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, 0.0f, 0, mTimeInterval));
	mPositions.push_back(AlbumPos(true, 4.0f, D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, 5.0f, D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(false, 6.0f, D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(false, 7.0f, D3DX_PI*.5f, mTimeInterval));

	mAlbumIndex = 0;
	mDisplayingAlbum = 0;
	mScrollingDuration = 0;
	mTotScrollingDuration = 0;
	vector<AlbumWithTracks*> albums = DatabaseManager::instance()->getAllAlbumsAndTracks();

	for (unsigned int i = 0; i < albums.size(); i++)
	{
		mLargeAlbums.push_back(snew LargeAlbumItem(*albums[i]->album, albums[i]->tracks));
		mLargeAlbums[i]->setPosition(mPositions[mPositions.size() - 1].xPos, mPositions[mPositions.size() - 1].yRotation);
		delete albums[i]->album;
		delete albums[i]; // vector is owned by LargeAlbumItem
	}

	unsigned int midPos = mPositions.size() / 2;

	for (unsigned int i = 0; i < mPositions.size(); i++)
	{
		int index = (int)mDisplayingAlbum - midPos + i;
		if (index >= 0 && index < (int)mLargeAlbums.size())
		{
			float xPos = mPositions[i].xPos;
			float yRot = mPositions[i].yRotation;
			mLargeAlbums[index]->setPosition(xPos, yRot);
			mLargeAlbums[index]->setVisible(true);
		}
	}

	D3DXVECTOR3 up(0,1,0);
	D3DXMatrixLookAtLH(&mView, &mCamPos, &mLookAt, &up);
	D3DXMatrixPerspectiveFovLH(&mProjection, 3.14159f*.25f, mWidth / mHeight, .1f, 100.0f);

	mResized = true;
	mFocused = false;

	mX = mY;
	mWidth = 100.0f;
	mHeight = 500.0f;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;

	mTarget = snew RenderTarget((int)mWidth, (int)mHeight, D3DCOLOR_XRGB(0, 0, 0), false);

	mTargetSprite = NULL;

	mTrackBox = snew TrackListBox(mX, mY, 200.0, 200.0f, listBox_callback, this);
	mTrackBox->setUseTrackNumbers(true);
	mTrackBox->focus();
	mWidgets.push_back(mTrackBox);

	std::string selBoxPath = FileManager::getContentAsset(std::string("Textures\\cdHighlight.png"));
	mSelectionSprite = snew Sprite(selBoxPath.c_str(), 0, 0, 50.0f, 50.0f);
	mSelectedThing = CollectionWindow::TRACK;

	mLargeAlbumLbl = snew SimpleLabel(0, 0, 200.0f, 30.0f, string(""), 26, DT_CENTER, D3DCOLOR_XRGB(255, 255, 255));
	mLargeAlbumLbl->setSizeToFit(true);
	mWidgets.push_back(mLargeAlbumLbl);

	mArtistLbl = snew SimpleLabel(0, 0, 200.0f, 30.0f, string(""), 26, DT_CENTER, D3DCOLOR_XRGB(255, 255, 255));
	mArtistLbl->setSizeToFit(true);
	mWidgets.push_back(mArtistLbl);

	std::string searchBtnPath = FileManager::getContentAsset(std::string("Textures\\search.png"));
	std::string searchBtnHoverPath = FileManager::getContentAsset(std::string("Textures\\search_hover.png"));
	std::string searchBtnDownPath = FileManager::getContentAsset(std::string("Textures\\search_down.png"));
	mSearchBtn = snew Button(0,0, 50.0f, 50.0f, searchBtnPath, btn_callback, this);
	mSearchBtn->setDownTexture(searchBtnDownPath.c_str());
	mSearchBtn->setHoverTexture(searchBtnHoverPath.c_str());
	mWidgets.push_back(mSearchBtn);

	std::string leftBtnPath = FileManager::getContentAsset(std::string("Textures\\arrow_left.png"));
	std::string leftBtnHoverPath = FileManager::getContentAsset(std::string("Textures\\arrow_left_hover.png"));
	std::string leftBtnDownPath = FileManager::getContentAsset(std::string("Textures\\arrow_left_down.png"));
	mLeftBtn = snew Button(0,0, 40.0f, 40.0f, leftBtnPath, btn_callback, this);
	mLeftBtn->setDownTexture(leftBtnDownPath.c_str());
	mLeftBtn->setHoverTexture(leftBtnHoverPath.c_str());
	mWidgets.push_back(mLeftBtn);

	std::string rightBtnPath = FileManager::getContentAsset(std::string("Textures\\arrow_right.png"));
	std::string rightBtnHoverPath = FileManager::getContentAsset(std::string("Textures\\arrow_right_hover.png"));
	std::string rightBtnDownPath = FileManager::getContentAsset(std::string("Textures\\arrow_right_down.png"));
	mRightBtn = snew Button(0,0, 40.0f, 40.0f, rightBtnPath, btn_callback, this);
	mRightBtn->setDownTexture(rightBtnDownPath.c_str());
	mRightBtn->setHoverTexture(rightBtnHoverPath.c_str());
	mWidgets.push_back(mRightBtn);

	mDoGetTracks = false;
	setTracks();
	mPlayingAlbum = -1;
	mPlayingTrack = -1;
	createBuffers();
}
LargeAlbumWidget::~LargeAlbumWidget()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		delete mLargeAlbums[i];
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		delete mWidgets[i];
	}
	ReleaseCOM(mVertexBuffer);
	ReleaseCOM(mIndexBuffer);
	delete mSelectionSprite;
	delete mTarget;
	lock.Unlock();
}

void LargeAlbumWidget::onDeviceLost()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceLost();
	}
	HR(mEffect->OnLostDevice());
	mTarget->onDeviceLost();
	mSelectionSprite->onDeviceLost();
	lock.Unlock();
}
void LargeAlbumWidget::onDeviceReset()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceReset();
	}
	HR(mEffect->OnResetDevice());
	mTarget->onDeviceReset();
	mSelectionSprite->onDeviceReset();
	recreateTargets();

	lock.Unlock();
}

void LargeAlbumWidget::recreateTargets()
{
	if (mTargetSprite == NULL)
	{
		mTargetSprite = snew Sprite(mTarget->getTexture(), mX, mY, mWidth, mHeight);
		mSprites.push_back(mTargetSprite);
	}
	else
	{
		mTarget->setDimensions((int)mWidth, (int)mHeight);
		mTargetSprite->setDest(mX, mY, mWidth, mHeight);
		mTargetSprite->replaceCurrentTexture(mTarget->getTexture(), false);
	}
}

void LargeAlbumWidget::update(float dt)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mAlbumsToAdd.size(); i++)
	{
		doAddAlbum(mAlbumsToAdd[i]);
		delete mAlbumsToAdd[i];
	}
	mAlbumsToAdd.clear();
	for (unsigned int i = 0; i < mTracksToAdd.size(); i++)
	{
		doAddTrack(mTracksToAdd[i]);
		delete mTracksToAdd[i];
	}
	mTracksToAdd.clear();

	lock.Unlock();
	
	if (mDoGetTracks)
	{
		setTracks();
		mDoGetTracks = false;;
	}
	int selTrack = mTrackBox->getSelectedIndex();


	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->update(dt);
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}

	if (mSelectedThing == CollectionWindow::ARTIST)
	{
		mSelectionSprite->setDest(mArtistLbl->getX() - 8.0f, mArtistLbl->getY() - mY, 
			mArtistLbl->getWidth() + 16.0f, mArtistLbl->getHeight());
	}
	else if (mSelectedThing == CollectionWindow::ALBUM)
	{
		mSelectionSprite->setDest(mLargeAlbumLbl->getX() - 8.0f, mLargeAlbumLbl->getY() - mY, 
			mLargeAlbumLbl->getWidth() + 16.0f, mLargeAlbumLbl->getHeight());
	}

	float scrollSpeed = SLOW_SCROLL_SPEED;
	float scrollSpeedInv = 1.0f / scrollSpeed;
	if (mFocused)
	{
		if (gInput->isKeyDown(VK_LEFT))
		{
			bool firstPress = mScrollingDuration < 0;
			if (firstPress)
			{
				mScrollingDuration = 0;
			}
			mScrollingDuration += dt;
			mTotScrollingDuration += dt;
			
			if (mTotScrollingDuration >= FAST_SCROLL_DELAY)
			{
				scrollSpeed = FAST_SCROLL_SPEED;
				scrollSpeedInv = 1.0f / scrollSpeed;
			}
			if (firstPress || mScrollingDuration >= scrollSpeedInv)
			{
				int mid = mPositions.size() / 2;
				mAlbumIndex--;
				if (mAlbumIndex < 0)
				{
					mAlbumIndex = 0;
				}

				while (mScrollingDuration > scrollSpeedInv)
				{
					mScrollingDuration -= scrollSpeedInv;
				}

				setTracks();
			}
		}
		else if (gInput->isKeyDown(VK_RIGHT))
		{
			bool firstPress = mScrollingDuration < 0;
			if (firstPress)
			{
				mScrollingDuration = 0;
			}
			mScrollingDuration += dt;
			mTotScrollingDuration += dt;
			
			if (mTotScrollingDuration >= FAST_SCROLL_DELAY)
			{
				scrollSpeed = FAST_SCROLL_SPEED;
				scrollSpeedInv = 1.0f / scrollSpeed;
			}
			if (firstPress || mScrollingDuration >= scrollSpeedInv)
			{
				mAlbumIndex++;
				if (mAlbumIndex >= (int)mLargeAlbums.size())
				{
					mAlbumIndex = mLargeAlbums.size() - 1;
				}
				while (mScrollingDuration > scrollSpeedInv)
				{
					mScrollingDuration -= scrollSpeedInv;
				}
				setTracks();
			}
		}
		else
		{
			mScrollingDuration = -1.0f;
			mTotScrollingDuration = -1.0f;
		}

		if (gInput->keyPressed(VK_DOWN))
		{
			if (mSelectedThing == CollectionWindow::ARTIST)
			{
				mTrackBox->focus();
				mTrackBox->setSelectedIndex(0);
				mSelectedThing = CollectionWindow::TRACK;
			}
			else if (mSelectedThing == CollectionWindow::TRACK && selTrack == mTrackBox->getNumItems() - 1)
			{
				mTrackBox->blur();
				mTrackBox->setSelectedIndex(-1);
				mSelectedThing = CollectionWindow::ALBUM;
			}
			else if (mSelectedThing == CollectionWindow::ALBUM)
			{
				mSelectedThing = CollectionWindow::ARTIST;
			}
			else
			{
				mTrackBox->focus();
			}
		}
		else if (gInput->keyPressed(VK_UP))
		{
			if (mSelectedThing == CollectionWindow::ARTIST)
			{
				mSelectedThing = CollectionWindow::ALBUM;
			}
			else if (mSelectedThing == CollectionWindow::TRACK && selTrack == 0)
			{
				mTrackBox->blur();
				mTrackBox->setSelectedIndex(-1);
				mSelectedThing = CollectionWindow::ARTIST;
			}
			else if (mSelectedThing == CollectionWindow::ALBUM)
			{
				mTrackBox->focus();
				mTrackBox->setSelectedIndex(mTrackBox->getNumItems() - 1);
				mSelectedThing = CollectionWindow::TRACK;
			}
			else
			{
				mTrackBox->focus();
			}

		}
		else if (gInput->keyPressed(VK_RETURN) && mSelectedThing != CollectionWindow::TRACK)
		{
			queueThing(mSelectedThing);
		}

		if (mAlbumIndex == mPlayingAlbum)
		{
			mTrackBox->setHighlightedItem(mPlayingTrack);
		}
		else
		{
			mTrackBox->setHighlightedItem(-1);
		}

	}  // if focused

	float offset = 0;

	if (mDisplayingAlbum != (float)mAlbumIndex)
	{
		if (mDisplayingAlbum - (float)mAlbumIndex > 3.0f)
		{
			scrollSpeed = REALLY_FAST_SCROLL_SPEED;
			scrollSpeedInv = 1.0f / scrollSpeed;
			mDisplayingAlbum = min((float)mLargeAlbums.size() - 1, mAlbumIndex + 3.0f);
		}
		else if (mDisplayingAlbum - (float)mAlbumIndex < -3.0f)
		{
			scrollSpeed = REALLY_FAST_SCROLL_SPEED;
			scrollSpeedInv = 1.0f / scrollSpeed;
			mDisplayingAlbum = max(0, mAlbumIndex - 3.0f);
		}
		else if (abs(mDisplayingAlbum - (float)mAlbumIndex) > 2.0f)
		{
			scrollSpeed = FAST_SCROLL_SPEED;
			scrollSpeedInv = 1.0f / scrollSpeed;
		}

		if (mDisplayingAlbum > mAlbumIndex)
		{
			mDisplayingAlbum -= dt * scrollSpeed;
			if (mDisplayingAlbum < mAlbumIndex)
			{
				mDisplayingAlbum = (float)mAlbumIndex;
			}
			offset = -(mDisplayingAlbum - (int)(mDisplayingAlbum));
		}
		else
		{
			mDisplayingAlbum += dt * scrollSpeed;
			if (mDisplayingAlbum > mAlbumIndex)
			{
				mDisplayingAlbum = (float)mAlbumIndex;
			}
			offset = -(mDisplayingAlbum - (int)(mDisplayingAlbum));
		}
		int mid = mPositions.size() / 2;

		for (unsigned int i = 0; i < mPositions.size(); i++)
		{
			int index = (int)mDisplayingAlbum - mid + i;
			if (index >= 0 && index < (int)mLargeAlbums.size())
			{
				float xPos = mPositions[i].xPos;
				float yRot = mPositions[i].yRotation;
				if (offset > 0 && i < mPositions.size() - 1)
				{
					xPos += (mPositions[i].xPos - mPositions[i + 1].xPos) * offset;
					yRot += (mPositions[i].yRotation - mPositions[i + 1].yRotation) * offset;
				}
				else if (offset < 0 && i > 0)
				{
					xPos += (mPositions[i].xPos - mPositions[i-1].xPos) * offset;
					yRot += (mPositions[i].yRotation - mPositions[i-1].yRotation) * offset;
				}
				mLargeAlbums[index]->setPosition(xPos, yRot);
				mLargeAlbums[index]->setVisible(true);
			}
		}
		int oneLess = (int)mDisplayingAlbum - mid - 1;
		int oneMore = (int)mDisplayingAlbum + mid + 1;
		for (int i = 0; i <= oneLess; i++)
		{
			mLargeAlbums[i]->setVisible(false);
		}
		for (int i = oneMore; i < (int)mLargeAlbums.size(); i++)
		{
			mLargeAlbums[i]->setVisible(false);
		}
	}
}
void LargeAlbumWidget::createBuffers()
{
		// Obtain a pointer to a new vertex buffer.
	HR(gDevice->CreateVertexBuffer(24 * sizeof(Vertex::VertexPosTexNormal), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED, &mVertexBuffer, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// cube's vertex data.

	Vertex::VertexPosTexNormal* v = 0;
	HR(mVertexBuffer->Lock(0, 0, (void**)&v, 0));

	float half_width = ALBUM_WIDTH * .5f;
	float half_height = ALBUM_HEIGHT * .5f;
	float half_depth = ALBUM_DEPTH * .5f;
	// front
	v[0] = Vertex::VertexPosTexNormal(-half_width, -half_height, -half_depth, 0.0f, 0.0f, 0, 0, -1.0f); // bottom-left
	v[1] = Vertex::VertexPosTexNormal(-half_width, half_height, -half_depth, 0.0f, 1.0f, 0, 0, -1.0f); // top-left
	v[2] = Vertex::VertexPosTexNormal(half_width, half_height, -half_depth, 1.0f, 1.0f, 0, 0, -1.0f);  //top-right
	v[3] = Vertex::VertexPosTexNormal(half_width, -half_height, -half_depth, 1.0f, 0.0f, 0, 0, -1.0f); // bottom right

	// left
	v[4] = Vertex::VertexPosTexNormal(-half_width, -half_height, half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f); // bottom-left
	v[5] = Vertex::VertexPosTexNormal(-half_width, half_height, half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f); // top-left
	v[6] = Vertex::VertexPosTexNormal(-half_width, half_height, -half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f);  //top-right
	v[7] = Vertex::VertexPosTexNormal(-half_width, -half_height, -half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f); // bottom right

	// back
	v[8] = Vertex::VertexPosTexNormal(-half_width, half_height, half_depth, 0.0f, 1.0f, 0, 0, 1.0f); // top-left
	v[9] = Vertex::VertexPosTexNormal(-half_width, -half_height, half_depth, 0.0f, 0.0f, 0, 0, 1.0f); // bottom-left
	v[10] = Vertex::VertexPosTexNormal(half_width, -half_height, half_depth, 1.0f, 0.0f, 0, 0, 1.0f); // bottom right
	v[11] = Vertex::VertexPosTexNormal(half_width, half_height, half_depth, 1.0f, 1.0f, 0, 0, 1.0f);  //top-right

	// right
	v[12] = Vertex::VertexPosTexNormal(half_width, half_height, half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f); // top-left
	v[13] = Vertex::VertexPosTexNormal(half_width, -half_height, half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f); // bottom-left
	v[14] = Vertex::VertexPosTexNormal(half_width, -half_height, -half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f); // bottom right
	v[15] = Vertex::VertexPosTexNormal(half_width, half_height, -half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f);  //top-right

	// top
	v[16] = Vertex::VertexPosTexNormal(-half_width, half_height, -half_depth, 0.0f, 0.0f, 0, 0, 1.0f); // bottom-left
	v[17] = Vertex::VertexPosTexNormal(-half_width, half_height, half_depth, 0.0f, 1.0f, 0, 0, 1.0f); // top-left
	v[18] = Vertex::VertexPosTexNormal(half_width, half_height, half_depth, 1.0f, 1.0f, 0, 0, 1.0f);  //top-right
	v[19] = Vertex::VertexPosTexNormal(half_width, half_height, -half_depth, 1.0f, 0.0f, 0, 0, 1.0f); // bottom right

	// bottom
	v[20] = Vertex::VertexPosTexNormal(-half_width, -half_height, half_depth, 0.0f, 1.0f, 0, 0, 1.0f); // top-left
	v[21] = Vertex::VertexPosTexNormal(-half_width, -half_height, -half_depth, 0.0f, 0.0f, 0, 0, 1.0f); // bottom-left
	v[22] = Vertex::VertexPosTexNormal(half_width, -half_height, -half_depth, 1.0f, 0.0f, 0, 0, 1.0f); // bottom right
	v[23] = Vertex::VertexPosTexNormal(half_width, -half_height, half_depth, 1.0f, 1.0f, 0, 0, 1.0f);  //top-right

	HR(mVertexBuffer->Unlock());

	// Obtain a pointer to a new index buffer.
	HR(gDevice->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIndexBuffer, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// cube's index data.

	WORD* k = 0;

	HR(mIndexBuffer->Lock(0, 0, (void**)&k, 0));

	// Front face.
	k[0] = 0; k[1] = 1; k[2] = 2;
	k[3] = 0; k[4] = 2; k[5] = 3;

	// Left
	k[6] = 4; k[7] = 5; k[8] = 6;
	k[9] = 4; k[10] = 6; k[11] = 7;

	// Back face.
	k[12] = 8; k[13] = 9; k[14] = 10;
	k[15] = 8; k[16] = 10; k[17] = 11;

	// Right
	k[18] = 12; k[19] = 13; k[20] = 14;
	k[21] = 12; k[22] = 14; k[23] = 15;

	// top
	k[24] = 16; k[25] = 17; k[26] = 18;
	k[27] = 16; k[28] = 18; k[29] = 19;

	// top
	k[30] = 20; k[31] = 21; k[32] = 22;
	k[33] = 20; k[34] = 22; k[35] = 23;

	HR(mIndexBuffer->Unlock());
}

void LargeAlbumWidget::preRender()
{
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->preRender();
	}
	mTarget->beginScene();

	if (mSelectedThing == CollectionWindow::ALBUM || mSelectedThing == CollectionWindow::ARTIST)
	{
		gWindowMgr->drawSprite(mSelectionSprite, (float)mTarget->getWidth(), (float)mTarget->getHeight());
	}
	
	HR(mEffect->SetTechnique(mTechnique));

	mNumTriangles = 0;

	UINT numPasses = 0;
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		if (mLargeAlbums[i]->getVisible())
		{
			HR(mEffect->Begin(&numPasses, 0));
			for (UINT j = 0; j < numPasses; j++)
			{
				HR(mEffect->BeginPass(j));

				HR(mEffect->SetTexture(mTextureHandle, mLargeAlbums[i]->getTexture()));
				HR(mEffect->SetMatrix(mViewHandle, &mView));
				HR(mEffect->SetMatrix(mWorldHandle, &mLargeAlbums[i]->getWorld()));
				HR(mEffect->SetMatrix(mProjectionHandle, &mProjection));
				HR(mEffect->SetVector(mLightColorHandle, &mLightColor));
				HR(mEffect->SetVector(mLight2ColorHandle, &mLight2Color));
				HR(mEffect->SetVector(mDiffuseHandle, &mDiffuseColor));
				HR(mEffect->SetVector(mAmbientHandle, &mAmbient));
				HR(mEffect->SetValue(mCamPosHandle, &mCamPos, sizeof(D3DXVECTOR3)));
				HR(mEffect->SetFloat(mSpecularPowerHandle, mSpecularPower));
				HR(mEffect->SetValue(mSpecularColorHandle, &mSpecularColor, sizeof(D3DXVECTOR3)));
				HR(mEffect->SetValue(mLightPosHandle, &mLightPos, sizeof(D3DXVECTOR3)));
				HR(mEffect->SetValue(mLight2PosHandle, &mLight2Pos, sizeof(D3DXVECTOR3)));
				HR(mEffect->SetVector(mFogColorHandle, &mFogColor));
				HR(mEffect->SetBool(mFogEnabledHandle, mFogEnabled));

				HR(mEffect->CommitChanges());

				HR(gDevice->SetStreamSource(0, mVertexBuffer, 0, sizeof(Vertex::VertexPosTexNormal)));
				HR(gDevice->SetIndices(mIndexBuffer));
				HR(gDevice->SetVertexDeclaration(Vertex::VertexPosTexNormal::Decl));

				HR(gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 36, 0, 14));
					
				mNumTriangles += 1;

				HR(mEffect->EndPass());
			}
			HR(mEffect->End());
		}
	}
	mTarget->endScene();
}

void LargeAlbumWidget::setPos(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	if (width != -999.0f)
	{
		mWidth = width;
	}

	if (height != -999.0f)
	{
		mHeight = height;
	}

	mX2 = mX + mWidth;
	mY2 = mY + mHeight;
	mSearchBtn->setPos(mX + mWidth - 75.0f, mY + mHeight - 75.0f);
	mLeftBtn->setPos(mX + 25.0f, mY + mHeight * .5f - mLeftBtn->getHeight() * .5f);
	mRightBtn->setPos(mX + mWidth - mRightBtn->getWidth() - 25.0f, mY + mHeight * .5f - mRightBtn->getHeight() * .5f);

	mTrackBox->setPos(mX + (mX2 - mX) * .5f - 200.0f, mY + 80.0f, 400.0f, 200.0f);
	mLargeAlbumLbl->setPos(mWidth / 2.0f - mLargeAlbumLbl->getWidth() / 2.0f, mY + mHeight - 50.0f);
	mArtistLbl->setPos(mWidth / 2.0f - mArtistLbl->getWidth() / 2.0f, mY + 40.0f);

	D3DXVECTOR3 up(0,1,0);
	D3DXMatrixLookAtLH(&mView, &mCamPos, &mLookAt, &up);
	D3DXMatrixPerspectiveFovLH(&mProjection, 3.14159f*.25f, mWidth / mHeight, .1f, 100.0f);
	recreateTargets();
}

std::vector<Sprite*> LargeAlbumWidget::getSprites()
{
	return mSprites;
}
std::vector<IWidget*> LargeAlbumWidget::getWidgets()
{
	return mWidgets;
}
int LargeAlbumWidget::getNumTriangles()
{
	return mNumTriangles;
}
bool LargeAlbumWidget::onMouseEvent(MouseEvent ev)
{
	if(mHasFocus && !mTrackBox->getIsFocused())
	{
		if (ev.getEvent() == MouseEvent::MOUSEWHEEL)
		{
			CSingleLock lock(&mCritSection);
			lock.Lock();
			// update list of items to display
			if (ev.getExtraHiData() < 0)
			{
				mAlbumIndex++;
				mAlbumIndex = min((int)mLargeAlbums.size() - 1, mAlbumIndex);
				setTracks();
			}
			else
			{
				mAlbumIndex--;
				mAlbumIndex = max(0, mAlbumIndex);
				setTracks();
			}

			lock.Unlock();
			return true;
		}
	} // if mousewheel
	// if clicked, give widget focus
	if (ev.getEvent() == MouseEvent::LBUTTONDOWN ||
		ev.getEvent() == MouseEvent::RBUTTONDOWN)
	{
		bool found = false;
		for (unsigned int i = 0; i < mWidgets.size(); i++)
		{
			if (!found && mWidgets[i]->isPointInside(ev.getX(), ev.getY()))
			{
				found = true;
				mWidgets[i]->focus();
				if (mWidgets[i] == mTrackBox)
				{
					mSelectedThing = CollectionWindow::TRACK;
				}
			}
			else if (mWidgets[i]->getIsFocused())
			{
				mWidgets[i]->blur();
			}
		}
		if (mArtistLbl->isPointInside(ev.getX(), ev.getY()))
		{
			mSelectedThing = CollectionWindow::ARTIST;
		}
		else if (mLargeAlbumLbl->isPointInside(ev.getX(), ev.getY()))
		{
			mSelectedThing = CollectionWindow::ALBUM;
		}
	}
	else if (ev.getEvent() == MouseEvent::LBUTTONDBLCLK)
	{
		if (mArtistLbl->isPointInside(ev.getX(), ev.getY()))
		{
			queueThing(CollectionWindow::ARTIST);
		}
		else if (mLargeAlbumLbl->isPointInside(ev.getX(), ev.getY()))
		{
			queueThing(CollectionWindow::ALBUM);
		}
	}
	bool consumed = false;
	if ( !ev.getConsumed() && ev.getEvent() == MouseEvent::RBUTTONUP && isPointInside(ev.getX(), ev.getY()))
	{
		mRightClickedTrack = mTrackBox->getItemAtPos(ev.getX(), ev.getY());
		if (mTrackBox->isPointInside(ev.getX(), ev.getY()) && mRightClickedTrack >= 0)
		{
			mRightClicked = CollectionWindow::RIGHT_TRACK;
			vector<ListItem*> items;
			items.push_back(snew SimpleListItem("Play Now", CollectionWindow::TRACK_PLAY_IMMEDIATE));
///			items.push_back(snew SimpleListItem("Replace Queue", TRACK_REPLACE_QUEUE));
			items.push_back(snew SimpleListItem("Queue Next", CollectionWindow::TRACK_QUEUE_NEXT));
			items.push_back(snew SimpleListItem("Append to Queue", CollectionWindow::TRACK_QUEUE_END));

			gWindowMgr->openContextMenu((float)ev.getX(), (float)ev.getY(), items, this);
			consumed = true;
		}
		else if (mArtistLbl->isPointInside(ev.getX(), ev.getY()))
		{
			mRightClicked = CollectionWindow::RIGHT_ARTIST;
			vector<ListItem*> items;
			items.push_back(snew SimpleListItem("Play All Now", CollectionWindow::ARTIST_PLAY_IMMEDIATE));
			items.push_back(snew SimpleListItem("Replace Queue", CollectionWindow::ARTIST_REPLACE_QUEUE));
			items.push_back(snew SimpleListItem("Queue All Next", CollectionWindow::ARTIST_QUEUE_NEXT));
			items.push_back(snew SimpleListItem("Append All to Queue", CollectionWindow::ARTIST_QUEUE_END));

			gWindowMgr->openContextMenu((float)ev.getX(), (float)ev.getY(), items, this);
			consumed = true;
		}
		else if (mLargeAlbumLbl->isPointInside(ev.getX(), ev.getY()))
		{
			mRightClicked = CollectionWindow::RIGHT_ALBUM;
			vector<ListItem*> items;
			items.push_back(snew SimpleListItem("Play All Now", CollectionWindow::ALBUM_PLAY_IMMEDIATE));
			items.push_back(snew SimpleListItem("Replace Queue", CollectionWindow::ALBUM_REPLACE_QUEUE));
			items.push_back(snew SimpleListItem("Queue All Next", CollectionWindow::ALBUM_QUEUE_NEXT));
			items.push_back(snew SimpleListItem("Append All to Queue", CollectionWindow::ALBUM_QUEUE_END));

			gWindowMgr->openContextMenu((float)ev.getX(), (float)ev.getY(), items, this);
			consumed = true;
		}
	}

	for (unsigned int i = 0; i < mWidgets.size() && !consumed; i++)
	{
		if (mWidgets[i]->onMouseEvent(ev))
		{
			consumed = true;
			break;
		}
	}

	return consumed;
}
void LargeAlbumWidget::refresh()
{
}
void LargeAlbumWidget::goToChar(char c)
{
	int index = -1;
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		string caps = FileManager::toUpper(mLargeAlbums[i]->getAlbum().Artist);
		if (caps[0] == c)
		{
			index = (int)i;
			break;
		}
	}

	if (index >= 0 && index != mAlbumIndex)
	{
		goToAlbum(index);
	}
}
void LargeAlbumWidget::goToString(string &s)
{
	int index = -1;
	unsigned int strLen = s.length();
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		string caps = FileManager::toUpper(mLargeAlbums[i]->getAlbum().VirtualArtist);
		unsigned int albumLen = caps.length();
		unsigned int j = 0;
		while (j < strLen && j < albumLen && s[j] == caps[j])
		{
			j++;
		}
		if (j == strLen)
		{
			index = (int)i;
			break;
		}
	}

	if (index >= 0 && index != mAlbumIndex)
	{
		goToAlbum(index);
	}
}
void LargeAlbumWidget::goToSong(Album a, Track t, bool doHighlight)
{
	mPlayingAlbum = -1;
	mPlayingTrack = -1;
	int index = -1;
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		if (mLargeAlbums[i]->getAlbum().Id == a.Id)
		{
			index = (int)i;
			break;
		}
	}

	if (index >= 0)
	{
		mPlayingAlbum = index;
		goToAlbum(index);
		for (unsigned int i = 0; i < mLargeAlbums[mAlbumIndex]->getTracks().size(); i++)
		{
			if (mLargeAlbums[mAlbumIndex]->getTracks()[i]->Id == t.Id)
			{
				if (doHighlight)
				{
					mTrackBox->setHighlightedItem(i);
					mPlayingTrack = i;
				}
				else
				{
					if (mHasFocus)
					{
						mTrackBox->setSelectedIndex(i);
						mTrackBox->focus();
					}
					mSelectedThing = CollectionWindow::TRACK;
				}
			}
		}
	}
}
int LargeAlbumWidget::getCurrentAlbum()
{
	if (mAlbumIndex >= 0 && mAlbumIndex < (int)mLargeAlbums.size())
	{
		return (int)mLargeAlbums[mAlbumIndex]->getAlbum().Id;
	}
	return -1;
}
void LargeAlbumWidget::goToAlbum(int index)
{
	if (index >= 0 && index != mAlbumIndex)
	{
		int currAlbum = mAlbumIndex;
		mAlbumIndex = index;
		setTracks();

		int mid = (int)mPositions.size() / 2;
		if (index > currAlbum)
		{
			for (int i = currAlbum - mid + 1; i < index - mid + 1 && i < (int)mLargeAlbums.size(); i++)
			{
				for (int j = i; j < index + mid && j < (int)mLargeAlbums.size(); j++)
				{
					int posIndex = max(0, min((int)mPositions.size() - 1, j - i));
					if (j >= 0)
					{
						mLargeAlbums[j]->addDest(mPositions[posIndex].xPos,
							mPositions[posIndex].yRotation,
							mPositions[posIndex].visible,
							mPositions[posIndex].time * .03f);
					}
				}
			}
		}
		else
		{
			for (int i = currAlbum + mid; i > index + mid && i >= 0; i--)
			{
				for (int j = i; j > index - mid && j >= 0; j--)
				{
					int posIndex = max(0, min((int)mPositions.size() - 1, (int)mPositions.size() - i + j));
					if (j < (int)mLargeAlbums.size())
					{
						mLargeAlbums[j]->addDest(mPositions[posIndex].xPos,
							mPositions[posIndex].yRotation,
							mPositions[posIndex].visible,
							mPositions[posIndex].time * .03f);
					}
				}
			}
		}
	}
}
void LargeAlbumWidget::onContextMenuSelected(ItemListBox* menu)
{
	if (mLargeAlbums.size() <= 0)
	{
		return;
	}
	int sel = menu->getSelectedItem()->getId();

	// tracks
	if (sel == CollectionWindow::TRACK_PLAY_IMMEDIATE)
	{
		MusicLibrary::instance()->playSong(((TrackListItem*)mTrackBox->getItem(mRightClickedTrack))->getTrack()->Filename);
	}
	else if (sel == CollectionWindow::TRACK_QUEUE_END)
	{
		MusicLibrary::instance()->getPlaylistWindow()->addTrackToQueueEnd(mTrackBox->getItem(mRightClickedTrack)->getId());
	}
	else if (sel == CollectionWindow::TRACK_QUEUE_NEXT)
	{
		MusicLibrary::instance()->getPlaylistWindow()->insertTrackToQueueNext(mTrackBox->getItem(mRightClickedTrack)->getId());
	}
	else if (sel == CollectionWindow::TRACK_REPLACE_QUEUE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithTrack(mTrackBox->getItem(mRightClickedTrack)->getId());
	}

	// artist
	else if (sel == CollectionWindow::ARTIST_PLAY_IMMEDIATE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithArtist(mArtistLbl->getString());
		MusicLibrary::instance()->playNextSong();
	}
	else if (sel == CollectionWindow::ARTIST_QUEUE_END)
	{
		MusicLibrary::instance()->getPlaylistWindow()->addArtistToQueueEnd(mArtistLbl->getString());
	}
	else if (sel == CollectionWindow::ARTIST_QUEUE_NEXT)
	{
		MusicLibrary::instance()->getPlaylistWindow()->insertArtistToQueueNext(mArtistLbl->getString());
	}
	else if (sel == CollectionWindow::ARTIST_REPLACE_QUEUE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithArtist(mArtistLbl->getString());
	}
	
	// Album
	else if (sel == CollectionWindow::ALBUM_PLAY_IMMEDIATE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithAlbum(mLargeAlbums[mAlbumIndex]->getAlbum());
		MusicLibrary::instance()->playNextSong();
	}
	else if (sel == CollectionWindow::ALBUM_QUEUE_END)
	{
		MusicLibrary::instance()->getPlaylistWindow()->addAlbumToQueueEnd(mLargeAlbums[mAlbumIndex]->getAlbum());
	}
	else if (sel == CollectionWindow::ALBUM_QUEUE_NEXT)
	{
		MusicLibrary::instance()->getPlaylistWindow()->insertAlbumToQueueNext(mLargeAlbums[mAlbumIndex]->getAlbum());
	}
	else if (sel == CollectionWindow::ALBUM_REPLACE_QUEUE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithAlbum(mLargeAlbums[mAlbumIndex]->getAlbum());
	}
}
void LargeAlbumWidget::queueThing(CollectionWindow::SELECTED_THING thing)
{
	if (mLargeAlbums.size() <= 0)
	{
		return;
	}
	if (thing == CollectionWindow::ALBUM)
	{
		vector<Track*> tracks = DatabaseManager::instance()->getTracks(mLargeAlbums[mAlbumIndex]->getAlbum());
		MusicLibrary::instance()->getPlaylistWindow()->clearItems();
		for (unsigned int i = 0; i < tracks.size(); i++)
		{
			MusicLibrary::instance()->getPlaylistWindow()->addItem(snew Track(*tracks[i]));
		}
		MusicLibrary::instance()->playNextSong();
	}
	else if (thing == CollectionWindow::ARTIST)
	{
		vector<Track*> tracks = DatabaseManager::instance()->getTracks(mLargeAlbums[mAlbumIndex]->getAlbum().Artist);
		MusicLibrary::instance()->getPlaylistWindow()->clearItems();
		for (unsigned int i = 0; i < tracks.size(); i++)
		{
			MusicLibrary::instance()->getPlaylistWindow()->addItem(snew Track(*tracks[i]));
		}
		MusicLibrary::instance()->playNextSong();
	}
}

bool LargeAlbumWidget::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY
		|| xPoint > mX2 || yPoint > mY2);
}
void LargeAlbumWidget::setTracks()
{
	mTrackBox->clearItems();
	if (mLargeAlbums.size() > 0 && mAlbumIndex >= 0)
	{
		vector<Track*> tracks = mLargeAlbums[mAlbumIndex]->getTracks();
		for (unsigned int i = 0; i < tracks.size(); i++)
		{
			mTrackBox->addItem(snew TrackListItem(tracks[i], false));
		}
		Album album = mLargeAlbums[mAlbumIndex]->getAlbum();
		int length = max(album.Artist.length(), album.Title.length()) + 1;
		char *buf = snew char[length];
		sprintf_s(buf, length, "%s", album.Title.c_str()); 
		mLargeAlbumLbl->setString(string(buf));
		mLargeAlbumLbl->setPos(mWidth / 2.0f - mLargeAlbumLbl->getWidth() / 2.0f, mY + mHeight - 50.0f);

		sprintf_s(buf, length, "%s", album.Artist.c_str()); 
		mArtistLbl->setString(string(buf));
		mArtistLbl->setPos(mWidth / 2.0f - mArtistLbl->getWidth() / 2.0f, mY + 40.0f);
		delete buf;
	}
	else
	{
	}

//	vector<Album*> albums = DatabaseManager::instance()->getAllAlbums();
}
void LargeAlbumWidget::focus()
{
	mFocused = true;
}
void LargeAlbumWidget::blur()
{
	mFocused = false;
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->blur();
	}
}
void LargeAlbumWidget::addAlbum(Album *album)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mAlbumsToAdd.push_back(snew Album(album->Id, album->NumTracks, album->Title, album->Year, album->Artist, album->VirtualArtist));
	lock.Unlock();
}
void LargeAlbumWidget::doAddAlbum(Album *album)
{
	int insertIndex = 0;
	vector<LargeAlbumItem*>::iterator iter = mLargeAlbums.begin();
	string newArtist = FileManager::toUpper(album->VirtualArtist);
	string newTitle = FileManager::toUpper(album->Title);
	if(newArtist=="BLACK KEYS"){
		newArtist[0]='B';
	}
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		Album currAlbum = mLargeAlbums[i]->getAlbum();
		string currArtist = FileManager::toUpper(currAlbum.VirtualArtist);
		string currTitle = FileManager::toUpper(currAlbum.Title);
		if (mLargeAlbums[i]->getAlbum().Id == album->Id) // already contains id
		{
			insertIndex = -1;
			break;
		}
		else if (currArtist == newArtist)
		{
			insertIndex = i;
			for (unsigned int j = i; j < mLargeAlbums.size() && currArtist == newArtist && newTitle >= currTitle; j++)
			{
				currAlbum = mLargeAlbums[j]->getAlbum();
				currArtist = FileManager::toUpper(currAlbum.VirtualArtist);
				currTitle = FileManager::toUpper(currAlbum.Title);
				if (currTitle == newTitle)
				{
					insertIndex = -1;
					break;
				}
				insertIndex++;
			}
			insertIndex--;
			break;
		}
		else if (currArtist > newArtist)
		{
			insertIndex =  i;
			break;
		}
		insertIndex++;
	}

	if (insertIndex >= 0)
	{
		iter += insertIndex;
		int diff = (insertIndex - mAlbumIndex);
		if (diff == 0 && iter != mLargeAlbums.end())
		{
			//iter++;
			//insertIndex++;
		}
		mLargeAlbums.insert(iter, snew LargeAlbumItem(*album));

		unsigned int midPos = mPositions.size() / 2;
		for (unsigned int i = 0; i < mPositions.size(); i++)
		{
			int index = i - midPos + mAlbumIndex;
			if (index >= 0 && index < (int)mLargeAlbums.size())
			{
				mLargeAlbums[index]->setPosition(mPositions[i].xPos, mPositions[i].yRotation);
				mLargeAlbums[index]->setVisible(true);
			}
		}
	}
	mDoGetTracks = true;
}

void LargeAlbumWidget::addTrack(Track* track)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();

	if (track->AlbumId > 0)
	{
		bool foundAlbum = false;
		unsigned int index = 0;
		for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
		{
			if (mLargeAlbums[i]->getAlbum().Id == track->AlbumId)
			{
				mLargeAlbums[i]->setTracksDirty();
			}
		}
	}
	lock.Unlock();

	mDoGetTracks = true;
}
void LargeAlbumWidget::doAddTrack(Track* track)
{
/*	int insertIndex = 0;
	for (unsigned int i = 0; i < mSmallItems.size(); i++)
	{
		if (mSmallItems[i]->getAlbum().Id == track->AlbumId)
		{
			insertIndex = i;
			break;
		}
	}

	if (insertIndex >= 0)
	{
		mSmallItems[insertIndex]->addTrack(track);
		mSmallItems[insertIndex]->onDeviceReset();
	}
*/
}
void LargeAlbumWidget::onItemSelected(ListItem* item, int index)
{
	TrackListItem* trackItem = static_cast<TrackListItem*>(item);
	if (trackItem != NULL)
	{
		SoundManager::instance()->playFile(trackItem->getTrack()->Filename.c_str());
		mTrackBox->setCurrentTrack(index);
	}
}

void LargeAlbumWidget::onBtnClicked(Button* btn)
{
	if (btn == mSearchBtn)
	{
		gWindowMgr->openSearch();
	}
	else if (btn == mLeftBtn && mAlbumIndex > 0)
	{
		mAlbumIndex--;

	}
	else if (btn == mRightBtn && mAlbumIndex < (int)mLargeAlbums.size() - 1)
	{
		mAlbumIndex++;
	}
}
