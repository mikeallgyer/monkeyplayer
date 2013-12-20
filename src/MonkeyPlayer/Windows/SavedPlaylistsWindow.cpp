// SavedPlaylistsWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays music directories we monitor

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "SavedPlaylistsWindow.h"
#include "MusicLoader.h"

#include <Shlobj.h>

using namespace MonkeyPlayer;

const float SavedPlaylistsWindow::WINDOW_WIDTH = 350.0f;
const float SavedPlaylistsWindow::WINDOW_HEIGHT = 200.0f;
const float SavedPlaylistsWindow::BUTTON_SIZE = 50.0f;

SavedPlaylistsWindow::SavedPlaylistsWindow()
{
	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\white.png"));

	mWidth = (float)Settings::instance()->getIntValue("PLAYLIST_WIDTH", (int)WINDOW_WIDTH);
	mHeight = WINDOW_HEIGHT;

	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mWidth, 300.0f, D3DXVECTOR4(0,0,0,1.0f));
	mSprites.push_back(mBackground);

	std::string playPath = FileManager::getContentAsset(std::string("Textures\\add.png"));
	std::string playDownPath = FileManager::getContentAsset(std::string("Textures\\add_down.png"));
	std::string playHoverPath = FileManager::getContentAsset(std::string("Textures\\add_hover.png"));
	mAddBtn = snew Button(0, 0, 100, 100.0f, playPath, btn_callback, this);
	mAddBtn->setDownTexture(playDownPath.c_str());
	mAddBtn->setHoverTexture(playHoverPath.c_str());
	mWidgets.push_back(mAddBtn);

	mPlaylistList = snew PlaylistListBox(0, 0, 50.0f, 50.0f, listBox_callback, this);
	mPlaylistList->setBgColor(D3DCOLOR_XRGB(50, 50, 50));
	mWidgets.push_back(mPlaylistList);
	setPlaylists();

	mFolderLabel = snew SimpleLabel(0, 0, 50.0f, 50.0f, std::string("Playlists"), 24);
	mFolderLabel->setTextColor(D3DXCOLOR(0xffa0f6f9));
	mWidgets.push_back(mFolderLabel);

	mPlaylistWindow = NULL;
}
SavedPlaylistsWindow::~SavedPlaylistsWindow()
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

void SavedPlaylistsWindow::onDeviceLost()
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
void SavedPlaylistsWindow::onDeviceReset()
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

int SavedPlaylistsWindow::getWidth()
{
	return (int)mWidth;
}
int SavedPlaylistsWindow::getHeight()
{
	return (int)mHeight;
}

void SavedPlaylistsWindow::update(float dt)
{

	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		int currX = r.right - (int)mWidth;
		int currY = gWindowMgr->getMainContentTop();

		mBackground->setDest(currX, currY, 
			(int)mWidth, (int)mHeight);

		mAddBtn->setPos(r.right - 50.0f, currY + mHeight - BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE);
		mFolderLabel->setPos((float)currX + 5.0f, currY + 2.0f, mWidth - 10.0f, 25.0);
		mPlaylistList->setPos((float)currX + 5.0f, currY + 25.0f, mWidth - 10.0f, mHeight - (45.0f + 25.0f));

		mResized = false;
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}
}

void SavedPlaylistsWindow::display()
{
}

std::vector<Sprite*> SavedPlaylistsWindow::getSprites()
{
	return mSprites;
}
std::vector<IWidget*> SavedPlaylistsWindow::getWidgets()
{
	return mWidgets;
}
bool SavedPlaylistsWindow::onMouseEvent(MouseEvent ev)
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

void SavedPlaylistsWindow::onBlur()
{
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		if (mWidgets[i]->getIsFocused())
		{
			mWidgets[i]->blur();
		}
	}
}
void SavedPlaylistsWindow::onFocus()
{
}

void SavedPlaylistsWindow::setPlaylists()
{
	mPlaylistList->clearItems();
	std::vector<Playlist*> playlists = DatabaseManager::instance()->getAllPlaylists();
	for (unsigned int i = 0; i < playlists.size(); i++)
	{
		vector<Track*> tracks = DatabaseManager::instance()->getTracksInPlaylist(playlists[i]->Id);
		mPlaylistList->addItem(snew PlaylistListItem(playlists[i]->Id, playlists[i]->Name.c_str(), tracks));
		for (unsigned int j = 0; j < tracks.size(); j++)
		{
			delete tracks[j];
		}
	}

}

void SavedPlaylistsWindow::setPlaylistWindow(MonkeyPlayer::PlaylistWindow *win)
{
	mPlaylistWindow = win;
}

void SavedPlaylistsWindow::onBtnPushed(Button* btn)
{
	if (btn == mAddBtn)
	{
	}
}

void SavedPlaylistsWindow::onItemSelected(ListItem* item, int index)
{
	PlaylistListItem* playlistItem = static_cast<PlaylistListItem*>(item);
	if (playlistItem != NULL && mPlaylistWindow != NULL)
	{
		vector<Track*> tracks = DatabaseManager::instance()->getTracksInPlaylist(playlistItem->getId());

		mPlaylistWindow->clearItems();
		mPlaylistWindow->addItems(tracks);
		mPlaylistWindow->setPlaylistName(playlistItem->toString());

	}
}
