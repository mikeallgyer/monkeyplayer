// Checkbox.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A clickable Checkbox

#include "Checkbox.h"
#include "FileManager.h"

const int Checkbox::TEXTURE_UP = 0;
const int Checkbox::TEXTURE_DOWN = 1;
const int Checkbox::TEXTURE_HOVER = 2;
const int Checkbox::TEXTURE_CHECKED_UP = 3;
const int Checkbox::TEXTURE_CHECKED_DOWN = 4;
const int Checkbox::TEXTURE_CHECKED_HOVER = 5;
const float Checkbox::CHECKBOX_DIMENSION = 30.0f;

Checkbox::Checkbox(float x, float y, string text,
	void (*CheckboxClickedCB)(void* ptrObj, Checkbox* btn), void* callbackObj)
{
	mX = x;
	mY = y;
	mWidth = CHECKBOX_DIMENSION;
	mHeight = CHECKBOX_DIMENSION;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;

	std::string uncheckedUpPath = FileManager::getContentAsset(std::string("Textures\\empty.png"));
	std::string uncheckedDownPath = FileManager::getContentAsset(std::string("Textures\\empty_down.png"));
	std::string uncheckedHoverPath = FileManager::getContentAsset(std::string("Textures\\empty_hover.png"));
	std::string checkedUpPath = FileManager::getContentAsset(std::string("Textures\\checkbox.png"));
	std::string checkedDownPath = FileManager::getContentAsset(std::string("Textures\\checkbox_down.png"));
	std::string checkedHoverPath = FileManager::getContentAsset(std::string("Textures\\checkbox_hover.png"));

	mCheckboxSprite = snew Sprite(uncheckedUpPath.c_str(), mX, mY, mWidth, mHeight);
	mCheckboxSprite->addTexture(TEXTURE_DOWN, uncheckedDownPath.c_str(), false);
	mCheckboxSprite->addTexture(TEXTURE_HOVER, uncheckedHoverPath.c_str(), false);
	mCheckboxSprite->addTexture(TEXTURE_CHECKED_UP, checkedUpPath.c_str(), false);
	mCheckboxSprite->addTexture(TEXTURE_CHECKED_DOWN, checkedDownPath.c_str(), false);
	mCheckboxSprite->addTexture(TEXTURE_CHECKED_HOVER, checkedHoverPath.c_str(), false);

	mLabel = snew Label(mX, mY, mWidth, mHeight, text, 18, DT_VCENTER, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	mLabel->setSizeToFit(true);

	mCallback = CheckboxClickedCB;
	mCallbackObj = callbackObj;
	mStartedOnTop = false;
	mChecked = false;
	mTextChanged = true;
}

Checkbox::~Checkbox()
{
	delete mCheckboxSprite;
	delete mLabel;
}

void Checkbox::onDeviceLost()
{
	mCheckboxSprite->onDeviceLost();
	mLabel->onDeviceLost();
}
void Checkbox::onDeviceReset()
{
	mCheckboxSprite->onDeviceReset();
	mLabel->onDeviceReset();
	mTextChanged = true;
}

void Checkbox::update(float dt)
{
	mLabel->update(dt);

	if (mTextChanged)
	{
		mSprites.clear();
		mSprites.push_back(mCheckboxSprite);
		vector<Sprite*> sprites = mLabel->getSprites();
		for (unsigned int i = 0; i < sprites.size(); i++)
		{
			mSprites.push_back(sprites[i]);
		}

		mTextChanged = false;
	}
}

void Checkbox::preRender()
{
	mLabel->preRender();
}

void Checkbox::setPos(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	if (width != 0 && height != 0)
	{
		mWidth = width;
		mHeight = height;
	}
	mX2 = mX + CHECKBOX_DIMENSION;
	mY2 = mY + CHECKBOX_DIMENSION;

	mCheckboxSprite->setDest(mX, mY, mWidth, mHeight);
	mLabel->setPos(mX + mWidth, mY, mWidth, mHeight);
	mTextChanged = true;
}
std::vector<Sprite*> Checkbox::getSprites()
{
	return mSprites;
}
int Checkbox::getNumTriangles()
{
	int num = mLabel->getNumTriangles();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		num += mSprites[i]->getNumTriangles();
	}
	return num;
}

bool Checkbox::onMouseEvent(MouseEvent e)
{
	if (e.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (isPointInside(e.getX(), e.getY()))
		{
			if (mStartedOnTop && gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
			{
				mCheckboxSprite->setTextureIndex(mChecked ? TEXTURE_CHECKED_DOWN : TEXTURE_DOWN);
			}
			else
			{
				mCheckboxSprite->setTextureIndex(mChecked ? TEXTURE_CHECKED_HOVER : TEXTURE_HOVER);
			}
		}
		else if (mCheckboxSprite->getCurrentIndex() != TEXTURE_UP)
		{
			mCheckboxSprite->setTextureIndex(mChecked ? TEXTURE_CHECKED_UP : TEXTURE_UP);
		}
	}
	else if (!e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONDOWN &&
		isPointInside(e.getX(), e.getY()))
	{
		mCheckboxSprite->setTextureIndex(mChecked ? TEXTURE_CHECKED_DOWN : TEXTURE_DOWN);
		mStartedOnTop = true;
	}
	else if (mStartedOnTop && !e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONUP &&
		isPointInside(e.getX(), e.getY()))
	{
		mChecked = !mChecked;
		mCheckboxSprite->setTextureIndex(mChecked ? TEXTURE_CHECKED_HOVER : TEXTURE_HOVER);
		mStartedOnTop = false;
		if (mCallback != NULL)
		{
			mCallback(mCallbackObj, this);
		}
	}

	return false;
}

void Checkbox::refresh()
{
	// simulate a mouse event
	MouseEvent ev(MouseEvent::MOUSEMOVE, gInput->currMouseX(), gInput->currMouseY(), 0, 0);
	onMouseEvent(ev);
}
bool Checkbox::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY || xPoint > mX2 || yPoint > mY2) || mLabel->isPointInside(x, y);
}

void Checkbox::setChecked(bool checked)
{
	mChecked = checked;
	mCheckboxSprite->setTextureIndex(mChecked ? TEXTURE_CHECKED_UP : TEXTURE_UP);
	mTextChanged = true;
}
bool Checkbox::getChecked()
{
	return mChecked;
}

void Checkbox::setText(string text)
{
	mText = text;
	mLabel->setString(text);
	mTextChanged = true;
}