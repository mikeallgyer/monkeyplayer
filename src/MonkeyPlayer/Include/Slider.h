// Slider.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A slider widget
#include <string>
#include <vector>

#include "IWidget.h"
#include "Sprite.h"

#ifndef SLIDER_H
#define SLIDER_H

namespace MonkeyPlayer
{
	class Slider : public IWidget
	{
	public:
		Slider(float x, float y, float width, float height, float min, float max, float step,
			const char* handleFile, const char* handleHoverFile, const char* handleDownFile,
			void (*sliderMovedCB)(void* ptrObj, Slider* btn) = NULL, void* callbackObj = NULL);
		Slider(float x, float y, float width, float height, float min, float max, float step,
			void (*sliderMovedCB)(void* ptrObj, Slider* btn) = NULL, void* callbackObj = NULL);
		~Slider();

		void onDeviceLost();
		void onDeviceReset();

		void setValue(float value);
		float getValue();
		void update(float dt);

		virtual void preRender();
		void display() {}
		void setPos(float x, float y, float width, float height);
		void setRange(float minValue, float maxValue);

		std::vector<Sprite*> getSprites();
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent e);

		void setCallback(void (*cb)(void* objPtr, Slider* slider), void* objPtr);

	protected:
		static const int TEXTURE_UP;
		static const int TEXTURE_HOVER;
		static const int TEXTURE_DOWN;

		std::vector<Sprite*> mSprites; 

		float mX, mY, mWidth, mHeight, mX2, mY2;
		float mMax, mMin, mStep, mCurrValue;
		Sprite*  mInnerLeft;
		Sprite*  mInnerMid;
		Sprite*  mInnerRight;
		Sprite*  mOuterLeft;
		Sprite*  mOuterMid;
		Sprite*  mOuterRight;

		Sprite* mHandleSprite;

		bool mStartedOnTop;
		
		void init(float x, float y, float width, float height, float min, float max, float step,
			const char* handleFile, const char* handleHoverFile, const char* handleDownFile, 
			void (*sliderMovedCB)(void* ptrObj, Slider* btn), void* callbackObj);

		void setHandlePosition();
		bool isPointInside(int x, int y);
		void updateValue(int mouseX);

		// callback
		void (*mCallback)(void* ptrObj, Slider* slider);
		void *mCallbackObj;

	};
}
#endif