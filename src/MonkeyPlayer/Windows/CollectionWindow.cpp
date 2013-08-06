// CollectionWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a collection of songs
#include <math.h>
#include <vector>

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "CollectionWindow.h"
#include "Settings.h"
#include "SoundManager.h"
#include "Vertex.h"

// when holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
const float CollectionWindow::BUTTON_REPEAT_TIME = .05f;
// upon first holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
const float CollectionWindow::BUTTON_REPEAT_DELAY = .5f;
// number of items to scroll when using page up/page down
const int CollectionWindow::NUM_PAGING_ITEMS = 10;
// magic scroll speed
const float CollectionWindow::SCROLL_SPEED = .2f;
const float CollectionWindow::ARTIST_LABEL_SIZE = 26.0f;

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

	mCurrWidth = 60;

	mUpDownTimer = 0;
	mPageTimer = 0;
	mHoldDelayPassed = false;

	mHasFocus = false;

	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mCurrWidth, 300.0f,
		D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));

	mSprites.push_back(mBackground);

	mCurrStyle = SmallAlbum;

	std::vector<Album*> albums = DatabaseManager::instance()->getAllAlbums();
	
	for (unsigned int i = 0; i < albums.size(); i++)
	{
		mSmallItems.push_back(snew SmallAlbumItem(0, 150, *albums[i]));
		delete albums[i];
	}
	mCurrStyle = UNDEFINED;

	mCurrDisplayAlbum = 0.0f;
	mCurrSelAlbum = -1;
	setDisplayStyle(SmallAlbum);
}
CollectionWindow::~CollectionWindow()
{
	ReleaseCOM(mFont);

	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mSmallItems.size(); i++)
	{
		delete mSmallItems[i];
	}
	for (unsigned int i = 0; i < mArtistLabels.size(); i++)
	{
		delete mArtistLabels[i];
	}
}

void CollectionWindow::onDeviceLost()
{
	HR(mFont->OnLostDevice());
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mSmallItems.size(); i++)
	{
		mSmallItems[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mArtistLabels.size(); i++)
	{
		mArtistLabels[i]->onDeviceLost();
	}
}
void CollectionWindow::onDeviceReset()
{
	HR(mFont->OnResetDevice());
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mSmallItems.size(); i++)
	{
		mSmallItems[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mArtistLabels.size(); i++)
	{
		mArtistLabels[i]->onDeviceReset();
	}
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
		doRedraw = true;
	}
	mTracksToAdd.clear();
	lock.Unlock();

	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mCurrWidth = gWindowMgr->getMainContentWidth();
		int height = r.bottom - gWindowMgr->getMainContentTop() - 
			(r.bottom - gWindowMgr->getMainContentBottom());
		mBackground->setDest(0, gWindowMgr->getMainContentTop(), mCurrWidth, height);

		mResized = false;
		doRedraw = true;
	}
	bool selChanged = false;

	int cursorDelta = 0;

	if (mHasFocus)
	{	
		if ((gInput->isKeyDown(VK_UP) || gInput->isKeyDown(VK_DOWN) ||
			gInput->isKeyDown(VK_NEXT) || gInput->isKeyDown(VK_PRIOR)))
		{
			// check initial delay
			if (!mHoldDelayPassed)
			{
				mUpDownTimer += dt;
				if (mUpDownTimer >= BUTTON_REPEAT_DELAY)
				{
					mHoldDelayPassed = true;
					// undo, because it's added below
					mUpDownTimer -= dt;
				}
			}
		}
		else
		{
			mUpDownTimer = 0;
			mHoldDelayPassed = false;
		}

		// arrow key
		// user just started pressing
		if (gInput->keyPressed(VK_DOWN))
		{
			cursorDelta = 1;
		} //user continuing to hold down
		else if (mHoldDelayPassed && gInput->isKeyDown(VK_DOWN))
		{
			mUpDownTimer += dt;
			if (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				cursorDelta = 1;
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
			while (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
		} 
		else if (gInput->keyPressed(VK_UP))
		{
			cursorDelta = -1;
		}//user continuing to hold down
		else if (mHoldDelayPassed && gInput->isKeyDown(VK_UP))
		{
			mUpDownTimer += dt;
			if (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				cursorDelta = -1;
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
			while (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
		}
		// page up/down
		// user just started pressing
		if (gInput->keyPressed(VK_NEXT))
		{
			cursorDelta = NUM_PAGING_ITEMS;
		} //user continuing to hold down
		else if (mHoldDelayPassed && gInput->isKeyDown(VK_NEXT))
		{
			mUpDownTimer += dt;
			if (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				cursorDelta = NUM_PAGING_ITEMS;
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
			while (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
		} 
		else if (gInput->keyPressed(VK_PRIOR))
		{
			cursorDelta = -NUM_PAGING_ITEMS;
		}//user continuing to hold down
		else if (mHoldDelayPassed && gInput->isKeyDown(VK_PRIOR))
		{
			mUpDownTimer += dt;
			if (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				cursorDelta = -NUM_PAGING_ITEMS;
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
			while (mUpDownTimer > BUTTON_REPEAT_TIME)
			{
				mUpDownTimer -= BUTTON_REPEAT_TIME;
			}
		}

		// home/end
		if (gInput->keyPressed(VK_END))
		{
			mUpDownTimer = 0;
			//cursorDelta = mItems.size();
		}
		if (gInput->keyPressed(VK_HOME))
		{
			mUpDownTimer = 0;
			//cursorDelta = -(int)mItems.size();
		}

		// enter
		if (gInput->keyPressed(VK_RETURN))
		{
			if (mCurrSelAlbum >= 0)
			{
				SoundManager::instance()->playFile(
					mSmallItems[mCurrSelAlbum]->getSelectedTrack()->Filename.c_str());
			}
		}
		if (cursorDelta != 0)
		{
			selChanged = true;
		}
	} // if focused

	// update list of items to display
	if (selChanged)
	{
		if (mCurrStyle == SmallAlbum)
		{
			moveSmallSelection(cursorDelta);
		}
		doRedraw = true;
	}
	if (doRedraw)
	{
		if (mCurrStyle == SmallAlbum)
		{
			updateSmallDisplay();
		}
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}
}

void CollectionWindow::moveSmallSelection(int cursorDelta)
{
	int incrementor = 1;
	bool movingDown = cursorDelta > 0;
	if (movingDown)
	{
		incrementor = -1;
	}
	while (cursorDelta != 0)
	{
		if (mSmallItems.size() >= 0)
		{
			if (cursorDelta > 0)
			{
				if (mCurrSelAlbum < 0)
				{
					mSmallItems[0]->selectFirst();
					mCurrSelAlbum = 0;
				}
				else 
				{
					if (mSmallItems[mCurrSelAlbum]->isLastSelected())
					{
						if (mCurrSelAlbum < ((int)mSmallItems.size() - 1))
						{
							mSmallItems[mCurrSelAlbum]->selectNone();
							mCurrSelAlbum++;
							mSmallItems[mCurrSelAlbum]->selectFirst();
						}
					}
					else 
					{
						mSmallItems[mCurrSelAlbum]->selectNext();
					}
				}
			}
			else
			{
				if (mCurrSelAlbum >= 0)
				{
					if (mSmallItems[mCurrSelAlbum]->isFirstSelected())
					{
						if (mCurrSelAlbum > 0)
						{
							mSmallItems[mCurrSelAlbum]->selectNone();
							mCurrSelAlbum--;
							mSmallItems[mCurrSelAlbum]->selectLast();
						}
					}
					else 
					{
						mSmallItems[mCurrSelAlbum]->selectPrevious();
					}
				}
			}
		}
		cursorDelta += incrementor;
	}
	float oldDisplayAlbum = mCurrDisplayAlbum;
	// moveUp is faster, so try that first
	moveUpToSmallSelection();

	// if that didn't move it
	if (oldDisplayAlbum == mCurrDisplayAlbum)
	{
		moveDownToSmallSelection();
	}
}

void CollectionWindow::moveUpToSmallSelection()
{
	float newDisplayAlbum = mCurrDisplayAlbum;
	if (mCurrSelAlbum >= 0 && mCurrSelAlbum < (int)mSmallItems.size())
	{
		int x = 0, y = 0;
		mSmallItems[mCurrSelAlbum]->getSelectionTopLeft(x, y); // relative to top-left of album item
		newDisplayAlbum = (float)mCurrSelAlbum + (y - ARTIST_LABEL_SIZE)
			/ mSmallItems[mCurrSelAlbum]->getHeight();
		// only move up if the selection is higher than what is visible
		if (newDisplayAlbum < mCurrDisplayAlbum)
		{
			mCurrDisplayAlbum = newDisplayAlbum;
		}
	}
}
void CollectionWindow::moveDownToSmallSelection()
{
	float newDisplayAlbum = mCurrDisplayAlbum;
	if (mCurrSelAlbum >= 0 && mCurrSelAlbum < (int)mSmallItems.size())
	{
		int x = 0, y = 0;
		mSmallItems[mCurrSelAlbum]->getSelectionBottomLeft(x, y); // relative to top-left of album item
		// work backwards from that track...note, if y / getHeight() == 1, back up a bit
		newDisplayAlbum = (float)mCurrSelAlbum + min(.999f, ((float)y / mSmallItems[mCurrSelAlbum]->getHeight()));

		// if top of album down to this track fits
		if ((mSmallItems[(int)newDisplayAlbum]->getHeight() - (float)y) < mBackground->getHeight())
		{
			// go to top of album
			newDisplayAlbum = (float)mCurrSelAlbum;
			float currHeight = (float)y;
			// if we're not at the top, AND the currently displayed plus the next album fits 
			while (newDisplayAlbum > 0 && 
					 (currHeight + mSmallItems[(int)newDisplayAlbum - 1]->getHeight()) < mBackground->getHeight())
			{
				currHeight += mSmallItems[(int)newDisplayAlbum - 1]->getHeight();
				newDisplayAlbum -= 1.0f;
			}
			if (newDisplayAlbum < 0)
			{
				newDisplayAlbum = 0;
			}
			else if (newDisplayAlbum >= 1.0f)
			{
				newDisplayAlbum -= 1.0f;
				float remainingSpace = mBackground->getHeight() - currHeight;
				float percentCurAlbum = remainingSpace / mSmallItems[(int)newDisplayAlbum]->getHeight();
				newDisplayAlbum += (1.0f - percentCurAlbum);
			}
		}
		// only move down if the selection is lower than what is visible
		if (newDisplayAlbum > mCurrDisplayAlbum)
		{
			mCurrDisplayAlbum = newDisplayAlbum;
		}
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
	return mWidgets;
}

bool CollectionWindow::onMouseEvent(MouseEvent ev)
{
	bool consumed = false;
	if(mHasFocus)
	{
		if (ev.getEvent() == MouseEvent::MOUSEWHEEL)
		{
			CSingleLock lock(&mCritSection);
			lock.Lock();
			// update list of items to display
			if (ev.getExtraHiData() < 0)
			{
				mCurrDisplayAlbum += SCROLL_SPEED;
				mCurrDisplayAlbum = min((float)mSmallItems.size() - .1f, mCurrDisplayAlbum);
			}
			else
			{
				mCurrDisplayAlbum -= SCROLL_SPEED;
				mCurrDisplayAlbum = max(0, mCurrDisplayAlbum);
			}
			if (mCurrStyle == SmallAlbum)
			{
				updateSmallDisplay();
			}

			lock.Unlock();
			return true;
		}
	}
	// if clicked, give widget focus
	if (ev.getEvent() == MouseEvent::LBUTTONDOWN ||
		ev.getEvent() == MouseEvent::RBUTTONDOWN)
	{
		if (isPointInside(ev.getX(), ev.getY()))
		{
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
		}
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		if ((isPointInside(ev.getX(), ev.getY()) || ev.getEvent() == MouseEvent::MOUSEMOVE ||
			ev.getEvent() == MouseEvent::MOUSEWHEEL) && mWidgets[i]->onMouseEvent(ev))
		{
			// check if this widget is a SmallAlbumItem
			if ((ev.getEvent() == MouseEvent::LBUTTONDOWN || 
				ev.getEvent() == MouseEvent::LBUTTONDBLCLK ||
				ev.getEvent() == MouseEvent::RBUTTONDOWN) && mCurrStyle == SmallAlbum)
			{
				try 
				{
					SmallAlbumItem* clickedItem = dynamic_cast<SmallAlbumItem*>(mWidgets[i]);
					if (clickedItem != NULL)
					{
						// find clicked item in our list
						for (int j  = (int)mCurrDisplayAlbum; j < (int)mSmallItems.size(); j++)
						{
							if (mSmallItems[j] == clickedItem && j != mCurrSelAlbum)
							{
								if (mCurrSelAlbum != -1)
								{
									mSmallItems[mCurrSelAlbum]->selectNone();
								}
								mCurrSelAlbum = j;
							}
						}
						if (ev.getEvent() == MouseEvent::LBUTTONDBLCLK)
						{
							SoundManager::instance()->playFile(
								clickedItem->getSelectedTrack()->Filename.c_str());
						}
					}
				}
				catch (std::bad_cast e) {}
			}
			break;
		}
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
}

void CollectionWindow::onFocus()
{
	mHasFocus = true;
}

void CollectionWindow::updateSmallDisplay()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mWidgets.clear();
	float fractPart;
	float intPart = 0;

	fractPart = modf(mCurrDisplayAlbum, &intPart);
	int topIndex = (int)intPart;

	if (topIndex < 0)
	{
		topIndex = 0;
	}

	float albumHeight = 0;
	int bottomIndex = topIndex;

	while (bottomIndex >= 0 && bottomIndex < (int)mSmallItems.size() && albumHeight < getHeight())
	{
		albumHeight += mSmallItems[bottomIndex]->getHeight();
		bottomIndex++;
	}

	// + 1 so we can partially draw the one after
	if (bottomIndex >= (int)mSmallItems.size())
	{
		bottomIndex--;
	}

	if (topIndex <= bottomIndex)
	{
		// position
		float currX = mBackground->getX() + 20.0f;
		float currY = mBackground->getY();
		std::string currArtist = "";
		int currLabelIndex = 0;
		for (int i = topIndex; i <= bottomIndex; i++)
		{
			if (mSmallItems[i]->getTracks().size() > 0)
			{
				std::string artist = mSmallItems[i]->getTracks()[0]->Artist;
				if (artist != currArtist)
				{
					currArtist = artist;
					Label* label = NULL;
					if (currLabelIndex < (int)mArtistLabels.size())
					{
						label = mArtistLabels[currLabelIndex];
					}
					else
					{
						label = snew Label(currX, currY, mBackground->getWidth(), ARTIST_LABEL_SIZE,
							currArtist, (int)ARTIST_LABEL_SIZE, DT_LEFT, D3DXCOLOR(0xffaaaaef), D3DXCOLOR(0xff000000));
						mArtistLabels.push_back(label);
					}
					label->setPos(currX, currY, mBackground->getWidth(), 26);
					label->setString(currArtist);
					mWidgets.push_back(label);
					currLabelIndex++;
	//				currY += label->getHeight();
				}
				if (i == topIndex)
				{
					currY -= mSmallItems[i]->getHeight() * fractPart;
				}
				mSmallItems[i]->setPos(currX, currY);
				mWidgets.insert(mWidgets.begin(), mSmallItems[i]);
				currY += mSmallItems[i]->getHeight();
			}
		}
	}
	lock.Unlock();
}
void CollectionWindow::setDisplayStyle(DISPLAY_STYLE style)
{
	if (style != mCurrStyle)
	{
		mCurrStyle = style;
		switch (mCurrStyle)
		{
		case SmallAlbum:
			updateSmallDisplay();
			break;
		case LargeAlbum:
			break;
		}
	}
}
void CollectionWindow::addAlbum(Album *album)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mAlbumsToAdd.push_back(snew Album(album->Id, album->NumTracks, album->Title, album->Year, album->Artist));
	lock.Unlock();
}
void CollectionWindow::doAddAlbum(Album *album)
{
	int insertIndex = 0;
	vector<SmallAlbumItem*>::iterator iter = mSmallItems.begin();
	for (unsigned int i = 0; i < mSmallItems.size(); i++)
	{
		if (mSmallItems[i]->getAlbum().Id == album->Id)
		{
			insertIndex = -1;
			break;
		}
		else if (mSmallItems[i]->getAlbum().Artist == album->Artist)
		{
			insertIndex = i;
			for (unsigned int j = i; j < mSmallItems.size() && mSmallItems[j]->getAlbum().Artist == album->Artist; j++)
			{
				if (mSmallItems[j]->getAlbum().Title > album->Artist)
				{
					insertIndex = j;
					break;
				}
				iter++;
			}
			break;
		}
		else if (mSmallItems[i]->getAlbum().Artist >= album->Artist)
		{
			insertIndex = i;
			break;
		}
		iter++;
	}

	if (insertIndex >= 0)
	{
		mSmallItems.insert(iter, snew SmallAlbumItem(0, 150, *album));
	}
}

void CollectionWindow::addTrack(Track* track)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();

	if (track->AlbumId > 0)
	{
		bool foundAlbum = false;
		for (unsigned int i = 0; i < mSmallItems.size(); i++)
		{
			if (mSmallItems[i]->getAlbum().Id == track->AlbumId)
			{
				foundAlbum = true;
			}
		}
		if (!foundAlbum)
		{
			Album album;
			DatabaseManager::instance()->getAlbum(track->AlbumId, &album);
			if (album.Id > 0)
			{
				mAlbumsToAdd.push_back(snew Album(album.Id, album.NumTracks, album.Title, album.Year, album.Artist));
			}
		}
		mTracksToAdd.push_back(snew Track(*track));
	}
	lock.Unlock();
}
void CollectionWindow::doAddTrack(Track* track)
{
	int insertIndex = 0;
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
}
