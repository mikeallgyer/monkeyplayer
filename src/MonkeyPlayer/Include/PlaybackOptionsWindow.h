// PlaybackOptionsWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains playback options

#include "Button.h"
#include "Checkbox.h"
#include "ComboBox.h"
#include "d3dUtil.h"
#include "ItemListBox.h"
#include "IWidget.h"
#include "IWindow.h"
#include "Label.h"
#include "Slider.h"
#include "SoundManager.h"
#include "Sprite.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef PLAYBACK_OPTIONS_WINDOW_H
#define PLAYBACK_OPTIONS_WINDOW_H

namespace MonkeyPlayer
{
	class PlaybackOptionsWindow : public IWindow
	{
	public:
		PlaybackOptionsWindow();
		~PlaybackOptionsWindow();

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
			PlaybackOptionsWindow* win = static_cast<PlaybackOptionsWindow*>(obj);
			if (win)
			{
				win->onBtnPushed(btn);
			}
		}
		static void chk_callback(void* obj, Checkbox* btn)
		{
			PlaybackOptionsWindow* win = static_cast<PlaybackOptionsWindow*>(obj);
			if (win)
			{
				win->onChkPushed(btn);
			}
		}
		static void combo_callback(void* obj, ComboBox* combo)
		{
			PlaybackOptionsWindow* win = static_cast<PlaybackOptionsWindow*>(obj);
			if (win)
			{
				win->onComboSelected(combo);
			}
		}

		void setX(int x);
		void setWidth(int width);

		Track* getNextSong(int &index);
		bool playNextSong();
		bool playPreviousSong();
		Track* getCurrentSong();
		bool playCurrentSong();
		bool isRepeatOn(); // excludes repeat playlist
		bool isRepeatPlaylistOn();
		bool isStopAfterOn();
		int getRepeat();
		int getStopAfter();

		static void soundEventCB(void *obj, SoundManager::SoundEvent ev)
		{
			PlaybackOptionsWindow* win = static_cast<PlaybackOptionsWindow*>(obj);
			if (win)
			{
				win->onSoundEvent(ev);
			}
		}

	private:
		std::vector<Sprite*> mSprites;
		std::vector<IWidget*> mWidgets;

		SimpleLabel* mTitleLabel;
		SimpleLabel* mPlayModeLabel;
		Button* mCreateListBtn;
		Sprite* mBackground;
		Checkbox* mRandomChk;
		Checkbox* mStopAfterChk;
		ComboBox* mOrderByCombo;
		Checkbox* mRepeatChk;
		ComboBox* mStopAfterCombo;

		vector<Track*> mHiddenList;
		int mHiddenListIndex;

		int mLastPlayedId;
		int mLastConfigedAlbum;
		string mLastConfigedArtist;

		int mX;
		int mCurrWidth;
		bool mResized;

		static const int WINDOW_HEIGHT;

		void initHiddenList(bool tryReadFirst = false);
		string getFilename();
		void readList();
		void writeList();
		vector<Track*> populateList();
		void createRepeatingList();
		void onSoundEvent(SoundManager::SoundEvent ev);

		void onBtnPushed(Button* btn);
		void onChkPushed(Checkbox* btn);
		void onComboSelected(ComboBox* combo);

	public:
		static enum ORDER_BY { SONG, ALBUM, ARTIST, QUEUE, NEVER };
	};
}
#endif
