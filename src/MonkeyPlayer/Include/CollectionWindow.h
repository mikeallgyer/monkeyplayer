// CollectionWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a collection of songs


#include "Button.h"
#include "d3dUtil.h"
#include "ItemListBox.h"
#include "IWidget.h"
#include "IWindow.h"
#include "Label.h"
#include "SmallAlbumItem.h"
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
	enum DISPLAY_STYLE { LargeAlbum, SmallAlbum, UNDEFINED };

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

	void updateSmallDisplay();
	void setDisplayStyle(DISPLAY_STYLE style);

	void addAlbum(Album* album);
	void addTrack(Track* track);
private:
	// when holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
	static const float BUTTON_REPEAT_TIME;
	// upon first holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
	static const float BUTTON_REPEAT_DELAY;
	// speed to scroll when using page up/page down
	static const int NUM_PAGING_ITEMS;
	// magic scroll speed
	static const float SCROLL_SPEED;
	// size of artist label
	static const float ARTIST_LABEL_SIZE;

	void doAddAlbum(Album* album);
	void doAddTrack(Track* track);

	ID3DXFont* mFont;

	std::vector<Sprite*> mSprites;
	std::vector<IWidget*> mWidgets;
	Sprite* mBackground;
	std::vector<SmallAlbumItem*> mSmallItems;
	std::vector<Label*> mArtistLabels;

	Label* mAlphabetLabel;
	Label* mLetterLabel;
	Button* mMagnifier;
	std::vector<Album*> mAlbumsToAdd;
	std::vector<Track*> mTracksToAdd;

	int mCurrWidth;
	bool mResized;

	float mCurrDisplayAlbum;  // album displayed at top of this window
	int mCurrSelAlbum;  // album containing current selection

	float mUpDownTimer;
	float mPageTimer;
	bool mHoldDelayPassed;

	bool mHasFocus;
	bool mMouseAlphabetStartedDown;
	char mHoverChar;
	bool mGoToHover;

protected:
	// synchronization
	static CCriticalSection mCritSection;
	DISPLAY_STYLE mCurrStyle;

	void moveSmallSelection(int cursorDelta);
	void moveUpToSmallSelection();
	void moveDownToSmallSelection();

};

#endif
