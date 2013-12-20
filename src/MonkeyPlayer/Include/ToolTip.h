// Checkbox.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A simple tooltip
#include <string>
#include <vector>

#include "IWidget.h"
#include "Label.h"
#include "Sprite.h"

#ifndef TOOL_TIP_H
#define TOOL_TIP_H

namespace MonkeyPlayer
{
	class ToolTip : public IWidget
	{
	public:
		ToolTip(float x, float y, string text);
		~ToolTip();

		void onDeviceLost();
		void onDeviceReset();

		void update(float dt);

		virtual void preRender();
		void display() {}
		void setPos(float x, float y, float width = 0, float height = 0);
		float getX() { return mX; }
		float getY() { return mY; }
		float getWidth() { return mWidth + mLabel->getWidth(); }
		float getHeight() { return mHeight; }

		std::vector<Sprite*> getSprites();
		int getNumTriangles();
		virtual bool onMouseEvent(MouseEvent e);
		virtual void refresh();
		void setVisible(bool visible, IWidget* widget);
		bool getVisible();
		void setText(string text);
		void setup(IWidget* parent, string text, int x, int y);

	protected:

		static const float TOOLTIP_DIMENSION;

		std::vector<Sprite*> mSprites;
		Sprite* mBackgroundSprite; 
		Sprite* mBorderSprite; 
		SimpleLabel* mLabel;
		IWidget* mParentWidget;

		float mX, mY, mWidth, mHeight, mX2, mY2;
		bool mStartedOnTop;
		string mText;
		bool mTextChanged;
		bool mVisible;
		float mTimer;

		bool isPointInside(int x, int y);

	};
}
#endif