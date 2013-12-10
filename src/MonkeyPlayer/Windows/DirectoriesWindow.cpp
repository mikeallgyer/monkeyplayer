// DirectoriesWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Displays music directories we monitor

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "DirectoriesWindow.h"
#include "MusicLoader.h"

#include <Shlobj.h>

using namespace MonkeyPlayer;

const float DirectoriesWindow::WINDOW_WIDTH = 350.0f;
const float DirectoriesWindow::BUTTON_SIZE = 50.0f;

DirectoriesWindow::DirectoriesWindow()
{
	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\white.png"));

	mWidth = (float)Settings::instance()->getIntValue("PLAYLIST_WIDTH", (int)WINDOW_WIDTH);

	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mWidth, 300.0f, D3DXVECTOR4(0,0,0,1.0f));
	mSprites.push_back(mBackground);

	std::string playPath = FileManager::getContentAsset(std::string("Textures\\add.png"));
	std::string playDownPath = FileManager::getContentAsset(std::string("Textures\\add_down.png"));
	std::string playHoverPath = FileManager::getContentAsset(std::string("Textures\\add_hover.png"));
	mAddBtn = snew Button(0, 0, 100, 100.0f, playPath, btn_callback, this);
	mAddBtn->setDownTexture(playDownPath.c_str());
	mAddBtn->setHoverTexture(playHoverPath.c_str());
	mWidgets.push_back(mAddBtn);

	mFolderList = snew ItemListBox(0, 0, 50.0f, 50.0f, NULL, NULL);
	mWidgets.push_back(mFolderList);
	setFolders();

	mFolderLabel = snew SimpleLabel(0, 0, 50.0f, 50.0f, std::string("Music Folders"));
	mFolderLabel->setTextColor(D3DXCOLOR(0XFFFFFFFF));
	mWidgets.push_back(mFolderLabel);

}
DirectoriesWindow::~DirectoriesWindow()
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

void DirectoriesWindow::onDeviceLost()
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
void DirectoriesWindow::onDeviceReset()
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

int DirectoriesWindow::getWidth()
{
	return (int)mWidth;
}
int DirectoriesWindow::getHeight()
{
	return (int)mHeight;
}

void DirectoriesWindow::update(float dt)
{

	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mWidth = WINDOW_WIDTH;
		int height = gWindowMgr->getMainContentTop();
		int currX = r.right - (int)mWidth;

		mBackground->setDest(currX, 0, 
			(int)mWidth, height);


		mAddBtn->setPos(r.right - 50.0f, (float)height - BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE);
		mFolderLabel->setPos((float)currX + 5.0f, 10.0f, mWidth - 10.0f, 25.0);
		mFolderList->setPos((float)currX + 5.0f, 25.0f, mWidth - 10.0f, (float)height - (45.0f + 25.0f));

		mResized = false;
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}
}

void DirectoriesWindow::display()
{
}

std::vector<Sprite*> DirectoriesWindow::getSprites()
{
	return mSprites;
}
std::vector<IWidget*> DirectoriesWindow::getWidgets()
{
	return mWidgets;
}
bool DirectoriesWindow::onMouseEvent(MouseEvent ev)
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

void DirectoriesWindow::onBlur()
{
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		if (mWidgets[i]->getIsFocused())
		{
			mWidgets[i]->blur();
		}
	}
}
void DirectoriesWindow::onFocus()
{
}

void DirectoriesWindow::setFolders()
{
	mFolderList->clearItems();
	std::vector<std::string> folders = DatabaseManager::instance()->getAllDirs();
	for (unsigned int i = 0; i < folders.size(); i++)
	{
		mFolderList->addItem(snew SimpleListItem(folders[i].c_str(), 0));
	}

}

void DirectoriesWindow::onBtnPushed(Button* btn)
{
	if (btn == mAddBtn)
	{
		BROWSEINFO bi = { 0 };
		TCHAR path[MAX_PATH];
		bi.lpszTitle = _T("Pick a Music Directory");
		bi.pszDisplayName = path;
		LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
		if ( pidl != 0 )
		{
			TCHAR fullPath[MAX_PATH];
			SHGetPathFromIDList(pidl, fullPath);

			// free memory used
			IMalloc * imalloc = 0;
			if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
			{
				imalloc->Free ( pidl );
				imalloc->Release ( );
			}

			if (strlen(fullPath) > 0)
			{
				DatabaseManager::instance()->addDir(std::string(fullPath));
				MusicLoader::instance()->loadDirectory(fullPath);
				setFolders();
			}
		}
	}
}
