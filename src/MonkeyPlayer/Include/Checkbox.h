// Checkbox.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A clickable Checkbox
#include <string>
#include <vector>

#include "IWidget.h"
#include "Label.h"
#include "Sprite.h"

#ifndef Checkbox_H
#define Checkbox_H

class Checkbox : public IWidget
{
public:
	Checkbox(float x, float y, string text,
		void (*CheckboxClickedCB)(void* ptrObj, Checkbox* btn) = NULL, void* callbackObj = NULL);
	~Checkbox();

	void onDeviceLost();
	void onDeviceReset();

	void update(float dt);

	virtual void preRender();
	void display() {}
	void setPos(float x, float y, float width = 0, float height = 0);
	float getX() { return mX; }
	float getY() { return mY; }
	float getWidth() { return mWidth + mLabel->getWidth(); }
	float getHeight() { return mHeight; }

	std::vector<Sprite*> getSprites();
	int getNumTriangles();
	virtual bool onMouseEvent(MouseEvent e);
	virtual void refresh();
	void setChecked(bool checked);
	bool getChecked();
	void setText(string text);

protected:

	static const int TEXTURE_UP;
	static const int TEXTURE_DOWN;
	static const int TEXTURE_HOVER;
	static const int TEXTURE_CHECKED_UP;
	static const int TEXTURE_CHECKED_HOVER;
	static const int TEXTURE_CHECKED_DOWN;
	static const float CHECKBOX_DIMENSION;
	std::vector<Sprite*> mSprites; 
	Label* mLabel;

	float mX, mY, mWidth, mHeight, mX2, mY2;
	bool mChecked;
	Sprite*  mCheckboxSprite;
	bool mStartedOnTop;
	string mText;
	bool mTextChanged;

	bool isPointInside(int x, int y);

	// callback
	void (*mCallback)(void* ptrObj, Checkbox* selItem);
	void *mCallbackObj;

};
#endif