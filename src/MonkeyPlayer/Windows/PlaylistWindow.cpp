// PlaylistWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a playlist of songs

#include <vector>

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "PlaylistWindow.h"
#include "Settings.h"
#include "SoundManager.h"
#include "Vertex.h"

const int PlaylistWindow::MIN_WINDOW_WIDTH = 100;

PlaylistWindow::PlaylistWindow()
{
	D3DXFONT_DESC font;
	font.Height = 16;
	font.Width = 0;
	font.Weight = 0;
	font.MipLevels = 1;
	font.Italic = false;
	font.CharSet = DEFAULT_CHARSET;
	font.OutputPrecision = OUT_DEFAULT_PRECIS;
	font.Quality = DEFAULT_QUALITY;
	font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(font.FaceName, _T("Times New Roman"));

	HR(D3DXCreateFontIndirect(gDevice, &font, &mFont));

	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\white.png"));

	mCurrWidth = mPreferredWidth = Settings::instance()->getIntValue("PLAYLIST_WIDTH", 350);

	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mCurrWidth, 300.0f, D3DXVECTOR4(1.0f, 1.0f,0.9f,1.0f));

	mSprites.push_back(mBackground);
	
	mListBox = snew TrackListBox(0, 0, 50.0f, 50.0f,
		listBox_callback,
		this);
	
//	mListBox->setBgColor(D3DXCOLOR(1.0f, 1.0f, .9f, 1.0f));

	mPlayNextSong = false;
	
	// register with SoundManager
	SoundManager::instance()->addCallback(soundEventCB, this);

	//mListBox->focus();

	mWidgets.push_back(mListBox);
	gInput->registerGlobalKey(VK_MEDIA_PLAY_PAUSE);
}
PlaylistWindow::~PlaylistWindow()
{
	ReleaseCOM(mFont);

	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		delete mWidgets[i];
	}
}

void PlaylistWindow::onDeviceLost()
{
	HR(mFont->OnLostDevice());
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceLost();
	}
}
void PlaylistWindow::onDeviceReset()
{
	HR(mFont->OnResetDevice());
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
int PlaylistWindow::getWidth()
{
	return (int)mCurrWidth;
}

int PlaylistWindow::getHeight()
{
	return (int)mBackground->getHeight();
}

void PlaylistWindow::update(float dt)
{

	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mCurrWidth = min(r.right/2, mPreferredWidth);
		int height = r.bottom - gWindowMgr->getMainContentTop();
		mBackground->setDest(r.right - mCurrWidth, gWindowMgr->getMainContentTop(), 
			mCurrWidth, height);

		mListBox->setPos(mBackground->getX() + 5,
			mBackground->getY() + 5,
			mBackground->getWidth() - 10,
			mBackground->getHeight() - 10);

		mResized = false;
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}

	if (gInput->keyPressed(VK_MEDIA_PLAY_PAUSE))
	{
		SoundManager::instance()->setPaused(!SoundManager::instance()->isPaused());
	}

	if (mPlayNextSong)
	{
		mPlayNextSong = false;
		std::string currFile = SoundManager::instance()->getCurrFile();
		int currTrackIndex = mListBox->findItem(currFile);
		if (currTrackIndex > -1)
		{
			currTrackIndex++;
			ListItem* nextItem = mListBox->setHighlightedItem(currTrackIndex);
			if (nextItem != NULL)
			{
				SoundManager::instance()->playFile(
					((TrackListItem*)nextItem)->getTrack()->Filename.c_str());
			}
		}

	}

}

void PlaylistWindow::display()
{

	// Let Direct3D know the vertex buffer, index buffer and vertex 
	// declaration we are using.

}
std::vector<Sprite*> PlaylistWindow::getSprites()
{
	return mSprites;
}
std::vector<IWidget*> PlaylistWindow::getWidgets()
{
	return mWidgets;
}

void PlaylistWindow::addItem(Track *item)
{
	mListBox->addItem(snew TrackListItem(item));
}
void PlaylistWindow::addItems(std::vector<Track*> items)
{
	std::vector<ListItem*> trackItems(items.size());
	for (unsigned int i = 0; i < items.size(); i++)
	{
		trackItems[i] = snew TrackListItem(items[i]);
	}
	mListBox->addItems(trackItems);
}

void PlaylistWindow::modifyItem(Track *item)
{
	ListItem* newItem = snew TrackListItem(item, false);
	mListBox->modifyItem(newItem);
	delete newItem;
}
void PlaylistWindow::modifyItems(std::vector<Track*> items)
{
	std::vector<ListItem*> trackItems(items.size());
	for (unsigned int i = 0; i < items.size(); i++)
	{
		trackItems[i] = snew TrackListItem(items[i], false);
	}
	mListBox->modifyItems(trackItems);
	for (unsigned int i = 0; i < items.size(); i++)
	{
		delete trackItems[i];
	}
}

void PlaylistWindow::onItemSelected(ListItem* item)
{
	TrackListItem* trackItem = static_cast<TrackListItem*>(item);
	if (trackItem != NULL)
	{
		SoundManager::instance()->playFile(trackItem->getTrack()->Filename.c_str());
		mListBox->setCurrentTrack(item);
	}
}

void PlaylistWindow::onSoundEvent(SoundManager::SoundEvent ev)
{
	if (ev == SoundManager::SOUND_FINISHED_EVENT)
	{
		// don't do it in the callback or SM gets confused
		mPlayNextSong = true;
	}
}
bool PlaylistWindow::onMouseEvent(MouseEvent ev)
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

void PlaylistWindow::onBlur()
{
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		if (mWidgets[i]->getIsFocused())
		{
			mWidgets[i]->blur();
		}
	}
}

void PlaylistWindow::onFocus()
{

}
