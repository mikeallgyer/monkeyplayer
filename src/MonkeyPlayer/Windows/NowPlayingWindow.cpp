// NowPlayingWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays currently playing info

#include "FileManager.h"
#include "NowPlayingWindow.h"

using namespace MonkeyPlayer;

using namespace MonkeyPlayer;

const float NowPlayingWindow::WINDOW_WIDTH = 150;
const float NowPlayingWindow::WINDOW_HEIGHT = 150;

NowPlayingWindow::NowPlayingWindow()
{
	mWidth = WINDOW_WIDTH;
	mHeight = WINDOW_HEIGHT;

	std::string whitePath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	Sprite* whiteTex = snew Sprite(whitePath.c_str(), 0, 0, mWidth, mHeight, D3DXVECTOR4(0, 0, 0, 1.0f));
	mSprites.push_back(whiteTex);

	float coverWidth = mWidth - IWindow::BORDER_THICKNESS;
	float coverHeight = mHeight - IWindow::BORDER_THICKNESS;
	mDefaultAlbumPath = FileManager::getContentAsset(std::string("Textures\\UnknownAlbum.jpg"));
	mAlbumArt = snew Sprite(mDefaultAlbumPath.c_str(), 1.0f, 1.0f, coverWidth, coverHeight);
	mSprites.push_back(mAlbumArt);

	std::string edgePath = FileManager::getContentAsset(std::string("Textures\\border_bottom.png"));
	Sprite* bottomEdge = snew Sprite(edgePath.c_str(), 0, coverHeight, coverWidth, IWindow::BORDER_THICKNESS);
	mSprites.push_back(bottomEdge);
	edgePath = FileManager::getContentAsset(std::string("Textures\\border_right.png"));
	Sprite* rightEdge = snew Sprite(edgePath.c_str(), coverWidth, 0, IWindow::BORDER_THICKNESS, coverHeight);
	mSprites.push_back(rightEdge);
	edgePath = FileManager::getContentAsset(std::string("Textures\\border_bottom_right.png"));
	Sprite* cornerEdge = snew Sprite(edgePath.c_str(), coverWidth, coverHeight, IWindow::BORDER_THICKNESS, 
		IWindow::BORDER_THICKNESS);
	mSprites.push_back(cornerEdge);

	mLabel = snew SimpleLabel(mWidth, mHeight - 20.0f, mWidth, 20.0f, string("hello"), 16, 256, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),
		D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));
	mLabel->setSizeToFit(true);
	mWidgets.push_back(mLabel);
	mIsPlaying = false;
	mLabelX = mWidth;

	SoundManager::instance()->addCallback(soundEventCB, this);

}
NowPlayingWindow::~NowPlayingWindow()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		delete mWidgets[i];
	}
}

void NowPlayingWindow::onDeviceLost()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceLost();
	}
}
void NowPlayingWindow::onDeviceReset()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceReset();
	}
}

int NowPlayingWindow::getWidth()
{
	return (int)mWidth;
}
int NowPlayingWindow::getHeight()
{
	return (int)mHeight;
}

void NowPlayingWindow::update(float dt)
{
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}
	if (mIsPlaying)
	{
		mLabelX -= dt * 35.0f;
		if ((mLabelX + mLabel->getWidth()) < 0)
		{
			mLabelX = mWidth;
		}
		mLabel->setPos(mLabelX, mLabel->getY());
	}
	else if (mLabel->getX() != mWidth)
	{
		mLabelX = mWidth;
		mLabel->setPos(mLabelX, mLabel->getY());
	}
}

void NowPlayingWindow::display()
{
}

std::vector<Sprite*> NowPlayingWindow::getSprites()
{
	return mSprites;
}
std::vector<IWidget*> NowPlayingWindow::getWidgets()
{
	return mWidgets;
}
bool NowPlayingWindow::onMouseEvent(MouseEvent ev)
{
	return false;
}

void NowPlayingWindow::onBlur()
{
}
void NowPlayingWindow::onFocus()
{
}

void NowPlayingWindow::updateCover(std::string &soundFile)
{
	AlbumArt *art = MetadataReader::getAlbumArt(soundFile.c_str());
	if (art != NULL)
	{
		IDirect3DTexture9* tex;
		HRESULT hr = D3DXCreateTextureFromFileInMemory(gDevice, art->data, art->length, &tex);
		if (hr != D3D_OK)
		{
			mAlbumArt->setTextureIndex(0);
		}
		else
		{
			mAlbumArt->addTexture(1, tex, true, true);
		}
		delete art;
	}
	else
	{
		mAlbumArt->setTextureIndex(0);
	}
}
void NowPlayingWindow::onSoundEvent(SoundManager::SoundEvent ev)
{
	switch (ev)
	{
	case SoundManager::START_EVENT:
		{
			Track currTrack;
			DatabaseManager::instance()->getTrack(SoundManager::instance()->getCurrFile(), &currTrack);
			if (mAlbumArt->getCurrentIndex() == 0 || (currTrack.Id != DatabaseStructs::INVALID_ID && 
				currTrack.AlbumId != DatabaseStructs::INVALID_ID &&
				mCurrentAlbum.Id != currTrack.AlbumId))
			{
				updateCover(currTrack.Filename);
			}
			string text = currTrack.Artist + " - " + currTrack.Title;
			mLabel->setString(text);
			mIsPlaying = true;
			DatabaseManager::instance()->getAlbum(currTrack.AlbumId, &mCurrentAlbum);
			
			break;
		} 
	case SoundManager::SOUND_FINISHED_EVENT:
	case SoundManager::STOP_EVENT:
		mIsPlaying = false;
		mLabelX = mWidth;
		break;
	}
}