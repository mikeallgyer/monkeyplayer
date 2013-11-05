// ControlWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// contains play/stop/etc buttons

#include "Button.h"
#include "d3dUtil.h"
#include "ItemListBox.h"
#include "IWidget.h"
#include "IWindow.h"
#include "Label.h"
#include "Slider.h"
#include "SoundManager.h"
#include "Sprite.h"

#include <d3dx9.h>
#include <tchar.h>
#include <vector>

#ifndef CONTROL_WINDOW_H
#define CONTROL_WINDOW_H

namespace MonkeyPlayer
{
	class ControlWindow : public IWindow
	{
	public:
		static enum TimeDisplayFormat
		{
			ELAPSED_AND_TOTAL = 0,
			REMAINING_AND_TOTAL,
			ELAPSED_ONLY,
			REMAINING_ONLY,
			INVALID
		};

		ControlWindow();
		~ControlWindow();

		void onDeviceLost();
		void onDeviceReset();

		int getWidth();
		int getHeight();

		void update(float dt);

		void display();

		std::vector<Sprite*> getSprites();
		std::vector<IWidget*> getWidgets();

		void setTimeFormat(TimeDisplayFormat fmt);
		void setTimeFormat(int fmt);

		bool onMouseEvent(MouseEvent ev);  // true if window consumed event

		void onBlur();
		void onFocus();

		static void btn_callback(void* obj, Button* btn)
		{
			ControlWindow* win = static_cast<ControlWindow*>(obj);
			if (win)
			{
				win->onBtnPushed(btn);
			}
		}
		static void soundEventCB(void *obj, SoundManager::SoundEvent ev)
		{
			ControlWindow* win = static_cast<ControlWindow*>(obj);
			if (win)
			{
				win->onSoundEvent(ev);
			}
		}
		static void sliderCB(void *obj, Slider* slider)
		{
			ControlWindow* win = static_cast<ControlWindow*>(obj);
			if (win)
			{
				win->onSliderChanged(slider);
			}
		}
		static void labelCB(void *obj, SimpleLabel* label)
		{
			ControlWindow* win = static_cast<ControlWindow*>(obj);
			if (win)
			{
				win->onLabelChanged(label);
			}
		}

	private:
		std::vector<Sprite*> mSprites;
		std::vector<IWidget*> mWidgets;
		Sprite* mBackground;

		Button* mPlayButton;
		Button* mPauseButton;
		Button* mNextButton;
		Button* mPreviousButton;
		Button* mStopButton;
		Button* mMuteButton;
		Button* mUnmuteButton;
		Slider* mVolumeSlider;
		Slider* mTimeSlider;
		SimpleLabel* mTimeLabel;
		Slider* mSpeedSlider;
		SimpleLabel* mSpeedLabel;
		Slider* mPitchSlider;
		SimpleLabel* mPitchLabel;

		int mCurrWidth;
		bool mResized;

		int mPlayIndex;
		int mMuteIndex;
		float mUpdateTime;
		TimeDisplayFormat mTimeFormat;

		static const int WINDOW_HEIGHT;
		static const float BUTTON_PLAY_SIZE;
		static const float BUTTON_OTHER_SIZE;
		static const float BUTTON_MUTE_SIZE;
		static const float VOLUME_WIDTH;
		static const float VOLUME_HEIGHT;
		static const float TIME_HEIGHT;
		static const float TIME_UPDATE_DELAY;
		static const float TIME_LABEL_HEIGHT;
		static const float SPEED_HEIGHT;
		static const float SPEED_WIDTH;

		void refreshSliders(float speed = 0, float pitch = 0);

		void onBtnPushed(Button* btn);
		void onSoundEvent(SoundManager::SoundEvent ev);
		void onSliderChanged(Slider* slider);
		void onLabelChanged(SimpleLabel* label);
	};
}
#endif
