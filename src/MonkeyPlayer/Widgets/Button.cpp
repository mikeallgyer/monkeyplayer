// Button.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A clickable button

#include "Button.h"

const int Button::TEXTURE_UP = 0;
const int Button::TEXTURE_HOVER = 1;
const int Button::TEXTURE_DOWN = 2;
const int Button::TEXTURE_TOGGLED = 3;

Button::Button(float x, float y, float width, float height, std::string &texture,
	void (*buttonClickedCB)(void* ptrObj, Button* btn), void* callbackObj)
{
	mX = x;
	mY = y;
	mWidth = width;
	mHeight = height;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;
	mButtonSprite = snew Sprite(texture.c_str(), mX, mY, mWidth, mHeight);
	mSprites.push_back(mButtonSprite);

	mCallback = buttonClickedCB;
	mCallbackObj = callbackObj;
	mStartedOnTop = false;
	mIsToggle = false;
	mToggled = false;
}

Button::~Button()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
}

void Button::onDeviceLost()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
}
void Button::onDeviceReset()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
}

void Button::update(float dt)
{
}

void Button::preRender()
{
}

void Button::setPos(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	if (width != -999.0f)
	{
		mWidth = width;
	}
	if (height != -999.0f)
	{
		mHeight = height;
	}
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;
	mButtonSprite->setDest(mX, mY, mWidth, mHeight);
}
void Button::setUpTexture(const char* texture)
{
	mButtonSprite->addTexture(TEXTURE_UP, texture, false);
}
void Button::setHoverTexture(const char* texture)
{
	mButtonSprite->addTexture(TEXTURE_HOVER, texture, false);
}
void Button::setDownTexture(const char* texture)
{
	mButtonSprite->addTexture(TEXTURE_DOWN, texture, false);
}
void Button::setToggledTexture(const char* texture)
{
	mButtonSprite->addTexture(TEXTURE_TOGGLED, texture, false);
}

std::vector<Sprite*> Button::getSprites()
{
	return mSprites;
}
int Button::getNumTriangles()
{
	int num = 0;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		num += mSprites[i]->getNumTriangles();
	}
	return num;
}

bool Button::onMouseEvent(MouseEvent e)
{
	if (e.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (isPointInside(e.getX(), e.getY()))
		{
			if (mStartedOnTop && gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
			{
				mButtonSprite->setTextureIndex(TEXTURE_DOWN);
			}
			else
			{
				mButtonSprite->setTextureIndex(TEXTURE_HOVER);
			}
		}
		else if (mButtonSprite->getCurrentIndex() != TEXTURE_UP &&
			!(mIsToggle && mToggled && mButtonSprite->getCurrentIndex() == TEXTURE_TOGGLED))
		{
			if (!mIsToggle || !mToggled)
			{
				mButtonSprite->setTextureIndex(TEXTURE_UP);
			}
			else
			{
				mButtonSprite->setTextureIndex(TEXTURE_TOGGLED);
			}
		}
	}
	else if (!e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONDOWN &&
		isPointInside(e.getX(), e.getY()))
	{
		mButtonSprite->setTextureIndex(TEXTURE_DOWN);
		mStartedOnTop = true;
	}
	else if (mStartedOnTop && !e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONUP &&
		isPointInside(e.getX(), e.getY()))
	{
		mButtonSprite->setTextureIndex(TEXTURE_HOVER);
		if (mIsToggle)
		{
			setToggled(!mToggled);
		}
		mStartedOnTop = false;
		if (mCallback != NULL)
		{
			mCallback(mCallbackObj, this);
		}
	}

	return false;
}

void Button::refresh()
{
	// simulate a mouse event
	MouseEvent ev(MouseEvent::MOUSEMOVE, gInput->currMouseX(), gInput->currMouseY(), 0, 0);
	onMouseEvent(ev);
}

void Button::setIsToggle(bool isToggle)
{
	mIsToggle = isToggle;
}
bool Button::getIsToggle()
{
	return mIsToggle;
}
void Button::setToggled(bool toggled)
{
	mToggled = toggled;
	if (mToggled)
	{
		mButtonSprite->setTextureIndex(TEXTURE_TOGGLED);
	}
	else
	{
		mButtonSprite->setTextureIndex(TEXTURE_UP);
	}
}
bool Button::getToggled()
{
	return mToggled;
}

bool Button::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY || xPoint > mX2 || yPoint > mY2);
}
