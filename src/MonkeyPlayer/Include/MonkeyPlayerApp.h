//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Main driver of program
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm
//
// Frank Luna (C) 2005 All Rights Reserved.

#ifndef MonkeyPlayer_APP_H
#define MonkeyPlayer_APP_H

#include "d3dApp.h"
#include <tchar.h>
#include "IDrawable.h"
#include "GfxStats.h"
#include <vector>

using namespace std;

namespace MonkeyPlayer
{
	class MonkeyPlayerApp : public D3DApp
	{
	public:
		MonkeyPlayerApp(HINSTANCE hInstance, std::string caption, D3DDEVTYPE deviceType, DWORD vertexProc);
		~MonkeyPlayerApp();

		bool checkDeviceCaps();
		void onDeviceLost();
		void onDeviceReset();
		void updateScene(float dt);
		void drawScene();

		GfxStats *getStats() { return mStats; }

	private:
		GfxStats *mStats;
		vector<IDrawable*> mDrawables;

	};
}
#endif