//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm:
//
// GfxStats.h by Frank Luna (C) 2005 All Rights Reserved.
//
// Interface for all classes that can be drawn

#include "d3dUtil.h"
#include <d3dx9.h>
#include <tchar.h>

#ifndef IDRAWABLE_H
#define IDRAWABLE_H

namespace MonkeyPlayer
{
	class ItemListBox;

	class IDrawable
	{
	public:
		virtual ~IDrawable() {}
		virtual void onDeviceLost() = 0;
		virtual void onDeviceReset() = 0;

		virtual void update(float dt) = 0;

		virtual void preRender() {}
		virtual void display() {}
		virtual int getNumTriangles() = 0;
		virtual void onContextMenuSelected(ItemListBox* menu) {}
	};
}
#endif
