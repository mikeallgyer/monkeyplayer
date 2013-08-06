//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Main entry point of program
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm
//
// [main.cpp] by Frank Luna (C) 2005 All Rights Reserved.


#include <crtdbg.h>

#include "d3dApp.h"
#include "MonkeyPlayerApp.h"

#include <MonkeyInput.h>
#include <tchar.h>

// program begins here
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// run-time memory check 
#if defined(DEBUG) | defined(_DEBUG)
int nOldState = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
_CrtSetDbgFlag(nOldState | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#define _CRTDBG_MAP_ALLOC
#define _INC_MALLOC
#endif

	MonkeyInput input(0);
	gInput = &input;
	MonkeyPlayerApp app(hInstance, "MonkeyPlayer", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	gApp = &app;
	gInput->setHwnd(gApp->getMainWnd());
	int retValue = gApp->run();
	return retValue;
}
