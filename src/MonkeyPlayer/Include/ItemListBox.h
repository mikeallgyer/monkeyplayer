// ItemListBox.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A box with selectable items
#include <map>
#include <vector>

#ifndef ITEM_LIST_BOX_H
#define ITEM_LIST_BOX_H

#include "IWidget.h"
#include "ListItem.h"
#include "RenderTarget.h"
#include "Sprite.h"
#include "VerticalScrollBar.h"

namespace MonkeyPlayer
{
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
		void setPos(float x, float y, bool autoSize);
		float getX();
		float getY();
		float getWidth();
		float getHeight();
		float getWidthToFit();
		float getHeightToFit();

		std::vector<Sprite*> getSprites();
		int getNumTriangles();
		virtual void clearItems();
		virtual void shuffleItems();
		void setItems(std::vector<ListItem*> items);
		void addItems(std::vector<ListItem*> items);
		void addItem(ListItem* item);
		void addItem(ListItem* item, unsigned int index);
		void addItems(std::vector<ListItem*> items, unsigned int index);
		void modifyItems(std::vector<ListItem*> items); // updates if item with id exists...does NOT claim memory
		void modifyItem(ListItem* item); // updates if item with id exists...does NOT claim memory
		virtual void removeItems(vector<int> items); 
		unsigned int getNumItemsDisplayed();
		virtual bool onMouseEvent(MouseEvent e);
		
		virtual int findItem(std::string &name);
		virtual ListItem* setHighlightedItem(std::string &name);
		virtual ListItem* setHighlightedItem(int index);
		ListItem* getSelectedItem();
		void setSelectedIndex(int index);
		int getSelectedIndex();
		vector<int> getSelectedIndices();
		void scrollToIndex(int index);
		ListItem* getItem(int index);
		vector<ListItem*> getItems();
		int getNumItems();

		bool getAllowSingleClickSelection();
		void setAllowSingleClickSelection(bool allow);
		void setAllowMultipleSelection(bool allow);

		bool isPointInside(int x, int y);
		int getItemAtPos(int x, int y);

		virtual void updateScrollbarVisibility();
		void setDrawables();

		static void scrollbar_callback(void* obj, VerticalScrollBar* scrollbar, float percent)
		{
			ItemListBox* b = static_cast<ItemListBox*>(obj);
			if (b)
			{
				b->onScrollbarMoved(scrollbar, percent);
			}
		}

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

		static const float HOVER_DURATION;

		std::vector<Sprite*> mSprites; 
		std::vector<Sprite*> mDrawableSprites; 
		std::vector<ListItem*> mItems;
		map<int, float> mHoverItems;
		int mCurrHoverIndex;

		ID3DXFont* mFont;
		D3DXCOLOR mFontColor;
		
		float mX, mY, mWidth, mHeight;
		float mTextX, mTextY, mTextWidth, mTextHeight;
		std::vector<int> mSelectedIndices;
		int mCurrSelection;
		bool mAllowMultipleSel;
		int mMultipleSelBegin;

		unsigned int mStartDisplayIndex;
		unsigned int mEndDisplayIndex;

		// main list area
		RenderTarget* mListTarget;
		Sprite* mListSprite;
		Sprite* mHighlightedSprite;
		Sprite* mHoverSprite;

		VerticalScrollBar* mScrollBar;

		int mFontHeight; 
		float mUpDownTimer;
		float mPageTimer;
		bool mHoldDelayPassed;

		bool mAllowSingleClickSelection;
		bool mStartedOnTop;
		bool mDoRedraw;

		void deleteItems();
		void updateScrollBar();
	
		void onScrollbarMoved(VerticalScrollBar* bar, float percent);

		// callback
		void (*mCallback)(void* ptrObj, ItemListBox* listBox);
		void *mCallbackObj;

	protected:
		// synchronization
		static CCriticalSection mCritSection;
	};
}
#endif