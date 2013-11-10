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

		int getWidth();
		int getHeight();

		void update(float dt);

		void display();

		std::vector<Sprite*> getSprites();
		std::vector<IWidget*> getWidgets();

		void clearItems();
		void addItem(Track* item);
		void addItems(std::vector<Track*> items, bool doWriteFile = true);
		void modifyItem(Track* item);
		void modifyItems(std::vector<Track*> items);
		bool playNextSong();
		bool playPreviousSong();
		bool playCurrentSong();

		void addTrackToQueueEnd(int id);
		void insertTrackToQueueNext(int id);
		void replaceQueueWithTrack(int id);

		void addAlbumToQueueEnd(Album a);
		void insertAlbumToQueueNext(Album a);
		void replaceQueueWithAlbum(Album a);

		void addArtistToQueueEnd(string &name);
		void insertArtistToQueueNext(string &name);
		void replaceQueueWithArtist(string &name);


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
		Button *mShuffleBtn;
		Button *mClearBtn;
		Button *mDelBtn;

		int mPreferredWidth;
		int mCurrWidth;
		bool mResized;
		int mCurrSongIndex;

		static const int MIN_WINDOW_WIDTH;

		void writeFile();
		void readFile();
		string getFilename();

		void onItemSelected(ListItem* item, int index);
		void onBtnClicked(Button* btn);
	};
}
#endif
