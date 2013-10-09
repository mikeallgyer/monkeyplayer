// Label.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A basic text label

#include "Label.h"

Label::Label(float x, float y, float width, float height, std::string &label, int fontSize,
				 DWORD format, D3DXCOLOR textColor, D3DXCOLOR bgColor,  const char* fontName)
{
	D3DXFONT_DESC font;
	font.Height = fontSize;
	font.Width = 0;
	font.Weight = 0;
	font.MipLevels = 1;
	font.Italic = false;
	font.CharSet = DEFAULT_CHARSET;
	font.OutputPrecision = OUT_DEFAULT_PRECIS;
	font.Quality = ANTIALIASED_QUALITY;//DEFAULT_QUALITY;
	font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(font.FaceName, _T(fontName));

	HR(D3DXCreateFontIndirect(gDevice, &font, &mFont));
	mFormat = format;
	mTextColor = textColor;
	mFontSize = (float)fontSize;
	mSizeToFit = false;

	mX = floor(x);
	mY = floor(y);
	mWidth = width;
	mHeight = height;

	mSprite = NULL;
	mText = label;

	mTarget = snew RenderTarget((int)width, (int)height, bgColor, false);

	mCallback = NULL;
	mCallbackObj = NULL;
	mRedraw = true;
}
Label::~Label()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	ReleaseCOM(mFont);

	delete mTarget;
}

void Label::onDeviceLost()
{
	HR(mFont->OnLostDevice());
	mTarget->onDeviceLost();
}
void Label::onDeviceReset()
{
	HR(mFont->OnResetDevice());

	mTarget->onDeviceReset();
	recreateTargets();
}
void Label::recreateTargets()
{
	if (mSprite == NULL)
	{
		if (mSizeToFit)
		{
			RECT r = { 0, 0, (int)mWidth, (int)mHeight };
			HR(mFont->DrawText(0, mText.c_str(), -1, &r, DT_CALCRECT, mTextColor));
			
			mWidth = (float)r.right;
			//mHeight = (float)r.bottom;
		}
		mSprite = snew Sprite(mTarget->getTexture(), mX, mY, mWidth, mHeight);
		mSprites.push_back(mSprite);
	}
	else
	{
		if (mSizeToFit)
		{
			RECT r = { 0, 0, (int)mWidth, (int)mHeight };
			HR(mFont->DrawText(0, mText.c_str(), -1, &r, DT_CALCRECT, mTextColor));
			
			mWidth = (float)r.right;
			//mHeight = (float)r.bottom;
			mTarget->setDimensions((int)mWidth, (int)mHeight);
		}
		mSprite->setDest(mX, mY, mWidth, mHeight);
		mSprite->replaceCurrentTexture(mTarget->getTexture(), false);
	}
	mRedraw = true;
}

void Label::update(float dt)
{
}

void Label::preRender()
{
	if  (mRedraw)
	{
		mTarget->beginScene();

		RECT r = { 0, 0, (int)mWidth, (int)mHeight };

		HR(mFont->DrawText(0, mText.c_str(), -1, &r, mFormat, mTextColor));

		mTarget->endScene();
		mRedraw = false;
	}
}
void Label::setPos(float x, float y, float width, float height)
{
	mX = floor(x);
	mY = floor(y);

	if (!mSizeToFit || width != mWidth || height != mHeight)
	{
		if (height > 0)
		{
			mHeight = height;
		}
		if (width > 0)
		{
			mWidth = width;
		}
		mTarget->setDimensions((int)mWidth, (int)mHeight);
	}
	recreateTargets();
}

void Label::setFormat(DWORD format)
{
	mFormat = format;
	mRedraw = true;
}

void Label::setTextColor(D3DXCOLOR color)
{
	mTextColor = color;
	mRedraw = true;
}

void Label::setString(std::string &str)
{
	mText = str;
	mRedraw = true;
	if (mSizeToFit)
	{
		recreateTargets();
	}
}
std::string Label::getString()
{
	return mText;
}

std::vector<Sprite*> Label::getSprites()
{
	return mSprites;
}
int Label::getNumTriangles()
{
	int total = 0;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		total += mSprites[i]->getNumTriangles();
	}

	return total;
}
bool Label::onMouseEvent(MouseEvent e)
{
	if (isPointInside(e.getX(), e.getY()))
	{
		if (e.getEvent() == MouseEvent::LBUTTONUP)
		{
			if (mCallback != NULL)
			{
				mCallback(mCallbackObj, this);
			}
			return true;
		}
		if (e.getEvent() == MouseEvent::LBUTTONDBLCLK)
		{
			return true;
		}
	}
	return false;
}
bool Label::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;
	float x2 = mX + mWidth;
	float y2 = mY + mHeight;

	return !(xPoint < mX || yPoint < mY || xPoint > x2 || yPoint > y2);
}
void Label::setSizeToFit(bool fit)
{
	mSizeToFit = fit;
	setPos(mX, mY);
	mRedraw = true;
	recreateTargets();
}

void Label::setCallback(void (*cb)(void* objPtr, Label* label), void* objPtr)
{
	mCallback = cb;
	mCallbackObj = objPtr;
}
