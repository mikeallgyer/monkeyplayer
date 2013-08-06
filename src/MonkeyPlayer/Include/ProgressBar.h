// ProgressBar.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A progress bar

#include <string>
#include <vector>

#include "IWidget.h"
#include "Sprite.h"

#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

class ProgressBar : public IWidget
{
public:
	ProgressBar(float x, float y, float width, float height);
	~ProgressBar();

	void onDeviceLost();
	void onDeviceReset();

	void update(float dt);

	virtual void preRender();
	void display() {}
	void setPos(float x, float y, float width, float height);
	float getX() { return mX; }
	float getY() { return mY; }
	float getWidth() { return mWidth; }
	float getHeight() { return mHeight; }
	void setMaxValue(float maxValue) { mMaxValue = maxValue; }
	float getMaxValue() { return mMaxValue; }
	void setCurrentValue(float currValue) { mCurrValue = min(mMaxValue, currValue); }
	float getCurrentValue() { return mCurrValue; }
	void setVisible(bool visible);
	void setNumTicks(int ticks);

	std::vector<Sprite*> getSprites();
	int getNumTriangles();
	bool isPointInside(int x, int y);

protected:

	static int DEFAULT_NUM_TICKS;
	static float TICK_MARGIN_PERCENT;

	std::vector<Sprite*> mSprites; 

	float mX, mY, mWidth, mHeight, mX2, mY2;
	Sprite*  mBackgroundSprite;
	Sprite*  mTickSprite;

	float mMaxValue;
	float mCurrValue;
	bool mVisible;
	bool mNeedsRecreated;
	int mNumTicks;

	float mTickWidth;
	float mMarginWidth;
	float mBarWidth;

	void recalculateSizes();

	// synchronization
	CCriticalSection mCritSection;
};
#endif