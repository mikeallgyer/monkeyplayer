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

	mTitleLabel = snew Label(0,0, 50.0f, 20.0f, string("Playback Options"), 24, DT_LEFT, D3DCOLOR_XRGB(255, 255, 255));
	mTitleLabel->setSizeToFit(true);
	mWidgets.push_back(mTitleLabel);

	std::string listPath = FileManager::getContentAsset(std::string("Textures\\list_btn.png"));
	std::string downPath = FileManager::getContentAsset(std::string("Textures\\list_down.png"));
	std::string hoverPath = FileManager::getContentAsset(std::string("Textures\\list_hover.png"));
	mCreateListBtn = snew Button(0, 0, 50.0f, 5.0f, listPath, btn_callback, this);
	mCreateListBtn->setDownTexture(downPath.c_str());
	mCreateListBtn->setHoverTexture(hoverPath.c_str());
	mWidgets.push_back(mCreateListBtn);

	mRandomChk = snew Checkbox(0, 0, "Random", chk_callback, this);
	mStopAfterChk = snew Checkbox(0, 0, "Stop After", chk_callback, this);
	mOrderByCombo = snew ComboBox(0, 0, "SELECT", 100.0f, combo_callback, this);
	mStopAfterCombo = snew ComboBox(0, 0, "SELECT", 100.0f, combo_callback, this);

	vector<ListItem*> orderByList;
	orderByList.push_back(snew SimpleListItem("Songs", (int)SONG));
	orderByList.push_back(snew SimpleListItem("Albums", (int)ALBUM));
	orderByList.push_back(snew SimpleListItem("Artists", (int)ARTIST));
	mOrderByCombo->setList(orderByList);
	mOrderByCombo->setText(orderByList[0]->toString());

	vector<ListItem*> repeatList;
	repeatList.push_back(snew SimpleListItem("Song", (int)SONG));
	repeatList.push_back(snew SimpleListItem("Album", (int)ALBUM));
	repeatList.push_back(snew SimpleListItem("Artist", (int)ARTIST));
	repeatList.push_back(snew SimpleListItem("Never", (int)NEVER));
	mStopAfterCombo->setList(repeatList);
	mStopAfterCombo->setText(repeatList[0]->toString());

	mWidgets.push_back(mRandomChk);
	mWidgets.push_back(mStopAfterChk);
	mWidgets.push_back(mStopAfterCombo);
	mWidgets.push_back(mOrderByCombo);
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
		mCreateListBtn->setPos((float)(mX + mCurrWidth - 150), gWindowMgr->getMainContentTop() - 50.0f, 128.0f, 32.0f);
		mTitleLabel->setPos((float)mX + 10.0f, 5.0f, 0, 20.0f);

		mRandomChk->setPos((float)mX + 15.0f, 40.0f);
		mOrderByCombo->setPos(mRandomChk->getX() + 140.0f, 40.f);

		mStopAfterChk->setPos((float)mX + 15.0f, 80.0f);
		mStopAfterCombo->setPos(mRandomChk->getX() + 140.0f, 80.f);
		
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
