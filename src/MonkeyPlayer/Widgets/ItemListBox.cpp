// ItemListBox.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A box with selectable items
#include <algorithm>    // std::random_shuffle

#include "d3dApp.h"
#include "ItemListBox.h"
#include "Settings.h"
#include "WindowManager.h"

using namespace MonkeyPlayer;

// when holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
const float ItemListBox::BUTTON_REPEAT_TIME = .05f;
// upon first holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
const float ItemListBox::BUTTON_REPEAT_DELAY = .5f;
// number of items to scroll when using page up/page down
const int ItemListBox::NUM_PAGING_ITEMS = 10;
// number of items to scroll when using scroll wheel
const int ItemListBox::NUM_SCROLLING_ITEMS = 5;

const int ItemListBox::TEXT_MARGIN_TOP = 0;
const int ItemListBox::TEXT_MARGIN_BOTTOM = 0;
const int ItemListBox::TEXT_MARGIN_LEFT = 5;
const int ItemListBox::TEXT_MARGIN_RIGHT = 25;

const float ItemListBox::HOVER_DURATION = 0.5f;

// used for synchronization
CCriticalSection ItemListBox::mCritSection;

ItemListBox::ItemListBox(float x, float y, float width, float height,
		void (*selectedtItemCB)(void* ptrObj, ItemListBox* selItem), void* callbackObj,
		D3DXCOLOR bgColor, D3DXCOLOR fontColor, D3DXVECTOR4 highlightColor)
{
	D3DXFONT_DESC font;
	font.Height = 16;
	font.Width = 0;
	font.Weight = FW_BOLD;
	font.MipLevels = 1;
	font.Italic = false;
	font.CharSet = DEFAULT_CHARSET;
	font.OutputPrecision = OUT_DEFAULT_PRECIS;
	font.Quality = DEFAULT_QUALITY;
	font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(font.FaceName, _T("Arial"));

	HR(D3DXCreateFontIndirect(gDevice, &font, &mFont));
//	D3DXCreateFont( gDevice, 20, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &mFont );
	mFontHeight = 16;
	mFontColor = fontColor;

	mX = x;
	mY = y;

	mTextX = mX + TEXT_MARGIN_LEFT;
	mTextY = mY + TEXT_MARGIN_TOP;

	mStartDisplayIndex = 0;
	mEndDisplayIndex = 0;

	mListSprite = NULL;
	mWidth = (float)width;
	mHeight = (float)height;

	mTextWidth = mWidth - (TEXT_MARGIN_LEFT + TEXT_MARGIN_RIGHT);
	mTextHeight = mHeight - (TEXT_MARGIN_TOP + TEXT_MARGIN_BOTTOM);

	mAllowSingleClickSelection = false;
	mAllowMultipleSel = false;
	mStartedOnTop = false;

	std::string whitePath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mHighlightedSprite = snew Sprite(whitePath.c_str(), 0, 0, mWidth, mHeight, highlightColor);

	mHoverSprite = snew Sprite(whitePath.c_str(), 0, 0, mWidth, mHeight, highlightColor);
	mCurrHoverIndex = -1;

	mCurrSelection = -1;
	mMultipleSelBegin = -1;

	mUpDownTimer = 0;
	mPageTimer = 0;
	mHoldDelayPassed = false;

	mCallback = selectedtItemCB;
	mCallbackObj = callbackObj;

	mListTarget = snew RenderTarget((int)width, (int)height, bgColor, false);
	mScrollBar = snew VerticalScrollBar(mX, mY, mHeight, scrollbar_callback, this);
	blur();
	mDoRedraw = true;
}
ItemListBox::~ItemListBox()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	deleteItems();
	ReleaseCOM(mFont);

	delete mListTarget;
	delete mHighlightedSprite;
	delete mHoverSprite;
	delete mScrollBar;
}

void ItemListBox::onDeviceLost()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	HR(mFont->OnLostDevice());
	mListTarget->onDeviceLost();
	mHighlightedSprite->onDeviceLost();
	mHoverSprite->onDeviceLost();
	mScrollBar->onDeviceLost();
	mDoRedraw = true;
}
void ItemListBox::onDeviceReset()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	HR(mFont->OnResetDevice());

	mListTarget->onDeviceReset();
	mHighlightedSprite->onDeviceReset();
	mHoverSprite->onDeviceReset();
	mScrollBar->onDeviceReset();
	recreateTargets();
	mDoRedraw = true;
}
void ItemListBox::recreateTargets()
{
	if (mListSprite == NULL)
	{
		mListSprite = snew Sprite(mListTarget->getTexture(), mX, mY, mWidth, mHeight);
		mSprites.push_back(mListSprite);
		setDrawables();
	}
	else
	{
		mListSprite->setDest(mX, mY, mWidth, mHeight);
		mListSprite->replaceCurrentTexture(mListTarget->getTexture(), false);
	}
	mDoRedraw = true;
}
void ItemListBox::setBgColor(D3DXCOLOR c)
{
	mListTarget->setColor(c);
	mDoRedraw = true;
}
void ItemListBox::update(float dt)
{
	bool selChanged = false;

	int cursorDelta = 0;

	if (mHoverItems.size() > 0)
	{
		for (map<int, float>::iterator iter = mHoverItems.begin(); iter != mHoverItems.end();)
		{
			if (iter->first != mCurrHoverIndex)
			{
				iter->second -= dt;
			}
			if (iter->second <= 0)
			{
				mHoverItems.erase(iter++);
			}
			else
			{
				if (iter->first != mCurrHoverIndex)
				{
					mDoRedraw = true;
				}
				++iter;
			}
		}
	}
	if (getIsFocused())
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

		bool selMultiple = mAllowMultipleSel && mMultipleSelBegin != -1 && (gInput->isKeyDown(VK_SHIFT) || gInput->keyReleased(VK_SHIFT));

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
			cursorDelta = mItems.size();
		}
		if (gInput->keyPressed(VK_HOME))
		{
			mUpDownTimer = 0;
			cursorDelta = -(int)mItems.size();
		}

		if (cursorDelta != 0)
		{
			if (!selMultiple)
			{
				mSelectedIndices.clear();
				if (mCurrSelection < 0)
				{
					mCurrSelection = 0;
					mSelectedIndices.push_back(mCurrSelection);
				}
				else
				{
					mCurrSelection = max(min(mCurrSelection + cursorDelta, (int)mItems.size() - 1), 0);
					mSelectedIndices.push_back(mCurrSelection);
				}
				mMultipleSelBegin = mCurrSelection;
			}
			else
			{
				mSelectedIndices.clear();
				mCurrSelection = max(min(mCurrSelection + cursorDelta, (int)mItems.size() - 1), 0);
				if (mCurrSelection < mMultipleSelBegin)
				{
					for (int i = mMultipleSelBegin; i >= mCurrSelection; i--)
					{
						if (i >= 0)
						{
							mSelectedIndices.push_back(i);
						}
					}
				}
				else
				{
					for (int i = mMultipleSelBegin; i <= mCurrSelection; i++)
					{
						if (i < (int)mItems.size())
						{
							mSelectedIndices.push_back(i);
						}
					}
				}
			}
			selChanged = true;
		}
	} // if focused
	
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	// update list of items to display
	if (selChanged)
	{
		if ((unsigned int)mCurrSelection < mStartDisplayIndex)
		{
			mStartDisplayIndex = mCurrSelection;
			mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
		}
		else if ((unsigned int)mCurrSelection > mEndDisplayIndex)
		{
			mEndDisplayIndex = mCurrSelection;
			mStartDisplayIndex = max(0, mEndDisplayIndex - max(0, getNumItemsDisplayed() - 1));
		}
		mDoRedraw = true;
	}

	// item selected by pressing enter
	if (getIsFocused() && gInput->keyPressed(VK_RETURN) && mCallback != NULL &&
		mCurrSelection >= 0 && mCurrSelection < (int)mItems.size())
	{
		mCallback(mCallbackObj, this);
		mDoRedraw = true;
	}
	lock.Unlock();

	// position scrollbar
	updateScrollBar();
}

void ItemListBox::updateScrollBar()
{
	if (mScrollBar->getVisible())
	{
		float currPos = (float)mStartDisplayIndex / max(1.0f, ((float)mItems.size() - (float)getNumItemsDisplayed()));
		mScrollBar->setHandlePosition(currPos);
	}
}
void ItemListBox::preRender()
{
	if (mDoRedraw)
	{
		CSingleLock lock(&mCritSection);
		lock.Lock();
	
		mListTarget->beginScene();

		int row = 0;
		if (mStartDisplayIndex != mEndDisplayIndex)
		{
			for (unsigned int i = mStartDisplayIndex; i <= mEndDisplayIndex && i < mItems.size(); i++)
			{
				bool selected = false;
				for (unsigned int k = 0; k < mSelectedIndices.size(); k++)
				{
					if (i == mSelectedIndices[k])
					{
						selected = true;
						break;
					}
				}

				if (selected)
				{
					mHighlightedSprite->setDest(TEXT_MARGIN_LEFT, TEXT_MARGIN_TOP + mFontHeight * row, (int)mTextWidth, mFontHeight);
					gWindowMgr->drawSprite(mHighlightedSprite, mWidth, mTextHeight);
				}
				else if (mHoverItems.find(i) != mHoverItems.end())
				{
					mHoverSprite->setDest(TEXT_MARGIN_LEFT, TEXT_MARGIN_TOP + mFontHeight * row, (int)mTextWidth, mFontHeight);
					mHoverSprite->setColor(D3DXVECTOR4(0.35f, 0.35f, 0.6f, mHoverItems[i] / HOVER_DURATION));
					gWindowMgr->drawSprite(mHoverSprite, mWidth, mTextHeight);
				}

				int y = TEXT_MARGIN_TOP + mFontHeight * row;
				RECT r = { TEXT_MARGIN_LEFT, y, (int)mTextWidth, y + mFontHeight };
				HR(mFont->DrawText(0, mItems[i]->toString().c_str(), -1, &r, DT_NOCLIP, mFontColor));
				row++;
			}
		}
		mListTarget->endScene();
		mDoRedraw = false;
		lock.Unlock();
	}
}
void ItemListBox::setPos(float x, float y, float width, float height)
{
	mX = floor(x);
	mY = floor(y);
	mWidth = width;
	mHeight = height;

	mTextX = mX + TEXT_MARGIN_LEFT;
	mTextY = mY + TEXT_MARGIN_TOP;

	mTextWidth = mWidth - (TEXT_MARGIN_LEFT + TEXT_MARGIN_RIGHT);
	mTextHeight = mHeight - (TEXT_MARGIN_TOP + TEXT_MARGIN_BOTTOM);

	mListTarget->setDimensions((int)width, (int)height);

	recreateTargets();

	mScrollBar->setPos(mX + mWidth - 10.0f, mY, mHeight);

	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
	mDoRedraw = true;
	lock.Unlock();
}
void ItemListBox::setPos(float x, float y, bool autoSize)
{
	if (autoSize)
	{
		setPos(x, y, getWidthToFit(), getHeightToFit());
	}
	else
	{
		setPos(x, y, (float)mWidth, mHeight);
	}
}
float ItemListBox::getX()
{
	return mX;
}
float ItemListBox::getY()
{
	return mY;
}
float ItemListBox::getWidth()
{
	return mWidth;
}
float ItemListBox::getHeight()
{
	return mHeight;
}
float ItemListBox::getWidthToFit()
{
	int maxWidth = TEXT_MARGIN_LEFT + TEXT_MARGIN_RIGHT;
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		RECT r = { 0, 0, (int)100, mFontHeight };
		HR(mFont->DrawText(0, mItems[i]->toString().c_str(), -1, &r, DT_NOCLIP | DT_CALCRECT, D3DCOLOR_XRGB(255, 255, 0)));
		int width = (int)(r.right - r.left);
		if (width > maxWidth)
		{
			maxWidth = width;
		}
	}
	lock.Unlock();
	maxWidth += (TEXT_MARGIN_LEFT + TEXT_MARGIN_RIGHT);
	return (float)maxWidth;
}
float ItemListBox::getHeightToFit()
{
	return (float)(TEXT_MARGIN_TOP + TEXT_MARGIN_BOTTOM + mFontHeight * (int)mItems.size());
}

std::vector<Sprite*> ItemListBox::getSprites()
{
	return mDrawableSprites;
}
int ItemListBox::getNumTriangles()
{
	int total = 0;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		total += mSprites[i]->getNumTriangles();
	}

	return total;
}
void ItemListBox::clearItems()
{
	this->deleteItems();
	mDoRedraw = true;
}
void ItemListBox::shuffleItems()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	std::random_shuffle(mItems.begin(), mItems.end());
	mDoRedraw = true;
	lock.Unlock();
}
// takes ownership of pointer
void ItemListBox::setItems(std::vector<ListItem*> items)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	deleteItems();
	mItems.clear();

	mItems = items;

	mStartDisplayIndex = 0;
	mEndDisplayIndex = min(mItems.size(), max(0, getNumItemsDisplayed() - 1));

	mDoRedraw = true;
	lock.Unlock();
	updateScrollbarVisibility();
}

void ItemListBox::addItems(std::vector<ListItem*> items)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
		
	for (unsigned int i = 0; i < items.size(); i++)
	{
		mItems.push_back(items[i]);
		mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
	}
	mDoRedraw = true;
	lock.Unlock();
	updateScrollbarVisibility();
}
void ItemListBox::addItem(ListItem* item)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	mItems.push_back(item);
	mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
	
	mDoRedraw = true;
	lock.Unlock();
	updateScrollbarVisibility();
}
void ItemListBox::addItem(ListItem* item, unsigned int index)
{
	if (index >= mItems.size())
	{
		addItem(item);
		return;
	}
	CSingleLock lock(&mCritSection);
	lock.Lock();
		
	vector<ListItem*>::iterator iter = mItems.begin() + index;

	mItems.insert(iter, item);
	mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
	mDoRedraw = true;
	lock.Unlock();
	updateScrollbarVisibility();
}
void ItemListBox::addItems(std::vector<ListItem*> items, unsigned int index)
{
	if (index >= mItems.size())
	{
		addItems(items);
		return;
	}
	CSingleLock lock(&mCritSection);
	lock.Lock();
		
	vector<ListItem*>::iterator iter = mItems.begin() + index;

	mItems.insert(iter, items.begin(), items.end());
	mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
	mDoRedraw = true;
	lock.Unlock();
	updateScrollbarVisibility();
}
// does nothing if an item isn't already in the list
void ItemListBox::modifyItems(std::vector<ListItem*> items)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		for (unsigned int j = 0; j < items.size(); j++)
		{
			if (mItems[i]->getId() == items[j]->getId())
			{
				mItems[i]->setData(items[j]);
			}
		}
	}
	mDoRedraw = true;
	lock.Unlock();
}
// does nothing if an item isn't already in the list
void ItemListBox::modifyItem(ListItem* item)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		if (mItems[i]->getId() == item->getId())
		{
			mItems[i]->setData(item);
		}
	}
	mDoRedraw = true;
	lock.Unlock();
}
void ItemListBox::removeItems(vector<int> items)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	map<int, bool> indices; // items to delete, no copies
	for (unsigned int i = 0; i < items.size(); i++)
	{
		if (indices.find(items[i]) == indices.end())
		{
			indices[items[i]] = true;
		}
	}
	
	vector<ListItem*> newItems;
	int lastSel = -1;
	int l=mItems.size();
	for (unsigned int i = 0; i < l; i++)
	{
		if (indices.find(i) == indices.end())
		{
			newItems.push_back(mItems[i]);
		}
		else
		{
			lastSel = (int)newItems.size() - 1;
			delete mItems[i];
		}
	}
	
	mItems = newItems;
	mStartDisplayIndex = min(mStartDisplayIndex, mItems.size() - 1);
	mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
	mSelectedIndices.clear();
	mCurrSelection = -1;
	if (lastSel >= 0)
	{
		lastSel = min(lastSel, (int)mItems.size() - 1);
		mCurrSelection = lastSel;
		mSelectedIndices.push_back(lastSel);
	}
	mMultipleSelBegin = -1;
	mDoRedraw = true;
	lock.Unlock();
	updateScrollbarVisibility();
}

unsigned int ItemListBox::getNumItemsDisplayed()
{
	return (unsigned int)max(0, floor(mTextHeight / (float)mFontHeight));
}

void ItemListBox::deleteItems()
{
	mStartDisplayIndex = mEndDisplayIndex = 0;
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		delete mItems[i];
	}
	mDoRedraw = true;
	mSelectedIndices.clear();
	mCurrSelection = -1;
	mMultipleSelBegin = -1;
	mItems.clear();
	updateScrollbarVisibility();
}

bool ItemListBox::onMouseEvent(MouseEvent e)
{
	if (mScrollBar->onMouseEvent(e))
	{
		return true;
	}

	if (getIsFocused()) //isPointInside(e.getX(), e.getY()))
	{
		if (e.getEvent() == MouseEvent::MOUSEWHEEL && mItems.size() > max(0, getNumItemsDisplayed() - 1))
		{
			int scrollAmt = Settings::instance()->getIntValue("SCROLL_SPEED", NUM_SCROLLING_ITEMS);
			CSingleLock lock(&mCritSection);
			lock.Lock();
	
			// update list of items to display
			if (e.getExtraHiData() < 0)
			{
				mEndDisplayIndex = min(mItems.size() - 1, mEndDisplayIndex + scrollAmt);
				mStartDisplayIndex = max(0, mEndDisplayIndex - max(0, getNumItemsDisplayed() - 1));
			}
			else 
			{
				mStartDisplayIndex = max(0, (int)mStartDisplayIndex - scrollAmt);
				mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
			}
			mDoRedraw = true;
			lock.Unlock();
			updateScrollBar();
			return true;
		}
		else if (isPointInside(e.getX(), e.getY()))
		{
			if (e.getEvent() == MouseEvent::LBUTTONDBLCLK)
			{
				int selected = getItemAtPos(e.getX(), e.getY());
				mSelectedIndices.clear();
				if (selected >= 0)
				{
					mCurrSelection = selected;
					mMultipleSelBegin = mCurrSelection;
					mSelectedIndices.push_back(mCurrSelection);
					if (mCallback != NULL)
					{
						mCallback(mCallbackObj, this);
					}
				}
				mDoRedraw = true;
				e.setConsumed(true);
				return true;
			}
			else if (e.getEvent() == MouseEvent::LBUTTONDOWN)
			{
				mSelectedIndices.clear();
				int selected = getItemAtPos(e.getX(), e.getY());
				if (selected >= 0)
				{
					mCurrSelection = selected;
					mSelectedIndices.push_back(mCurrSelection);
					if (gInput->isKeyDown(VK_SHIFT) || gInput->keyReleased(VK_SHIFT))
					{
						if (mCurrSelection < mMultipleSelBegin)
						{
							for (int i = mMultipleSelBegin; i > mCurrSelection; i--)
							{
								if (i > 0)
								{
									mSelectedIndices.push_back(i);
								}
							}
						}
						else
						{
							for (int i = mMultipleSelBegin; i < mCurrSelection; i++)
							{
								if (i < (int)mItems.size())
								{
									mSelectedIndices.push_back(i);
								}
							}
						}
					}
					else
					{
						mMultipleSelBegin = mCurrSelection;
					}
					mStartedOnTop = true;
				}
				mDoRedraw = true;
				e.setConsumed(true);
				return true;
			}
			else if (e.getEvent() == MouseEvent::LBUTTONUP)
			{
				if (mAllowSingleClickSelection && mStartedOnTop)
				{
					int selected = getItemAtPos(e.getX(), e.getY());
					if (selected >= 0)
					{
						mCurrSelection = selected;
						mSelectedIndices.push_back(mCurrSelection);
						if (gInput->isKeyDown(VK_SHIFT) || gInput->keyReleased(VK_SHIFT))
						{
							if (mCurrSelection < mMultipleSelBegin)
							{
								for (int i = mMultipleSelBegin; i > mCurrSelection; i--)
								{
									if (i > 0)
									{
										mSelectedIndices.push_back(i);
									}
								}
							}
							else
							{
								for (int i = mMultipleSelBegin; i < mCurrSelection; i++)
								{
									if (i < (int)mItems.size())
									{
										mSelectedIndices.push_back(i);
									}
								}
							}
						}
						else
						{
							mMultipleSelBegin = mCurrSelection;
						}
						if (mCallback != NULL)
						{
							mCallback(mCallbackObj, this);
						}
					}
					mStartedOnTop = false;
				}
				mDoRedraw = true;
				e.setConsumed(true);
				return true;
			}
			else if (e.getEvent() == MouseEvent::MOUSEMOVE)
			{
				int index = getItemAtPos(e.getX(), e.getY());
				if (index >= 0)
				{
					if (mHoverItems.find(index) != mHoverItems.end())
					{
						mHoverItems[index] = HOVER_DURATION;
					}
					else
					{
						mHoverItems.insert(pair<int, float>(index, HOVER_DURATION));
					}
					mCurrHoverIndex = index;
					mDoRedraw = true;
				}
				else
				{
					mCurrHoverIndex = -1;
				}
			}
		}
		else
		{
			mCurrHoverIndex = -1;
		}
	}
	return false;
}
int ItemListBox::findItem(std::string &name)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		if (mItems[i]->toString() == name)
		{
			lock.Unlock();
			return (int)i;
		}
	}
	lock.Unlock();
	return -1;
}
ListItem* ItemListBox::setHighlightedItem(std::string &name)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		if (mItems[i]->toString() == name)
		{
			lock.Unlock();
			return mItems[i];;
		}
	}
	mDoRedraw = true;
	lock.Unlock();
	return NULL;
}
ListItem* ItemListBox::setHighlightedItem(int index)
{
	return getItem(index);
}
ListItem* ItemListBox::getSelectedItem()
{
	return getItem(mCurrSelection);
}
void ItemListBox::setSelectedIndex(int index)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	if (index < (int)mItems.size())
	{
		mCurrSelection = index;
		mMultipleSelBegin = mCurrSelection;
		mSelectedIndices.clear();
		mSelectedIndices.push_back(index);
		mDoRedraw = true;
	}
	lock.Unlock();
}
int ItemListBox::getSelectedIndex()
{
	return mCurrSelection;
}
vector<int> ItemListBox::getSelectedIndices()
{
	return mSelectedIndices;
}
void ItemListBox::scrollToIndex(int index)
{
	if (index < 0 || index >= mItems.size())
	{
		return;
	}

	if (index < mStartDisplayIndex)
	{
		mStartDisplayIndex = index;
		mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));
		mDoRedraw = true;
	}
	else if (index > mEndDisplayIndex)
	{
		mEndDisplayIndex = index;
		mStartDisplayIndex = max(0, mEndDisplayIndex - max(0, getNumItemsDisplayed() - 1));
		mDoRedraw = true;
	}
}

ListItem* ItemListBox::getItem(int index)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	if (index >= 0 && index < (int)mItems.size())
	{
		lock.Unlock();
		return mItems[index];
	}
	lock.Unlock();
	return NULL;
}
vector<ListItem*> ItemListBox::getItems()
{
	return mItems;
}
int ItemListBox::getNumItems()
{
	return mItems.size();
}

bool ItemListBox::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x2 = mX + mWidth;
	float y2 = mY + mHeight;

	return !(xPoint < mX || yPoint < mY || xPoint > x2 || yPoint > y2);
}

int ItemListBox::getItemAtPos(int x, int y)
{
	int itemIndex = -1;

	int listX = (int)mX + TEXT_MARGIN_LEFT;
	int listX2 = listX + (int)mTextWidth;
	int listY = (int)mY + TEXT_MARGIN_TOP;
	int listY2 = listY + (int)mTextHeight;
	if (!(x < listX || y < listY || x > listX2 || y > listY2))
	{
		y -= listY;
		
		int index = (y / mFontHeight) + mStartDisplayIndex;

		if (index >= 0 && index < (int)mItems.size())
		{
			itemIndex = index;
		}
	}

	return itemIndex;
}
void ItemListBox::updateScrollbarVisibility()
{
	mScrollBar->setVisible(getNumItemsDisplayed() < getNumItems());
	setDrawables();
}
void ItemListBox::setDrawables()
{
	mDrawableSprites.clear();
	mDrawableSprites.push_back(mListSprite);
	if (mScrollBar->getVisible())
	{
		for (unsigned int i = 0; i < mScrollBar->getSprites().size(); i++)
		{
			mDrawableSprites.push_back(mScrollBar->getSprites()[i]);
		}
	}
}

void ItemListBox::setAllowSingleClickSelection(bool allow)
{
	mAllowSingleClickSelection = allow;
}
bool ItemListBox::getAllowSingleClickSelection() 
{
	return mAllowSingleClickSelection;
}

void ItemListBox::setAllowMultipleSelection(bool allow)
{
	mAllowMultipleSel = allow;
}


void ItemListBox::onScrollbarMoved(VerticalScrollBar* bar, float percent)
{
	mStartDisplayIndex = max(percent * ((float)mItems.size() - (float)getNumItemsDisplayed()), 0);
	mEndDisplayIndex = min(mItems.size(), mStartDisplayIndex + max(0, getNumItemsDisplayed() - 1));

	mDoRedraw = true;
}
