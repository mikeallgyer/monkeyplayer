// SmallAlbumTile.cpp
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
#include "SmallAlbumTile.h"
#include "Settings.h"
#include "SoundManager.h"
#include "WindowManager.h"

using namespace MonkeyPlayer;

const int SmallAlbumTile::TRACK_FONT_HEIGHT = 20;
int SmallAlbumTile::TEXT_MARGIN_LEFT = 125;
int SmallAlbumTile::TEXT_MARGIN_RIGHT = 0;
int SmallAlbumTile::TEXT_MARGIN_TOP = 25;
int SmallAlbumTile::TEXT_MARGIN_BOTTOM = 0;
float SmallAlbumTile::SELECTION_BOX_MARGIN = 2.0f;
float SmallAlbumTile::TRACK_TIME_START = 10.0f;
float SmallAlbumTile::MIN_HEIGHT = 200.0f;
float SmallAlbumTile::MAX_HEIGHT = 2048.0f;
float SmallAlbumTile::STD_WIDTH = 700.0f;

ID3DXFont* SmallAlbumTile::mArtistFont = NULL;
ID3DXFont* SmallAlbumTile::mAlbumFont = NULL;
ID3DXFont* SmallAlbumTile::mTrackFont = NULL;
bool SmallAlbumTile::mFontLost = false;
bool SmallAlbumTile::mFontReset = false;

Album SmallAlbumTile::mEmptyAlbum;
vector<Track*> SmallAlbumTile::mEmptyTracks;

// used for synchronization
CCriticalSection SmallAlbumTile::mCritSection;

SmallAlbumTile::SmallAlbumTile()
{
	init(0, 0);
}
SmallAlbumTile::SmallAlbumTile(float x, float y,
		void (*selectedtItemCB)(void* ptrObj, Track* selItem), void* callbackObj)
{
	init(x, y, selectedtItemCB, callbackObj);
}
void SmallAlbumTile::init(float x, float y,
		void (*selectedtItemCB)(void* ptrObj, Track* selItem), void* callbackObj)
{
	if (mArtistFont == NULL)
	{
		D3DXFONT_DESC font;
		font.Height = 26;
		font.Width = 0;
		font.Weight = 0;
		font.MipLevels = 1;
		font.Italic = false;
		font.CharSet = DEFAULT_CHARSET;
		font.OutputPrecision = OUT_DEFAULT_PRECIS;
		font.Quality = DEFAULT_QUALITY;
		font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		_tcscpy_s(font.FaceName, _T("Times New Roman"));

		HR(D3DXCreateFontIndirect(gDevice, &font, &mArtistFont));
	}
	if (mAlbumFont == NULL)
	{
		D3DXFONT_DESC font;
		font.Height = 26;
		font.Width = 0;
		font.Weight = 0;
		font.MipLevels = 1;
		font.Italic = false;
		font.CharSet = DEFAULT_CHARSET;
		font.OutputPrecision = OUT_DEFAULT_PRECIS;
		font.Quality = DEFAULT_QUALITY;
		font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		_tcscpy_s(font.FaceName, _T("Times New Roman"));

		HR(D3DXCreateFontIndirect(gDevice, &font, &mAlbumFont));
	}
	if (mTrackFont == NULL)
	{
		D3DXFONT_DESC font;
		font.Height = 20;
		font.Width = 0;
		font.Weight = 0;
		font.MipLevels = 1;
		font.Italic = false;
		font.CharSet = DEFAULT_CHARSET;
		font.OutputPrecision = OUT_DEFAULT_PRECIS;
		font.Quality = DEFAULT_QUALITY;
		font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		_tcscpy_s(font.FaceName, _T("Times New Roman"));

		HR(D3DXCreateFontIndirect(gDevice, &font, &mTrackFont));
	}

	mAlbumDimension = 150;

	mAlbumTitleX = mAlbumDimension + 50;
	mAlbumTitleY = TEXT_MARGIN_TOP;
	mTrackTitleX = mAlbumTitleX;
	mTrackTitleY = 30 + TEXT_MARGIN_TOP;

	mTrackTitleWidth = 300  + TEXT_MARGIN_TOP;;
	mTrackTimeWidth = 70;
	mTrackRatingWidth = 50;

	mX = floor(x);
	mY = floor(y);

	mTargetSprite = NULL;

	mRedraw = true;
	mTryTexture = true;
	mTextureFailed = false;

//	getAlbum()CoverFile = FileManager::getContentAsset(std::string("Textures\\UnknownAlbum.jpg"));
	mAlbumCoverSprite = snew Sprite(0, (float)TEXT_MARGIN_TOP, (float)mAlbumDimension, (float)mAlbumDimension);
	mAlbumCoverSprite->setTextureOwned(false);

	std::string selBoxPath = FileManager::getContentAsset(std::string("Textures\\selectedBox.png"));
	mSelectionSprite = snew Sprite(selBoxPath.c_str(), 0, 0, (float)mTrackTitleWidth + SELECTION_BOX_MARGIN, 
		(float)TRACK_FONT_HEIGHT + SELECTION_BOX_MARGIN);

	std::string selCDBoxPath = FileManager::getContentAsset(std::string("Textures\\cdHighlight.png"));
	mCDSelectionSprite = snew Sprite(selCDBoxPath.c_str(), 0, (float)TEXT_MARGIN_TOP, (float)mAlbumDimension, (float)mAlbumDimension);

	setAlbumItem(NULL);
	
	std::string whitePath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mHighlightedSprite = snew Sprite(whitePath.c_str(), 0, 0, mWidth, getHeight(), D3DXVECTOR4(0.0f, 0.0f, 0.85f, 1.0f));

	mCurrSelection = -1;
	mAlbumSelected = false;

	mCallback = selectedtItemCB;
	mCallbackObj = callbackObj;

	mTarget = snew RenderTarget((int)mWidth, (int)getHeight(), D3DXCOLOR(0, 0, 0, 0.2f), false);

	blur();
}

SmallAlbumTile::~SmallAlbumTile()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}

	if (mAlbumItem != NULL)
	{
		// shouuld be done in manager
		//delete mAlbumItem;
	}
	delete mAlbumCoverSprite;
	delete mSelectionSprite;
	delete mCDSelectionSprite;
//	ReleaseCOM(mFont);

	delete mTarget;
	delete mHighlightedSprite;
}

void SmallAlbumTile::onDeviceLost()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	if (!mFontLost)
	{
		HR(mArtistFont->OnLostDevice());
		HR(mAlbumFont->OnLostDevice());
		HR(mTrackFont->OnLostDevice());
		mFontLost = true;
	}
	mTarget->onDeviceLost();
	mHighlightedSprite->onDeviceLost();
	mSelectionSprite->onDeviceLost();
	mCDSelectionSprite->onDeviceLost();
}
void SmallAlbumTile::onDeviceReset()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	if (!mFontReset)
	{
		HR(mArtistFont->OnResetDevice());
		HR(mAlbumFont->OnResetDevice());
		HR(mTrackFont->OnResetDevice());
		mFontReset = true;
	}
	mRedraw = true;
	mTarget->onDeviceReset();
	mHighlightedSprite->onDeviceReset();
	mSelectionSprite->onDeviceReset();
	mCDSelectionSprite->onDeviceReset();
	recreateTargets();
}
void SmallAlbumTile::recreateTargets()
{
	if (mTargetSprite == NULL)
	{
		mTargetSprite = snew Sprite(mTarget->getTexture(), mX, mY, mWidth, getHeight());
		mSprites.push_back(mTargetSprite);
	}
	else
	{
		mTarget->setDimensions((int)mWidth, (int)getHeight());
		mTargetSprite->setDest(mX, mY, mWidth, getHeight());
		mTargetSprite->replaceCurrentTexture(mTarget->getTexture(), false);
	}

}
void SmallAlbumTile::update(float dt)
{
	bool selChanged = false;

	int cursorDelta = 0;

	if (getIsFocused())
	{	
	} // if focused

	mFontLost = mFontReset = false;
}

void SmallAlbumTile::preRender()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	if (mRedraw || (!mTextureFailed && mTryTexture))
	{
		mTarget->beginScene();

		if (mAlbumItem != NULL)
		{
			// don't keep trying if it didn't work before
			if (!mTextureFailed)
			{
				mAlbumCoverSprite->replaceCurrentTexture(AlbumTextureManager::instance()->getTexture(getAlbum().Id, mTryTexture), false);
			
				mTextureFailed = AlbumTextureManager::instance()->isTextureBad(getAlbum().Id);
			}

			gWindowMgr->drawSprite(mAlbumCoverSprite, mWidth, getHeight());

			if (mAlbumSelected)
			{
				gWindowMgr->drawSprite(mCDSelectionSprite, mWidth, getHeight());
			}
			// title
			RECT r = { mAlbumTitleX, mAlbumTitleY, 
				mAlbumTitleX + (int)mTrackTitleWidth + (int)TRACK_TIME_START + mTrackTimeWidth, 
				mAlbumTitleY + (int)getHeight() };
			HR(mAlbumFont->DrawText(0, getAlbum().Title.c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffaaaacd)));

			// genre
			r.left = (int)mAlbumCoverSprite->getX();
			r.right = r.top + 150;
			r.top = (int)(mAlbumCoverSprite->getX() + mAlbumCoverSprite->getHeight()) + 10 +  + TEXT_MARGIN_TOP;;
			r.bottom = r.top + 50;
			HR(mTrackFont->DrawText(0, mGenreString.c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffffffff)));

			// year
			r.top += TRACK_FONT_HEIGHT;
			r.bottom = r.top + 50;
			HR(mTrackFont->DrawText(0, mYearString.c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffffffff)));

			// tracks
			r.left = mTrackTitleX;
			r.top = mTrackTitleY;
			r.right = r.left + mTrackTitleWidth;
			r.bottom = r.top + TRACK_FONT_HEIGHT;
			std::stringstream str;
			for (unsigned int i = 0; i < getTracks().size(); i++)
			{
				// draw selection box
				if (!mAlbumSelected && i == mCurrSelection)
				{
					mSelectionSprite->setDest(r.left - SELECTION_BOX_MARGIN, r.top - SELECTION_BOX_MARGIN, 
						mTrackTitleWidth + TRACK_TIME_START + mTrackTimeWidth + SELECTION_BOX_MARGIN +  SELECTION_BOX_MARGIN,
						TRACK_FONT_HEIGHT + SELECTION_BOX_MARGIN +  SELECTION_BOX_MARGIN);

					gWindowMgr->drawSprite(mSelectionSprite, mWidth, getHeight());
				}
				// draw track title
				str << (getTracks()[i]->TrackNumber) << ". " << getTracks()[i]->Title;
				HR(mTrackFont->DrawText(0, str.str().c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffffffff)));

				// draw time
				str.str("");
				r.left += mTrackTitleWidth + (int)TRACK_TIME_START;
				r.right = r.left + mTrackTimeWidth;
				str << SoundManager::getTimeString(getTracks()[i]->Length * 1000);
				HR(mTrackFont->DrawText(0, str.str().c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffffffff)));

				r.left = mTrackTitleX;
				r.right = r.left + mTrackTitleWidth;
				r.top += TRACK_FONT_HEIGHT;
				r.bottom += TRACK_FONT_HEIGHT;
				str.str("");
			}
		}
		mTarget->endScene();
		mRedraw = false;
	}
	lock.Unlock();
}
void SmallAlbumTile::selectNext()
{
	mCurrSelection = min((int)getTracks().size() - 1, mCurrSelection + 1);
	if (mAlbumItem != NULL) mAlbumItem->selectNext();
	mRedraw = true;
}
void SmallAlbumTile::selectPrevious()
{
	mCurrSelection = max(0, mCurrSelection - 1);
	if (mAlbumItem != NULL) mAlbumItem->selectPrevious();
	mRedraw = true;
}
void SmallAlbumTile::selectNone()
{
	mCurrSelection = -1;
	mAlbumSelected = false;
	if (mAlbumItem != NULL) mAlbumItem->selectNone();
	mRedraw = true;
}
void SmallAlbumTile::selectFirst()
{
	mCurrSelection = 0;
	if (mAlbumItem != NULL) mAlbumItem->selectFirst();
	mRedraw = true;
}
void SmallAlbumTile::selectLast()
{
	mCurrSelection = getTracks().size() - 1;
	if (mAlbumItem != NULL) mAlbumItem->selectLast();
	mRedraw = true;
}
void SmallAlbumTile::setSelectedIndex(int index)
{
	mCurrSelection = max(0, min(getTracks().size() - 1, index));
	mRedraw = true;
}
bool SmallAlbumTile::isFirstSelected()
{
	return mCurrSelection == 0;
}
bool SmallAlbumTile::isLastSelected()
{
	return mCurrSelection == (getTracks().size() - 1);
}
int SmallAlbumTile::getSelectedIndex()
{
	return mCurrSelection;
}
void SmallAlbumTile::getSelectionTopLeft(int &x, int &y)
{
	x = mTrackTitleX;
	y = mTrackTitleY;

	if (mCurrSelection > -1)
	{
		y += TRACK_FONT_HEIGHT * mCurrSelection;
	}
}
void SmallAlbumTile::getSelectionBottomLeft(int &x, int &y)
{
	x = mTrackTitleX;
	y = mTrackTitleY;

	if (mCurrSelection > -1)
	{
		y += TRACK_FONT_HEIGHT * (mCurrSelection + 1);
	}
}

void SmallAlbumTile::setPos(float x, float y)
{
	mX = floor(x);
	mY = floor(y);

	if (mTargetSprite != NULL)
	{
		mTargetSprite->setDest(mX, mY, mWidth, getHeight());
	}
	if (mAlbumItem != NULL) mAlbumItem->setPos(x, y);
}

std::vector<Sprite*> SmallAlbumTile::getSprites()
{
	return mSprites;
}
int SmallAlbumTile::getNumTriangles()
{
	int total = 0;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		total += mSprites[i]->getNumTriangles();
	}

	return total;

}

bool SmallAlbumTile::onMouseEvent(MouseEvent e)
{
	if (getIsFocused()) //isPointInside(e.getX(), e.getY()))
	{
		if ((e.getEvent() == MouseEvent::LBUTTONDOWN ||
			 e.getEvent() == MouseEvent::RBUTTONDOWN || e.getEvent() == MouseEvent::RBUTTONUP ||
			 e.getEvent() == MouseEvent::LBUTTONDBLCLK) && isPointInside(e.getX(), e.getY()))
		{
			int relX = e.getX() - (int)mX;
			int relY = e.getY() - (int)mY;


			if (relX >= mTrackTitleX && relY >= mTrackTitleY)
			{
				int selTrack = (relY - mTrackTitleY) / TRACK_FONT_HEIGHT;
				if (selTrack < (int)getTracks().size())
				{
					mAlbumSelected = false;
					mCurrSelection = selTrack;
					mRedraw = true;
					return true;
				}
			}
			else if (isPointInsideAlbum(e.getX(), e.getY()))
			{
				mAlbumSelected = true;
				mRedraw = true;
				return true;
			}
		}
	}
	return false;
}
bool SmallAlbumTile::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x2 = mX + mWidth;
	float y2 = mY + getHeight();

	return !(xPoint < mX || yPoint < mY || xPoint > x2 || yPoint > y2);
}
bool SmallAlbumTile::isPointInsideAlbum(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x1 = (float)mAlbumCoverSprite->getX() + mX;
	float y1 = (float)mAlbumCoverSprite->getY() + mY;
	float x2 = x1 + mAlbumCoverSprite->getWidth();
	float y2 = y1 + mAlbumCoverSprite->getHeight();

	return !(xPoint < x1 || yPoint < y1 || xPoint > x2 || yPoint > y2);
}

int SmallAlbumTile::getItemAtPos(int x, int y)
{
	int itemIndex = -1;
	int relX = x - (int)mX;
	int relY = y - (int)mY;


	if (relX >= mTrackTitleX && relY >= mTrackTitleY)
	{
		int selTrack = (relY - mTrackTitleY) / TRACK_FONT_HEIGHT;
		if (selTrack < (int)getTracks().size())
		{
			itemIndex = selTrack;
		}
	}
	return itemIndex;
}

// does NOT take ownership
void SmallAlbumTile::setAlbumItem(SmallAlbumItem* albumItem)
{
	mAlbumItem = albumItem;
	recalculateSize();
	if (mAlbumItem != NULL)
	{
		recreateTargets();
	}
}
SmallAlbumItem* SmallAlbumTile::getAlbumItem()
{
	return mAlbumItem;
}

void SmallAlbumTile::recalculateSize()
{
	mWidth = STD_WIDTH;
	if (mAlbumItem != NULL)
	{
		mAlbumItem->recalculateSize();
	}
}
Track* SmallAlbumTile::getSelectedTrack()
{
	if (mCurrSelection >= 0 && mCurrSelection < (int)getTracks().size())
	{
		return getTracks()[mCurrSelection];
	}
	return NULL;
}
bool SmallAlbumTile::getAlbumSelected()
{
	return mAlbumSelected;
}
void SmallAlbumTile::setAlbumSelected(bool sel)
{
	mAlbumSelected = sel;
	mAlbumItem->setAlbumSelected(sel);
	mRedraw = true;
}
void SmallAlbumTile::addTrack(Track *track)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mCurrSelection = -1;
	mRedraw = true;

	lock.Unlock();
}