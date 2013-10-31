// IWindow.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// interface for any window containing "stuff"

#include "IDrawable.h"
#include "Sprite.h"
#include <MonkeyInput.h>

#include <vector>

#ifndef I_WINDOW_H
#define I_WINDOW_H

#include "IWidget.h"

namespace MonkeyPlayer
{
	class IWindow : public IDrawable
	{
	public:
		static const float BORDER_THICKNESS;

		void preRender()
		{
			std::vector<IWidget*> widgets = getWidgets();
			for (unsigned int i = 0; i < widgets.size(); i++)
			{
				widgets[i]->preRender();
			}

		}
		
		void setDepth(int depth)
		{
			mDepth = depth;
		}

		int getDepth() 
		{
			return mDepth; 
		}
		virtual int getWidth() = 0;
		virtual int getHeight() = 0;

		int getNumTriangles()
		{
			std::vector<Sprite*> sprites = getSprites();
			int total = 0;
			for (unsigned int i = 0; i < sprites.size(); i++)
			{
				total += sprites[i]->getNumTriangles();
			}

			std::vector<IWidget*> widgets = getWidgets();
			for (unsigned int i = 0; i < widgets.size(); i++)
			{
				total += widgets[i]->getNumTriangles();
			}

			return total;
		}
		virtual std::vector<Sprite*> getSprites() = 0;
		virtual std::vector<IWidget*> getWidgets() = 0;
		virtual bool onMouseEvent(MouseEvent ev) = 0;  // true if window consumed event

		virtual void onBlur() {}
		virtual void onFocus() {}

	protected:
		int mDepth;

	};
}
#endif