// Button.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A clickable button
#include <string>
#include <vector>

#include "IWidget.h"
#include "Sprite.h"

#ifndef BUTTON_H
#define BUTTON_H

namespace MonkeyPlayer
{
	class Button : public IWidget
	{
	public:
		Button(float x, float y, float width, float height, std::string &texture,
			void (*buttonClickedCB)(void* ptrObj, Button* btn) = NULL, void* callbackObj = NULL);
		~Button();

		void onDeviceLost();
		void onDeviceReset();

		void update(float dt);

		virtual void preRender();
		void display() {}
		void setPos(float x, float y, float width = -999.0f, float height = -999.0f);
		float getX() { return mX; }
		float getY() { return mY; }
		float getWidth() { return mWidth; }
		float getHeight() { return mHeight; }

		void setUpTexture(const char* texture);
		void setHoverTexture(const char* texture);
		void setDownTexture(const char* texture);
		void setToggledTexture(const char* texture);

		std::vector<Sprite*> getSprites();
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent e);
		virtual void refresh();

		void setIsToggle(bool isToggle);
		bool getIsToggle();
		void setToggled(bool toggled);
		bool getToggled();

	protected:

		static const int TEXTURE_UP;
		static const int TEXTURE_HOVER;
		static const int TEXTURE_DOWN;
		static const int TEXTURE_TOGGLED;
		std::vector<Sprite*> mSprites; 

		float mX, mY, mWidth, mHeight, mX2, mY2;
		Sprite*  mButtonSprite;
		bool mStartedOnTop;
		bool mIsToggle;
		bool mToggled;

		bool isPointInside(int x, int y);

		// callback
		void (*mCallback)(void* ptrObj, Button* selItem);
		void *mCallbackObj;

	};
}
#endif