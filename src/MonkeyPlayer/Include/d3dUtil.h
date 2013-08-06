//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm:
//
// d3dUtil.h by Frank Luna (C) 2005 All Rights Reserved.
//
// Contains various utility code for DirectX applications, such as, clean up
// and debugging code.

#ifndef D3DUTIL_H
#define D3DUTIL_H

// debug...may slow things down a bit
#if defined(DEBUG) | defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#define _AFXDLL
#include <afxmt.h>
#include <Afxwin.h>
//#include <windows.h>

#include <MonkeyInput.h>
#include <Logger.h>
#include<dinput.h>
#include<d3d9.h>
#include<d3dx9.h>
#include<dxerr.h>
#include<string>
#include<sstream>

// globals
class D3DApp;
//class MonkeyInput;
extern D3DApp *gApp;
extern IDirect3DDevice9* gDevice;
extern MonkeyInput* gInput;

// clean up macros
#define ReleaseCOM(x) { if (x) { x->Release(); x = 0; } }

// debug macro
#if defined(DEBUG) | defined(_DEBUG)
   #ifndef HR
		#define HR(x)							\
		{											\
			HRESULT hr = x;					\
			if (FAILED(hr))					\
			{										\
				DXTrace(__FILE__, __LINE__, hr, #x, TRUE); \
			}										\
		}
	#endif
#else
	#ifndef HR
		#define HR(x) x;
	#endif
#endif
#endif

// memory leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
#define snew new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define snew new
#endif