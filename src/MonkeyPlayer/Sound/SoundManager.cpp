// SoundManager.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// handles the actual playing of music


#include <iomanip>
#include <sstream>
#include <string>

#include "SoundManager.h"
#include "d3dUtil.h"
#include "Settings.h"

using namespace MonkeyPlayer;

SoundManager* SoundManager::mInstance = NULL;
const float SoundManager::MIN_SPEED = .25f;
const float SoundManager::MAX_SPEED = 4.0f;
//#define MEM_LEAK_TEST

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        MessageBox(0, FMOD_ErrorString(result), "Sound engine error!", 0);
		  PostQuitMessage(1);
	 }
}
SoundManager::SoundManager()
{
#ifndef MEM_LEAK_TEST
	unsigned int version;
	ERRCHECK(FMOD::System_Create(&mSystem));
	ERRCHECK(mSystem->getVersion(&version));

	if (version < FMOD_VERSION)
	{
		MessageBox(0, "Bad version of FMOD", "Error", 0);
		PostQuitMessage(1);
	}

	ERRCHECK(mSystem->init(32, FMOD_INIT_NORMAL, 0));

	mSystem->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &mPitch);
	mPitch->setParameter(FMOD_DSP_PITCHSHIFT_FFTSIZE, 4096.0f);

	mSystem->addDSP(mPitch, 0);
	mPitch->setActive(true);

	mSound = NULL;
	mChannel = NULL;

	mVolume = max(0, min(1.0f, Settings::instance()->getFloatValue("Volume", .5f)));
	mAlteredFreq = 1.0f;
	mAlteredPitch = 1.0f;
	mMuted = false;
	mStopPermanately = false;
	mStoppedManually = false;
	mCurrFile = "";
#endif
}
SoundManager::~SoundManager()
{
	mCallbacks.clear();
#ifndef MEM_LEAK_TEST
	mStopPermanately = true;
	mStoppedManually = true;
	if (mChannel != NULL)
	{
		mChannel->stop();
	}
	if (mSound != NULL)
	{
		mSound->release();
	}
	mSystem->close();
	mSystem->release();
	mSystem = NULL;

	Settings::instance()->setValue("Volume", mVolume);
#endif
}
void SoundManager::addCallback(void (*cb)(void* objPtr, SoundEvent ev), void* objPtr)
{
	mCallbacks.push_back(cb);
	mCallbackObj.push_back(objPtr);
}
void SoundManager::removeCallback(void* objPtr)
{
	for (unsigned int i = 0; i < mCallbacks.size(); i++)
	{
		if (mCallbackObj[i] == objPtr)
		{
			mCallbacks[i] = NULL;
			mCallbackObj[i] = NULL;
		}
	}
}
void SoundManager::update()
{
#ifndef MEM_LEAK_TEST
	mSystem->update();
#endif
}


void SoundManager::playFile(const char* filename)
{
#ifndef MEM_LEAK_TEST
	mStopPermanately = true;
	if (mChannel != NULL)
	{
		mStoppedManually = true;
		mChannel->stop();
		notifyAll(STOP_EVENT);
	}
	if (mSound != NULL)
	{
		mSound->release();
	}
	mStopPermanately = false;
	FMOD_RESULT result = mSystem->createSound(filename, FMOD_HARDWARE | FMOD_CREATESTREAM, 0, &mSound);
	if (result != FMOD_OK)
	{
		char errMsg[300];
		sprintf_s(errMsg, 300, "Error playing %s", filename);
		MessageBox(0, errMsg, "Error", 0);
		mSound = NULL;
		mChannel = NULL;
		mCurrFile = "";
	}
	else
	{
		mSound->setMode(FMOD_LOOP_OFF);
		mSystem->playSound(FMOD_CHANNEL_FREE, mSound, false, &mChannel);
		mChannel->setVolume(mVolume);
		mChannel->setMute(mMuted);

		mChannel->setCallback(&trackEnd);

		mChannel->getFrequency(&mFreq);
		setSpeed(1.0f);

		mCurrFile = filename;
		notifyAll(START_EVENT);
	}
#endif
}
bool SoundManager::isPaused()
{
#ifndef MEM_LEAK_TEST
	bool paused = true;
	if (mChannel != NULL)
	{
		mChannel->getPaused(&paused);
	}
	return paused;
#endif
	return false;
}
bool SoundManager::isPlaying()
{
#ifndef MEM_LEAK_TEST
	bool playing = false;
	if (mChannel != NULL)
	{
		FMOD_RESULT result = mChannel->isPlaying(&playing);
		playing &= (result == FMOD_OK);
	}
	return playing;
#endif
	return false;
}
bool SoundManager::isSongLoaded()
{
	return mChannel != NULL || mSound != NULL || isPlaying();
}
void SoundManager::setPaused(bool paused)
{
	if (mChannel != NULL)
	{
		mChannel->setPaused(paused);
		notifyAll(paused ? PAUSE_EVENT : UNPAUSE_EVENT);
	}
}
void SoundManager::togglePaused()
{
	if (!isPlaying() && mChannel == NULL && mSound != NULL)
	{
		play();
	}
	else
	{
		setPaused(!isPaused());
	}
}
bool SoundManager::isMuted()
{
/*	bool muted = false;
	if (mChannel != NULL)
	{
		FMOD_RESULT result = mChannel->getMute(&muted);
		muted &= (result == FMOD_OK);
	}
	return muted;
*/
	return mMuted;
}
void SoundManager::setMuted(bool muted)
{
	mMuted = muted;
#ifndef MEM_LEAK_TEST
	if (mChannel != NULL)
	{
		mChannel->setMute(mMuted);
	}
#endif
	notifyAll(muted ? MUTE_EVENT : UNMUTE_EVENT);

}
void SoundManager::toggleMuted()
{
	setMuted(!isMuted());
}

void SoundManager::stop()
{
	if (mChannel != NULL)
	{
		if (mStopPermanately)
		{
			mStoppedManually = true;
			mChannel->stop();
			mChannel = NULL;
		}
		else
		{ 
			// fake stop...so we can still seek
			mChannel->setPaused(true);
			mChannel->setPosition(0, FMOD_TIMEUNIT_MS);
		}
		notifyAll(STOP_EVENT);
	}
}

void SoundManager::play()
{
	if (mChannel == NULL && mSound != NULL) // restart
	{
		mStoppedManually = true;
		mSystem->playSound(FMOD_CHANNEL_FREE, mSound, false, &mChannel);
		mChannel->setVolume(mVolume);
		mChannel->setMode(mMuted);
		notifyAll(START_EVENT);
	}
	else if (mChannel != NULL && mSound != NULL) // resume
	{
		setPaused(false);
		notifyAll(UNPAUSE_EVENT);
	}
}

void SoundManager::setVolume(float volume)
{
	mVolume = max(0, min(1.0f, volume));
	if (mChannel != NULL)
	{
		mChannel->setVolume(mVolume);
		notifyAll(VOLUME_EVENT);
	}
}

void SoundManager::setSpeed(float speed)
{
	if (isSongLoaded())
	{
		mAlteredFreq = max(MIN_SPEED, min(MAX_SPEED, speed));
		mChannel->setFrequency(mFreq * mAlteredFreq);
		mPitch->remove();
		mAlteredPitch = 1.0f / mAlteredFreq;
		mPitch->setParameter(FMOD_DSP_PITCHSHIFT_PITCH, mAlteredPitch);
		mSystem->addDSP(mPitch, 0);
		notifyAll(SPEED_EVENT);
		notifyAll(PITCH_EVENT);
	}
}

void SoundManager::setPitch(float pitch)
{
	if (isSongLoaded())
	{
		mPitch->remove();
		mAlteredPitch = pitch;//1.0f / mAlteredFreq;
		mPitch->setParameter(FMOD_DSP_PITCHSHIFT_PITCH, mAlteredPitch);
		mSystem->addDSP(mPitch, 0);
		notifyAll(PITCH_EVENT);
	}
}

float SoundManager::getVolume()
{
	return mVolume;
}
float SoundManager::getSpeed()
{
	return mAlteredFreq;
}
float SoundManager::getPitch()
{
	return mAlteredPitch;
}
unsigned int SoundManager::getCurrLength()
{
	unsigned int len = 1;
#ifndef MEM_LEAK_TEST
	if (mSound != NULL)
	{
		FMOD_RESULT rslt = mSound->getLength(&len, FMOD_TIMEUNIT_MS);
		if (rslt != FMOD_OK)
		{
			len = 1;
		}
	}
#endif
	return len;
}
unsigned int SoundManager::getCurrPosition()
{
	unsigned int pos = 0;
#ifndef MEM_LEAK_TEST
	if (mChannel != NULL)
	{
		FMOD_RESULT rslt = mChannel->getPosition(&pos, FMOD_TIMEUNIT_MS);
		if (rslt != FMOD_OK)
		{
			pos = 0;
		}
	}
#endif
	return pos;
}
void SoundManager::setCurrPosition(unsigned int pos)
{
	if (mChannel != NULL)
	{
		FMOD_RESULT rslt = mChannel->setPosition(pos, FMOD_TIMEUNIT_MS);
		notifyAll(SEEK_EVENT);
	}
}

std::string SoundManager::getCurrFile()
{
	return mCurrFile;
}
SoundManager* SoundManager::instance()
{
	if (mInstance == NULL)
	{
		mInstance = snew SoundManager();
	}
	return mInstance;
}
void SoundManager::shutdown()
{
	if (mInstance != NULL)
	{
		delete mInstance;
		mInstance = NULL;
	}
}

std::string SoundManager::getTimeString(int milliseconds)
{
	int totalSeconds = milliseconds / 1000;
	int seconds = totalSeconds % 60;
	int minutes = (totalSeconds / 60) % 60;
	int hours = totalSeconds / 3600;
	std::stringstream str;
	str << std::setfill('0');
	if (hours > 0)
	{
		str << hours << ":" << std::setw(2);
	}
	str << minutes << ":" << std::setw(2) << seconds;
	return str.str();
}
void SoundManager::notifyAll(SoundEvent ev)
{
	for (unsigned int i = 0; i < mCallbacks.size(); i++)
	{
		if (mCallbacks[i] != NULL)
		{
			mCallbacks[i](mCallbackObj[i], ev);
		}
	}
}
FMOD_RESULT F_CALLBACK SoundManager::trackEnd(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, 
		void* commanddata1, void* commanddata2)
{
	if (type == FMOD_CHANNEL_CALLBACKTYPE_END)
	{
		SoundManager* mgr = SoundManager::instance();
		mgr->mChannel = NULL;
		// notify subscribers
		if (!mgr->mStoppedManually)
		{
			mgr->notifyAll(SOUND_FINISHED_EVENT);
		}
		else
		{
			mgr->notifyAll(STOP_EVENT);
		}
		mgr->mStoppedManually = false;
		// reload so we can still seek before pressing play
		if (!mgr->mStopPermanately)
		{
			mgr->mSystem->playSound(FMOD_CHANNEL_FREE, mgr->mSound, false, &mgr->mChannel);
			mgr->mChannel->setVolume(mgr->mVolume);
			mgr->mChannel->setMute(mgr->mMuted);
			mgr->mChannel->setPaused(true);
		}
	}
	return FMOD_OK;
}