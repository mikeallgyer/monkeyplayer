// PlaybackOptionsWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains playback options

#include <vector>

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "PlaybackOptionsWindow.h"
#include "Settings.h"
#include "SoundManager.h"
#include "Vertex.h"

const int PlaybackOptionsWindow::WINDOW_HEIGHT = 100;

PlaybackOptionsWindow::PlaybackOptionsWindow()
{
	mCurrWidth = 0;//gWindowMgr->getMainContentWidth();

	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mCurrWidth, 300.0f, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));

	mSprites.push_back(mBackground);

	mChk = snew Checkbox(0, 0, "Option Option Option Option Option Option Option ", chk_callback, this);
	mCombo = snew ComboBox(0, 0, "Selection A", 100.0f, combo_callback, this);
	mWidgets.push_back(mChk);
	mWidgets.push_back(mCombo);
	mX = 0;
}
PlaybackOptionsWindow::~PlaybackOptionsWindow()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		delete mWidgets[i];
	}
}
int PlaybackOptionsWindow::getWidth()
{
	return (int)mCurrWidth;
}

int PlaybackOptionsWindow::getHeight()
{
	return (int)mBackground->getHeight();
}

void PlaybackOptionsWindow::onDeviceLost()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceLost();
	}
}
void PlaybackOptionsWindow::onDeviceReset()
{
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceReset();
	}
	mResized = true;
}

void PlaybackOptionsWindow::update(float dt)
{
	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mCurrWidth = gWindowMgr->getMainContentWidth() - mX;
		mBackground->setDest(mX, 0, mCurrWidth, gWindowMgr->getMainContentTop());
		
		mChk->setPos((float)mX, 0);
		mCombo->setPos((float)mX + mChk->getWidth(), 0);
		
		mResized = false;

	}

	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}

}
void PlaybackOptionsWindow::display()
{

}

std::vector<Sprite*> PlaybackOptionsWindow::getSprites()
{
	return mSprites;
}

std::vector<IWidget*> PlaybackOptionsWindow::getWidgets()
{
	return mWidgets;
}
void PlaybackOptionsWindow::setX(int x)
{
	mX = x;
	mResized = true;
}
void PlaybackOptionsWindow::setWidth(int width)
{
	mCurrWidth = width;
	mResized = true;
}

bool PlaybackOptionsWindow::onMouseEvent(MouseEvent ev)
{
	// if clicked, give widget focus
	if (ev.getEvent() == MouseEvent::LBUTTONDOWN ||
		ev.getEvent() == MouseEvent::RBUTTONDOWN)
	{
		bool found = false;
		for (unsigned int i = 0; i < mWidgets.size(); i++)
		{
			if (!found && mWidgets[i]->isPointInside(ev.getX(), ev.getY()))
			{
				found = true;
				mWidgets[i]->focus();
			}
			else if (!mWidgets[i]->getIsFocused())
			{
				mWidgets[i]->blur();
			}
		}
	}
	bool consumed = false;
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		if (mWidgets[i]->onMouseEvent(ev))
		{
			consumed = true;
			break;
		}
	}
	return consumed;
}

void PlaybackOptionsWindow::onBlur()
{
}

void PlaybackOptionsWindow::onFocus()
{

}

void PlaybackOptionsWindow::onBtnPushed(Button* btn)
{
}
void PlaybackOptionsWindow::onChkPushed(Checkbox *btn)
{
}
void PlaybackOptionsWindow::onComboSelected(ComboBox *btn)
{
}
