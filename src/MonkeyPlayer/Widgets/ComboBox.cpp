// ComboBox.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A ComboBox

#include "ComboBox.h"
#include "FileManager.h"

const int ComboBox::BACKGROUND_UP = 0;
const int ComboBox::BACKGROUND_DOWN = 1;

const int ComboBox::ARROW_UP = 0;
const int ComboBox::ARROW_HOVER = 1;
const int ComboBox::ARROW_DOWN = 2;

const float ComboBox::COMBOBOX_DIMENSION = 25.0f;

ComboBox::ComboBox(float x, float y, string text, float width,
	void (*ComboBoxClickedCB)(void* ptrObj, ComboBox* btn), void* callbackObj)
{
	mX = x;
	mY = y;
	mWidth = width;
	mHeight = COMBOBOX_DIMENSION;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;

	std::string downArrowPath = FileManager::getContentAsset(std::string("Textures\\down_arrow.png"));
	std::string downArrowHoverPath = FileManager::getContentAsset(std::string("Textures\\down_arrow_hover.png"));
	std::string downArrowDownPath = FileManager::getContentAsset(std::string("Textures\\down_arrow_down.png"));

	mArrowSprite = snew Sprite(downArrowPath.c_str(), mX, mY, mWidth, mHeight);
	mArrowSprite->addTexture(ARROW_HOVER, downArrowHoverPath.c_str(), false);
	mArrowSprite->addTexture(ARROW_DOWN, downArrowDownPath.c_str(), false);

	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\combo_back.png"));
	std::string bgDownPath = FileManager::getContentAsset(std::string("Textures\\combo_back_down.png"));

	mBackgroundSprite = snew Sprite(bgPath.c_str(), mX, mY, mWidth, mHeight);
	mBackgroundSprite->addTexture(BACKGROUND_DOWN, bgDownPath.c_str(), false);

	mLabel = snew Label(mX + 8.0f, mY, mWidth, mHeight, text, 16, DT_VCENTER, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

	mListBox = snew ItemListBox(mX, mY + COMBOBOX_DIMENSION, getWidth(), 200.0f, listBox_callback, this,
		 D3DCOLOR_XRGB(50, 50, 50), D3DCOLOR_XRGB(255, 255, 255), D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));
	mListBox->setAllowSingleClickSelection(true);

	mCallback = ComboBoxClickedCB;
	mCallbackObj = callbackObj;
	mStartedOnTop = false;
	mDroppedDown = false;
	mTextChanged = true;
}

ComboBox::~ComboBox()
{
	delete mArrowSprite;
	delete mBackgroundSprite;
	delete mLabel;
	delete mListBox;
}

void ComboBox::onDeviceLost()
{
	mArrowSprite->onDeviceLost();
	mBackgroundSprite->onDeviceLost();
	mLabel->onDeviceLost();
	mListBox->onDeviceLost();
}
void ComboBox::onDeviceReset()
{
	mArrowSprite->onDeviceReset();
	mBackgroundSprite->onDeviceReset();
	mLabel->onDeviceReset();
	mListBox->onDeviceReset();
	mTextChanged = true;
}

void ComboBox::update(float dt)
{
	mLabel->update(dt);
	mListBox->update(dt);

	if (mTextChanged)
	{
		mSprites.clear();
		mSprites.push_back(mArrowSprite);
		mSprites.push_back(mBackgroundSprite);
		vector<Sprite*> sprites = mLabel->getSprites();
		for (unsigned int i = 0; i < sprites.size(); i++)
		{
			mSprites.push_back(sprites[i]);
		}

		if (mDroppedDown)
		{
			sprites = mListBox->getSprites();
			for (unsigned int i = 0; i < sprites.size(); i++)
			{
				mSprites.push_back(sprites[i]);
			}
		}
		mTextChanged = false;
	}
}

void ComboBox::preRender()
{
	mLabel->preRender();
	mListBox->preRender();
}

void ComboBox::setPos(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	if (width != 0)
	{
		mWidth = width;
	}
	if (height != 0)
	{
		mHeight = height;
	}
	mX2 = mX + mWidth + COMBOBOX_DIMENSION;
	mY2 = mY + mHeight;
	mLabel->setPos(mX + 8.0f, mY, mWidth, mHeight);
	mListBox->setPos(mX, mY + COMBOBOX_DIMENSION, getWidth(), 100.0f);

	mArrowSprite->setDest(mX + mWidth, mY, COMBOBOX_DIMENSION, COMBOBOX_DIMENSION);
	mBackgroundSprite->setDest(mX, mY, mWidth, mHeight);
	mTextChanged = true;
}
std::vector<Sprite*> ComboBox::getSprites()
{
	return mSprites;
}
int ComboBox::getNumTriangles()
{
	int num = mLabel->getNumTriangles();
	num += mArrowSprite->getNumTriangles();
	num += mBackgroundSprite->getNumTriangles();
	if (mDroppedDown)
	{
		num += mListBox->getNumTriangles();
	}

	return num;
}

bool ComboBox::onMouseEvent(MouseEvent e)
{
	if (mDroppedDown)
	{
		if (mListBox->onMouseEvent(e))
		{
			return true;
		}
	}

	if (e.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (isPointInside(e.getX(), e.getY()))
		{
			if (mStartedOnTop && gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
			{
				mArrowSprite->setTextureIndex(ARROW_DOWN);
			}
			else
			{
				mArrowSprite->setTextureIndex(ARROW_HOVER);
			}
		}
		else if (mArrowSprite->getCurrentIndex() != ARROW_UP)
		{
			mArrowSprite->setTextureIndex(ARROW_UP);
		}
	}
	else if (!e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONDOWN &&
		isPointInside(e.getX(), e.getY()))
	{
		mArrowSprite->setTextureIndex(ARROW_DOWN);
		mStartedOnTop = true;
		e.setConsumed(true);
	}
	else if (mStartedOnTop && !e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONUP &&
		isPointInside(e.getX(), e.getY()))
	{
		mDroppedDown = !mDroppedDown;
		
		if (mDroppedDown)
		{
			mListBox->focus();
		}
		else
		{
			mListBox->blur();
		}
		mArrowSprite->setTextureIndex(ARROW_HOVER);
		mStartedOnTop = false;
		mTextChanged = true;
		e.setConsumed(true);
	}
	
	if (mDroppedDown && e.getEvent() == MouseEvent::LBUTTONDOWN &&
		!isPointInside(e.getX(), e.getY()) && !mListBox->isPointInside(e.getX(), e.getY()))
	{
		mDroppedDown = false;
		mTextChanged = true;
		mListBox->blur();
		e.setConsumed(true);
	}

	return false;
}

void ComboBox::refresh()
{
	// simulate a mouse event
	MouseEvent ev(MouseEvent::MOUSEMOVE, gInput->currMouseX(), gInput->currMouseY(), 0, 0);
	onMouseEvent(ev);
}
bool ComboBox::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY || xPoint > mX2 || yPoint > mY2) || mLabel->isPointInside(x, y);
}

void ComboBox::setDroppedDown(bool droppedDown)
{
	mDroppedDown = droppedDown;
//	mArrowSprite->setTextureIndex(mChecked ? TEXTURE_CHECKED_UP : TEXTURE_UP);
	mTextChanged = true;
	if (mDroppedDown)
	{
		mListBox->focus();
	}
	else
	{
		mListBox->blur();
	}
}
bool ComboBox::getDroppedDown()
{
	return mDroppedDown;
}

void ComboBox::setText(string text)
{
	mText = text;
	mLabel->setString(text);
	mTextChanged = true;
}
string ComboBox::getText()
{
	return mText;
}
ListItem* ComboBox::getSelectedItem()
{
	return mListBox->getSelectedItem();
}
int ComboBox::getSelectedIndex()
{
	return mListBox->getSelectedIndex();
}
	
void ComboBox::setSelectedIndex(int index)
{
	mListBox->setSelectedIndex(index);
	mLabel->setString(mListBox->getSelectedItem()->toString());
	mTextChanged = true;
}

void ComboBox::setList(vector<ListItem*> list)
{
	mListBox->clearItems();
	mListBox->addItems(list);
}

void ComboBox::onItemSelected(ItemListBox* listBox)
{
	mDroppedDown = false;
	mLabel->setString(listBox->getSelectedItem()->toString());
	mTextChanged = true;
	mListBox->blur();
	if (mCallback != NULL)
	{
		mCallback(mCallbackObj, this);
	}
}
