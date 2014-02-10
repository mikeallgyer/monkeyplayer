// PlaylistListBox.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A box with selectable playlistS

#include <algorithm>    // std::random_shuffle
#include "d3dApp.h"
#include "ItemListBox.h"
#include "Settings.h"
#include "PlaylistListBox.h"
#include "WindowManager.h"

using namespace MonkeyPlayer;

PlaylistListBox::PlaylistListBox(float x, float y, float width, float height,
	void (*selectedtItemCB)(void* ptrObj, ItemListBox* selItem), void* callbackObj)
	: ItemListBox(x, y, width, height, selectedtItemCB, callbackObj)
{
	RECT r = { 0, 0, (int)100, mFontHeight };
	HR(mFont->DrawText(0, "00000:00:00", -1, &r, DT_NOCLIP | DT_CALCRECT, D3DCOLOR_XRGB(255, 255, 0)));
	mTimeWidth = r.right;
}

void PlaylistListBox::preRender()
{
//	ItemListBox::preRender();
//	return;
	if (mDoRedraw)
	{
		CSingleLock lock(&mCritSection);
		lock.Lock();
		mListTarget->beginScene();
		int row = 0;
		char buf[512];

		// bottleneck??
		if (mStartDisplayIndex != mEndDisplayIndex)
		{
			D3DCOLOR defColor = D3DCOLOR_XRGB(255, 255, 255);
			D3DCOLOR selColor = D3DCOLOR_XRGB(195, 222, 165);
			D3DCOLOR currColor = defColor;
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
					mHighlightedSprite->setDest(ItemListBox::TEXT_MARGIN_LEFT, ItemListBox::TEXT_MARGIN_TOP + mFontHeight * row, (int)mTextWidth, mFontHeight);
					gWindowMgr->drawSprite(mHighlightedSprite, mWidth, mHeight);
				}
				else if (mHoverItems.find(i) != mHoverItems.end())
				{
					mHoverSprite->setDest(TEXT_MARGIN_LEFT, TEXT_MARGIN_TOP + mFontHeight * row, (int)mTextWidth, mFontHeight);
					mHoverSprite->setColor(D3DXVECTOR4(0.35f, 0.35f, 0.6f, mHoverItems[i] / HOVER_DURATION));
					gWindowMgr->drawSprite(mHoverSprite, mWidth, mTextHeight);
				}
				currColor = defColor;
				int y = ItemListBox::TEXT_MARGIN_TOP + mFontHeight * row;
				RECT r = { ItemListBox::TEXT_MARGIN_LEFT, y, ItemListBox::TEXT_MARGIN_LEFT + (int)mTextWidth - mTimeWidth, y + mFontHeight };
				sprintf_s(buf, 512, "%d. %s", (i + 1), mItems[i]->toString().c_str());

				HR(mFont->DrawText(0, buf, -1, &r, DT_LEFT, currColor));
				
				// time
				r.right = ItemListBox::TEXT_MARGIN_LEFT + (int)mTextWidth;
				HR(mFont->DrawText(0, ((PlaylistListItem*)mItems[i])->getTime().c_str(), -1, &r, DT_RIGHT, currColor));
				row++;
			}
		}
		mListTarget->endScene();

		lock.Unlock();
		mDoRedraw = false;

	}
}
void PlaylistListBox::clearItems()
{
	ItemListBox::clearItems();
}
void PlaylistListBox::shuffleItems()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	std::random_shuffle(mItems.begin(), mItems.end());

	mDoRedraw = true;
	lock.Unlock();
}

int PlaylistListBox::findItem(std::string &name)
{
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		if (((PlaylistListItem*)mItems[i])->toString() == name)
		{
			return i;
		}
	}
	return -1;
}
void PlaylistListBox::removeItems(vector<int> items)
{
	ItemListBox::removeItems(items);
}
