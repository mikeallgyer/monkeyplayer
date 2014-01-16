// SavedPlaylistsWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays directories we monitor

#include "Button.h"
#include "d3dUtil.h"
#include "DatabaseManager.h"
#include "IWidget.h"
#include "PlaylistListBox.h"
#include "IWindow.h"
#include "Label.h"
#include "MetadataReader.h"
#include "PlaylistWindow.h"
#include "SoundManager.h"
#include "Sprite.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef SAVED_PLAYLIST_WINDOW_H
#define SAVED_PLAYLIST_WINDOW_H

namespace MonkeyPlayer
{
	class SavedPlaylistsWindow : public IWindow
	{
	public:
		static const float WINDOW_WIDTH;
		static const float WINDOW_HEIGHT;
		static const float BUTTON_SIZE;

		SavedPlaylistsWindow();
		~SavedPlaylistsWindow();

		void onDeviceLost();
		void onDeviceReset();

		int getWidth();
		int getHeight();

		void update(float dt);

		void display();

		std::vector<Sprite*> getSprites();
		std::vector<IWidget*> getWidgets();

		bool onMouseEvent(MouseEvent ev);  // true if window consumed event

		void onBlur();
		void onFocus();

		void setPlaylistWindow(PlaylistWindow* win);

		static void btn_callback(void* obj, Button* btn)
		{
			SavedPlaylistsWindow* win = static_cast<SavedPlaylistsWindow*>(obj);
			if (win)
			{
				win->onBtnPushed(btn);
			}
		}
		static void listBox_callback(void* obj, ItemListBox* listBox)
		{
			SavedPlaylistsWindow* win = static_cast<SavedPlaylistsWindow*>(obj);
			if (win)
			{
				win->onItemSelected(listBox->getSelectedItem(), ((TrackListBox*)listBox)->getSelectedIndex());
			}
		}
		static void db_playlist_callback(void* obj)
		{
			SavedPlaylistsWindow* win = static_cast<SavedPlaylistsWindow*>(obj);
			if (win)
			{
				win->onDBPlaylistEvent();
			}
		}
	private:
		std::vector<Sprite*> mSprites;
		std::vector<IWidget*> mWidgets;
		Sprite* mBackground;
		Button* mAddBtn;
		PlaylistListBox* mPlaylistList;
		SimpleLabel* mFolderLabel;

		bool mResized;
		float mWidth, mHeight;
		std::string mDefaultAlbumPath;

		PlaylistWindow* mPlaylistWindow;
		
		void setPlaylists();
		void onBtnPushed(Button* btn);
		void onItemSelected(ListItem* item, int index);
		void onDBPlaylistEvent();
	};
}
#endif
