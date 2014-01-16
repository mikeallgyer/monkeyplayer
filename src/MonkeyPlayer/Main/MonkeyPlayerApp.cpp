//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Main driver of program
//
// Some code used from, or inspired by, Frank D. Luna http://www.d3dcoder.net/d3d9c.htm
//
// Frank Luna (C) 2005 All Rights Reserved.

#include <stdlib.h>
#include <string>

#include "d3dUtil.h"

#include "AlbumTextureManager.h"
#include "Camera.h"
#include "ControlWindow.h"
#include "CollectionWindow.h"
#include "DirectoriesWindow.h"
#include "MonkeyPlayerApp.h"
#include "MusicLibrary.h"
#include "MusicLoader.h"
#include "NowPlayingWindow.h"
#include "PlaylistWindow.h"
#include "PlaybackOptionsWindow.h"
#include "SavedPlaylistsWindow.h"
#include "SoundManager.h"
#include "Vertex.h"
#include "WindowManager.h"

#include <tchar.h>

#include <fileref.h>
#include <tag.h>

using namespace MonkeyPlayer;

MonkeyPlayerApp::MonkeyPlayerApp(HINSTANCE hInstance, std::string caption, D3DDEVTYPE deviceType, DWORD vertexProc)
: D3DApp(hInstance, caption, deviceType, vertexProc)
{
	if (!checkDeviceCaps())
	{
		MessageBox(0, "Sorry, your system does not meet the minimum requirements", 0, 0);
		PostQuitMessage(0);
	}
	else
	{
		Logger::instance()->write("Creating random seed");
		std::srand(Settings::instance()->getSeed());

		mStats = snew GfxStats();

		WindowManager* mgr  = snew WindowManager();
		mDrawables.push_back(mgr);
		gWindowMgr = mgr;

		PlaylistWindow* pl = snew PlaylistWindow();
		mgr->addWindow(pl);
		mgr->addWindowBesideMain(pl);
		MusicLibrary::instance()->setPlaylistWindow(pl);
		
		ControlWindow* cw = snew ControlWindow();
		mgr->addWindow(cw);
		mgr->addWindowBelowMain(cw);

		NowPlayingWindow* nowPlaying = snew NowPlayingWindow();

		PlaybackOptionsWindow* optionsWin = snew PlaybackOptionsWindow();
		optionsWin->setX(nowPlaying->getWidth());
		mgr->addWindow(optionsWin);
		MusicLibrary::instance()->setPlaybackOptionsWindow(optionsWin);

		mgr->addWindow(nowPlaying);
		mgr->addWindowAboveMain(nowPlaying);
		
		DirectoriesWindow* dirWin = snew DirectoriesWindow();
		mgr->addWindow(dirWin);

		SavedPlaylistsWindow* spWin = snew SavedPlaylistsWindow();
		mgr->addWindow(spWin);
		spWin->setPlaylistWindow(pl);

		pl->setTopPos(mgr->getMainContentTop() + spWin->getHeight());

		CollectionWindow* collWin = snew CollectionWindow();
		mgr->addWindow(collWin);
		mgr->setCollectionWindow(collWin);
		mgr->requestFocusedWindow(collWin);

		MusicLoader::instance()->setPlaylistWindow(pl);
		MusicLoader::instance()->setCollectionWindow(collWin);
	}

	mCamera = snew Camera();
	this->onDeviceReset();

	Vertex::initVertexDeclarations();
}

MonkeyPlayerApp::~MonkeyPlayerApp()
{
	delete mCamera;
	delete mStats;

	// do this first so threads are stopped
	MusicLoader::terminate();
	for (unsigned int i = 0; i < mDrawables.size(); i++)
	{
		delete mDrawables[i];
	}
	
	Vertex::destroyVertexDeclarations();
	SoundManager::shutdown();
	Settings::destroy();
	Logger::destroy();
	MusicLibrary::destroy();
	AlbumTextureManager::destroy();
}

bool MonkeyPlayerApp::checkDeviceCaps()
{
	D3DCAPS9 caps;
	HR(gDevice->GetDeviceCaps(&caps));

	// Check for vertex shader version 2.0 support.
	if( caps.VertexShaderVersion < D3DVS_VERSION(2, 0) )
		return false;

	// Check for pixel shader version 2.0 support.
	if( caps.PixelShaderVersion < D3DPS_VERSION(2, 0) )
		return false;

	// Check render target support.  The adapter format can be either the display mode format
	// for windowed mode, or D3DFMT_X8R8G8B8 for fullscreen mode, so we need to test against
	// both.  We use D3DFMT_X8R8G8B8 as the render texture format and D3DFMT_D24X8 as the 
	// render texture depth format.
	D3DDISPLAYMODE mode;
	m3dObj->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);

	// Windowed.
	if(FAILED(m3dObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, mDeviceType, mode.Format, 
		D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8)))
		return false;
	if(FAILED(m3dObj->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, mDeviceType, mode.Format,
		D3DFMT_X8R8G8B8, D3DFMT_D24X8)))
		return false;

	// Fullscreen.
	if(FAILED(m3dObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, mDeviceType, D3DFMT_X8R8G8B8, 
		D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8)))
		return false;
	if(FAILED(m3dObj->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, mDeviceType, D3DFMT_X8R8G8B8,
		D3DFMT_X8R8G8B8, D3DFMT_D24X8)))
		return false;
 
	if( caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP )
	{
		HRESULT hr = D3D_OK;

		// Windowed.
		hr = m3dObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_AUTOGENMIPMAP,
			D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8);
//		if(hr == D3DOK_NOAUTOGEN)
//			mAutoGenMips = false;

		// Fullscreen.
		hr = m3dObj->CheckDeviceFormat(D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, D3DUSAGE_AUTOGENMIPMAP,
			D3DRTYPE_TEXTURE, D3DFMT_X8R8G8B8);
//		if(hr == D3DOK_NOAUTOGEN)
//			mAutoGenMips = false;

	}

	return true;
}
void MonkeyPlayerApp::onDeviceLost()
{
	if (!mAlreadyLost)
	{
		Logger::instance()->write("Device Lost.");
		mStats->onDeviceLost();
		for (unsigned int i = 0; i < mDrawables.size(); i++)
		{
			mDrawables[i]->onDeviceLost();
		}
	}
	mAlreadyLost = true;
}
void MonkeyPlayerApp::onDeviceReset()
{
	Logger::instance()->write("Device Reset.");
	mAlreadyLost = false;
	mStats->onDeviceReset();

	for (unsigned int i = 0; i < mDrawables.size(); i++)
	{
		mDrawables[i]->onDeviceReset();
	}
	// Sets up the camera 1000 units back looking at the origin.
	D3DXMATRIX V;
	D3DXVECTOR3 pos(0.0f, 0.0f, -10.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);

	RECT R;
	GetClientRect(this->mHwnd, &R);
	mScreenWidth = R.right;
	mScreenHeight = R.bottom;

	mCamera->configure(pos, target, up, D3DX_PI*0.25f, R.right, R.bottom);

	// reset states
	HR(gDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
	HR(gDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
	HR(gDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));

	HR(gDevice->SetRenderState(D3DRS_LIGHTING, false));
	
	HR(gDevice->SetRenderState(D3DRS_ALPHAREF, 10));
	HR(gDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER));
	
	HR(gDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
	HR(gDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));

	HR(gDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE));
	HR(gDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD));
	HR(gDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA));
	HR(gDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA));

	// 2D texture coordinates
	HR(gDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2));
}
void MonkeyPlayerApp::updateScene(float dt)
{
	gInput->poll();
	mStats->update(dt);
	SoundManager::instance()->update();
	MusicLibrary::instance()->update(dt);
	AlbumTextureManager::instance()->update(dt);
	for (unsigned int i = 0; i < mDrawables.size(); i++)
	{
		mDrawables[i]->update(dt);
	}
}
void MonkeyPlayerApp::drawScene()
{
	for (unsigned int i = 0; i < mDrawables.size(); i++)
	{
		mDrawables[i]->preRender();
	}

	HR(gDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255, 255, 105),	1.0f, 0));

	HR(gDevice->BeginScene());

	mStats->setTriangles(0);

	for (unsigned int i = 0; i < mDrawables.size(); i++)
	{
		mDrawables[i]->display();
		mStats->addTriangles(mDrawables[i]->getNumTriangles());
	}

	mStats->display();
	HR(gDevice->EndScene());
	HRESULT res = gDevice->Present(0, 0, 0, 0);

	if (res == D3DERR_DEVICELOST)
	{
	}

}
