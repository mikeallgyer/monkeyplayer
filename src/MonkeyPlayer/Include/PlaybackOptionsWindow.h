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
	static void combo_callback(void* obj, ComboBox* box)
	{
		PlaybackOptionsWindow* win = static_cast<PlaybackOptionsWindow*>(obj);
		if (win)
		{
			win->onComboSelected(box);
		}
	}

	void setX(int x);
	void setWidth(int width);

private:
	std::vector<Sprite*> mSprites;
	std::vector<IWidget*> mWidgets;

	Label* mTitleLabel;
	Label* mPlayModeLabel;
	Button* mCreateListBtn;
	Sprite* mBackground;
	Checkbox* mRandomChk;
	Checkbox* mStopAfterChk;
	ComboBox* mOrderByCombo;
	Checkbox* mRepeatChk;
	ComboBox* mStopAfterCombo;

	int mX;
	int mCurrWidth;
	bool mResized;

	static const int WINDOW_HEIGHT;

	void onBtnPushed(Button* btn);
	void onChkPushed(Checkbox* btn);
	void onComboSelected(ComboBox* btn);

	static enum ORDER_BY { SONG, ALBUM, ARTIST, NEVER };
};

#endif
