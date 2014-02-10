// PlaybackOptionsWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains playback options

#include <fstream>
#include <vector>
#include <algorithm>    // std::random_shuffle
//#include <random>       // std::default_random_engine
//#include <chrono>       // std::chrono::system_clock

#include "Button.h"
#include "ComboBox.h"
#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "MusicLibrary.h"
#include "PlaybackOptionsWindow.h"
#include "Settings.h"
#include "SoundManager.h"
#include "Vertex.h"

using namespace MonkeyPlayer;

const int PlaybackOptionsWindow::WINDOW_HEIGHT = 100;

PlaybackOptionsWindow::PlaybackOptionsWindow()
{
	mCurrWidth = 0;//gWindowMgr->getMainContentWidth();

	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mCurrWidth, 300.0f, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));

	mSprites.push_back(mBackground);

	mTitleLabel = snew SimpleLabel(0,0, 50.0f, 20.0f, string("Playback Options"), 24, DT_LEFT, D3DCOLOR_XRGB(255, 255, 255));
	mTitleLabel->setSizeToFit(true);
	mWidgets.push_back(mTitleLabel);

	mPlayModeLabel = snew SimpleLabel(0,0, 50.0f, 20.0f, string("Mode:"), 18, DT_LEFT, D3DCOLOR_XRGB(255, 255, 255));
	mPlayModeLabel->setSizeToFit(true);
	mWidgets.push_back(mPlayModeLabel);

	std::string listPath = FileManager::getContentAsset(std::string("Textures\\list_btn.png"));
	std::string downPath = FileManager::getContentAsset(std::string("Textures\\list_down.png"));
	std::string hoverPath = FileManager::getContentAsset(std::string("Textures\\list_hover.png"));
	mCreateListBtn = snew Button(0, 0, 50.0f, 5.0f, listPath, btn_callback, this);
	mCreateListBtn->setDownTexture(downPath.c_str());
	mCreateListBtn->setHoverTexture(hoverPath.c_str());
	mWidgets.push_back(mCreateListBtn);

	mRandomChk = snew Checkbox(0, 0, "Random", chk_callback, this);
	mRandomChk->setChecked(Settings::instance()->getBoolValue(Settings::OPT_RANDOM_ON, false));
	mStopAfterChk = snew Checkbox(0, 0, "Stop After", chk_callback, this);
	mStopAfterChk->setChecked(Settings::instance()->getBoolValue(Settings::OPT_STOP_AFTER_ON, false));
	mRepeatChk = snew Checkbox(0, 0, "Repeat", chk_callback, this);
	mRepeatChk->setChecked(!mStopAfterChk->getChecked());

	mOrderByCombo = snew ComboBox(0, 0, "SELECT", 100.0f, combo_callback, this);
	mStopAfterCombo = snew ComboBox(0, 0, "SELECT", 100.0f, combo_callback, this);

	vector<ListItem*> orderByList;
	orderByList.push_back(snew SimpleListItem("Songs", (int)SONG));
	orderByList.push_back(snew SimpleListItem("Albums", (int)ALBUM));
	orderByList.push_back(snew SimpleListItem("Artists", (int)ARTIST));
	mOrderByCombo->setList(orderByList);
	mOrderByCombo->setSelectedIndex(Settings::instance()->getIntValue(Settings::OPT_ORDER_BY, 0));

	vector<ListItem*> repeatList;
	repeatList.push_back(snew SimpleListItem("Song", (int)SONG));
	repeatList.push_back(snew SimpleListItem("Album", (int)ALBUM));
	repeatList.push_back(snew SimpleListItem("Artist", (int)ARTIST));
	repeatList.push_back(snew SimpleListItem("Queue", (int)QUEUE));
	repeatList.push_back(snew SimpleListItem("Never", (int)NEVER));
	mStopAfterCombo->setList(repeatList);
	mStopAfterCombo->setText(repeatList[0]->toString());
	mStopAfterCombo->setSelectedIndex(Settings::instance()->getIntValue(Settings::OPT_STOP_AFTER, 3));

	mWidgets.push_back(mRandomChk);
	mWidgets.push_back(mStopAfterChk);
	mWidgets.push_back(mRepeatChk);
	mWidgets.push_back(mStopAfterCombo);
	mWidgets.push_back(mOrderByCombo);
	mX = 0;

	mHiddenListIndex = -1;
	initHiddenList(true);
	SoundManager::instance()->addCallback(soundEventCB, this);
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
	SoundManager::instance()->removeCallback(this);
	for (unsigned int j = 0; j < mHiddenList.size(); j++)
	{
		delete mHiddenList[j];
	}
}
void PlaybackOptionsWindow::initHiddenList(bool tryReadFirst)
{
	mLastConfigedAlbum = -1;
	mLastConfigedArtist = "";
	for (unsigned int j = 0; j < mHiddenList.size(); j++)
	{
		delete mHiddenList[j];
	}
	mHiddenList.clear();
	if (tryReadFirst)
	{
		readList();
	}
	if (!tryReadFirst || mHiddenList.size() < 1)
	{
//		mHiddenList = populateList();
		mHiddenListIndex = -1;
	}
}
string PlaybackOptionsWindow::getFilename()
{
	string retVal;

#ifdef _DEBUG
	retVal = "hidden_playlist.cfg";
#else
	TCHAR appPath[1024];
	Settings::instance()->getAppDataPath(appPath, "hidden_playlist.cfg"); 
	retVal = appPath;
#endif

	return retVal;
}

void PlaybackOptionsWindow::readList()
{
	try
	{
		ifstream infile(getFilename().c_str());

		vector<Track*> tracks;
		string currLine;
		int index = -1;

		infile >> index;

		while (infile.good())
		{
			getline(infile, currLine);

			if (currLine != "")
			{
				Track *t = snew Track();
				DatabaseManager::instance()->getTrack(currLine, t);

				if (t->Id >= 0)
				{
					mHiddenList.push_back(t);
				}
			}
		}
		infile.close();
		mHiddenListIndex = index;
	}
	catch (...) {}
}
void PlaybackOptionsWindow::writeList()
{
	try 
	{
		ofstream outfile(getFilename().c_str());
		outfile << mHiddenListIndex << std::endl;

		for (unsigned int i = 0; i < mHiddenList.size(); i++)
		{
			outfile << mHiddenList[i]->Filename << std::endl;
		}
		outfile.close();
	}
	catch (...)
	{
	}
}
vector<Track*> PlaybackOptionsWindow::populateList()
{
	vector<Track*> tracks;
	if (mOrderByCombo->getSelectedItem()->getId() == SONG)
	{
		tracks = DatabaseManager::instance()->getAllTracksVector();
		if (mRandomChk->getChecked())
		{
			std::random_shuffle(tracks.begin(), tracks.end());
		}
	}
	else if (mOrderByCombo->getSelectedItem()->getId() == ALBUM)
	{
		vector<AlbumWithTracks*> albums = DatabaseManager::instance()->getAllAlbumsAndTracks();
		if (mRandomChk->getChecked())
		{
			std::random_shuffle(albums.begin(), albums.end());
		}
		for (unsigned int i = 0; i < albums.size(); i++)
		{
			for (unsigned int j = 0; j < albums[i]->tracks.size(); j++)
			{
				tracks.push_back(albums[i]->tracks[j]);
			}
			delete albums[i]->album;
			delete albums[i];
		}
	}
	else if (mOrderByCombo->getSelectedItem()->getId() == ARTIST)
	{
		vector<ArtistWithTracks*> artists = DatabaseManager::instance()->getAllArtistsAndTracks();
		if (mRandomChk->getChecked())
		{
			std::random_shuffle(artists.begin(), artists.end());
		}
		for (unsigned int i = 0; i < artists.size(); i++)
		{
			for (unsigned int j = 0; j < artists[i]->tracks.size(); j++)
			{
				tracks.push_back(artists[i]->tracks[j]);
			}
			delete artists[i];
		}
	}
	return tracks;
}
void PlaybackOptionsWindow::createRepeatingList()
{
	string f = SoundManager::instance()->getCurrFile();
	mLastPlayedId = -1;
	if (f != "")
	{
		Track t;
		DatabaseManager::instance()->getTrack(f, &t);
		if (t.Id != DatabaseStructs::INVALID_ID)
		{
			mLastPlayedId = t.Id;

			if (isRepeatOn())
			{
				if (mStopAfterCombo->getSelectedItem()->getId() == ARTIST)
				{
					if (mLastConfigedArtist != t.Artist)
					{
						for (unsigned int j = 0; j < mHiddenList.size(); j++)
						{
							delete mHiddenList[j];
						}
						mHiddenList.clear();

						mHiddenList = DatabaseManager::instance()->getTracks(t.Artist);
						mLastConfigedAlbum = -1;
						mLastConfigedArtist = t.Artist;
					}
				}
				else if (mStopAfterCombo->getSelectedItem()->getId() == ALBUM)
				{
					if (mLastConfigedAlbum != t.AlbumId)
					{
						for (unsigned int j = 0; j < mHiddenList.size(); j++)
						{
							delete mHiddenList[j];
						}
						mHiddenList.clear();

						mHiddenList = DatabaseManager::instance()->getTracks(t.AlbumId);
						mLastConfigedAlbum = t.AlbumId;
						mLastConfigedArtist = "";
					}
				}
				mHiddenListIndex = -1;
				for (unsigned int j = 0; j < mHiddenList.size(); j++)
				{
					if (mHiddenList[j]->Id == t.Id)
					{
						mHiddenListIndex = j;
						break;
					}
				}
				writeList();
			}
		}
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
		mCreateListBtn->setPos((float)(mX + 10.0f), gWindowMgr->getMainContentTop() - 50.0f, 128.0f, 32.0f);
		mTitleLabel->setPos((float)mX + 10.0f, 5.0f, 0, 20.0f);
		mPlayModeLabel->setPos((float)mX + 15.0f, 40.0f, 0, 20.0f);

		mOrderByCombo->setPos(mX + 70.0f, 40.f);
		mRandomChk->setPos((float)mX + 250.0f, 40.0f);

		mRepeatChk->setPos((float)(mX + mCurrWidth - 300), 80.0f);
		mStopAfterChk->setPos((float)(mX + mCurrWidth - 300), 110.0f);
		mStopAfterCombo->setPos((float)(mX + mCurrWidth - 150), 110.f);
		if (mRepeatChk->getChecked())
		{
			mStopAfterCombo->setPos(mStopAfterCombo->getX(), mRepeatChk->getY());
		}
		else
		{
			mStopAfterCombo->setPos(mStopAfterCombo->getX(), mStopAfterChk->getY());
		}
		
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
Track* PlaybackOptionsWindow::getNextSong(int &index)
{
	if (mHiddenList.size() < 1 || 
		(mHiddenListIndex == mHiddenList.size() - 1 && mStopAfterChk->getChecked() && mStopAfterCombo->getSelectedItem()->getId() != NEVER))
	{
		return NULL;
	}
	index = mHiddenListIndex;
	if (index < 0)
	{
		index = 0;
	}
	if (index == mHiddenList.size() - 1) // end of list
	{
		if (mStopAfterChk->getChecked()) // NEVER is chosen, otherwise top if would've been true
		{
			initHiddenList(false); // create new
		}
		index = 0;
	}
	else
	{
		index++;
	}
	return mHiddenList[index];
}
bool PlaybackOptionsWindow::playNextSong()
{
	int index;
	Track* t = getNextSong(index);

	if (t != NULL)
	{
		mHiddenListIndex = index;
		SoundManager::instance()->playFile(t->Filename.c_str());
		return true;
	}
	return false;
}
bool PlaybackOptionsWindow::playPreviousSong()
{
	if (mHiddenListIndex > 0 && mHiddenList.size() > 0)
	{
		mHiddenListIndex--;
		SoundManager::instance()->playFile(mHiddenList[mHiddenListIndex]->Filename.c_str());
		return true;
	}
	return false;
}
Track* PlaybackOptionsWindow::getCurrentSong()
{
	if (mHiddenList.size() < 1 || mHiddenListIndex < 0)
	{
		return NULL;
	}
	return mHiddenList[mHiddenListIndex];
}
bool PlaybackOptionsWindow::playCurrentSong()
{
	if (mHiddenList.size() < 1 || mHiddenListIndex < 0)
	{
		return false;
	}
	SoundManager::instance()->playFile(mHiddenList[mHiddenListIndex]->Filename.c_str());
	return true;
}
// excludes repeat playlist
bool PlaybackOptionsWindow::isRepeatOn() 
{
	return mRepeatChk->getChecked() && mStopAfterCombo->getSelectedItem()->getId() != NEVER &&
		mStopAfterCombo->getSelectedItem()->getId() != QUEUE;
}
bool PlaybackOptionsWindow::isRepeatPlaylistOn()
{
	return mRepeatChk->getChecked() && (mStopAfterCombo->getSelectedItem()->getId() == QUEUE);
}
bool PlaybackOptionsWindow::isStopAfterOn() 
{
	return mStopAfterChk->getChecked();
}
int PlaybackOptionsWindow::getRepeat()
{
	return mStopAfterCombo->getSelectedItem()->getId();
}
int PlaybackOptionsWindow::getStopAfter()
{
	return mStopAfterCombo->getSelectedItem()->getId();
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
	for (int i = (int)mWidgets.size() - 1; i >= 0; i--)
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
	if (btn == mCreateListBtn)
	{
		MusicLibrary::instance()->getPlaylistWindow()->clearItems();

		vector<Track*> list = populateList();
		MusicLibrary::instance()->getPlaylistWindow()->addItems(list);
	}
}
void PlaybackOptionsWindow::onChkPushed(Checkbox *btn)
{
	bool recreate = false;
	if (btn == mStopAfterChk)
	{
		if (mStopAfterChk->getChecked())
		{
			mRepeatChk->setChecked(false);
			mStopAfterCombo->setPos(mStopAfterCombo->getX(), mStopAfterChk->getY());
		}
		else
		{
			mStopAfterChk->setChecked(true);
			mStopAfterCombo->setPos(mStopAfterCombo->getX(), mStopAfterChk->getY());
		}
		Settings::instance()->setValue(Settings::OPT_STOP_AFTER_ON, mStopAfterChk->getChecked());
	}
	else if (btn == mRepeatChk)
	{
		if (mRepeatChk->getChecked())
		{
			mStopAfterChk->setChecked(false);
			mStopAfterCombo->setPos(mStopAfterCombo->getX(), mRepeatChk->getY());
		}
		else
		{
			mRepeatChk->setChecked(true);
			mStopAfterCombo->setPos(mStopAfterCombo->getX(), mRepeatChk->getY());
		}
		Settings::instance()->setValue(Settings::OPT_STOP_AFTER_ON, mStopAfterChk->getChecked());
		createRepeatingList();
	}
	else if (btn == mRandomChk)
	{
		Settings::instance()->setValue(Settings::OPT_RANDOM_ON, mRandomChk->getChecked());
		if (mRepeatChk->getChecked())
		{
			createRepeatingList();
		}
		else
		{
			recreate = true;
		}
	}
	if (recreate)
	{
		initHiddenList();
		writeList();
	}
}
void PlaybackOptionsWindow::onComboSelected(ComboBox *combo)
{
	if (combo == mStopAfterCombo)
	{
		Settings::instance()->setValue(Settings::OPT_STOP_AFTER, mStopAfterCombo->getSelectedIndex());
	}
	else if (combo == mOrderByCombo)
	{
		Settings::instance()->setValue(Settings::OPT_ORDER_BY, mOrderByCombo->getSelectedIndex());
	}

	if (isRepeatOn() && mStopAfterCombo->getSelectedItem()->getId() != NEVER)
	{
		createRepeatingList();
	}
	else
	{
		initHiddenList();
		writeList();
	}
}

void PlaybackOptionsWindow::onSoundEvent(SoundManager::SoundEvent ev)
{
	if (ev == SoundManager::START_EVENT)
	{
		createRepeatingList();
	}
}
