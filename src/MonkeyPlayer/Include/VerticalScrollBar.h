// VerticalScrollBar.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Vertical scrollbar
#include <string>
#include <vector>

#include "IWidget.h"
#include "Sprite.h"

#ifndef VERTICAL_SCROLL_BAR_H
#define VERTICAL_SCROLL_BAR_H

namespace MonkeyPlayer
{
	class VerticalScrollBar : public IWidget
	{
	public:
		VerticalScrollBar(float x, float y, float height,
			void (*scrollBarMovedCB)(void* ptrObj, VerticalScrollBar* bar, float percent) = NULL, void* callbackObj = NULL);
		~VerticalScrollBar();

		void onDeviceLost();
		void onDeviceReset();

		void update(float dt);

		virtual void preRender();
		void display() {}
		void setPos(float x, float y, float height = -999.0f);
		void setHandlePosition(float percent);
		float getX() { return mX; }
		float getY() { return mY; }
		float getWidth() { return mWidth; }
		float getHeight() { return mHeight; }

		std::vector<Sprite*> getSprites();
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent e);
		virtual void refresh();

		bool isPointInside(int x, int y);
		bool isPointInsideHandle(int x, int y);
		void setHandleAtMouse(int y);

	protected:

		static const float HANDLE_WIDTH;
		static const float HANDLE_HEIGHT;
		static const float PATH_WIDTH;
		std::vector<Sprite*> mSprites; 

		float mX, mY, mWidth, mHeight, mX2, mY2;
		Sprite*  mHandleSprite;
		Sprite*  mPathSprite;
		bool mStartedOnTop;
		int mStartedY;
		float mHandleMinY;
		float mHandleMaxY;
		float mMouseMinY;
		float mMouseMaxY;
		float mCurrHandlePos; // 0-1
		bool mVisible;

		// callback
		void (*mCallback)(void* ptrObj, VerticalScrollBar* bar, float percent);
		void *mCallbackObj;

	};
}
#endif