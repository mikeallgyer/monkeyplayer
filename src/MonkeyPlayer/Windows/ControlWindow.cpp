// ControlWindow.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains play/stop/etc buttons

#include <iomanip>
#include <vector>

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "FileManager.h"
#include "MetadataReader.h"
#include "MusicLibrary.h"
#include "ControlWindow.h"
#include "Settings.h"
#include "SoundManager.h"
#include "Vertex.h"

using namespace MonkeyPlayer;

const int ControlWindow::WINDOW_HEIGHT = 100;
const float ControlWindow::BUTTON_PLAY_SIZE = 100.0f;
const float ControlWindow::BUTTON_OTHER_SIZE = 75.0f;
const float ControlWindow::BUTTON_MUTE_SIZE = 50.0f;
const float ControlWindow::VOLUME_WIDTH = 150.0f;
const float ControlWindow::VOLUME_HEIGHT = 25.0f;
const float ControlWindow::TIME_HEIGHT = 16.0f;
const float ControlWindow::TIME_UPDATE_DELAY = 1.0f;
const float ControlWindow::TIME_LABEL_HEIGHT = 25.0f;
const float ControlWindow::SPEED_HEIGHT = 15.0f;
const float ControlWindow::SPEED_WIDTH = 100.0f;

ControlWindow::ControlWindow()
{
	mCurrWidth = 0;//gWindowMgr->getMainContentWidth();

	std::string bgPath = FileManager::getContentAsset(std::string("Textures\\white.png"));
	mBackground = snew Sprite(bgPath.c_str(), 50.0f, 5.0f, (float)mCurrWidth, 300.0f, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));

	mSprites.push_back(mBackground);

	// play button
	std::string playPath = FileManager::getContentAsset(std::string("Textures\\play.png"));
	mPlayButton = snew Button(0, 0,
		BUTTON_PLAY_SIZE, BUTTON_PLAY_SIZE, playPath, btn_callback, this);
	playPath = FileManager::getContentAsset(std::string("Textures\\play_hover.png"));
	mPlayButton->setHoverTexture(playPath.c_str());
	playPath = FileManager::getContentAsset(std::string("Textures\\play_down.png"));
	mPlayButton->setDownTexture(playPath.c_str());
	mWidgets.push_back(mPlayButton);
	mPlayIndex = mWidgets.size() - 1;

	// pause button
	std::string pausePath = FileManager::getContentAsset(std::string("Textures\\pause.png"));
	mPauseButton = snew Button(0, 0,
		BUTTON_PLAY_SIZE, BUTTON_PLAY_SIZE, pausePath, btn_callback, this);
	pausePath = FileManager::getContentAsset(std::string("Textures\\pause_hover.png"));
	mPauseButton->setHoverTexture(pausePath.c_str());
	pausePath = FileManager::getContentAsset(std::string("Textures\\pause_down.png"));
	mPauseButton->setDownTexture(pausePath.c_str());
	
	// FF button
	std::string FFPath = FileManager::getContentAsset(std::string("Textures\\next.png"));
	mNextButton = snew Button(0, 0,
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE, FFPath, btn_callback, this);
	FFPath = FileManager::getContentAsset(std::string("Textures\\next_hover.png"));
	mNextButton->setHoverTexture(FFPath.c_str());
	FFPath = FileManager::getContentAsset(std::string("Textures\\next_down.png"));
	mNextButton->setDownTexture(FFPath.c_str());
	mWidgets.push_back(mNextButton);

	// RR button
	std::string RRPath = FileManager::getContentAsset(std::string("Textures\\previous.png"));
	mPreviousButton = snew Button(0, 0,
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE, RRPath, btn_callback, this);
	RRPath = FileManager::getContentAsset(std::string("Textures\\previous_hover.png"));
	mPreviousButton->setHoverTexture(RRPath.c_str());
	RRPath = FileManager::getContentAsset(std::string("Textures\\previous_down.png"));
	mPreviousButton->setDownTexture(RRPath.c_str());
	mWidgets.push_back(mPreviousButton);

	// stop button
	std::string stopPath = FileManager::getContentAsset(std::string("Textures\\stop.png"));
	mStopButton = snew Button(0, 0,
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE, stopPath, btn_callback, this);
	stopPath = FileManager::getContentAsset(std::string("Textures\\stop_hover.png"));
	mStopButton->setHoverTexture(stopPath.c_str());
	stopPath = FileManager::getContentAsset(std::string("Textures\\stop_down.png"));
	mStopButton->setDownTexture(stopPath.c_str());
	mWidgets.push_back(mStopButton);

	// mute button
	std::string mutePath = FileManager::getContentAsset(std::string("Textures\\mute.png"));
	mMuteButton = snew Button(0, 0,
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE, mutePath, btn_callback, this);
	mutePath = FileManager::getContentAsset(std::string("Textures\\mute_hover.png"));
	mMuteButton->setHoverTexture(mutePath.c_str());
	mutePath = FileManager::getContentAsset(std::string("Textures\\mute_down.png"));
	mMuteButton->setDownTexture(mutePath.c_str());
	mWidgets.push_back(mMuteButton);
	mMuteIndex = mWidgets.size() - 1;

	// unmute button
	std::string unmutePath = FileManager::getContentAsset(std::string("Textures\\unmute.png"));
	mUnmuteButton = snew Button(0, 0,
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE, unmutePath, btn_callback, this);
	unmutePath = FileManager::getContentAsset(std::string("Textures\\unmute_hover.png"));
	mUnmuteButton->setHoverTexture(unmutePath.c_str());
	unmutePath = FileManager::getContentAsset(std::string("Textures\\unmute_down.png"));
	mUnmuteButton->setDownTexture(unmutePath.c_str());

	// volume
	mVolumeSlider = snew Slider(0, 0, VOLUME_WIDTH, VOLUME_HEIGHT, 0, 1.0f, .02f);
	mVolumeSlider->setCallback(sliderCB, this);
	mVolumeSlider->setValue(SoundManager::instance()->getVolume());
	mWidgets.push_back(mVolumeSlider);

	// time
	mTimeSlider = snew Slider(0, 0, (float)mCurrWidth, TIME_HEIGHT, 0, 1.0f, .02f);
	mTimeSlider->setCallback(sliderCB, this);
	mTimeSlider->setValue(0);
	mWidgets.push_back(mTimeSlider);

	// time label
	mTimeLabel = snew SimpleLabel(300,0, 200.0f, 200.0f, std::string(""), 20,
		DT_NOCLIP | DT_RIGHT | DT_VCENTER,
		D3DCOLOR_XRGB(255, 255, 255));
	mTimeLabel->setCallback(labelCB, this);
	mWidgets.push_back(mTimeLabel);
	setTimeFormat(Settings::instance()->getIntValue("TIME_LABEL_FORMAT", 0));
	
	// speed
	mSpeedSlider = snew Slider(0, 0, SPEED_WIDTH, SPEED_HEIGHT, SoundManager::MIN_SPEED, SoundManager::MAX_SPEED, .25f);
	mSpeedSlider->setCallback(sliderCB, this);
	mSpeedSlider->setValue(1.0f);
	mWidgets.push_back(mSpeedSlider);

	// speed label
	mSpeedLabel = snew SimpleLabel(300,0, 200.0f, 200.0f, std::string("Speed: 1.0x"), 16, 256,
		D3DCOLOR_XRGB(255, 255, 255));
	mWidgets.push_back(mSpeedLabel);

	// pitch
	mPitchSlider = snew Slider(0, 0, SPEED_WIDTH, SPEED_HEIGHT, SoundManager::MIN_SPEED, SoundManager::MAX_SPEED, .25f);
	mPitchSlider->setCallback(sliderCB, this);
	mPitchSlider->setValue(1.0f);
	mWidgets.push_back(mPitchSlider);

	// pitch label
	mPitchLabel = snew SimpleLabel(300,0, 200.0f, 200.0f, std::string("Pitch: 1.0x"), 16, 256,
		D3DCOLOR_XRGB(255, 255, 255));
	mWidgets.push_back(mPitchLabel);

	// register with SoundManager
	SoundManager::instance()->addCallback(soundEventCB, this);

	mUpdateTime = TIME_UPDATE_DELAY;  // set label immediately
}
ControlWindow::~ControlWindow()
{
	Settings::instance()->setValue("TIME_LABEL_FORMAT", (int)mTimeFormat);

	// unregister with SoundManager
	SoundManager::instance()->removeCallback(this);

	Button* other = (mWidgets[mPlayIndex] == mPlayButton) ? mPauseButton : mPlayButton;
	Button* other2 = (mWidgets[mMuteIndex] == mMuteButton) ? mUnmuteButton: mMuteButton;

	delete other;
	delete other2;
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		delete mWidgets[i];
	}
}
int ControlWindow::getWidth()
{
	return (int)mCurrWidth;
}

int ControlWindow::getHeight()
{
	return (int)mBackground->getHeight();
}

void ControlWindow::onDeviceLost()
{
	Button* other = (mWidgets[mPlayIndex] == mPlayButton) ? mPauseButton : mPlayButton;
	Button* other2 = (mWidgets[mMuteIndex] == mMuteButton) ? mUnmuteButton: mMuteButton;

	other->onDeviceLost();
	other2->onDeviceLost();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->onDeviceLost();
	}
}
void ControlWindow::onDeviceReset()
{
	Button* other = (mWidgets[mPlayIndex] == mPlayButton) ? mPauseButton : mPlayButton;
	Button* other2 = (mWidgets[mMuteIndex] == mMuteButton) ? mUnmuteButton: mMuteButton;

	other->onDeviceReset();
	other2->onDeviceReset();
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

void ControlWindow::update(float dt)
{
	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mCurrWidth = gWindowMgr->getMainContentWidth();
		mBackground->setDest(0, r.bottom - WINDOW_HEIGHT, mCurrWidth, WINDOW_HEIGHT);

		mPlayButton->setPos(mCurrWidth * .5f - BUTTON_PLAY_SIZE * .5f, gApp->getHeight() - (float)WINDOW_HEIGHT,
		BUTTON_PLAY_SIZE, BUTTON_PLAY_SIZE);
		mPauseButton->setPos(mCurrWidth * .5f - BUTTON_PLAY_SIZE * .5f, gApp->getHeight() - (float)WINDOW_HEIGHT,
		BUTTON_PLAY_SIZE, BUTTON_PLAY_SIZE);

		mNextButton->setPos(mCurrWidth * .5f + (BUTTON_PLAY_SIZE * .5f), 
			gApp->getHeight() - (float)WINDOW_HEIGHT + (BUTTON_PLAY_SIZE - BUTTON_OTHER_SIZE) * .5f, // centered vertically
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE);

		mPreviousButton->setPos(mCurrWidth * .5f - (BUTTON_PLAY_SIZE * .5f + BUTTON_OTHER_SIZE), 
			gApp->getHeight() - (float)WINDOW_HEIGHT + (BUTTON_PLAY_SIZE - BUTTON_OTHER_SIZE) * .5f, // centered vertically
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE);

		mStopButton->setPos(mCurrWidth * .5f - (BUTTON_PLAY_SIZE * .5f + BUTTON_OTHER_SIZE * 2.0f), 
			gApp->getHeight() - (float)WINDOW_HEIGHT + (BUTTON_PLAY_SIZE - BUTTON_OTHER_SIZE) * .5f, // centered vertically
		BUTTON_OTHER_SIZE, BUTTON_OTHER_SIZE);

		mMuteButton->setPos(mCurrWidth * .5f + (BUTTON_PLAY_SIZE * .5f + BUTTON_OTHER_SIZE), 
			gApp->getHeight() - (float)WINDOW_HEIGHT + (BUTTON_PLAY_SIZE - BUTTON_MUTE_SIZE) * .5f, // centered vertically
		BUTTON_MUTE_SIZE, BUTTON_MUTE_SIZE);
		mUnmuteButton->setPos(mCurrWidth * .5f + (BUTTON_PLAY_SIZE * .5f + BUTTON_OTHER_SIZE), 
			gApp->getHeight() - (float)WINDOW_HEIGHT + (BUTTON_PLAY_SIZE - BUTTON_MUTE_SIZE) * .5f, // centered vertically
		BUTTON_MUTE_SIZE, BUTTON_MUTE_SIZE);

		mVolumeSlider->setPos(mCurrWidth * .5f + (BUTTON_PLAY_SIZE * 2.0f), 
			gApp->getHeight() - (float)WINDOW_HEIGHT + (BUTTON_PLAY_SIZE - VOLUME_HEIGHT) * .5f, // centered vertically
			VOLUME_WIDTH, VOLUME_HEIGHT);

		mTimeSlider->setPos(0, gApp->getHeight() - (float)WINDOW_HEIGHT - TIME_HEIGHT * .5f, (float)mCurrWidth, TIME_HEIGHT);

		mTimeLabel->setPos(0,
						gApp->getHeight() - (float)WINDOW_HEIGHT + (BUTTON_PLAY_SIZE - TIME_LABEL_HEIGHT) * .5f, // centered vertically
						mStopButton->getX(),  // put it against stop button
						TIME_LABEL_HEIGHT);

		mSpeedLabel->setPos(5, gApp->getHeight() - (float)WINDOW_HEIGHT + 20.0f);
		mSpeedSlider->setPos(100.0f, gApp->getHeight() - (float)WINDOW_HEIGHT + 20.0f);

		mPitchLabel->setPos(5, gApp->getHeight() - (float)WINDOW_HEIGHT + 60.0f);
		mPitchSlider->setPos(100.0f, gApp->getHeight() - (float)WINDOW_HEIGHT + 60.0f);
		mResized = false;
	}

	if (gInput->keyPressed(VK_SPACE))
	{
		SoundManager::instance()->togglePaused();
	}

	mUpdateTime += dt;
	if (mUpdateTime >= TIME_UPDATE_DELAY)
	{
		int currTime = (int)SoundManager::instance()->getCurrPosition();
		int currLength = (int)SoundManager::instance()->getCurrLength();

		switch(mTimeFormat)
		{
		case ELAPSED_AND_TOTAL:
			{
				std::string currTimeStr = SoundManager::getTimeString(currTime);
				std::string currLengthStr = SoundManager::getTimeString(currLength);
				std::string labelTxt = currTimeStr + " / " + currLengthStr;
				mTimeLabel->setString(labelTxt);
				break;
			}
		case REMAINING_AND_TOTAL:
			{
				std::string currTimeStr = SoundManager::getTimeString(currLength - currTime);
				std::string currLengthStr = SoundManager::getTimeString(currLength);
				std::string labelTxt = "-" + currTimeStr + " / " + currLengthStr;
				mTimeLabel->setString(labelTxt);
				break;
			}
		case ELAPSED_ONLY:
			{
				std::string currTimeStr = SoundManager::getTimeString(currTime);
				std::string labelTxt = currTimeStr;
				mTimeLabel->setString(labelTxt);
				break;
			}
		case REMAINING_ONLY:
			{
				std::string currTimeStr = SoundManager::getTimeString(currLength - currTime);
				std::string labelTxt = "-" + currTimeStr;
				mTimeLabel->setString(labelTxt);
				break;
			}
		}
		mUpdateTime = 0;

	}
	for (unsigned int i = 0; i < mWidgets.size(); i++)
	{
		mWidgets[i]->update(dt);
	}

	mTimeSlider->setValue((float)SoundManager::instance()->getCurrPosition());
}
void ControlWindow::display()
{

}

std::vector<Sprite*> ControlWindow::getSprites()
{
	return mSprites;
}

std::vector<IWidget*> ControlWindow::getWidgets()
{
	return mWidgets;
}

void ControlWindow::setTimeFormat(TimeDisplayFormat fmt)
{
	mTimeFormat = fmt;
	mUpdateTime = TIME_UPDATE_DELAY;
}
void ControlWindow::setTimeFormat(int fmt)
{
	if (fmt >= 0 && fmt < INVALID)
	{
		setTimeFormat((TimeDisplayFormat)fmt);
	}
	else
	{
		setTimeFormat(ELAPSED_AND_TOTAL);
	}
}
bool ControlWindow::onMouseEvent(MouseEvent ev)
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
	else if (ev.getEvent() == MouseEvent::MOUSEMOVE)
	{
		if (mPlayButton->isPointInside(ev.getX(), ev.getY()))
		{
			if (SoundManager::instance()->isPlaying())
			{
				gWindowMgr->getToolTip()->setup(mPlayButton, "Pause (space)", ev.getX(), ev.getY());
			}
			else
			{
				gWindowMgr->getToolTip()->setup(mPlayButton, "Play (space)", ev.getX(), ev.getY());
			}
		}
		else if (mStopButton->isPointInside(ev.getX(), ev.getY()))
		{
			gWindowMgr->getToolTip()->setup(mStopButton, "Stop", ev.getX(), ev.getY());
		}
		else if (mPreviousButton->isPointInside(ev.getX(), ev.getY()))
		{
			gWindowMgr->getToolTip()->setup(mPreviousButton, "Previous", ev.getX(), ev.getY());
		}
		else if (mNextButton->isPointInside(ev.getX(), ev.getY()))
		{
			gWindowMgr->getToolTip()->setup(mNextButton, "Next", ev.getX(), ev.getY());
		}
		else if (mMuteButton->isPointInside(ev.getX(), ev.getY()))
		{
			gWindowMgr->getToolTip()->setup(mMuteButton, "Toggle Mute", ev.getX(), ev.getY());
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

void ControlWindow::onBlur()
{
}

void ControlWindow::onFocus()
{

}

void ControlWindow::refreshSliders(float speed, float pitch)
{
	if (speed > 0)
	{
		stringstream str;
		str << std::fixed << std::setprecision(2) << "Speed: " << speed << "x";
		mSpeedLabel->setString(str.str());
		mSpeedSlider->setValue(speed);
	}
	if (pitch > 0)
	{
		stringstream str;
		str << std::fixed << std::setprecision(2) << "Pitch: " << pitch << "x";
		mPitchLabel->setString(str.str());
		mPitchSlider->setValue(pitch);
	}
}
void ControlWindow::onBtnPushed(Button* btn)
{
	if (btn == mPlayButton || btn == mPauseButton)
	{
		if (SoundManager::instance()->isSongLoaded())
		{
			SoundManager::instance()->togglePaused();
		}
		else
		{
			MusicLibrary::instance()->playCurrentSong();
		}
	}
	else if (btn == mStopButton)
	{
		SoundManager::instance()->stop();
	}
	else if (btn == mNextButton)
	{
		MusicLibrary::instance()->playNextSong();
	}
	else if (btn == mPreviousButton)
	{
		MusicLibrary::instance()->playPreviousSong();
	}
	else if (btn == mMuteButton || btn == mUnmuteButton)
	{
		SoundManager::instance()->toggleMuted();
	}
}

void ControlWindow::onSoundEvent(SoundManager::SoundEvent ev)
{
	float len = (float)SoundManager::instance()->getCurrLength();
	switch (ev)
	{
	case SoundManager::START_EVENT:
		mTimeSlider->setRangeAndStep(0, len, len / 1000.0f);
		mSpeedSlider->setValue(SoundManager::instance()->getSpeed());
	case SoundManager::UNPAUSE_EVENT:
		mWidgets[mPlayIndex] = mPauseButton;
		mPauseButton->refresh();
		break;
	case SoundManager::PAUSE_EVENT:
	case SoundManager::STOP_EVENT:
	case SoundManager::SOUND_FINISHED_EVENT:
		mWidgets[mPlayIndex] = mPlayButton;
		mPlayButton->refresh();
		break;
	case SoundManager::MUTE_EVENT:
		mWidgets[mMuteIndex] = mUnmuteButton;
		mUnmuteButton->refresh();
		break;
	case SoundManager::UNMUTE_EVENT:
		mWidgets[mMuteIndex] = mMuteButton;
		mMuteButton->refresh();
		break;
	case SoundManager::SPEED_EVENT:
		refreshSliders(SoundManager::instance()->getSpeed(), 0);
		break;
	case SoundManager::PITCH_EVENT:
		refreshSliders(0, SoundManager::instance()->getPitch());
	break;
	}

	// redraw time
	mUpdateTime = TIME_UPDATE_DELAY; 
}
void ControlWindow::onSliderChanged(Slider* slider)
{
	if (slider == mVolumeSlider)
	{
		SoundManager::instance()->setVolume(slider->getValue());
	}
	else if (slider == mTimeSlider)
	{
		SoundManager::instance()->setCurrPosition((unsigned int)slider->getValue());
	}
	else if (slider == mSpeedSlider)
	{
		float speed = slider->getValue();
		refreshSliders(speed, 0);
		SoundManager::instance()->setSpeed(speed);
	}
	else if (slider == mPitchSlider)
	{
		float pitch = slider->getValue();
		refreshSliders(0, pitch);
		SoundManager::instance()->setPitch(pitch);
	}
}

void ControlWindow::onLabelChanged(SimpleLabel* label)
{
	if (label == mTimeLabel)
	{
		setTimeFormat(((int)(mTimeFormat + 1)) % INVALID);
	}
}
