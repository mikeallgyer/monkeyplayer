//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm:
//
// d3dApp.h by Frank Luna (C) 2005 All Rights Reserved.
//
// Contains the base Direct3D application class which provides the
// framework interface for the sample applications.  Clients are to derive 
// from D3DApp, override the framework methods, and instantiate only a single
// instance of the derived D3DApp class.  At the same time, the client should
// set the global application  pointer (gd3dApp) to point to the one and only
// instance, e.g., gd3dApp = new HelloD3DApp(hInstance);
// 

#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dUtil.h"
#include "Camera.h"
#include "GfxStats.h"
#include "WindowManager.h"

#include <MonkeyInput.h>
#include <string>

namespace MonkeyPlayer
{
	public class D3DApp
	{

	public:
		D3DApp(HINSTANCE hInstance, std::string caption, D3DDEVTYPE deviceType, DWORD vertexProc);
		virtual ~D3DApp();

		// derived methods
		virtual bool checkDeviceCaps() { return true; }
		virtual void onDeviceLost() {}
		virtual void onDeviceReset() {}
		virtual void updateScene(float dt) {}
		virtual void drawScene() {}

		// these are not usually overridden
		virtual void initMainWindow();
		virtual void initDirect3D();
		virtual int run();
		virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

		void enableFullScreen(bool enable);
		bool isDeviceLost();

		// get/set
		HINSTANCE getAppInstance();
		HWND getMainWnd();

		Camera* getCamera();

		virtual GfxStats *getStats() = 0;

		int getWidth() { return mScreenWidth; }
		int getHeight() { return mScreenHeight; }

		bool getIsActive() { return mActive; }
	protected:
		void processResize();

		std::string mCaption;
		D3DDEVTYPE mDeviceType;
		DWORD mVertexProc;

		// window/direct3d stuff
		HINSTANCE mHAppInstance;
		HWND mHwnd;
		IDirect3D9* m3dObj;
		bool mPaused;
		bool mUpdateOnly;
		D3DPRESENT_PARAMETERS mPresParams;

		int mScreenWidth;
		int mScreenHeight;
		Camera *mCamera;

		bool mActive;
	};
// globals 
extern MonkeyPlayer::D3DApp* gApp;
extern IDirect3DDevice9* gDevice;
extern MonkeyInput* gInput;
extern MonkeyPlayer::WindowManager* gWindowMgr;
}


#endif