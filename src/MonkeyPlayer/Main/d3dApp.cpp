//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm:
//
// d3dApp.cpp by Frank Luna (C) 2005 All Rights Reserved.

#include "d3dApp.h"
#include "DatabaseManager.h"
#include "Settings.h"
//#include <windows.h>

D3DApp* gApp = 0;
IDirect3DDevice9* gDevice = 0;
MonkeyInput* gInput = 0;
WindowManager* gWindowMgr = 0;

LRESULT CALLBACK
MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// don't process msgs unless app has been created
	if (gApp != 0)
	{
		return gApp->msgProc(msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
D3DApp::D3DApp(HINSTANCE hInstance, std::string caption, D3DDEVTYPE deviceType, DWORD vertexProc)
{
	mCaption = caption;
	mDeviceType = deviceType;
	mVertexProc = vertexProc;

	mHAppInstance = hInstance;
	mHwnd = 0;
	m3dObj = 0;
	mPaused = false;
	ZeroMemory(&mPresParams, sizeof(mPresParams));

	initMainWindow();
	initDirect3D();


}
D3DApp::~D3DApp()
{
	ReleaseCOM(m3dObj);
	ReleaseCOM(gDevice);

	DatabaseManager::shutdown();
}

void D3DApp::initMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mHAppInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass failed!", 0, 0);
		PostQuitMessage(0);
	}

	RECT R = {0, 0, Settings::instance()->getIntValue("width", 800), Settings::instance()->getIntValue("height", 600)};
	AdjustWindowRect(&R, WS_TILED, false);
	mHwnd = CreateWindow("D3DWndClassName", mCaption.c_str(), WS_OVERLAPPEDWINDOW | CS_DBLCLKS,
		100, 100, R.right, R.bottom, 0, 0, mHAppInstance, 0);

	if (!mHwnd)
	{
		MessageBox(0, "CreateWindow failed!", 0, 0);
		PostQuitMessage(0);
	}
	ShowWindow(mHwnd, SW_SHOW);
	UpdateWindow(mHwnd);

}
void D3DApp::initDirect3D()
{
	// create IDirect3D object
	m3dObj = Direct3DCreate9(D3D_SDK_VERSION);
	
	if (!m3dObj)
	{
		MessageBox(0, "Direct3DCreate9 failed!", 0, 0);
		PostQuitMessage(0);
	}

	// verify hw spport for formats in windowed and fullscreen modes
	D3DDISPLAYMODE mode;
	m3dObj->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
	HR(m3dObj->CheckDeviceType(D3DADAPTER_DEFAULT, mDeviceType, mode.Format, mode.Format, true));
	HR(m3dObj->CheckDeviceType(D3DADAPTER_DEFAULT, mDeviceType, D3DFMT_X8R8G8B8, D3DFMT_X8R8G8B8, false));

	// check for requested vertex processing and pure device
	D3DCAPS9 caps;
	HR(m3dObj->GetDeviceCaps(D3DADAPTER_DEFAULT, mDeviceType, &caps));

	DWORD devBehaviorFlags = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
	{
		devBehaviorFlags |= mVertexProc;
	}
	else
	{
		devBehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	
	// if HW transform and light and pure device
	if (caps.DevCaps & D3DDEVCAPS_PUREDEVICE &&
		devBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
	{
		devBehaviorFlags |= D3DCREATE_PUREDEVICE;
	}

	// fill presentation parameters
	mPresParams.BackBufferWidth = 0;
	mPresParams.BackBufferHeight = 0;
	mPresParams.BackBufferFormat = D3DFMT_UNKNOWN;
	mPresParams.BackBufferCount = 1;
	mPresParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	mPresParams.MultiSampleQuality = 0;
	mPresParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	mPresParams.hDeviceWindow = mHwnd;
	mPresParams.Windowed = true;
	mPresParams.EnableAutoDepthStencil = true;
	mPresParams.AutoDepthStencilFormat = D3DFMT_D24S8;
	mPresParams.Flags = 0;
	mPresParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	mPresParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// create device
	HR(m3dObj->CreateDevice(D3DADAPTER_DEFAULT,
		mDeviceType,mHwnd,
		devBehaviorFlags,
		&mPresParams,
		&gDevice));


}
int D3DApp::run()
{
	MSG msg;

	msg.message = WM_NULL;

	__int64 countsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	float secsPerCount =	1.0f / (float)countsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);
	__int64 currTimeStamp = 0;

	while (msg.message != WM_QUIT)
	{
		// Windows message
		if (PeekMessage(&msg,  0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{ // game stuff
			if (mPaused)
			{
				Sleep(20);
				continue;
			}

			if (!isDeviceLost())
			{
				QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
				float dt = (currTimeStamp - prevTimeStamp) * secsPerCount;

				updateScene(dt);
				drawScene();

				prevTimeStamp = currTimeStamp;
			}
		}
	}
	return (int)msg.wParam;
}

// presentation parameters should be set prior to calling this
void D3DApp::processResize()
{

	Settings::instance()->setValue("width", (int)mPresParams.BackBufferWidth);
	Settings::instance()->setValue("height", (int)mPresParams.BackBufferHeight);

	onDeviceLost();
	HR(gDevice->Reset(&mPresParams));
	onDeviceReset();

}

LRESULT D3DApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool minOrMaxed = false; // window not "restored"

	RECT clientRect = { 0, 0, 0, 0 };

	bool processed = false;
	if (gInput != NULL)
	{
		gInput->processMessage(msg, wParam, lParam);
	}
	if (processed)
	{
		return 0;
	}

	switch (msg)
	{
		// WM_ACTIVE is sent when the window is activated or deactivated.
		// We pause the game when the main window is deactivated and 
		// unpause it when it becomes active.
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) 
		{
//			mPaused = true;
			mActive = false;
		}
		else
		{
			mPaused = false;
			mActive = true;;
		}
		return 0;

		// user resizing window
	case WM_SIZE:
		if (gDevice)
		{
			mPresParams.BackBufferWidth = LOWORD(lParam);
			mPresParams.BackBufferHeight= HIWORD(lParam);

			if (wParam == SIZE_MINIMIZED)
			{
				mPaused = true;
				minOrMaxed = true;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mPaused = false;
				minOrMaxed = true;
				processResize();
			}
			// Restored is any resize that is not a minimize or maximize.
			// For example, restoring the window to its default size
			// after a minimize or maximize, or from dragging the resize
			// bars.
			else if (wParam == SIZE_RESTORED) 
			{
				mPaused = false;

				// Are we restoring from a mimimized or maximized state, 
				// and are in windowed mode?  Do not execute this code if 
				// we are restoring to full screen mode.
				if (minOrMaxed && mPresParams.Windowed)
				{
					processResize();
				}
				else
				{
					// No, which implies the user is resizing by dragging
					// the resize bars.  However, we do not reset the device
					// here because as the user continuously drags the resize
					// bars, a stream of WM_SIZE messages is sent to the window,
					// and it would be pointless (and slow) to reset for each
					// WM_SIZE message received from dragging the resize bars.
					// So instead, we reset after the user is done resizing the
					// window and releases the resize bars, which sends a
					// WM_EXITSIZEMOVE message.
				}
				minOrMaxed = false;
			}
		}
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		GetClientRect(mHwnd, &clientRect);
		mPresParams.BackBufferWidth = clientRect.right;
		mPresParams.BackBufferHeight = clientRect.bottom;
		processResize();
		
		return 0;

	// WM_CLOSE is sent when the user presses the 'X' button in the
	// caption bar menu.
	case WM_CLOSE:
		DestroyWindow(mHwnd);
		return 0;

		// window is being destroyed
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			enableFullScreen(false);
		}
		else if (wParam == 'F')
		{
			enableFullScreen(true);
		}
		return 0;
		}

		// if we processed it, we should have returned already
		return DefWindowProc(mHwnd, msg, wParam, lParam);
}

void D3DApp::enableFullScreen(bool enable)
{
	if ((BOOL)enable == mPresParams.Windowed)
	{
		if (enable)
		{
			int width = GetSystemMetrics(SM_CXSCREEN);
			int height= GetSystemMetrics(SM_CYSCREEN);

			mPresParams.BackBufferFormat = D3DFMT_X8R8G8B8;
			mPresParams.BackBufferWidth = width;
			mPresParams.BackBufferHeight= height;
			mPresParams.Windowed = false;

			// Change the window style to a more fullscreen friendly style.
			SetWindowLongPtr(mHwnd, GWL_STYLE, WS_POPUP);

			// If we call SetWindowLongPtr, MSDN states that we need to call
			// SetWindowPos for the change to take effect.  In addition, we 
			// need to call this function anyway to update the window dimensions.
			SetWindowPos(mHwnd, HWND_TOP, 0, 0, width, height, SWP_NOZORDER | SWP_SHOWWINDOW);	
		}
		else
		{
			RECT r = {0, 0, 800, 600 };
			AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);

			mPresParams.BackBufferFormat = D3DFMT_X8R8G8B8;
			mPresParams.BackBufferWidth = 800;
			mPresParams.BackBufferHeight = 600;
			mPresParams.Windowed = true;

			// Change the window style to a more windowed friendly style.
			SetWindowLongPtr(mHwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			// If we call SetWindowLongPtr, MSDN states that we need to call
			// SetWindowPos for the change to take effect.  In addition, we 
			// need to call this function anyway to update the window dimensions.
			SetWindowPos(mHwnd, HWND_TOP, 100, 100, r.right, r.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
		}

		onDeviceLost();
		HR(gDevice->Reset(&mPresParams));
		onDeviceReset();
	}
}
bool D3DApp::isDeviceLost()
{
	HRESULT hr = gDevice->TestCooperativeLevel();

	// if lost and can't reset
	if (hr == D3DERR_DEVICELOST)
	{
		Sleep(20);
		return true;
	}
	// uh-oh
	else if (hr == D3DERR_DRIVERINTERNALERROR)
	{
		MessageBox(0, "Internal driver error...exiting.", 0, 0);
		PostQuitMessage(0);
		return true;
	}
	// lost but we can restore it
	else if (hr == D3DERR_DEVICENOTRESET)
	{
		onDeviceLost();
		HR(gDevice->Reset(&mPresParams));
		onDeviceReset();
		return false;
	}
	return false;

}


// get/set
HINSTANCE D3DApp::getAppInstance()
{
	return mHAppInstance;
}
HWND D3DApp::getMainWnd()
{
	return mHwnd;
}
Camera* D3DApp::getCamera()
{
	return mCamera;
}
