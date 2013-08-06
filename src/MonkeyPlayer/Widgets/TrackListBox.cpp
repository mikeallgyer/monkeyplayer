// TrackListBox.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A box with selectable tracks

#include "d3dApp.h"
#include "ItemListBox.h"
#include "Settings.h"
#include "TrackListBox.h"
#include "WindowManager.h"

TrackListBox::TrackListBox(float x, float y, float width, float height,
	void (*selectedtItemCB)(void* ptrObj, ListItem* selItem), void* callbackObj)
	: ItemListBox(x, y, width, height, selectedtItemCB, callbackObj)
{
	RECT r = { 0, 0, (int)100, mFontHeight };
	HR(mFont->DrawText(0, "000:00:00", -1, &r, DT_NOCLIP | DT_CALCRECT, D3DCOLOR_XRGB(255, 255, 0)));
	mTimeWidth = r.right;
	mCurrPlaying = NULL;
}

void TrackListBox::preRender()
{
	ItemListBox::preRender();
	return;
	CSingleLock lock(&mCritSection, true);
	mListTarget->beginScene();
	int row = 0;
	if (mStartDisplayIndex != mEndDisplayIndex)
	{
		D3DCOLOR defColor = D3DCOLOR_XRGB(255, 255, 255);
		D3DCOLOR selColor = D3DCOLOR_XRGB(175, 175, 0);
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
			currColor = (mItems[i] == mCurrPlaying) ? selColor : defColor;
			int y = ItemListBox::TEXT_MARGIN_TOP + mFontHeight * row;
			RECT r = { ItemListBox::TEXT_MARGIN_LEFT, y, ItemListBox::TEXT_MARGIN_LEFT + (int)mTextWidth - mTimeWidth, y + mFontHeight };
			HR(mFont->DrawText(0, mItems[i]->toString().c_str(), -1, &r, DT_NOCLIP, currColor));
			
			// time
			r.right = ItemListBox::TEXT_MARGIN_LEFT + (int)mTextWidth;
			HR(mFont->DrawText(0, ((TrackListItem*)mItems[i])->getTime().c_str(), -1, &r, DT_RIGHT, currColor));
			row++;
		}
	}
	mListTarget->endScene();

	lock.Unlock();

}
void TrackListBox::setCurrentTrack(ListItem* track)
{
	mCurrPlaying = track;
}
int TrackListBox::findItem(std::string &name)
{
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		if (((TrackListItem*)mItems[i])->getTrack()->Filename == name)
		{
			return i;
		}
	}
	return -1;
}
ListItem* TrackListBox::setHighlightedItem(std::string &name)
{
	for (unsigned int i = 0; i < mItems.size(); i++)
	{
		if (((TrackListItem*)mItems[i])->getTrack()->Filename == name)
		{
			setCurrentTrack(mItems[i]);
			return mItems[i];
		}
	}
	return NULL;
}
ListItem* TrackListBox::setHighlightedItem(int index)
{
	if (index >= 0 && index < (int)mItems.size())
	{
		setCurrentTrack(mItems[index]);
		return mItems[index];
	}
	return NULL;
}
