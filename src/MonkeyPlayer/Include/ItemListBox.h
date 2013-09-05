// ItemListBox.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A box with selectable items
#include <map>
#include <vector>

#include "IWidget.h"
#include "ListItem.h"
#include "RenderTarget.h"
#include "Sprite.h"

#ifndef ITEM_LIST_BOX_H
#define ITEM_LIST_BOX_H

class ItemListBox : public IWidget
{
public:
	ItemListBox(float x, float y, float width, float height,
		void (*selectedtItemCB)(void* ptrObj, ItemListBox* selItem) = NULL, void* callbackObj = NULL,
		D3DXCOLOR bgColor = D3DCOLOR_RGBA(0, 0, 0, 255), D3DXCOLOR fontColor = D3DCOLOR_XRGB(255, 255, 255),
		D3DXVECTOR4 highlightColor = D3DXVECTOR4(0.0f, 0.0f, 0.85f, 1.0f));
	~ItemListBox();

	void onDeviceLost();
	void onDeviceReset();
	void recreateTargets();

	void setBgColor(D3DXCOLOR c);

	void update(float dt);

	virtual void preRender();
	void display() {}
	void setPos(float x, float y, float width, float height);

	std::vector<Sprite*> getSprites();
	int getNumTriangles();
	void clearItems();
	void setItems(std::vector<ListItem*> items);
	void addItems(std::vector<ListItem*> items);
	void addItem(ListItem* item);
	void modifyItems(std::vector<ListItem*> items); // updates if item with id exists...does NOT claim memory
	void modifyItem(ListItem* item); // updates if item with id exists...does NOT claim memory
	unsigned int getNumItemsDisplayed();
	virtual bool onMouseEvent(MouseEvent e);
	
	virtual int findItem(std::string &name);
	virtual ListItem* setHighlightedItem(std::string &name);
	virtual ListItem* setHighlightedItem(int index);
	ListItem* getSelectedItem();
	void setSelectedIndex(int index);
	int getSelectedIndex();
	ListItem* getItem(int index);
	int getNumItems();

	bool getAllowSingleClickSelection();
	void setAllowSingleClickSelection(bool allow);

	bool isPointInside(int x, int y);

protected:
// when holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
	static const float BUTTON_REPEAT_TIME;
// upon first holding up/down/pgUp/pgDn, it won't repeat until this interval passes (seconds)
	static const float BUTTON_REPEAT_DELAY;
	// number of items to scroll when using page up/page down
	static const int NUM_PAGING_ITEMS;
	// number of items to scroll when using scroll wheel
	static const int NUM_SCROLLING_ITEMS;

	static const int TEXT_MARGIN_TOP;
	static const int TEXT_MARGIN_BOTTOM;
	static const int TEXT_MARGIN_LEFT;
	static const int TEXT_MARGIN_RIGHT;

	std::vector<Sprite*> mSprites; 
	std::vector<ListItem*> mItems;
	std::map<int, ListItem*> mItemMap;
	Sprite* mScrollbar;

	ID3DXFont* mFont;
	D3DXCOLOR mFontColor;
	
	float mX, mY, mWidth, mHeight;
	float mTextX, mTextY, mTextWidth, mTextHeight;
	std::vector<int> mSelectedIndices;
	int mCurrSelection;

	unsigned int mStartDisplayIndex;
	unsigned int mEndDisplayIndex;

	// main list area
	RenderTarget* mListTarget;
	Sprite* mListSprite;
	Sprite*  mHighlightedSprite;

	// scrolling area
	Sprite* mScrollHandle;
	float mScrollBarWidth;
	float mScrollBarHeight;

	int mFontHeight; 
	float mUpDownTimer;
	float mPageTimer;
	bool mHoldDelayPassed;

	bool mAllowSingleClickSelection;
	bool mStartedOnTop;
	bool mDoRedraw;

	void deleteItems();
	void updateScrollBar();
	
	int getItemAtPos(int x, int y);
	// callback
	void (*mCallback)(void* ptrObj, ItemListBox* listBox);
	void *mCallbackObj;

protected:
	// synchronization
	static CCriticalSection mCritSection;
};
#endif