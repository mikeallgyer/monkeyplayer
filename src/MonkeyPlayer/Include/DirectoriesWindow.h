// DirectoriesWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays directories we monitor

#include "Button.h"
#include "d3dUtil.h"
#include "DatabaseManager.h"
#include "IWidget.h"
#include "ItemListBox.h"
#include "IWindow.h"
#include "Label.h"
#include "MetadataReader.h"
#include "SoundManager.h"
#include "Sprite.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef MUSIC_DIR_WINDOW_H
#define MUSIC_DIR_WINDOW_H

namespace MonkeyPlayer
{
	class DirectoriesWindow : public IWindow
	{
	public:
		static const float WINDOW_WIDTH;
		static const float BUTTON_SIZE;

		DirectoriesWindow();
		~DirectoriesWindow();

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

		static void btn_callback(void* obj, Button* btn)
		{
			DirectoriesWindow* win = static_cast<DirectoriesWindow*>(obj);
			if (win)
			{
				win->onBtnPushed(btn);
			}
		}

	private:
		std::vector<Sprite*> mSprites;
		std::vector<IWidget*> mWidgets;
		Sprite* mBackground;
		Button* mAddBtn;
		ItemListBox* mFolderList;
		SimpleLabel* mFolderLabel;

		bool mResized;
		float mWidth, mHeight;
		std::string mDefaultAlbumPath;
		
		void setFolders();
		void onBtnPushed(Button* btn);
	};
}
#endif
