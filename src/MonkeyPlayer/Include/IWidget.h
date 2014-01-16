// IWidget.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// interface for any widget containable
// in an IWindow (think Control in C#)

#include <vector>

#ifndef I_WIDGET_H
#define I_WIDGET_H

#include "IDrawable.h"
#include "Sprite.h"

namespace MonkeyPlayer
{
	class IWidget : public IDrawable
	{
	public:
		virtual std::vector<Sprite*> getSprites() = 0;
		virtual std::vector<IWidget*> getWidgets() { return mEmptyWidgetList; }
		virtual bool onMouseEvent(MouseEvent e) { return false; } 
		virtual bool getIsFocused()
		{
			return mFocused;
		}
		virtual void focus()
		{
			mFocused = true;
		}
		virtual void blur()
		{
			mFocused = false;
		}
		virtual void setPos(float x, float y) {}
		virtual float getHeight() { return 0; }
		virtual float getWidth() { return 0; }
		virtual bool isPointInside(int x, int y) = 0;
		virtual void setVisible(bool vis) {}
		virtual bool getVisible() { return true; }
		
		
	protected:
		bool mFocused;
	private:
		std::vector<IWidget*> mEmptyWidgetList;

	};
}
#endif