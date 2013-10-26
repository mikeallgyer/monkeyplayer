// SmallAlbumItem.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays a single album and its tracks

#include <sstream>

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "SmallAlbumItem.h"
#include "Settings.h"
#include "SoundManager.h"
#include "WindowManager.h"

const int SmallAlbumItem::TRACK_FONT_HEIGHT = 20;
int SmallAlbumItem::TEXT_MARGIN_LEFT = 125;
int SmallAlbumItem::TEXT_MARGIN_RIGHT = 0;
int SmallAlbumItem::TEXT_MARGIN_TOP = 25;
int SmallAlbumItem::TEXT_MARGIN_BOTTOM = 0;
float SmallAlbumItem::SELECTION_BOX_MARGIN = 2.0f;
float SmallAlbumItem::TRACK_TIME_START = 10.0f;

ID3DXFont* SmallAlbumItem::mArtistFont = NULL;
ID3DXFont* SmallAlbumItem::mAlbumFont = NULL;
ID3DXFont* SmallAlbumItem::mTrackFont = NULL;
bool SmallAlbumItem::mFontLost = false;
bool SmallAlbumItem::mFontReset = false;

// used for synchronization
CCriticalSection SmallAlbumItem::mCritSection;

SmallAlbumItem::SmallAlbumItem(float x, float y, Album album,
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

	mAlbumCoverFile = FileManager::getContentAsset(std::string("Textures\\UnknownAlbum.jpg"));
	mAlbumCoverSprite = snew Sprite(mAlbumCoverFile.c_str(), 0, (float)TEXT_MARGIN_TOP, (float)mAlbumDimension, (float)mAlbumDimension);

	std::string selBoxPath = FileManager::getContentAsset(std::string("Textures\\selectedBox.png"));
	mSelectionSprite = snew Sprite(selBoxPath.c_str(), 0, 0, (float)mTrackTitleWidth + SELECTION_BOX_MARGIN, 
		(float)TRACK_FONT_HEIGHT + SELECTION_BOX_MARGIN);

	std::string selCDBoxPath = FileManager::getContentAsset(std::string("Textures\\cdHighlight.png"));
	mCDSelectionSprite = snew Sprite(selCDBoxPath.c_str(), 0, (float)TEXT_MARGIN_TOP, (float)mAlbumDimension, (float)mAlbumDimension);

	setAlbum(album);
	
	std::string whitePath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mHighlightedSprite = snew Sprite(whitePath.c_str(), 0, 0, mWidth, mHeight, D3DXVECTOR4(0.0f, 0.0f, 0.85f, 1.0f));

	mCurrSelection = -1;
	mAlbumSelected = false;

	mCallback = selectedtItemCB;
	mCallbackObj = callbackObj;

	mTarget = snew RenderTarget((int)mWidth, (int)mHeight, D3DXCOLOR(0, 0, 0, 0.2f), false);

	blur();
}
SmallAlbumItem::~SmallAlbumItem()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		delete mTracks[i];
	}
	delete mAlbumCoverSprite;
	delete mSelectionSprite;
	delete mCDSelectionSprite;
//	ReleaseCOM(mFont);

	delete mTarget;
	delete mHighlightedSprite;
}

void SmallAlbumItem::onDeviceLost()
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
void SmallAlbumItem::onDeviceReset()
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
void SmallAlbumItem::recreateTargets()
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
void SmallAlbumItem::update(float dt)
{
	bool selChanged = false;

	int cursorDelta = 0;

	if (getIsFocused())
	{	
	} // if focused

	mFontLost = mFontReset = false;
}

void SmallAlbumItem::preRender()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	if (mRedraw)
	{
		mTarget->beginScene();

		gWindowMgr->drawSprite(mAlbumCoverSprite, mWidth, mHeight);

		if (mAlbumSelected)
		{
			gWindowMgr->drawSprite(mCDSelectionSprite, mWidth, mHeight);
		}
		// title
		RECT r = { mAlbumTitleX, mAlbumTitleY, 
			mAlbumTitleX + (int)mTrackTitleWidth + (int)TRACK_TIME_START + mTrackTimeWidth, 
			mAlbumTitleY + (int)mHeight };
		HR(mAlbumFont->DrawText(0, mAlbum.Title.c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffaaaacd)));

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
		for (unsigned int i = 0; i < mTracks.size(); i++)
		{
			// draw selection box
			if (!mAlbumSelected && i == mCurrSelection)
			{
				mSelectionSprite->setDest(r.left - SELECTION_BOX_MARGIN, r.top - SELECTION_BOX_MARGIN, 
					mTrackTitleWidth + TRACK_TIME_START + mTrackTimeWidth + SELECTION_BOX_MARGIN +  SELECTION_BOX_MARGIN,
					TRACK_FONT_HEIGHT + SELECTION_BOX_MARGIN +  SELECTION_BOX_MARGIN);

				gWindowMgr->drawSprite(mSelectionSprite, mWidth, mHeight);
			}
			// draw track title
			str << (mTracks[i]->TrackNumber) << ". " << mTracks[i]->Title;
			HR(mTrackFont->DrawText(0, str.str().c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffffffff)));

			// draw time
			str.str("");
			r.left += mTrackTitleWidth + (int)TRACK_TIME_START;
			r.right = r.left + mTrackTimeWidth;
			str << SoundManager::getTimeString(mTracks[i]->Length * 1000);
			HR(mTrackFont->DrawText(0, str.str().c_str(), -1, &r, DT_LEFT, D3DXCOLOR(0xffffffff)));

			r.left = mTrackTitleX;
			r.right = r.left + mTrackTitleWidth;
			r.top += TRACK_FONT_HEIGHT;
			r.bottom += TRACK_FONT_HEIGHT;
			str.str("");
		}
		mTarget->endScene();
		mRedraw = false;
	}
	lock.Unlock();
}
void SmallAlbumItem::selectNext()
{
	mCurrSelection = min((int)mTracks.size() - 1, mCurrSelection + 1);
	mRedraw = true;
}
void SmallAlbumItem::selectPrevious()
{
	mCurrSelection = max(0, mCurrSelection - 1);
	mRedraw = true;
}
void SmallAlbumItem::selectNone()
{
	mCurrSelection = -1;
	mAlbumSelected = false;
	mRedraw = true;
}
void SmallAlbumItem::selectFirst()
{
	mCurrSelection = 0;
	mRedraw = true;
}
void SmallAlbumItem::selectLast()
{
	mCurrSelection = mTracks.size() - 1;
	mRedraw = true;
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
		y += TRACK_FONT_HEIGHT * mCurrSelection;
	}
}
void SmallAlbumItem::getSelectionBottomLeft(int &x, int &y)
{
	x = mTrackTitleX;
	y = mTrackTitleY;

	if (mCurrSelection > -1)
	{
		y += TRACK_FONT_HEIGHT * (mCurrSelection + 1);
	}
}

void SmallAlbumItem::setPos(float x, float y)
{
	mX = floor(x);
	mY = floor(y);

	if (mTargetSprite != NULL)
	{
		mTargetSprite->setDest(mX, mY, mWidth, mHeight);
	}
}

std::vector<Sprite*> SmallAlbumItem::getSprites()
{
	return mSprites;
}
int SmallAlbumItem::getNumTriangles()
{
	int total = 0;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		total += mSprites[i]->getNumTriangles();
	}

	return total;

}

bool SmallAlbumItem::onMouseEvent(MouseEvent e)
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
				if (selTrack < (int)mTracks.size())
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
bool SmallAlbumItem::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x2 = mX + mWidth;
	float y2 = mY + mHeight;

	return !(xPoint < mX || yPoint < mY || xPoint > x2 || yPoint > y2);
}
bool SmallAlbumItem::isPointInsideAlbum(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x1 = (float)mAlbumCoverSprite->getX() + mX;
	float y1 = (float)mAlbumCoverSprite->getY() + mY;
	float x2 = x1 + mAlbumCoverSprite->getWidth();
	float y2 = y1 + mAlbumCoverSprite->getHeight();

	return !(xPoint < x1 || yPoint < y1 || xPoint > x2 || yPoint > y2);
}

int SmallAlbumItem::getItemAtPos(int x, int y)
{
	int itemIndex = -1;
	int relX = x - (int)mX;
	int relY = y - (int)mY;


	if (relX >= mTrackTitleX && relY >= mTrackTitleY)
	{
		int selTrack = (relY - mTrackTitleY) / TRACK_FONT_HEIGHT;
		if (selTrack < (int)mTracks.size())
		{
			itemIndex = selTrack;
		}
	}
	return itemIndex;
}

void SmallAlbumItem::setAlbum(Album album)
{
	for (unsigned int i = 0; i < mTracks.size(); i++)
	{
		delete mTracks[i];
	}
	mGenreString = "";
	mYearString = "";
	mTracks.clear();
	mAlbum = album;
	mTracks = DatabaseManager::instance()->getTracks(mAlbum);
	getAlbumInfo();
}

void SmallAlbumItem::getAlbumInfo()
{
	// get cover art 
	AlbumArt *art = NULL;
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
	mHeight = (float)max((float)mAlbumDimension, mTrackTitleY + (float)mTracks.size() * TRACK_FONT_HEIGHT);

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
	mRedraw = true;
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
		mRedraw = true;
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