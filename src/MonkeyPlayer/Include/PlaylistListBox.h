// TrackListBox.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A box with selectable tracks

#include "ItemListBox.h"
#include "Sprite.h"

#ifndef PLAYLIST_LIST_BOX_H
#define PLAYLIST_LIST_BOX_H

namespace MonkeyPlayer
{
	class PlaylistListBox : public ItemListBox
	{
	public:
		PlaylistListBox(float x, float y, float width, float height,
			void (*selectedtItemCB)(void* ptrObj, ItemListBox* selItem) = NULL, void* callbackObj = NULL);

		void preRender();
		void setCurrentTrack(int index);

		void clearItems();
		virtual void shuffleItems();
		virtual int findItem(std::string &name);
		void removeItems(vector<int> items); 

	private:
		int mTimeWidth;
	};
}
#endif