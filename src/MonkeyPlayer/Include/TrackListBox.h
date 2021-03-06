// TrackListBox.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A box with selectable tracks

#include "ItemListBox.h"
#include "Sprite.h"

#ifndef TRACK_LIST_BOX_H
#define TRACK_LIST_BOX_H

namespace MonkeyPlayer
{
	class TrackListBox : public ItemListBox
	{
	public:
		TrackListBox(float x, float y, float width, float height,
			void (*selectedtItemCB)(void* ptrObj, ItemListBox* selItem) = NULL, void* callbackObj = NULL);

		void preRender();
		void setCurrentTrack(int index);

		void clearItems();
		virtual void shuffleItems();
		virtual int findItem(std::string &name);
		virtual ListItem* setHighlightedItem(std::string &name);
		virtual ListItem* setHighlightedItem(int index);
		virtual int getHighlightedIndex();
		void removeItems(vector<int> items); 
		void setUseTrackNumbers(bool useTrackNo);

	private:
		int mTimeWidth;
		int mHighlightedIndex;
		bool mUseTrackNumbers;
	};
}
#endif