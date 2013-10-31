// NowPlayingWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays currently playing info

#include "d3dUtil.h"
#include "DatabaseManager.h"
#include "IWidget.h"
#include "IWindow.h"
#include "Label.h"
#include "MetadataReader.h"
#include "SoundManager.h"
#include "Sprite.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef NOW_PLAYING_WINDOW_H
#define NOW_PLAYING_WINDOW_H

namespace MonkeyPlayer
{
	class NowPlayingWindow : public IWindow
	{
	public:

		NowPlayingWindow();
		~NowPlayingWindow();

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

		static void soundEventCB(void *obj, SoundManager::SoundEvent ev)
		{
			NowPlayingWindow* win = static_cast<NowPlayingWindow*>(obj);
			if (win)
			{
				win->onSoundEvent(ev);
			}
		}
		
	private:
		static const float WINDOW_WIDTH;
		static const float WINDOW_HEIGHT;

		std::vector<Sprite*> mSprites;
		std::vector<IWidget*> mWidgets;
		Sprite* mAlbumArt;
		Album mCurrentAlbum;

		float mWidth, mHeight;
		std::string mDefaultAlbumPath;

		void updateCover(std::string &soundFile);
		void onSoundEvent(SoundManager::SoundEvent ev);
		
	};
}
#endif
