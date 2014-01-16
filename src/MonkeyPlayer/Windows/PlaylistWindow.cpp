// PlaylistWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains a playlist of songs

#include "../Winforms/SavePlaylistForm.h"

#include <fstream>
#include <vector>

#include "d3dApp.h"
#include "DatabaseManager.h"
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
	mListBox->setAllowMultipleSelection(true);

	std::string shuffleUp = FileManager::getContentAsset(std::string("Textures\\shuffle_up.png"));
	std::string shuffleDown = FileManager::getContentAsset(std::string("Textures\\shuffle_down.png"));
	std::string shuffleHover = FileManager::getContentAsset(std::string("Textures\\shuffle_hover.png"));

	mShuffleBtn = snew Button(0, 0, 50.0f, 50.0f, shuffleUp, button_callback, this);
	mShuffleBtn->setDownTexture(shuffleDown.c_str());
	mShuffleBtn->setHoverTexture(shuffleHover.c_str());

	std::string clearUp = FileManager::getContentAsset(std::string("Textures\\clear.png"));
	std::string clearDown = FileManager::getContentAsset(std::string("Textures\\clear_down.png"));
	std::string clearHover = FileManager::getContentAsset(std::string("Textures\\clear_hover.png"));
	mClearBtn = snew Button(50.0f, 50.0f, 50.0f, 50.0f, clearUp, button_callback, this);
	mClearBtn->setDownTexture(clearDown.c_str());
	mClearBtn->setHoverTexture(clearHover.c_str());

	std::string delUp = FileManager::getContentAsset(std::string("Textures\\del.png"));
	std::string delDown = FileManager::getContentAsset(std::string("Textures\\del_down.png"));
	std::string delHover = FileManager::getContentAsset(std::string("Textures\\del_hover.png"));
	mDelBtn = snew Button(50.0f, 50.0f, 50.0f, 50.0f, delUp, button_callback, this);
	mDelBtn->setDownTexture(delDown.c_str());
	mDelBtn->setHoverTexture(delHover.c_str());

	std::string saveUp = FileManager::getContentAsset(std::string("Textures\\save.png"));
	std::string saveDown = FileManager::getContentAsset(std::string("Textures\\save_down.png"));
	std::string saveHover = FileManager::getContentAsset(std::string("Textures\\save_hover.png"));
	mSaveBtn = snew Button(50.0f, 50.0f, 50.0f, 50.0f, saveUp, button_callback, this);
	mSaveBtn->setDownTexture(saveDown.c_str());
	mSaveBtn->setHoverTexture(saveHover.c_str());

	mCurrSongIndex = -1;
	
	mHeaderLbl = snew SimpleLabel(0, 0, 50.0f, 50.0f, std::string("Queue"), 24);
	mHeaderLbl->setTextColor(D3DXCOLOR(0xffa0f6f9));
	mHeaderLbl->setSizeToFit(true);

	mLengthLbl = snew SimpleLabel(0, 0, 50.0f, 50.0f, std::string("00:00"), 18);
	mLengthLbl->setTextColor(D3DXCOLOR(0xffa0f6f9));
	mLengthLbl->setSizeToFit(true);

	mWidgets.push_back(mLengthLbl);
	mWidgets.push_back(mListBox);
	mWidgets.push_back(mShuffleBtn);
	mWidgets.push_back(mClearBtn);
	mWidgets.push_back(mDelBtn);
	mWidgets.push_back(mSaveBtn);
	mWidgets.push_back(mHeaderLbl);
	readFile();
	setPlaylistName(mPlaylistName, false);
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

void PlaylistWindow::setTopPos(int pos)
{
	mTopPos = pos;
}
int PlaylistWindow::getWidth()
{
	return (int)mCurrWidth;
}

int PlaylistWindow::getHeight()
{
	return mCurrHeight;
}

void PlaylistWindow::update(float dt)
{
	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mCurrWidth = min(r.right/2, mPreferredWidth);
		mCurrHeight = r.bottom - mTopPos;
		int currX = r.right - mCurrWidth;
		mBackground->setDest(currX, mTopPos, 
			mCurrWidth, mCurrHeight);

		mHeaderLbl->setPos((float)currX + 5.0f, mTopPos + 2.0f);

		mListBox->setPos(mBackground->getX() + 5,
			mBackground->getY() + 30.0f,
			mBackground->getWidth() - 10,
			mBackground->getHeight() - 110);

		mLengthLbl->setPos(mListBox->getX() + mListBox->getWidth() - mLengthLbl->getWidth(), 
			mListBox->getY() + mListBox->getHeight() + 2.0f);

		mShuffleBtn->setPos(mBackground->getX() + 5,
			mBackground->getY() + mBackground->getHeight() - mShuffleBtn->getHeight() - 10);

		mClearBtn->setPos(mBackground->getX() + 50.0f,
			mBackground->getY() + mBackground->getHeight() - mShuffleBtn->getHeight() - 10);

		mDelBtn->setPos(mBackground->getX() + mBackground->getWidth() - 50.0f,
			mBackground->getY() + mBackground->getHeight() - mShuffleBtn->getHeight() - 10);

		mSaveBtn->setPos(mBackground->getX() + mBackground->getWidth() - 100.0f,
			mBackground->getY() + mBackground->getHeight() - mShuffleBtn->getHeight() - 10);

		mResized = false;
	}

	if (gInput->keyPressed(VK_DELETE))
	{
		mListBox->removeItems(mListBox->getSelectedIndices());
		writeFile();
		setPlaylistName(mPlaylistName, false);
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

	setPlaylistName("");
	// done by setPlaylistName(): writeFile();
}
void PlaylistWindow::addItem(Track *item)
{
	mListBox->addItem(snew TrackListItem(item));

	setPlaylistName(mPlaylistName, false);
	writeFile();
}
void PlaylistWindow::addItems(std::vector<Track*> items, bool doWriteFile)
{
	std::vector<ListItem*> trackItems(items.size());
	for (unsigned int i = 0; i < items.size(); i++)
	{
		trackItems[i] = snew TrackListItem(items[i]);
	}
	mListBox->addItems(trackItems);
	setPlaylistName(mPlaylistName, false);

	if (doWriteFile)
	{
		writeFile();
	}
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
bool PlaylistWindow::playNextSong(bool loop)
{
	int index;
	Track *t = getNextSong(loop, index);
	if (t != NULL)
	{
		SoundManager::instance()->playFile(t->Filename.c_str());
		mCurrSongIndex = index;
		mListBox->setHighlightedItem(mCurrSongIndex);
		mListBox->scrollToIndex(mCurrSongIndex);
		writeFile();
		return true;
	}
	return false;
}
Track* PlaylistWindow::getNextSong(bool loop, int &index)
{
	if (mListBox->getNumItems() - 1 > 0 && (mCurrSongIndex < (mListBox->getNumItems() - 1) || loop))
	{
		index = mCurrSongIndex;
		if (mCurrSongIndex < 0)
		{
			index = 0;
		}
		else 
		{
			index++;
			if (index >= mListBox->getNumItems())
			{
				index = 0;
			}
		}
		return ((TrackListItem*)mListBox->getItem(index))->getTrack();
	}
	return NULL;
}
bool PlaylistWindow::playPreviousSong()
{
	Track *t = getPreviousSong();
	if (t != NULL)
	{
		SoundManager::instance()->playFile(t->Filename.c_str());
		mCurrSongIndex--;
		mListBox->setHighlightedItem(mCurrSongIndex);
		mListBox->scrollToIndex(mCurrSongIndex);
		writeFile();
		return true;
	}
	return false;
}
Track* PlaylistWindow::getPreviousSong()
{
	int index = mCurrSongIndex;
	if (mListBox->getNumItems() > 0 && mCurrSongIndex > 0)
	{
		if (mCurrSongIndex > mListBox->getNumItems() - 1)
		{
			index = mListBox->getNumItems() - 1;
		}
		else 
		{
			index--;
		}
		return ((TrackListItem*)mListBox->getItem(index))->getTrack();
	}
	return NULL;
}
bool PlaylistWindow::playCurrentSong()
{
	ListItem* item = mListBox->getItem(mListBox->getHighlightedIndex());
	if (item  != NULL)
	{
		TrackListItem* t = (TrackListItem*)item;
		SoundManager::instance()->playFile(t->getTrack()->Filename.c_str());
		mListBox->scrollToIndex(mCurrSongIndex);
		mCurrSongIndex = mListBox->getHighlightedIndex();
		return true;
	}
	return false;
}
bool PlaylistWindow::hasNextSong()
{
	return mListBox->getNumItems() > 0 && mListBox->getHighlightedIndex() < (mListBox->getNumItems() - 1);
}
void PlaylistWindow::addTrackToQueueEnd(int id)
{
	Track* t = snew Track();
	DatabaseManager::instance()->getTrack(id, t);
	if (t->Id >= 0)
	{
		mListBox->addItem(snew TrackListItem(t));
	}
	setPlaylistName(mPlaylistName, false);

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

	setPlaylistName(mPlaylistName, false);
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
	setPlaylistName(mPlaylistName, false);

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
	setPlaylistName(mPlaylistName, false);
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

	setPlaylistName(mPlaylistName, false);
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

	setPlaylistName(mPlaylistName, false);
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

	setPlaylistName(mPlaylistName, false);
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

	setPlaylistName(mPlaylistName, false);
	writeFile();
}
void PlaylistWindow::replaceQueueWithArtist(string &name)
{
	vector<Track*> tracks = DatabaseManager::instance()->getTracks(name);
	clearItems();
	addItems(tracks);

	setPlaylistName(mPlaylistName, false);
	writeFile();
}
void PlaylistWindow::setPlaylistName(string name, bool doWriteFile)
{
	mPlaylistName = name;
	int count = mListBox->getItems().size();
	int length = 0;
	vector<ListItem*> items = mListBox->getItems();
	for (unsigned int i = 0; i < items.size(); i++)
	{
		length += ((TrackListItem*)items[i])->getTrack()->Length;
	}
	if (mPlaylistName == "")
	{
		std::stringstream ss;
		ss << "Queue - " << count << " songs";
		mHeaderLbl->setString(ss.str());
		mLengthLbl->setString(SoundManager::getTimeString(length * 1000));
	}
	else
	{
		std::stringstream ss;
		ss << "Queue (" << mPlaylistName << ") - " << count << " songs";
		mHeaderLbl->setString(ss.str());
		mLengthLbl->setString(SoundManager::getTimeString(length * 1000));
	}
	mResized = true; // set flag so time label is positioned correctly
	if (doWriteFile)
	{
		writeFile();
	}
}

void PlaylistWindow::onItemSelected(ListItem* item, int index)
{
	TrackListItem* trackItem = static_cast<TrackListItem*>(item);
	if (trackItem != NULL)
	{
		SoundManager::instance()->playFile(trackItem->getTrack()->Filename.c_str());
		mListBox->setCurrentTrack(index);
		mCurrSongIndex = index;
		writeFile();
	}
}

void PlaylistWindow::onBtnClicked(Button* btn)
{
	if (btn == mShuffleBtn)
	{
		mListBox->shuffleItems();
		writeFile();
	}
	else if (btn == mClearBtn)
	{
		mListBox->clearItems();
		writeFile();
		mCurrSongIndex = -1;
		setPlaylistName(mPlaylistName, false);
	}
	else if (btn == mDelBtn)
	{
		mListBox->removeItems(mListBox->getSelectedIndices());
		writeFile();
		setPlaylistName(mPlaylistName, false);
	}
	else if (btn == mSaveBtn)
	{
		SavePlaylistForm^ form = gcnew SavePlaylistForm();
		if (mPlaylistName != "")
		{
			form->setPlaylistName(mPlaylistName);
		}
		form->ShowDialog();

		if (form->getIsFinished())
		{
			string name = form->getPlaylistName();
			vector<ListItem*> items = mListBox->getItems();
			vector<Track*> tracks;
			for (unsigned int i = 0; i < items.size(); i++)
			{
				Track* t = ((TrackListItem*)items[i])->getTrack();
				tracks.push_back(t);
			}

			DatabaseManager::instance()->savePlaylist(name, tracks);
			setPlaylistName(name);
		}
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

		ofstream outfile(getFilename().c_str());
		outfile << mListBox->getHighlightedIndex() << std::endl;
		outfile << mPlaylistName << std::endl;

		for (unsigned int i = 0; i < items.size(); i++)
		{
			outfile << ((TrackListItem*)items[i])->getTrack()->Filename << std::endl;
		}
		outfile.close();
	}
	catch (...)
	{
	}
}
void PlaylistWindow::readFile()
{
	try
	{
		ifstream infile(getFilename().c_str());

		vector<Track*> tracks;
		string currLine;
		int index = -1;

		infile >> index;

		getline(infile, mPlaylistName);
		getline(infile, mPlaylistName);

		while (infile.good())
		{
			getline(infile, currLine);

			if (currLine != "")
			{
				Track *t = snew Track();
				DatabaseManager::instance()->getTrack(currLine, t);

				if (t->Id >= 0)
				{
					tracks.push_back(t);
				}
			}
		}
		infile.close();

		addItems(tracks, false);
		mListBox->setHighlightedItem(index);
		mCurrSongIndex = index;
	}
	catch (...) {}
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