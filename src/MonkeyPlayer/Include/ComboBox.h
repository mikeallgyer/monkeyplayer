// ComboBox.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A  ComboBox
#include <string>
#include <vector>

#include "ItemListBox.h"
#include "IWidget.h"
#include "Label.h"
#include "Sprite.h"

#ifndef COMBOBOX_H
#define COMBOBOX_H

class ComboBox : public IWidget
{
public:
	ComboBox(float x, float y, string text, float width,
		void (*ComboBoxClickedCB)(void* ptrObj, ComboBox* btn) = NULL, void* callbackObj = NULL);
	~ComboBox();

	void onDeviceLost();
	void onDeviceReset();

	void update(float dt);

	virtual void preRender();
	void display() {}
	void setPos(float x, float y, float width = 0, float height = 0);
	float getX() { return mX; }
	float getY() { return mY; }
	float getWidth() { return mWidth + COMBOBOX_DIMENSION; }
	float getHeight() { return mHeight; }

	std::vector<Sprite*> getSprites();
	int getNumTriangles();
	virtual bool onMouseEvent(MouseEvent e);
	virtual void refresh();
	void setDroppedDown(bool droppedDown);
	bool getDroppedDown();
	void setText(string text);
	void setList(vector<ListItem*> list);

	static void listBox_callback(void* obj, ItemListBox* listBox)
	{
		ComboBox* win = static_cast<ComboBox*>(obj);
		if (win)
		{
			win->onItemSelected(listBox);
		}
	}

protected:

	static const int BACKGROUND_UP;
	static const int BACKGROUND_DOWN;

	static const int ARROW_UP;
	static const int ARROW_HOVER;
	static const int ARROW_DOWN;
	static const float COMBOBOX_DIMENSION;
	std::vector<Sprite*> mSprites; 
	Label* mLabel;
	ItemListBox* mListBox;

	float mX, mY, mWidth, mHeight, mX2, mY2;
	bool mDroppedDown;
	Sprite*  mArrowSprite;
	Sprite*  mBackgroundSprite;
	bool mStartedOnTop;
	string mText;
	bool mTextChanged;

	bool isPointInside(int x, int y);

	void onItemSelected(ItemListBox* listBox);

	// callback
	void (*mCallback)(void* ptrObj, ComboBox* selItem);
	void *mCallbackObj;

};
#endif