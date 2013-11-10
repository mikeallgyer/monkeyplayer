// CollectionWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a collection of songs
#include <math.h>
#include <vector>

#include "d3dApp.h"
#include "CollectionWindow.h"
#include "Settings.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "MusicLibrary.h"
#include "Settings.h"
#include "SmallAlbumManager.h"
#include "Vertex.h"

using namespace MonkeyPlayer;

const float CollectionWindow::ARTIST_LABEL_SIZE = 26.0f;

const int CollectionWindow::TRACK_PLAY_IMMEDIATE = 0;
const int CollectionWindow::TRACK_QUEUE_NEXT = 1;
const int CollectionWindow::TRACK_QUEUE_END = 2;
const int CollectionWindow::TRACK_REPLACE_QUEUE = 3;

const int CollectionWindow::ALBUM_PLAY_IMMEDIATE = 4;
const int CollectionWindow::ALBUM_QUEUE_NEXT = 5;
const int CollectionWindow::ALBUM_QUEUE_END = 6;
const int CollectionWindow::ALBUM_REPLACE_QUEUE = 7;

const int CollectionWindow::ARTIST_PLAY_IMMEDIATE = 8;
const int CollectionWindow::ARTIST_QUEUE_NEXT = 9;
const int CollectionWindow::ARTIST_QUEUE_END = 10;
const int CollectionWindow::ARTIST_REPLACE_QUEUE = 11;

// magic scroll speed
const float CollectionWindow::SCROLL_SPEED = .2f;

// used for synchronization
CCriticalSection CollectionWindow::mCritSection;

CollectionWindow::CollectionWindow()
{
	D3DXFONT_DESC font;
	font.Height = 16;
	font.Width = 0;
	font.Weight = 0;
	font.MipLevels = 1;
	font.Italic = false;
	font.CharSet = DEFAULT_CHARSET;
	font.OutputPrecision = OUT_DEFAULT_PRECIS;
	font.Quality = DEFAULT_QUALITY;
	font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(font.FaceName, _T("Times New Roman"));

	HR(D3DXCreateFontIndirect(gDevice, &font, &mFont));

	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\white.png"));

	mX = 0;
	mY = 0;
	mCurrWidth = 60;

	mUpDownTimer = 0;
	mPageTimer = 0;
	mHoldDelayPassed = false;

	mHasFocus = false;

	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mCurrWidth, 300.0f,
		D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));

	mSprites.push_back(mBackground);

	mAlphabetLabel = snew SimpleLabel(0, 0, 200.0f, 40.0f, string("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), 30, DT_CENTER | DT_NOCLIP,
		D3DCOLOR_XRGB(200, 200, 200), D3DCOLOR_ARGB(0, 0, 0, 0), "COURIER NEW");
	mAlphabetLabel->setSizeToFit(true);
	mLetterLabel = snew SimpleLabel(-1000.0f, 0, 80.0f, 80.0f, string("A"), 40, DT_CENTER | DT_NOCLIP,
		D3DCOLOR_XRGB(255, 255, 255), D3DCOLOR_ARGB(0, 0, 0, 0), "COURIER NEW");
	mLetterLabel->setSizeToFit(true);

	mWidgets.push_back(mAlphabetLabel);
	mWidgets.push_back(mLetterLabel);

	mSmallAlbumManager = snew SmallAlbumManager();
	mLargeAlbumWidget = snew LargeAlbumWidget(0, 0, 100.0f, 100.0f);
//	mWidgets.push_back(mLargeAlbumWidget);

	std::string magPath = FileManager::getContentAsset(std::string("Textures\\small_empty.png"));
	mMagnifier = snew Button(-1000.0f, 0, 40.0f, 40.0f, magPath);
	mWidgets.push_back(mMagnifier);	

	mGoToSongChk = snew Checkbox(0, 0, "Go to current song", chk_callback, this);
	mGoToSongChk->setChecked(Settings::instance()->getBoolValue(Settings::OPT_GO_TO_SONG, false));

	mWidgets.push_back(mGoToSongChk);

	std::string smallBtnPath = FileManager::getContentAsset(std::string("Textures\\small_album.png"));
	std::string smallBtnHoverPath = FileManager::getContentAsset(std::string("Textures\\small_album_hover.png"));
	std::string smallBtnDownPath = FileManager::getContentAsset(std::string("Textures\\small_album_down.png"));

	std::string largeBtnPath = FileManager::getContentAsset(std::string("Textures\\large_album.png"));
	std::string largeBtnHoverPath = FileManager::getContentAsset(std::string("Textures\\large_album_hover.png"));
	std::string largeBtnDownPath = FileManager::getContentAsset(std::string("Textures\\large_album_down.png"));

	int displayType = Settings::instance()->getIntValue(Settings::OPT_DISPLAY_TYPE, SmallAlbum);

	mSmallAlbumBtn = snew Button(0, 0, 40.0f, 40.0f, smallBtnPath, btn_callback, this);
	mSmallAlbumBtn->setHoverTexture(smallBtnHoverPath.c_str());
	mSmallAlbumBtn->setDownTexture(smallBtnDownPath.c_str());
	mSmallAlbumBtn->setToggledTexture(smallBtnDownPath.c_str());
	mSmallAlbumBtn->setIsToggle(true);
	mSmallAlbumBtn->setToggled(displayType == SmallAlbum);

	mLargeAlbumBtn = snew Button(0, 0, 40.0f, 40.0f, largeBtnPath, btn_callback, this);
	mLargeAlbumBtn->setHoverTexture(largeBtnHoverPath.c_str());
	mLargeAlbumBtn->setDownTexture(largeBtnDownPath.c_str());
	mLargeAlbumBtn->setToggledTexture(largeBtnDownPath.c_str());
	mLargeAlbumBtn->setIsToggle(true);
	mLargeAlbumBtn->setToggled(displayType == LargeAlbum);


	mWidgets.push_back(mSmallAlbumBtn);
	mWidgets.push_back(mLargeAlbumBtn);

	mCurrStyle = UNDEFINED_STYLE;
	setDisplayStyle((DISPLAY_STYLE)displayType);
	mMouseAlphabetStartedDown = false;
	mGoToHover = false;

	SoundManager::instance()->addCallback(sound_callback, this);

	int index = Settings::instance()->getIntValue(Settings::LAST_ALBUM_VIEWED, -1);
	Album album;
	album.Id = index;
	Track t;
	mLargeAlbumWidget->goToSong(album, t, false);
	mSmallAlbumManager->goToSong(album, t, false);
}
CollectionWindow::~CollectionWindow()
{
	int index = mSmallAlbumManager->getCurrentAlbum();
	if (mCurrStyle == LargeAlbum)
	{
		index = mLargeAlbumWidget->getCurrentAlbum();
	}
	Settings::instance()->setValue(Settings::LAST_ALBUM_VIEWED, index); 
	SoundManager::instance()->removeCallback(this);
	ReleaseCOM(mFont);

	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mArtistLabels.size(); i++)
	{
		delete mArtistLabels[i];
	}
	delete mAlphabetLabel;
	delete mLetterLabel;
	delete mMagnifier;
	delete mGoToSongChk;
	delete mSmallAlbumManager;
	delete mLargeAlbumWidget;
	delete mSmallAlbumBtn;
	delete mLargeAlbumBtn;
}

void CollectionWindow::onDeviceLost()
{
	HR(mFont->OnLostDevice());
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mArtistLabels.size(); i++)
	{
		mArtistLabels[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceLost();
	}
	mLargeAlbumWidget->onDeviceLost();
	mSmallAlbumManager->onDeviceLost();
}
void CollectionWindow::onDeviceReset()
{
	HR(mFont->OnResetDevice());
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mArtistLabels.size(); i++)
	{
		mArtistLabels[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceReset();
	}
	mLargeAlbumWidget->onDeviceReset();
	mSmallAlbumManager->onDeviceReset();
	mResized = true;
}
int CollectionWindow::getWidth()
{
	return (int)mCurrWidth;
}

int CollectionWindow::getHeight()
{
	return (int)mBackground->getHeight();
}

void CollectionWindow::update(float dt)
{
	bool doRedraw = false;
	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mCurrWidth = gWindowMgr->getMainContentWidth();
		mY = (float)gWindowMgr->getMainContentTop();
		int height = r.bottom - (int)mY - 
			(r.bottom - gWindowMgr->getMainContentBottom());
		mBackground->setDest(0, (int)mY, mCurrWidth, height);

		mAlphabetLabel->setPos(mCurrWidth / 2.0f - 100.0f, (float)gWindowMgr->getMainContentTop(), 200.0f, 30.0f);
		mLetterLabel->setPos(-1000.0f, -100.0f, 80.0f, 80.0f);
		mMagnifier->setPos(-1000.0f, -1000.0f);
		mGoToSongChk->setPos(mCurrWidth - 200.0f, mY + 75.0f, 100.0f);

		mSmallAlbumBtn->setPos(mCurrWidth - 90.0f, (float)gWindowMgr->getMainContentTop() + 20.0f);
		mLargeAlbumBtn->setPos(mCurrWidth - 150.0f, (float)gWindowMgr->getMainContentTop() + 20.0f);

		mSmallAlbumManager->setPos(0, (float)gWindowMgr->getMainContentTop(), (float)mCurrWidth, (float)height);
		mLargeAlbumWidget->setPos(0, (float)gWindowMgr->getMainContentTop(), (float)mCurrWidth, (float)height);

		mResized = false;
		doRedraw = true;
	}
	bool selChanged = false;

	int cursorDelta = 0;

	if (mHasFocus)
	{
		if (mGoToHover)
		{
			if (mCurrStyle == SmallAlbum)
			{
				mSmallAlbumManager->goToChar(mHoverChar);
			}
			else if (mCurrStyle == LargeAlbum)
			{
				mLargeAlbumWidget->goToChar(mHoverChar);
			}
			mGoToHover = false;
		}
	}
	mAlphabetLabel->update(dt);
	mLetterLabel->update(dt);
	mMagnifier->update(dt);
	mGoToSongChk->update(dt);
	mSmallAlbumManager->update(dt);
	mLargeAlbumWidget->update(dt);

	if (mCurrStyle == SmallAlbum)
	{
		if (mSmallAlbumManager->getAlbumsChanged())
		{
			setDrawableWidgets();
		}
	}
	else if (mCurrStyle == LargeAlbum)
	{
	}
}

void CollectionWindow::display()
{
}
std::vector<Sprite*> CollectionWindow::getSprites()
{
	return mSprites;
}
std::vector<IWidget*> CollectionWindow::getWidgets()
{
	return mWidgetsToDraw;
}

bool CollectionWindow::onMouseEvent(MouseEvent ev)
{
	bool consumed = false;
	if (mHasFocus || ev.getEvent() == MouseEvent::LBUTTONDOWN ||
		ev.getEvent() == MouseEvent::RBUTTONDOWN)
	{
		if (mCurrStyle == SmallAlbum)
		{
			mSmallAlbumManager->onMouseEvent(ev);
		}
		if (mCurrStyle == LargeAlbum)
		{
			mLargeAlbumWidget->onMouseEvent(ev);
		}
	} // if mousewheel
	// if clicked, give widget focus
	if (ev.getEvent() == MouseEvent::LBUTTONDOWN ||
		ev.getEvent() == MouseEvent::RBUTTONDOWN)
	{
		if (isPointInside(ev.getX(), ev.getY()))
		{
			mHasFocus = true;
			consumed = true;
			bool found = false;
			for (unsigned int i = 0; i < mWidgets.size(); i++)
			{
				if (!found && mWidgets[i]->isPointInside(ev.getX(), ev.getY()))
				{
					found = true;
					mWidgets[i]->focus();
				}
				else if (!mWidgets[i]->getIsFocused())
				{
					mWidgets[i]->blur();
				}
			}
			if (mCurrStyle == SmallAlbum && !found && mSmallAlbumManager->isPointInside(ev.getX(), ev.getY()))
			{
				found = true;
				mSmallAlbumManager->onFocus();
			}
			else
			{
				mSmallAlbumManager->onBlur();
			}
		}
		if (mAlphabetLabel->isPointInside(ev.getX(), ev.getY()))
		{
			mMouseAlphabetStartedDown = true;
		}
		else
		{
			mMouseAlphabetStartedDown = false;
		}
	}// if mousedown
	
	// go to letter
	if (ev.getEvent() == MouseEvent::LBUTTONUP && mMouseAlphabetStartedDown &&
		mAlphabetLabel->isPointInside(ev.getX(), ev.getY()))
	{
		mGoToHover = true;
	}

	if (ev.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (mAlphabetLabel->isPointInside(ev.getX(), ev.getY()))
		{
			int index = (int)(((ev.getX() - mAlphabetLabel->getX()) / (mAlphabetLabel->getWidth())) * 26.0f);
			mHoverChar = (char)(65 + index);
			mLetterLabel->setString(string(1, mHoverChar));
			mLetterLabel->setPos(mAlphabetLabel->getX() + (float)index * mAlphabetLabel->getWidth() / 26.0f - 10.0f,
				mAlphabetLabel->getY(), 80.0f, 80.0f);
			mMagnifier->setPos(mLetterLabel->getX() - 9.0f, mLetterLabel->getY() + 0.0f);
		}
		else if (mLetterLabel->getX() >= 0) // hide
		{
			mLetterLabel->setPos(-1000.0f, mAlphabetLabel->getY() - 10.0f, 80.0f, 80.0f);
			mMagnifier->setPos(-1000.0f, -1000.0f);
		}

	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onMouseEvent(ev);
	}
	
	return consumed;
}
bool CollectionWindow::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x2 = mBackground->getX() + mBackground->getWidth();
	float y2 = mBackground->getY() + mBackground->getHeight();

	return !(xPoint < mBackground->getX() || yPoint < mBackground->getY()
		|| xPoint > x2 || yPoint > y2);
}

void CollectionWindow::onBlur()
{
	mHasFocus = false;
	mSmallAlbumManager->onBlur();
	mLargeAlbumWidget->blur();
}

void CollectionWindow::onFocus()
{
	mHasFocus = true;
	if (mCurrStyle == SmallAlbum)
	{
		mSmallAlbumManager->onFocus();
	}
	else if (mCurrStyle == LargeAlbum)
	{
		mLargeAlbumWidget->focus();
	}
}

void CollectionWindow::setDisplayStyle(DISPLAY_STYLE style)
{
	if (style != mCurrStyle)
	{
		mCurrStyle = style;
		if (mCurrStyle == SmallAlbum)
		{
			mSmallAlbumManager->updateSmallDisplay();
			if (mHasFocus)
			{
				mSmallAlbumManager->onFocus();
			}
			mLargeAlbumWidget->blur();
		}
		else if (mCurrStyle == LargeAlbum)
		{
			if (mHasFocus)
			{
				mLargeAlbumWidget->focus();
			}
			mSmallAlbumManager->onBlur();
		}

		setDrawableWidgets();
		Settings::instance()->setValue(Settings::OPT_DISPLAY_TYPE, mCurrStyle);
	}
}
void CollectionWindow::addAlbum(Album *album)
{
	mSmallAlbumManager->addAlbum(album);
	mLargeAlbumWidget->addAlbum(album);
}
void CollectionWindow::addTrack(Track* track)
{
	mSmallAlbumManager->addTrack(track);
	mLargeAlbumWidget->addTrack(track);
}
void CollectionWindow::setDrawableWidgets()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mWidgetsToDraw.clear();

	if (mCurrStyle == SmallAlbum)
	{
		vector<IWidget*> widgets = mSmallAlbumManager->getWidgets();
		for (unsigned int i = 0; i < widgets.size(); i++)
		{
			mWidgetsToDraw.push_back(widgets[i]);
		}
	}
	else if (mCurrStyle == LargeAlbum)
	{
		if (mHasFocus)
		{
			mLargeAlbumWidget->focus();
		}
		mSmallAlbumManager->onBlur();

		mWidgetsToDraw.push_back(mLargeAlbumWidget);
		for (unsigned int i = 0; i < mLargeAlbumWidget->getWidgets().size(); i++)
		{
			mWidgetsToDraw.push_back(mLargeAlbumWidget->getWidgets()[i]);
		}
	}

	mWidgetsToDraw.push_back(mAlphabetLabel);
	mWidgetsToDraw.push_back(mMagnifier);
	mWidgetsToDraw.push_back(mLetterLabel);
	mWidgetsToDraw.push_back(mGoToSongChk);
	mWidgetsToDraw.push_back(mSmallAlbumBtn);
	mWidgetsToDraw.push_back(mLargeAlbumBtn);
	lock.Unlock();
}

void CollectionWindow::onBtnPushed(Button* btn)
{
	if (btn == mSmallAlbumBtn)
	{
		if (btn->getToggled())
		{
			mLargeAlbumBtn->setToggled(false);
			setDisplayStyle(SmallAlbum);
		}
		else // disable "clicking it off"
		{
			btn->setToggled(true);
		}
	}
	else if (btn == mLargeAlbumBtn)
	{
		if (btn->getToggled())
		{
			mSmallAlbumBtn->setToggled(false);
			setDisplayStyle(LargeAlbum);
		}
		else // disable "clicking it off"
		{
			btn->setToggled(true);
		}
	}
}
void CollectionWindow::goToSong()
{
	if (SoundManager::instance()->isPlaying())
	{
		string file = SoundManager::instance()->getCurrFile();

		if (file.length() > 0)
		{
			Track track;
			Album album;
			DatabaseManager::instance()->getTrack(file, &track);
			if (track.Id != DatabaseStructs::INVALID_ID)
			{
				DatabaseManager::instance()->getAlbum(track.AlbumId, &album);

				if (album.Id != DatabaseStructs::INVALID_ID)
				{
					mLargeAlbumWidget->goToSong(album, track);
					mSmallAlbumManager->goToSong(album, track);
				}
			}
		}
	}
}
void CollectionWindow::goToSong(string file)
{
	if (file.length() > 0)
	{
		Track track;
		Album album;
		DatabaseManager::instance()->getTrack(file, &track);
		if (track.Id != DatabaseStructs::INVALID_ID)
		{
			DatabaseManager::instance()->getAlbum(track.AlbumId, &album);

			if (album.Id != DatabaseStructs::INVALID_ID)
			{
				mLargeAlbumWidget->goToSong(album, track, false);
				mSmallAlbumManager->goToSong(album, track);
			}
		}
	}
}
void CollectionWindow::onChkPushed(Checkbox* chk)
{
	if (chk == mGoToSongChk)
	{
		if (mGoToSongChk->getChecked())
		{
			goToSong();
		}
		Settings::instance()->setValue(Settings::OPT_GO_TO_SONG, mGoToSongChk->getChecked());
	}
}
void CollectionWindow::onSoundEvent(SoundManager::SoundEvent ev)
{
	if (mGoToSongChk->getChecked() && ev == SoundManager::START_EVENT)
	{
		goToSong();
	}
}
