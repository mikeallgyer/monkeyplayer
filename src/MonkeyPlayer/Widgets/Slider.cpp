// Slider.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A slider widget

#include "FileManager.h"
#include "Slider.h"

using namespace MonkeyPlayer;

const float Slider::INF_STEPS = 0;
const int Slider::TEXTURE_UP = 0;
const int Slider::TEXTURE_HOVER = 1;
const int Slider::TEXTURE_DOWN = 2;
const float Slider::MAX_STEPS = 5000.0f;

Slider::Slider(float x, float y, float width, float height, float min, float max, float step,
		void (*sliderMovedCB)(void* ptrObj, Slider* btn), void* callbackObj)
{
	init(x, y, width, height, min, max, step,
		FileManager::getContentAsset(std::string("Textures\\empty.png")).c_str(),
		FileManager::getContentAsset(std::string("Textures\\empty_hover.png")).c_str(),
		FileManager::getContentAsset(std::string("Textures\\empty_down.png")).c_str(),
		sliderMovedCB, callbackObj);
}
Slider::Slider(float x, float y, float width, float height, float min, float max, float step,
		const char* handleFile,  const char* handleHoverFile, const char* handleDownFile,
		void (*sliderMovedCB)(void* ptrObj, Slider* btn), void* callbackObj)
{
	init(x, y, width, height, min, max, step, handleFile, handleHoverFile, handleDownFile,
		sliderMovedCB, callbackObj);
}
void Slider::init(float x, float y, float width, float height, float min, float max, float step,
		const char* handleFile, const char* handleHoverFile, const char* handleDownFile,
		void (*sliderMovedCB)(void* ptrObj, Slider* btn), void* callbackObj)
{
	mX = x;
	mY = y;
	mWidth = width;
	mHeight = height;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;
	mMin = min;
	mMax = max;
	mStep = step;
	mCurrValue = min;
	mCallback = sliderMovedCB;
	mCallbackObj = callbackObj;

	std::string barInLeftPath = FileManager::getContentAsset(std::string("Textures\\slider_in_left.png"));
	std::string barInMidPath = FileManager::getContentAsset(std::string("Textures\\slider_in_mid.png"));
	std::string barInRightPath = FileManager::getContentAsset(std::string("Textures\\slider_in_right.png"));

	std::string barOutLeftPath = FileManager::getContentAsset(std::string("Textures\\slider_out_left.png"));
	std::string barOutMidPath = FileManager::getContentAsset(std::string("Textures\\slider_out_mid.png"));
	std::string barOutRightPath = FileManager::getContentAsset(std::string("Textures\\slider_out_right.png"));

	mInnerLeft = snew Sprite(barInLeftPath.c_str(), 0, 0, 20, 20);
	mInnerMid = snew Sprite(barInMidPath.c_str(), 0, 0, 20, 20);
	mInnerRight = snew Sprite(barInRightPath.c_str(), 0, 0, 20, 20);

	mOuterLeft = snew Sprite(barOutLeftPath.c_str(), 0, 0, 20, 20);
	mOuterMid = snew Sprite(barOutMidPath.c_str(), 0, 0, 20, 20);
	mOuterRight = snew Sprite(barOutRightPath.c_str(), 0, 0, 20, 20);

	mSprites.push_back(mOuterLeft);
	mSprites.push_back(mOuterMid);
	mSprites.push_back(mOuterRight);

	mSprites.push_back(mInnerLeft);
	mSprites.push_back(mInnerMid);
	mSprites.push_back(mInnerRight);

	mHandleSprite = snew Sprite(handleFile, 0, 0, 20, 20);
	mHandleSprite->addTexture(TEXTURE_HOVER, handleHoverFile, false, false);
	mHandleSprite->addTexture(TEXTURE_DOWN, handleDownFile, false, false);

	mSprites.push_back(mHandleSprite);
	setPos(x, y, width, height);
	createSteps();
	mStartedOnTop = false;
}

void Slider::createSteps()
{
	mSteps.clear();
	mSteps.push_back(mMin);
	mSteps.push_back(mMax);

	float numSteps = (mMax - mMin) / mStep;
	if (numSteps > MAX_STEPS)
	{
		mStep = (mMax - mMin) / MAX_STEPS;
	}
	for (float s = mMin + mStep; s < mMax; s += mStep)
	{
		mSteps.push_back(s);
	}
}
Slider::~Slider()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
}

void Slider::onDeviceLost()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
}
void Slider::onDeviceReset()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
}

void Slider::setValue(float value)
{
	mCurrValue = max(mMin, min(mMax, value));

	setHandlePosition();
}

float Slider::getValue()
{
	return mCurrValue;
}
void Slider::update(float dt)
{
}

void Slider::preRender()
{
}

void Slider::setPos(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	if (width > 0)
	{
		mWidth = width;
	}
	if (height > 0)
	{
		mHeight = height;
	}
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;

	float halfHeight = mHeight * .5f;

	mInnerLeft->setDest(mX, mY, halfHeight, mHeight);
	mOuterLeft->setDest(mX, mY, halfHeight, mHeight);

	mInnerMid->setDest(mX + halfHeight, mY, mWidth - mHeight, mHeight);
	mOuterMid->setDest(mX + halfHeight, mY, mWidth - mHeight, mHeight);

	mInnerRight->setDest(mX + mWidth - halfHeight, mY, halfHeight, mHeight);
	mOuterRight->setDest(mX + mWidth - halfHeight, mY, halfHeight, mHeight);

	setHandlePosition();
}
void Slider::setRange(float minValue, float maxValue)
{
	mMin = minValue;
	mMax = maxValue;
	createSteps();
}
void Slider::setRangeAndStep(float minValue, float maxValue, float step)
{
	mMin = minValue;
	mMax = maxValue;
	mStep = step;
	createSteps();
}

std::vector<Sprite*> Slider::getSprites()
{
	return mSprites;
}
int Slider::getNumTriangles()
{
	int num = 0;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		num += mSprites[i]->getNumTriangles();
	}
	return num;
}

bool Slider::onMouseEvent(MouseEvent e)
{
	if (e.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (isPointInside(e.getX(), e.getY()))
		{
			if (mStartedOnTop && gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
			{
				mHandleSprite->setTextureIndex(TEXTURE_DOWN);
			}
			else
			{
				mHandleSprite->setTextureIndex(TEXTURE_HOVER);
			}
		}
		else if (!gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
		{
			mHandleSprite->setTextureIndex(TEXTURE_UP);
		}
		if (mStartedOnTop && gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
		{
			updateValue(e.getX());
		}
		if (mStartedOnTop && !gInput->isMouseButtonDown(MonkeyInput::MOUSE1))
		{
			mStartedOnTop = false;
			mHandleSprite->setTextureIndex(TEXTURE_UP);
		}
	}
	else if (!e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONDOWN &&
		isPointInside(e.getX(), e.getY()))
	{
		mHandleSprite->setTextureIndex(TEXTURE_DOWN);
		mStartedOnTop = true;
		updateValue(e.getX());
	}
	else if (mStartedOnTop && !e.getConsumed() && e.getEvent() == MouseEvent::LBUTTONUP &&
		isPointInside(e.getX(), e.getY()))
	{
		mHandleSprite->setTextureIndex(TEXTURE_HOVER);
		mStartedOnTop = false;
	}

	return false;
}
void Slider::setCallback(void (*cb)(void* objPtr, Slider* slider), void* objPtr)
{
	mCallback =  cb;
	mCallbackObj = objPtr;
}
void Slider::setHandlePosition()
{
	float halfHeight = mHeight * .5f;//////////////write  function to get x1, x2
	float percent = (mCurrValue - mMin) / (mMax - mMin);
	mHandleSprite->setDest((mX - halfHeight) + (mWidth) * percent, mY, mHeight, mHeight);
}
bool Slider::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY || xPoint > mX2 || yPoint > mY2);
}

void Slider::updateValue(int mouseX)
{
	float halfHeight = mHeight * .5f;
	float relPos = max(mX, min(mX + mWidth, (float)mouseX));
	relPos -= mX;
	float percent = relPos / (mWidth);
	float val = mMin + ((mMax - mMin) * percent);

	if (mStep != INF_STEPS && val != mMin && val != mMax)
	{
		float stepped = mMin;
		while (stepped < val)
		{
			stepped += mStep;
		}
		float prev = stepped - mStep;
		if ((val - prev) < (stepped - val))
		{
			val = prev;
		}
		else
		{
			val = stepped;
		}
	}
	setValue(val);

	if (mCallback != NULL)
	{
		mCallback(mCallbackObj, this);
	}
}