
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm:
//
// GfxStats.h by Frank Luna (C) 2005 All Rights Reserved.
//
// Class used for keeping track of and displaying the frames rendered
// per second, milliseconds per frame, and vertex and triangle counts.

#include "d3dUtil.h"
#include "GfxStats.h"
#include <d3dx9.h>
#include <tchar.h>

GfxStats::GfxStats()
{
	mFps = 0;
	mFrameDuration = 0;
	mNumTriangles = 0;

	D3DXFONT_DESC font;
	font.Height = 16;
	font.Width = 0;
	font.Weight = 0;
	font.MipLevels = 1;
	font.Italic = false;
	font.CharSet = DEFAULT_CHARSET;
	font.OutputPrecision = OUT_DEFAULT_PRECIS;
	font.Quality = DEFAULT_QUALITY;
	font.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_tcscpy_s(font.FaceName, _T("Times New Roman"));

	HR(D3DXCreateFontIndirect(gDevice, &font, &mFont));
}
GfxStats::~GfxStats()
{
	ReleaseCOM(mFont);
}

void GfxStats::onDeviceLost()
{
	HR(mFont->OnLostDevice());
}
void GfxStats::onDeviceReset()
{
	HR(mFont->OnResetDevice());
}

void GfxStats::addTriangles(DWORD n)
{
	mNumTriangles += n;
}
void GfxStats::setTriangles(DWORD n)
{
	mNumTriangles = n;
}
void GfxStats::subtractTriangles(DWORD n)
{
	mNumTriangles = max(n, 0);
}

void GfxStats::update(float dt)
{
	static float numFrames = 0;
	static float timeElapsed = 0;

	numFrames++;

	timeElapsed += dt;

	// compute fps once per second
	if (timeElapsed >= 1.0f)
	{
		mFps = numFrames;

		mFrameDuration = 1000.0f / mFps;

		timeElapsed = 0;
		numFrames = 0;
	}
}

void GfxStats::display()
{
	static char buffer[300];

	sprintf_s(buffer, "FPS: %.2f\nMilliseconds per frame: %.4f\nNumber of triangles: %d", 
		mFps, mFrameDuration, mNumTriangles);

	RECT r = { 0, 0, 0, 0 };
	HR(mFont->DrawText(0, buffer, -1, &r, DT_NOCLIP, D3DCOLOR_XRGB(255, 0, 0)));
}