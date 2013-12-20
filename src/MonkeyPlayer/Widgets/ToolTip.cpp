// ToolTip.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A simple tooltip
#include "ToolTip.h"
#include "FileManager.h"

using namespace MonkeyPlayer;

const float ToolTip::TOOLTIP_DIMENSION = 25.0f;

ToolTip::ToolTip(float x, float y, string text)
{
	mX = x;
	mY = y;
	mWidth = TOOLTIP_DIMENSION;
	mHeight = TOOLTIP_DIMENSION;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;

	mLabel = snew SimpleLabel(mX, mY, mWidth, mHeight, text, 18, DT_VCENTER, D3DCOLOR_XRGB(0,0,0),
		D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f));
	mLabel->setSizeToFit(true);

	std::string whitePath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mBackgroundSprite = snew Sprite(whitePath.c_str(), mX, mY, mWidth, mHeight, D3DXVECTOR4(1.0f, 1.0f, .8826f, 1.0));
	mBorderSprite = snew Sprite(whitePath.c_str(), mX, mY, mWidth, mHeight, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0));

	mStartedOnTop = false;
	mVisible = false;
	mTextChanged = true;
	mParentWidget = NULL;
}

ToolTip::~ToolTip()
{
	delete mLabel;
	delete mBackgroundSprite;
	delete mBorderSprite;
}

void ToolTip::onDeviceLost()
{
	mLabel->onDeviceLost();
	mBackgroundSprite->onDeviceLost();
	mBorderSprite->onDeviceLost();
}
void ToolTip::onDeviceReset()
{
	mLabel->onDeviceReset();
	mBackgroundSprite->onDeviceReset();
	mTextChanged = true;
	mBorderSprite->onDeviceReset();
}

void ToolTip::update(float dt)
{
	mLabel->update(dt);

	if (mTextChanged)
	{
		mSprites.clear();
		mSprites.push_back(mBorderSprite);
		mSprites.push_back(mBackgroundSprite);
		vector<Sprite*> sprites = mLabel->getSprites();
		for (unsigned int i = 0; i < sprites.size(); i++)
		{
			mSprites.push_back(sprites[i]);
		}

		mTextChanged = false;
	}

	if (mVisible)
	{
		mTimer -= dt;
		if (mTimer <= 0)
		{
			mVisible = false;
		}
	}
}

void ToolTip::preRender()
{
	if (mVisible)
	{
		mLabel->preRender();
	}
}

void ToolTip::setPos(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	if (width != 0 && height != 0)
	{
		mWidth = width;
		mHeight = height;
	}
	mX2 = mX + TOOLTIP_DIMENSION;
	mY2 = mY + TOOLTIP_DIMENSION;

	const float BORDER_WIDTH = 1.0f;
	mLabel->setPos(mX, mY);
	mWidth = mLabel->getWidth();
	mHeight = mLabel->getHeight();
	mBorderSprite->setDest(mX - BORDER_WIDTH, mY - BORDER_WIDTH, mWidth + BORDER_WIDTH * 2.0f, mHeight + BORDER_WIDTH * 2.0f);
	mBackgroundSprite->setDest(mX, mY, mWidth, mHeight);
	mTextChanged = true;
}
std::vector<Sprite*> ToolTip::getSprites()
{
	if (!mVisible)
	{
		static vector<Sprite*> empty;
		return empty;
	}
	return mSprites;
}
int ToolTip::getNumTriangles()
{
	int num = mLabel->getNumTriangles();
	return num;
}

bool ToolTip::onMouseEvent(MouseEvent e)
{
	if (e.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (mParentWidget != NULL && !mParentWidget->isPointInside(e.getX(), e.getY()))
		{
			mParentWidget = NULL;
			mVisible = false;
		}
	}
	else if (e.getEvent() == MouseEvent::LBUTTONDOWN || e.getEvent() == MouseEvent::RBUTTONDOWN)
	{
		mVisible = false;
	}
	return false;
}

void ToolTip::refresh()
{
	// simulate a mouse event
	MouseEvent ev(MouseEvent::MOUSEMOVE, gInput->currMouseX(), gInput->currMouseY(), 0, 0);
	onMouseEvent(ev);
}
bool ToolTip::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY || xPoint > mX2 || yPoint > mY2) || mLabel->isPointInside(x, y);
}

void ToolTip::setVisible(bool visible, IWidget* widget)
{
	mVisible = visible;
	mParentWidget = widget;
	mTextChanged = true;
}
bool ToolTip::getVisible()
{
	return mVisible;
}

void ToolTip::setText(string text)
{
	mText = text;
	mLabel->setString(text);
	mTextChanged = true;
}
void ToolTip::setup(IWidget* parent, string text, int x, int y)
{
	if ((!mVisible && mParentWidget != parent) || mText != text)
	{
		setVisible(true, parent);
		setText(text);
		setPos((float)x, (float)y + 25.0f);
		mTimer = 5.0f;
	}
}
