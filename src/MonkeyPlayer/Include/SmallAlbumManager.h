// SmallAlbumManager.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// small albums

#include "Button.h"
#include "d3dUtil.h"
#include "ItemListBox.h"
#include "IWidget.h"
#include "IWindow.h"
#include "Label.h"
#include "LargeAlbumWidget.h"
#include "SmallAlbumItem.h"
#include "Sprite.h"
#include "TrackListBox.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef SMALL_ALBUM_MANAGER_H
#define SMALL_ALBUM_MANAGER_H

#include "CollectionWindow.h"

class SmallAlbumManager : IDrawable
{
public:

	SmallAlbumManager();
	~SmallAlbumManager();

	void onDeviceLost();
	void onDeviceReset();

	int getWidth();
	int getHeight();
	bool getAlbumsChanged();

	void setPos(float x, float y, float width, float height);
	void update(float dt);

	void display();
	int getNumTriangles();

	std::vector<Sprite*> getSprites();
	std::vector<IWidget*> getWidgets();

	bool onMouseEvent(MouseEvent ev);  // true if window consumed event
	bool isPointInside(int x, int y);
	void onBlur();
	void onFocus();

	void updateSmallDisplay();

	void getDrawableRange(int &topIndex, int &bottomIndex, float &fractPart);
	void goToChar(char c);
	void goToSong(Album a, Track t);

	void addAlbum(Album* album);
	void addTrack(Track* track);
	void onContextMenuSelected(ItemListBox* menu);
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
	std::vector<SmallAlbumItem*> mSmallItems;
	std::vector<Label*> mArtistLabels;
	std::vector<Album*> mAlbumsToAdd;
	std::vector<Track*> mTracksToAdd;

	int mCurrWidth;
	int mCurrHeight;
	float mCurrX;
	float mCurrY;
	bool mResized;
	bool mDoRedraw;
	bool mAlbumsChanged;

	float mCurrDisplayAlbum;  // album displayed at top of this window
	int mCurrSelAlbum;  // album containing current selection

	float mUpDownTimer;
	float mPageTimer;
	bool mHoldDelayPassed;

	bool mHasFocus;

	bool mGoToChar;
	char mChar;

	bool mGoToSong;
	int mGoToAlbumId;
	int mGoToSongId;

	CollectionWindow::SELECTED_THING mSelectedThing;

	CollectionWindow::RIGHT_CLICKED_ITEM mRightClicked;
	int mRightClickedTrack;
	SmallAlbumItem* mRightClickedAlbum;
	string mRightClickedArtist;

protected:
	// synchronization
	static CCriticalSection mCritSection;

	void moveSmallSelection(int cursorDelta);
	void moveUpToSmallSelection();
	void moveDownToSmallSelection();
};

#endif
