// PlaylistWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a playlist of songs

#include <fstream>
#include <vector>

#include "Button.h"
#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "MusicLibrary.h"
#include "PlaylistWindow.h"
#include "Settings.h"
#include "SoundManager.h"
#include "Vertex.h"

using namespace MonkeyPlayer;

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

	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mCurrWidth, 300.0f, D3DXVECTOR4(0, 0, 0, 1.0f));//1.0f, 1.0f, 0.9f, 1.0f));

	mSprites.push_back(mBackground);
	
	mListBox = snew TrackListBox(0, 0, 50.0f, 50.0f,
		listBox_callback,
		this);
	mListBox->setBgColor(D3DCOLOR_XRGB(50, 50, 50));

	std::string shuffleUp = FileManager::getContentAsset(std::string("Textures\\shuffle_up.png"));
	std::string shuffleDown = FileManager::getContentAsset(std::string("Textures\\shuffle_down.png"));
	std::string shuffleHover = FileManager::getContentAsset(std::string("Textures\\shuffle_hover.png"));

	mShuffleBtn = snew Button(0, 0, 50.0f, 50.0f, shuffleUp, button_callback, this);
	mShuffleBtn->setDownTexture(shuffleDown.c_str());
	mShuffleBtn->setHoverTexture(shuffleHover.c_str());

	mCurrSongIndex = -1;
	
//	mListBox->setBgColor(D3DXCOLOR(1.0f, 1.0f, .9f, 1.0f));

	//mListBox->focus();

	mWidgets.push_back(mListBox);
	mWidgets.push_back(mShuffleBtn);
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
			mBackground->getHeight() - 80);

		mShuffleBtn->setPos(mBackground->getX() + 5,
			mBackground->getY() + mBackground->getHeight() - mShuffleBtn->getHeight() - 10);

		mResized = false;
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
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

void PlaylistWindow::clearItems()
{
	mListBox->clearItems();
	mListBox->setHighlightedItem(-1);
	mCurrSongIndex = -1;

	writeFile();
}
void PlaylistWindow::addItem(Track *item)
{
	mListBox->addItem(snew TrackListItem(item));

	writeFile();
}
void PlaylistWindow::addItems(std::vector<Track*> items)
{
	std::vector<ListItem*> trackItems(items.size());
	for (unsigned int i = 0; i < items.size(); i++)
	{
		trackItems[i] = snew TrackListItem(items[i]);
	}
	mListBox->addItems(trackItems);

	writeFile();
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
bool PlaylistWindow::playNextSong()
{
	if (mCurrSongIndex < (mListBox->getNumItems() - 1))
	{
		if (mCurrSongIndex < 0)
		{
			mCurrSongIndex = 0;
		}
		else 
		{
			mCurrSongIndex++;
		}
		SoundManager::instance()->playFile(((TrackListItem*)mListBox->getItem(mCurrSongIndex))->getTrack()->Filename.c_str());
		mListBox->setCurrentTrack(mCurrSongIndex);
		return true;
	}
	return false;
}
bool PlaylistWindow::playPreviousSong()
{
	if (mCurrSongIndex > 0)
	{
		if (mCurrSongIndex > mListBox->getNumItems() - 1)
		{
			mCurrSongIndex = mListBox->getNumItems() - 1;
		}
		else 
		{
			mCurrSongIndex--;
		}
		SoundManager::instance()->playFile(((TrackListItem*)mListBox->getItem(mCurrSongIndex))->getTrack()->Filename.c_str());
		mListBox->setCurrentTrack(mCurrSongIndex);
		return true;
	}
	return false;
}

void PlaylistWindow::addTrackToQueueEnd(int id)
{
	Track* t = snew Track();
	DatabaseManager::instance()->getTrack(id, t);
	if (t->Id >= 0)
	{
		mListBox->addItem(snew TrackListItem(t));
	}

	writeFile();
}
void PlaylistWindow::insertTrackToQueueNext(int id)
{
	Track* t = snew Track();
	DatabaseManager::instance()->getTrack(id, t);
	if (t->Id >= 0)
	{
		int index = mListBox->getHighlightedIndex();
		mListBox->addItem(snew TrackListItem(t), index >= 0 ? (unsigned int)index + 1 : 0);
	}

	writeFile();
}
void PlaylistWindow::replaceQueueWithTrack(int id)
{
	Track* t = snew Track();
	DatabaseManager::instance()->getTrack(id, t);
	if (t->Id >= 0)
	{
		mListBox->clearItems();
		mListBox->addItem(snew TrackListItem(t));
	}

	writeFile();
}

void PlaylistWindow::addAlbumToQueueEnd(Album a)
{
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(a);
	vector<ListItem*> items(tracks.size());
	for (unsigned int i = 0; i < tracks.size(); i++)
	{
		items[i] = snew TrackListItem(tracks[i]);
	}
	mListBox->addItems(items);

	writeFile();
}
void PlaylistWindow::insertAlbumToQueueNext(Album a)
{
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(a);
	vector<ListItem*> items(tracks.size());
	for (unsigned int i = 0; i < tracks.size(); i++)
	{
		items[i] = snew TrackListItem(tracks[i]);
	}

	int index = mListBox->getHighlightedIndex();
	mListBox->addItems(items, index >= 0 ? (unsigned int)index + 1 : 0);

	writeFile();
}
void PlaylistWindow::replaceQueueWithAlbum(Album a)
{
	clearItems();
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(a);
	vector<ListItem*> items(tracks.size());
	for (unsigned int i = 0; i < tracks.size(); i++)
	{
		items[i] = snew TrackListItem(tracks[i]);
	}
	mListBox->addItems(items);

	writeFile();
}

void PlaylistWindow::addArtistToQueueEnd(string &name)
{
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(name);
	vector<ListItem*> items(tracks.size());
	for (unsigned int i = 0; i < tracks.size(); i++)
	{
		items[i] = snew TrackListItem(tracks[i]);
	}
	int index = mListBox->getHighlightedIndex();
	mListBox->addItems(items);

	writeFile();
}

void PlaylistWindow::insertArtistToQueueNext(string &name)
{
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(name);
	vector<ListItem*> items(tracks.size());
	for (unsigned int i = 0; i < tracks.size(); i++)
	{
		items[i] = snew TrackListItem(tracks[i]);
	}
	int index = mListBox->getHighlightedIndex();
	mListBox->addItems(items, index >= 0 ? (unsigned int)index + 1 : 0);

	writeFile();
}
void PlaylistWindow::replaceQueueWithArtist(string &name)
{
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(name);
	clearItems();
	addItems(tracks);

	writeFile();
}

void PlaylistWindow::onItemSelected(ListItem* item, int index)
{
	TrackListItem* trackItem = static_cast<TrackListItem*>(item);
	if (trackItem != NULL)
	{
		SoundManager::instance()->playFile(trackItem->getTrack()->Filename.c_str());
		mListBox->setCurrentTrack(index);
		mCurrSongIndex = index;
	}
}

void PlaylistWindow::onBtnClicked(Button* btn)
{
	if (btn == mShuffleBtn)
	{
		mListBox->shuffleItems();
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

void PlaylistWindow::writeFile()
{
	try 
	{

		vector<ListItem*> items = mListBox->getItems();

		if (items.size() > 0)
		{
			ofstream outfile(getFilename().c_str());

			for (unsigned int i = max(0, mListBox->getHighlightedIndex()); i < items.size(); i++)
			{
				outfile << ((TrackListItem*)items[i])->getTrack()->Filename << std::endl;
			}
			outfile.close();
		}
	}
	catch (...)
	{
	}
}
void PlaylistWindow::readFile()
{
}
string PlaylistWindow::getFilename()
{
	string retVal;

#ifdef _DEBUG
	retVal = "playlist.cfg";
#else
	TCHAR appPath[1024];
	Settings::instance()->getAppDataPath(appPath, "playlist.cfg"); 
	retVal = appPath;
#endif

	return retVal;
}