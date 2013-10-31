// SmallAlbumManager.cpp
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
#include "MusicLibrary.h"
#include "Settings.h"
#include "SmallAlbumManager.h"
#include "SoundManager.h"
#include "Vertex.h"

using namespace MonkeyPlayer;

// when holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
const float SmallAlbumManager::BUTTON_REPEAT_TIME = .05f;
// upon first holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
const float SmallAlbumManager::BUTTON_REPEAT_DELAY = .5f;
// number of items to scroll when using page up/page down
const int SmallAlbumManager::NUM_PAGING_ITEMS = 10;
// magic scroll speed
const float SmallAlbumManager::SCROLL_SPEED = .2f;
const float SmallAlbumManager::ARTIST_LABEL_SIZE = 26.0f;

// used for synchronization
CCriticalSection SmallAlbumManager::mCritSection;

SmallAlbumManager::SmallAlbumManager()
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

	mCurrWidth = 60;

	mUpDownTimer = 0;
	mPageTimer = 0;
	mHoldDelayPassed = false;

	mHasFocus = false;

	std::vector<Album*> albums = DatabaseManager::instance()->getAllAlbums();
	
	for (unsigned int i = 0; i < albums.size(); i++)
	{
		mSmallItems.push_back(snew SmallAlbumItem(0, 150, *albums[i]));
		delete albums[i];
	}

	mCurrDisplayAlbum = 0.0f;
	mCurrSelAlbum = -1;

	mGoToChar = false;
	mChar = 'A';

	mGoToSong = false;
	mGoToAlbumId = -1;
	mGoToSongId = -1;

	mDoRedraw = false;
	mAlbumsChanged = false;

	mSelectedThing = CollectionWindow::ALBUM;
}
SmallAlbumManager::~SmallAlbumManager()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
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
	lock.Unlock();
}

void SmallAlbumManager::onDeviceLost()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
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
	lock.Unlock();
}
void SmallAlbumManager::onDeviceReset()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
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
	lock.Unlock();
}
int SmallAlbumManager::getWidth()
{
	return (int)mCurrWidth;
}

int SmallAlbumManager::getHeight()
{
	return (int)mCurrHeight;
}
bool SmallAlbumManager::getAlbumsChanged()
{
	bool retVal = mAlbumsChanged;
	mAlbumsChanged = false;
	return retVal;
}

void SmallAlbumManager::setPos(float x, float y, float width, float height)
{
	mCurrX = x;
	mCurrY = y;
	mCurrWidth = (int)width;
	mCurrHeight = (int)height;
	mDoRedraw = true;
}
void SmallAlbumManager::update(float dt)
{
	bool doRedraw = mDoRedraw;
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
		mResized = false;
		doRedraw = true;
	}

	bool selChanged = false;

	if (mGoToChar)
	{
		for (unsigned int i = 0; i < mSmallItems.size(); i++)
		{
			if (mSmallItems[i]->getAlbum().Artist.length() > 0 &&
				mSmallItems[i]->getAlbum().Artist[0] == mChar)
			{
				mUpDownTimer = 0;
				if (mCurrSelAlbum >= 0 && mCurrSelAlbum < ((int)mSmallItems.size()))
				{
					mSmallItems[mCurrSelAlbum]->selectNone();
				}
				mSmallItems[i]->selectFirst();
				mCurrSelAlbum = i;
				
				float oldDisplayAlbum = mCurrDisplayAlbum;
				// moveUp is faster, so try that first
				moveUpToSmallSelection();

				// if that didn't move it
				if (oldDisplayAlbum == mCurrDisplayAlbum)
				{
					moveDownToSmallSelection();
				}
				doRedraw = true;
				break;
			}
		}
		mGoToChar = false;
		mAlbumsChanged = true;
	}
	else if (mGoToSong)
	{
		for (unsigned int i = 0; i < mSmallItems.size(); i++)
		{
			if (mSmallItems[i]->getAlbum().Id == mGoToAlbumId)
			{
				mUpDownTimer = 0;
				if (mCurrSelAlbum >= 0 && mCurrSelAlbum < ((int)mSmallItems.size()))
				{
					mSmallItems[mCurrSelAlbum]->selectNone();
				}
				mSmallItems[i]->selectFirst();
				mCurrSelAlbum = i;
				
				float oldDisplayAlbum = mCurrDisplayAlbum;
				// moveUp is faster, so try that first
				moveUpToSmallSelection();

				for (unsigned int j = 0; j < mSmallItems[j]->getTracks().size(); j++)
				{
					if (mSmallItems[i]->getTracks()[j]->Id == mGoToSongId)
					{
						break;
					}
					mSmallItems[i]->selectNext();;
				}
				moveDownToSmallSelection();
				doRedraw = true;
				mDoHighlight = false;
				break;
			}
		}
		mGoToSong = false;
		mGoToAlbumId = -1;
		mGoToSongId = -1;
		mAlbumsChanged = true;
	}

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
		// left/right
		if (gInput->keyPressed(VK_RIGHT) || gInput->keyPressed(VK_LEFT))
		{
			if (mCurrSelAlbum >= 0)
			{
				mSmallItems[mCurrSelAlbum]->setAlbumSelected(!mSmallItems[mCurrSelAlbum]->getAlbumSelected());
			}
		} //user continuing to hold down

		// home/end
		if (gInput->keyPressed(VK_END))
		{
			mUpDownTimer = 0;
			if (mCurrSelAlbum >= 0 && mCurrSelAlbum < ((int)mSmallItems.size()))
			{
				mSmallItems[mCurrSelAlbum]->selectNone();
			}
			mSmallItems[mSmallItems.size() - 1]->selectLast();
			mCurrSelAlbum = mSmallItems.size() - 1;
			moveDownToSmallSelection();
			doRedraw = true;
			//cursorDelta = mItems.size();
		}
		if (gInput->keyPressed(VK_HOME))
		{
			mUpDownTimer = 0;
			if (mCurrSelAlbum >= 0 && mCurrSelAlbum < ((int)mSmallItems.size()))
			{
				mSmallItems[mCurrSelAlbum]->selectNone();
			}
			mSmallItems[0]->selectFirst();
			mCurrSelAlbum = 0;
			moveUpToSmallSelection();
			doRedraw = true;
			//cursorDelta = -(int)mItems.size();
		}

		// enter
		if (gInput->keyPressed(VK_RETURN))
		{
			if (mCurrSelAlbum >= 0)
			{
				if (mSmallItems[mCurrSelAlbum]->getAlbumSelected())
				{
					vector<Track*> tracks = mSmallItems[mCurrSelAlbum]->getTracks();
					MusicLibrary::instance()->getPlaylistWindow()->clearItems();
					for (unsigned int i = 0; i < tracks.size(); i++)
					{
						MusicLibrary::instance()->getPlaylistWindow()->addItem(snew Track(*tracks[i]));
					}
					MusicLibrary::instance()->playSong(tracks[0]->Filename);
				}
				else
				{
					SoundManager::instance()->playFile(
						mSmallItems[mCurrSelAlbum]->getSelectedTrack()->Filename.c_str());
				}
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
		moveSmallSelection(cursorDelta);
		doRedraw = true;
	}
	if (doRedraw)
	{
		updateSmallDisplay();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}
}

void SmallAlbumManager::moveSmallSelection(int cursorDelta)
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
					if (mSmallItems[mCurrSelAlbum]->getAlbumSelected())
					{
						if (mCurrSelAlbum < ((int)mSmallItems.size() - 1))
						{
							mSmallItems[mCurrSelAlbum]->selectNone();
							mCurrSelAlbum++;
							mSmallItems[mCurrSelAlbum]->selectFirst();
							mSmallItems[mCurrSelAlbum]->setAlbumSelected(true);
						}
					}
					else if (mSmallItems[mCurrSelAlbum]->isLastSelected())
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
					if (mSmallItems[mCurrSelAlbum]->getAlbumSelected())
					{
						if (mCurrSelAlbum > 0)
						{
							mSmallItems[mCurrSelAlbum]->selectNone();
							mCurrSelAlbum--;
							mSmallItems[mCurrSelAlbum]->selectFirst();
							mSmallItems[mCurrSelAlbum]->setAlbumSelected(true);
						}
					}
					else if (mSmallItems[mCurrSelAlbum]->isFirstSelected())
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

void SmallAlbumManager::moveUpToSmallSelection()
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
void SmallAlbumManager::moveDownToSmallSelection()
{
	float newDisplayAlbum = mCurrDisplayAlbum;
	if (mCurrSelAlbum >= 0 && mCurrSelAlbum < (int)mSmallItems.size())
	{
		int x = 0, y = 0;
		mSmallItems[mCurrSelAlbum]->getSelectionBottomLeft(x, y); // relative to top-left of album item
		// work backwards from that track...note, if y / getHeight() == 1, back up a bit
		newDisplayAlbum = (float)mCurrSelAlbum + min(.999f, ((float)y / mSmallItems[mCurrSelAlbum]->getHeight()));

		// if top of album down to this track fits
		if ((mSmallItems[(int)newDisplayAlbum]->getHeight() - (float)y) < mCurrHeight)
		{
			// go to top of album
			newDisplayAlbum = (float)mCurrSelAlbum;
			float currHeight = (float)y;
			// if we're not at the top, AND the currently displayed plus the next album fits 
			while (newDisplayAlbum > 0 && 
					 (currHeight + mSmallItems[(int)newDisplayAlbum - 1]->getHeight()) < mCurrHeight)
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
				float remainingSpace = mCurrHeight - currHeight;
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
void SmallAlbumManager::display()
{
}
int SmallAlbumManager::getNumTriangles()
{
	return 0;
}
std::vector<Sprite*> SmallAlbumManager::getSprites()
{
	return mSprites;
}
std::vector<IWidget*> SmallAlbumManager::getWidgets()
{
	return mWidgets;
}

bool SmallAlbumManager::onMouseEvent(MouseEvent ev)
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
			updateSmallDisplay();

			lock.Unlock();
			return true;
		}
	} // if mousewheel
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
	}// if mousedown
	
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		if ((isPointInside(ev.getX(), ev.getY()) || ev.getEvent() == MouseEvent::MOUSEMOVE ||
			ev.getEvent() == MouseEvent::MOUSEWHEEL) && mWidgets[i]->onMouseEvent(ev))
		{
			// check if this widget is a SmallAlbumItem
			if ((ev.getEvent() == MouseEvent::LBUTTONDOWN || 
				ev.getEvent() == MouseEvent::LBUTTONDBLCLK ||
				ev.getEvent() == MouseEvent::RBUTTONDOWN ||
				ev.getEvent() == MouseEvent::RBUTTONUP))
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
							if (clickedItem->getAlbumSelected())
							{
								vector<Track*> tracks = clickedItem->getTracks();
								MusicLibrary::instance()->getPlaylistWindow()->clearItems();
								for (unsigned int i = 0; i < tracks.size(); i++)
								{
									MusicLibrary::instance()->getPlaylistWindow()->addItem(snew Track(*tracks[i]));
								}
								MusicLibrary::instance()->playNextSong();
							}
							else if (clickedItem->getSelectedTrack() != NULL)
							{
								SoundManager::instance()->playFile(
									clickedItem->getSelectedTrack()->Filename.c_str());
							}
						}
						else if (ev.getEvent() == MouseEvent::RBUTTONUP)
						{
							mRightClickedTrack = clickedItem->getItemAtPos(ev.getX(), ev.getY());
							if (clickedItem->isPointInsideAlbum(ev.getX(), ev.getY()))
							{
								mRightClicked = CollectionWindow::RIGHT_ALBUM;
								vector<ListItem*> items;
								items.push_back(snew SimpleListItem("Play All Now", CollectionWindow::ALBUM_PLAY_IMMEDIATE));
								items.push_back(snew SimpleListItem("Replace Queue", CollectionWindow::ALBUM_REPLACE_QUEUE));
								items.push_back(snew SimpleListItem("Queue All Next", CollectionWindow::ALBUM_QUEUE_NEXT));
								items.push_back(snew SimpleListItem("Append All to Queue", CollectionWindow::ALBUM_QUEUE_END));

								gWindowMgr->openContextMenu((float)ev.getX(), (float)ev.getY(), items, this);
								mRightClickedAlbum = clickedItem;
								consumed = true;
							}
							else if (mRightClickedTrack >= 0)
							{
								mRightClicked = CollectionWindow::RIGHT_TRACK;
								vector<ListItem*> items;
								items.push_back(snew SimpleListItem("Play Now", CollectionWindow::TRACK_PLAY_IMMEDIATE));
					///			items.push_back(snew SimpleListItem("Replace Queue", TRACK_REPLACE_QUEUE));
								items.push_back(snew SimpleListItem("Queue Next", CollectionWindow::TRACK_QUEUE_NEXT));
								items.push_back(snew SimpleListItem("Append to Queue", CollectionWindow::TRACK_QUEUE_END));

								gWindowMgr->openContextMenu((float)ev.getX(), (float)ev.getY(), items, this);
								mRightClickedAlbum = clickedItem;
								consumed = true;
							}
						}
						break;
					}
				}
				catch (std::bad_cast e) {}
			}
		}
		// queue artist
		if ((ev.getEvent() == MouseEvent::LBUTTONDBLCLK || ev.getEvent() == MouseEvent::RBUTTONDOWN)
			&& mWidgets[i]->isPointInside(ev.getX(), ev.getY()))
		{
			SimpleLabel* clickedLabel = dynamic_cast<SimpleLabel*>(mWidgets[i]);
			if (clickedLabel != NULL)
			{
				if (ev.getEvent() == MouseEvent::LBUTTONDBLCLK)
				{
					MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithArtist(clickedLabel->getString());
					MusicLibrary::instance()->getPlaylistWindow()->playNextSong();
				}
				else
				{
					mRightClicked = CollectionWindow::RIGHT_ARTIST;
					vector<ListItem*> items;
					items.push_back(snew SimpleListItem("Play All Now", CollectionWindow::ARTIST_PLAY_IMMEDIATE));
					items.push_back(snew SimpleListItem("Replace Queue", CollectionWindow::ARTIST_REPLACE_QUEUE));
					items.push_back(snew SimpleListItem("Queue All Next", CollectionWindow::ARTIST_QUEUE_NEXT));
					items.push_back(snew SimpleListItem("Append All to Queue", CollectionWindow::ARTIST_QUEUE_END));

					gWindowMgr->openContextMenu((float)ev.getX(), (float)ev.getY(), items, this);
					mRightClickedArtist = clickedLabel->getString();
					consumed = true;
				}
				break;
			}
		}
	}
	return consumed;
}
bool SmallAlbumManager::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x2 = mCurrX + mCurrWidth;
	float y2 = mCurrY + mCurrHeight;

	return !(xPoint < mCurrX || yPoint < mCurrY || xPoint > x2 || yPoint > y2);
}

void SmallAlbumManager::onBlur()
{
	mHasFocus = false;
}

void SmallAlbumManager::onFocus()
{
	mHasFocus = true;
}
void SmallAlbumManager::goToChar(char c)
{
	mGoToChar = true;
	mChar = c;
}

void SmallAlbumManager::goToSong(Album a, Track t, bool doHighlight)
{
	mGoToAlbumId = a.Id;
	mGoToSongId = t.Id;
	mGoToSong = true;
	mDoHighlight = doHighlight;
}

void SmallAlbumManager::updateSmallDisplay()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mWidgets.clear();
	
	int topIndex = 0;
	int bottomIndex = topIndex;
	float fractPart = 0;
	getDrawableRange(topIndex, bottomIndex, fractPart);

	if (topIndex <= bottomIndex)
	{
		// position
		float currX = mCurrX + 20.0f;
		float currY = mCurrY;
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
					SimpleLabel* label = NULL;
					if (currLabelIndex < (int)mArtistLabels.size())
					{
						label = mArtistLabels[currLabelIndex];
					}
					else
					{
						label = snew SimpleLabel(currX, currY, (float)mCurrWidth, ARTIST_LABEL_SIZE,
							currArtist, (int)ARTIST_LABEL_SIZE, DT_LEFT, D3DXCOLOR(0xffaaaaef), D3DXCOLOR(0xff000000));
						mArtistLabels.push_back(label);
					}
					label->setPos(currX, currY, (float)mCurrWidth, 26);
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
	mAlbumsChanged = true;
	mDoRedraw = false;
	lock.Unlock();
}
void SmallAlbumManager::getDrawableRange(int &topIndex, int &bottomIndex, float &fractPart)
{
	float intPart = 0;

	fractPart = modf(mCurrDisplayAlbum, &intPart);
	topIndex = (int)intPart;

	if (topIndex < 0)
	{
		topIndex = 0;
	}

	float albumHeight = 0;
	bottomIndex = topIndex;

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
}
void SmallAlbumManager::onContextMenuSelected(ItemListBox* menu)
{
	int sel = menu->getSelectedItem()->getId();

	// tracks
	if (sel == CollectionWindow::TRACK_PLAY_IMMEDIATE)
	{
		MusicLibrary::instance()->playSong(mRightClickedAlbum->getTracks()[mRightClickedTrack]->Filename);
	}
	else if (sel == CollectionWindow::TRACK_QUEUE_END)
	{
		MusicLibrary::instance()->getPlaylistWindow()->addTrackToQueueEnd(mRightClickedAlbum->getTracks()[mRightClickedTrack]->Id);
	}
	else if (sel == CollectionWindow::TRACK_QUEUE_NEXT)
	{
		MusicLibrary::instance()->getPlaylistWindow()->insertTrackToQueueNext(mRightClickedAlbum->getTracks()[mRightClickedTrack]->Id);
	}
	else if (sel == CollectionWindow::TRACK_REPLACE_QUEUE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithTrack(mRightClickedAlbum->getTracks()[mRightClickedTrack]->Id);
	}
	// artist
	else if (sel == CollectionWindow::ARTIST_PLAY_IMMEDIATE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithArtist(mRightClickedArtist);
		MusicLibrary::instance()->playNextSong();
	}
	else if (sel == CollectionWindow::ARTIST_QUEUE_END)
	{
		MusicLibrary::instance()->getPlaylistWindow()->addArtistToQueueEnd(mRightClickedArtist);
	}
	else if (sel == CollectionWindow::ARTIST_QUEUE_NEXT)
	{
		MusicLibrary::instance()->getPlaylistWindow()->insertArtistToQueueNext(mRightClickedArtist);
	}
	else if (sel == CollectionWindow::ARTIST_REPLACE_QUEUE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithArtist(mRightClickedArtist);
	}

	// Album
	else if (sel == CollectionWindow::ALBUM_PLAY_IMMEDIATE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithAlbum(mRightClickedAlbum->getAlbum());
		MusicLibrary::instance()->playNextSong();
	}
	else if (sel == CollectionWindow::ALBUM_QUEUE_END)
	{
		MusicLibrary::instance()->getPlaylistWindow()->addAlbumToQueueEnd(mRightClickedAlbum->getAlbum());
	}
	else if (sel == CollectionWindow::ALBUM_QUEUE_NEXT)
	{
		MusicLibrary::instance()->getPlaylistWindow()->insertAlbumToQueueNext(mRightClickedAlbum->getAlbum());
	}
	else if (sel == CollectionWindow::ALBUM_REPLACE_QUEUE)
	{
		MusicLibrary::instance()->getPlaylistWindow()->replaceQueueWithAlbum(mRightClickedAlbum->getAlbum());
	}
}
void SmallAlbumManager::addAlbum(Album *album)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mAlbumsToAdd.push_back(snew Album(album->Id, album->NumTracks, album->Title, album->Year, album->Artist));
	lock.Unlock();
}
void SmallAlbumManager::doAddAlbum(Album *album)
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
				if (mSmallItems[j]->getAlbum().Title == album->Title)
				{
					insertIndex = -1;
					break;
				}
				else if (mSmallItems[j]->getAlbum().Title > album->Title)
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

	mAlbumsChanged = true; 
}

void SmallAlbumManager::addTrack(Track* track)
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
void SmallAlbumManager::doAddTrack(Track* track)
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
