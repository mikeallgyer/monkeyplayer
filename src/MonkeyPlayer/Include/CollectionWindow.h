// CollectionWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a collection of songs


#include "Button.h"
#include "Checkbox.h"
#include "d3dUtil.h"
#include "ItemListBox.h"
#include "IWidget.h"
#include "IWindow.h"
#include "Label.h"
#include "LargeAlbumWidget.h"
#include "SmallAlbumManager.h"
#include "SoundManager.h"
#include "Sprite.h"
#include "TrackListBox.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef COLLECTION_WINDOW_H
#define COLLECTION_WINDOW_H

class CollectionWindow : public IWindow
{
public:
	enum DISPLAY_STYLE { LargeAlbum, SmallAlbum, UNDEFINED_STYLE };

	CollectionWindow();
	~CollectionWindow();

	void onDeviceLost();
	void onDeviceReset();

	int getWidth();
	int getHeight();

	void update(float dt);

	void display();

	std::vector<Sprite*> getSprites();
	std::vector<IWidget*> getWidgets();

	bool onMouseEvent(MouseEvent ev);  // true if window consumed event
	bool isPointInside(int x, int y);
	void onBlur();
	void onFocus();

	void setDisplayStyle(DISPLAY_STYLE style);

	void addAlbum(Album* album);
	void addTrack(Track* track);

	static void btn_callback(void* obj, Button* btn)
	{
		CollectionWindow* win = static_cast<CollectionWindow*>(obj);
		if (win)
		{
			win->onBtnPushed(btn);
		}
	}
	static void chk_callback(void* obj, Checkbox* btn)
	{
		CollectionWindow* win = static_cast<CollectionWindow*>(obj);
		if (win)
		{
			win->onChkPushed(btn);
		}
	}
	static void sound_callback(void* obj, SoundManager::SoundEvent ev)
	{
		CollectionWindow* win = static_cast<CollectionWindow*>(obj);
		if (win)
		{
			win->onSoundEvent(ev);
		}
	}
private:
	void goToSong();
	void onBtnPushed(Button* btn);
	void onChkPushed(Checkbox* chk);
	void onSoundEvent(SoundManager::SoundEvent ev);
	
	// size of artist label
	static const float ARTIST_LABEL_SIZE;

	ID3DXFont* mFont;

	std::vector<Sprite*> mSprites;
	std::vector<IWidget*> mWidgets;
	std::vector<IWidget*> mWidgetsToDraw;
	Sprite* mBackground;
	std::vector<Label*> mArtistLabels;
	LargeAlbumWidget* mLargeAlbumWidget;
	SmallAlbumManager* mSmallAlbumManager;

	Label* mAlphabetLabel;
	Label* mLetterLabel;
	Button* mMagnifier;
	Button* mSmallAlbumBtn;
	Button* mLargeAlbumBtn;
	Checkbox* mGoToSongChk;
	std::vector<Album*> mAlbumsToAdd;
	std::vector<Track*> mTracksToAdd;

	float mX, mY;
	int mCurrWidth;
	bool mResized;

	float mUpDownTimer;
	float mPageTimer;
	bool mHoldDelayPassed;

	bool mHasFocus;
	bool mMouseAlphabetStartedDown;
	char mHoverChar;
	bool mGoToHover;

protected:

	void setDrawableWidgets();

	// synchronization
	static CCriticalSection mCritSection;
	DISPLAY_STYLE mCurrStyle;
};

#endif
