// ProgressBar.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A progress bar

#include "FileManager.h"
#include "ProgressBar.h"

using namespace MonkeyPlayer;

int ProgressBar::DEFAULT_NUM_TICKS = 40;
float ProgressBar::TICK_MARGIN_PERCENT = .2f;

ProgressBar::ProgressBar(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	mWidth = width;
	mHeight = height;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;
	mMaxValue = 100.0f;
	mCurrValue = 55.0f;

	mBackgroundSprite = snew Sprite(FileManager::getContentAsset(std::string("Textures\\white.png")).c_str(), mX, mY, mWidth, mHeight, D3DXVECTOR4(0, 0, 0, 1.0f));
	mTickSprite = snew Sprite(mBackgroundSprite->getTexture(), mX, mY, mWidth, mHeight);
	mTickSprite->setColor(D3DXVECTOR4(0, 1.0f, 0, 1.0f));
	mTickSprite->setTextureOwned(false);
	mSprites.push_back(mBackgroundSprite);
	mSprites.push_back(mTickSprite);

	for (int i = 0; i < DEFAULT_NUM_TICKS - 1; i++)
	{
		mSprites.push_back(snew Sprite(mTickSprite->getTexture(), mX, mY, mWidth, mHeight));
		mSprites[mSprites.size() - 1]->setColor(mSprites[1]->getColor());
		mSprites[mSprites.size() - 1]->setTextureOwned(false);
	}

	mNumTicks = DEFAULT_NUM_TICKS;
	recalculateSizes();
	mNeedsRecreated = false;
}

ProgressBar::~ProgressBar()
{
	// first is background, second is tick, rest are copies
	delete mBackgroundSprite;
	delete mTickSprite;
	for (unsigned int i = 2; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
}

void ProgressBar::onDeviceLost()
{
	mBackgroundSprite->onDeviceLost();
	mTickSprite->onDeviceLost();
}
void ProgressBar::onDeviceReset()
{
	mBackgroundSprite->onDeviceReset();
	mTickSprite->onDeviceReset();
}

void ProgressBar::update(float dt)
{
	if (mVisible || mNeedsRecreated)
	{
		Logger::instance()->write("LOCK update");
		CSingleLock lock(&mCritSection);
		lock.Lock();

		if (mNeedsRecreated)
		{
			for (unsigned int i = 2; i < mSprites.size(); i++)
			{
				delete mSprites[i];
			}

			mSprites.clear();
			if (mVisible)
			{
				mSprites.push_back(mBackgroundSprite);
				mSprites.push_back(mTickSprite);
				for(int i = 0; i < mNumTicks - 1; i++)
				{
					mSprites.push_back(snew Sprite(mTickSprite->getTexture(), mX, mY, mWidth, mHeight));
					mSprites[mSprites.size() - 1]->setColor(mSprites[1]->getColor());
					mSprites[mSprites.size() - 1]->setTextureOwned(false);
				}
			}
			mNeedsRecreated = false;
		}

		mBarWidth = mWidth * (mCurrValue / mMaxValue);

		if (mVisible)
		{
			for (unsigned int i = 1; i < mSprites.size(); i++)
			{
				mSprites[i]->setDest(0, 0, 0, 0);
			}

			float cursor = 0;
			int i = 1;
			while (cursor < mBarWidth)
			{
				if ((cursor + mTickWidth) > mBarWidth)
				{
					float remaining = mBarWidth - cursor;
					mSprites[i]->setDest(mX + cursor, mY, remaining, mHeight);
					cursor += remaining;
				}
				else
				{
					mSprites[i]->setDest(mX + cursor, mY, mTickWidth, mHeight);
					cursor += mTickWidth;
				}
				cursor += mMarginWidth;
				i++;
			}
		}
		lock.Unlock();
		Logger::instance()->write("UNLOCK");
	}
}

void ProgressBar::preRender()
{
}

void ProgressBar::setPos(float x, float y, float width, float height)
{
	Logger::instance()->write("LOCK setpos");
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mX = x;
	mY = y;
	mWidth = width;
	mHeight = height;
	mX2 = mX + mWidth;
	mY2 = mY + mHeight;
	mBackgroundSprite->setDest(mX, mY, mWidth, mHeight);
	mTickSprite->setDest(mX, mY, mWidth*.5f, mHeight);

	lock.Unlock();
	Logger::instance()->write("UNLOCK");
	recalculateSizes();
}

std::vector<Sprite*> ProgressBar::getSprites()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	return mSprites;
}
int ProgressBar::getNumTriangles()
{
	int num = 0;
	if (mVisible)
	{
		for (unsigned int i = 0; i < mSprites.size(); i++)
		{
			num += mSprites[i]->getNumTriangles();
		}
	}
	return num;
}
void ProgressBar::setVisible(bool visible)
{
	mVisible = visible;

	mNeedsRecreated = true;
}

void ProgressBar::setNumTicks(int ticks)
{
	Logger::instance()->write("LOCK setNumTicks");
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mNumTicks = ticks;
	
	for (unsigned int i = 2; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	mSprites.clear();

	mSprites.push_back(mBackgroundSprite);
	mSprites.push_back(mTickSprite);
	for(int i = 0; i < mNumTicks - 1; i++)
	{
		mSprites.push_back(snew Sprite(mTickSprite->getTexture(), mX, mY, mWidth, mHeight));
		mSprites[mSprites.size() - 1]->setColor(mSprites[1]->getColor());
		mSprites[mSprites.size() - 1]->setTextureOwned(false);
	}
	lock.Unlock();
	Logger::instance()->write("UNLOCK");
}

bool ProgressBar::isPointInside(int x, int y)
{
	float xPoint = (float)x;
	float yPoint = (float)y;

	return !(xPoint < mX || yPoint < mY || xPoint > mX2 || yPoint > mY2);
}

void ProgressBar::recalculateSizes()
{
	Logger::instance()->write("LOCK recalculateSizes");
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mTickWidth = mWidth / ((float)mNumTicks + (float)(mNumTicks - 1) * TICK_MARGIN_PERCENT);
	mMarginWidth = mTickWidth * TICK_MARGIN_PERCENT;

	lock.Unlock();
	Logger::instance()->write("UNLOCK");
}
