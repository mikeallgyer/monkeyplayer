//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm:
//
// GfxStats.h by Frank Luna (C) 2005 All Rights Reserved.
//
// Class used for keeping track of and displaying the frames rendered
// per second, milliseconds per frame, and vertex and triangle counts.


#ifndef GFX_STATS_H
#define GFX_STATS_H

#include <d3dx9.h>
#include "IDrawable.h"

namespace MonkeyPlayer
{
	class GfxStats : public IDrawable
	{
	public:
		GfxStats();
		~GfxStats();

		void onDeviceLost();
		void onDeviceReset();

		void addTriangles(DWORD n);
		void subtractTriangles(DWORD n);
		void setTriangles(DWORD n);

		void update(float dt);
		void display();
		int getNumTriangles() { return 0; }

	private:
		ID3DXFont* mFont;
		float mFps;
		float mFrameDuration;
		DWORD mNumTriangles;
	};
}
#endif