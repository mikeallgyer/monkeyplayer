// Label.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A basic text label
#include <string>
#include <vector>
#include "IWidget.h"
#include "RenderTarget.h"
#include "Sprite.h"

#ifndef LABEL_H
#define LABEL_H

namespace MonkeyPlayer
{
	class SimpleLabel : public IWidget
	{
	public:
		SimpleLabel(float x, float y, float width, float height, std::string &label, int fontSize = 16,
			DWORD format = DT_NOCLIP, D3DXCOLOR textColor = D3DCOLOR_XRGB(255, 255, 255), 
			D3DXCOLOR bgColor = D3DCOLOR_ARGB(0, 0, 0, 0), const char* fontName = "Arial Bold");
		~SimpleLabel();

		void onDeviceLost();
		void onDeviceReset();

		void recreateTargets();

		void update(float dt);

		virtual void preRender();
		void display() {}
		void setPos(float x, float y, float width = 0, float height = 0);
		void setFormat(DWORD format);
		void setTextColor(D3DXCOLOR color);

		void setString(std::string &str);
		std::string getString();

		std::vector<Sprite*> getSprites();
		bool isPointInside(int x, int y);
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent e);

		float getHeight() { return mHeight; }
		float getWidth() { return mWidth; }
		float getX() { return mX; }
		float getY() { return mY; }
		virtual void setVisible(bool vis) { mVisible = vis; }
		virtual bool getVisible() { return mVisible; }


		void setSizeToFit(bool fit);
		void setCallback(void (*cb)(void* objPtr, SimpleLabel* label), void* objPtr);

	protected:

		std::vector<Sprite*> mSprites; 

		float mX, mY, mWidth, mHeight;
		Sprite* mSprite;
		RenderTarget* mTarget;
		std::string mText;

		ID3DXFont* mFont;
		DWORD mFormat;
		D3DXCOLOR mTextColor;
		float mFontSize;
		bool mSizeToFit;
		bool mRedraw;
		bool mVisible;

		// callback
		void (*mCallback)(void* ptrObj, SimpleLabel* label);
		void *mCallbackObj;

	};
}
#endif