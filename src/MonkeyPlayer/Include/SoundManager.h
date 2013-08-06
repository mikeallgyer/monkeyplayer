// SoundManager.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// handles the actual playing of music

//#include <windows.h>
#include <stdio.h>
#include <conio.h>

#include "d3dUtil.h"
//#include "fmod.h"
#include "fmod.hpp"
#include "fmod_errors.h"

#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

class SoundManager
{
public:
	static enum SoundEvent { START_EVENT, STOP_EVENT, PAUSE_EVENT, UNPAUSE_EVENT, VOLUME_EVENT, SEEK_EVENT,
		MUTE_EVENT, UNMUTE_EVENT, SOUND_FINISHED_EVENT};

	void addCallback(void (*cb)(void* objPtr, SoundEvent ev), void* objPtr);
	void removeCallback(void* objPtr);
	void update();
	void playFile(const char* filename);
	bool isPlaying();
	bool isPaused();
	void setPaused(bool paused);
	void togglePaused();
	bool isMuted();
	void setMuted(bool paused);
	void toggleMuted();
	void stop();
	void play();
	void setVolume(float volume);
	float getVolume();
	unsigned int getCurrLength();  // milliseconds
	unsigned int getCurrPosition();  // milliseconds
	void setCurrPosition(unsigned int pos); // milliseconds
	std::string getCurrFile();
	
	static std::string getTimeString(int milliseconds);
	static SoundManager* instance();
	static void shutdown();

private:
	SoundManager();
	~SoundManager();

	static SoundManager* mInstance;

	FMOD::System* mSystem;
	FMOD::Sound* mSound;
	FMOD::Channel* mChannel;
	
	float mVolume;
	bool mMuted;
	std::vector<void (*)(void* ptrObj_EVENT, SoundEvent ev) > mCallbacks;
	std::vector<void*> mCallbackObj;
	bool mStopPermanately;
	std::string mCurrFile;
	bool mStoppedManually;

	void notifyAll(SoundEvent ev);
	static FMOD_RESULT F_CALLBACK trackEnd(FMOD_CHANNEL *channel_EVENT, FMOD_CHANNEL_CALLBACKTYPE type_EVENT, 
		void* commanddata1_EVENT, void* commanddata2);

};

#endif