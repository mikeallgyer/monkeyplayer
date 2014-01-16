// PlaylistWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a playlist of songs


#include "Button.h"
#include "d3dUtil.h"
#include "ItemListBox.h"
#include "IWidget.h"
#include "IWindow.h"
#include "SoundManager.h"
#include "Sprite.h"
#include "TrackListBox.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef PLAYLIST_WINDOW_H
#define PLAYLIST_WINDOW_H

namespace MonkeyPlayer
{
	class PlaylistWindow : public IWindow
	{
	public:
		PlaylistWindow();
		~PlaylistWindow();

		void onDeviceLost();
		void onDeviceReset();

		void setTopPos(int pos);
		int getWidth();
		int getHeight();

		void update(float dt);

		void display();

		std::vector<Sprite*> getSprites();
		std::vector<IWidget*> getWidgets();

		void clearItems();
		vector<ListItem*> getItems() { return mListBox->getItems(); }
		int getHighlightedIndex() { return mListBox->getHighlightedIndex(); }
		void addItem(Track* item);
		void addItems(std::vector<Track*> items, bool doWriteFile = true);
		void modifyItem(Track* item);
		void modifyItems(std::vector<Track*> items);
		bool playNextSong(bool loop);
		Track* getNextSong(bool loop, int &index);
		bool playPreviousSong();
		Track* getPreviousSong();
		bool playCurrentSong();
		bool hasNextSong();

		void addTrackToQueueEnd(int id);
		void insertTrackToQueueNext(int id);
		void replaceQueueWithTrack(int id);

		void addAlbumToQueueEnd(Album a);
		void insertAlbumToQueueNext(Album a);
		void replaceQueueWithAlbum(Album a);

		void addArtistToQueueEnd(string &name);
		void insertArtistToQueueNext(string &name);
		void replaceQueueWithArtist(string &name);

		void setPlaylistName(string name, bool doWriteFile = true);

		static void listBox_callback(void* obj, ItemListBox* listBox)
		{
			PlaylistWindow* win = static_cast<PlaylistWindow*>(obj);
			if (win)
			{
				win->onItemSelected(listBox->getSelectedItem(), ((TrackListBox*)listBox)->getSelectedIndex());
			}
		}

		static void button_callback(void* obj, Button* btn)
		{
			PlaylistWindow* win = static_cast<PlaylistWindow*>(obj);
			if (win)
			{
				win->onBtnClicked(btn);
			}
		}

		bool onMouseEvent(MouseEvent ev);  // true if window consumed event

		void onBlur();
		void onFocus();

	private:

		ID3DXFont* mFont;

		std::vector<Sprite*> mSprites;
		std::vector<IWidget*> mWidgets;
		Sprite* mBackground;

		TrackListBox *mListBox;
		SimpleLabel* mHeaderLbl;
		SimpleLabel* mLengthLbl;
		Button *mShuffleBtn;
		Button *mClearBtn;
		Button *mDelBtn;
		Button *mSaveBtn;

		int mPreferredWidth;
		int mCurrWidth;
		int mCurrHeight;
		bool mResized;
		int mCurrSongIndex;
		int mTopPos;
		string mPlaylistName;

		static const int MIN_WINDOW_WIDTH;

		void writeFile();
		void readFile();
		string getFilename();

		void onItemSelected(ListItem* item, int index);
		void onBtnClicked(Button* btn);
	};
}
#endif
