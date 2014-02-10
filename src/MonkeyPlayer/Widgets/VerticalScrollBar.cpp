// VerticalScrollBar.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A clickable VerticalScrollBar

#include "FileManager.h"
#include "VerticalScrollBar.h"

using namespace MonkeyPlayer;

const float VerticalScrollBar::HANDLE_WIDTH = 12.0f;
const float VerticalScrollBar::HANDLE_HEIGHT = 30.0f;
const float VerticalScrollBar::PATH_WIDTH = 1.0f;

VerticalScrollBar::VerticalScrollBar(float x, float y, float height,
	void (*scrollBarMovedCB)(void* ptrObj, VerticalScrollBar* bar, float percent), void* callbackObj)
{
	mX = x;
	mY = y;
	mWidth = HANDLE_WIDTH;
	mHeight = height;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;
	mHandleMinY = mY;
	mHandleMaxY = mY2 - HANDLE_HEIGHT;
	mMouseMinY = mY + HANDLE_HEIGHT * .5f;
	mMouseMaxY = mY2 - HANDLE_HEIGHT * .5f;

	std::string scrollPath = FileManager::getContentAsset(std::string("Textures\\scrollHandle.png"));
	mHandleSprite = snew Sprite(scrollPath.c_str(), mX, mY, HANDLE_WIDTH, HANDLE_HEIGHT);

	std::string whitePath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mPathSprite = snew Sprite(whitePath.c_str(), mX, mY, PATH_WIDTH, PATH_WIDTH);

	mSprites.push_back(mPathSprite);
	mSprites.push_back(mHandleSprite);

	mCallback = scrollBarMovedCB;
	mCallbackObj = callbackObj;
	mStartedOnTop = false;
	mVisible = true;
	setHandlePosition(0);
}

VerticalScrollBar::~VerticalScrollBar()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
}

void VerticalScrollBar::onDeviceLost()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
}
void VerticalScrollBar::onDeviceReset()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
}

void VerticalScrollBar::update(float dt)
{
}

void VerticalScrollBar::preRender()
{
}

void VerticalScrollBar::setPos(float x, float y, float height)
{
	mX = x;
	mY = y;
	if (height != -999.0f)
	{
		mHeight = height;
	}
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;

	mHandleMinY = mY;
	mHandleMaxY = mY2 - HANDLE_HEIGHT;
	mMouseMinY = mY + HANDLE_HEIGHT * .5f;
	mMouseMaxY = mY2 - HANDLE_HEIGHT * .5f;

	mPathSprite->setDest(mX + HANDLE_WIDTH * .5f, mY, PATH_WIDTH, mHeight);

	setHandlePosition(mCurrHandlePos);
}
void VerticalScrollBar::setHandlePosition(float percent)
{
	mCurrHandlePos = min(1.0f, max(0, percent));

	mHandleSprite->setDest(mX, mHandleMinY + (mHandleMaxY - mHandleMinY) * mCurrHandlePos, HANDLE_WIDTH, HANDLE_HEIGHT);
}

std::vector<Sprite*> VerticalScrollBar::getSprites()
{
	return mSprites;
}
int VerticalScrollBar::getNumTriangles()
{
	int num = 0;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		num += mSprites[i]->getNumTriangles();
	}
	return num;
}

bool VerticalScrollBar::onMouseEvent(MouseEvent e)
{
	if (mStartedOnTop && e.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
		{
				setHandleAtMouse(e.getY());
		}
		else
		{
			mStartedOnTop = false;
		}
	}
	if (e.getEvent() == MouseEvent::LBUTTONDOWN)
	{
		if (isPointInside(e.getX(), e.getY()))
		{
			if (isPointInsideHandle(e.getX(), e.getY()))
			{
				mStartedY = -(e.getY() - mHandleSprite->getY());
			}
			else
			{
				// too tired to think of a better way
				if (e.getY() < mMouseMinY)
				{
					mStartedY = e.getY() - mY;
				}
				else if (e.getY() > mMouseMaxY)
				{
					mStartedY = mY2 - e.getY();
				}
				else 
				{
					mStartedY = -(mHandleSprite->getHeight() * .5f);
				}
				setHandleAtMouse(e.getY());
			}
			mStartedOnTop = true;
		}
	}
	else if (e.getEvent() == MouseEvent::LBUTTONUP)
	{
		mStartedOnTop = false;
	}
	return false;
}

void VerticalScrollBar::refresh()
{
	// simulate a mouse event
	MouseEvent ev(MouseEvent::MOUSEMOVE, gInput->currMouseX(), gInput->currMouseY(), 0, 0);
	onMouseEvent(ev);
}

bool VerticalScrollBar::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY || xPoint > mX2 || yPoint > mY2);
}
bool VerticalScrollBar::isPointInsideHandle(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mHandleSprite->getX() || yPoint < mHandleSprite->getY() ||
		xPoint > (mHandleSprite->getX() + mHandleSprite->getWidth()) ||
		yPoint > (mHandleSprite->getY() + mHandleSprite->getHeight()));
}

void VerticalScrollBar::setHandleAtMouse(int y)
{
	y += mStartedY;
	float cappedY = min(mHandleMaxY, max(mHandleMinY, (float)y));

	float percent = (cappedY - mHandleMinY) / (mHandleMaxY - mHandleMinY);
	setHandlePosition(percent);

	if (mCallbackObj != NULL)
	{
		mCallback(mCallbackObj, this, percent);
	}
}
