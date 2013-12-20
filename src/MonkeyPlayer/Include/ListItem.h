// ListItem.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A selectable item in a ListItemBox

#include <string>
#include <vector>

#include "DatabaseStructs.h"
#include "FileManager.h"
#include "IWidget.h"
#include "Sprite.h"
#include "SoundManager.h"

#ifndef LIST_ITEM_H
#define LIST_ITEM_H

namespace MonkeyPlayer
{
	class ListItem 
	{
	public:
		virtual int getId() = 0;
		virtual std::string toString() = 0;
		virtual ~ListItem() {}
		virtual void setData(ListItem* item) = 0;

	};

	class SimpleListItem : public ListItem
	{
	public:
		SimpleListItem(const char* title, int id) : mTitle(title), mId(id) {}
		int getId() { return mId; }
		std::string toString() { return mTitle; }
		void setData(ListItem* item) 
		{
			mTitle = item->toString();
			mId = item->getId();
		}
		
	private:
		std::string mTitle;
		int mId;
	};

	class TrackListItem : public ListItem
	{
	public:
		TrackListItem()
		{
			mTrack = NULL;
			mDisplayStr = "<No data available>";
			mManageMemory = true;
			mDisplayTime = "";
		}
		TrackListItem(Track* t, bool manageMemory = true)
		{
			mTrack = t;
			setDisplayName(t);
			mManageMemory = manageMemory;
			setDisplayTime();
		}
		~TrackListItem()
		{
			if (mManageMemory)
			{
				delete mTrack;
			}
		}
		int getId() { return mTrack->Id; }
		std::string toString() { return mDisplayStr; }

		void setData(ListItem* item)
		{
			Track* track = ((TrackListItem*)item)->mTrack;
			mTrack->setTrackInfo(*track);
			setDisplayName(track);
			setDisplayTime();
		}
		Track* getTrack()
		{
			return mTrack;
		}
		std::string getTime()
		{
			return mDisplayTime;
		}

	private:
		void setDisplayName(Track* track)
		{
			if (track->Title.length() > 0 && track->Artist != DatabaseStructs::DEF_EMPTY_ARTIST)
			{
				mDisplayStr = track->Artist + " - " + track->Title;
			} 
			else
			{
				mDisplayStr = FileManager::getFileName(track->Title);
			}
		}
		void setDisplayTime()
		{
			if (mTrack == NULL || mTrack->Length < 0)
			{
				mDisplayTime = "";
				return;
			}

			mDisplayTime = SoundManager::getTimeString(mTrack->Length * 1000);
		}
		Track* mTrack;
		std::string mDisplayStr;
		std::string mDisplayTime;
		bool mManageMemory;
	};

	class PlaylistListItem : public ListItem
	{
	public:
		PlaylistListItem()
		{
			mId = -1;
			mDisplayStr = "<No data available>";
			mDisplayTime = "";
		}
		// does not claim memory!
		PlaylistListItem(int id, string name, vector<Track*> t)
		{
			mId = id;
			mDisplayStr = name;
			int total = 0;

			for (unsigned int i=0; i < t.size(); i++)
			{
				total += t[i]->Length;
			}
			mDisplayTime = SoundManager::getTimeString(total * 1000);
		}
		~PlaylistListItem()
		{
		}
		int getId() { return mId; }
		std::string toString() { return mDisplayStr; }

		void setData(ListItem* item)
		{
			mDisplayStr = ((PlaylistListItem*)item)->mDisplayStr;
			mDisplayTime = ((PlaylistListItem*)item)->mDisplayTime;
		}
		std::string getTime()
		{
			return mDisplayTime;
		}

	private:
		int mId;
		std::string mDisplayStr;
		std::string mDisplayTime;
	};
}
#endif