// RenderTarget.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A class for rendering to a surface

#include "RenderTarget.h"

using namespace MonkeyPlayer;

RenderTarget::RenderTarget(int width, int height, D3DXCOLOR bgColor, bool ownTexture)
{
	mWidth = width;
	mHeight = height;
	mBgColor = bgColor;

	mTexture = NULL;
	mSurface = NULL;
	mBackBuffer = NULL;

	mRTS = NULL;

	mOwnTexture = ownTexture;
}
RenderTarget::~RenderTarget()
{
	if (mOwnTexture)
	{
		ReleaseCOM(mTexture);
	}
	ReleaseCOM(mSurface);
	ReleaseCOM(mRTS);
}

void RenderTarget::onDeviceLost()
{
	ReleaseCOM(mSurface);
	ReleaseCOM(mTexture);
	ReleaseCOM(mRTS);
}
void RenderTarget::onDeviceReset()
{
	recreateTargets();
}

void RenderTarget::recreateTargets()
{
	ReleaseCOM(mSurface);
	ReleaseCOM(mTexture);
	ReleaseCOM(mRTS);

	mViewport.X = 0;
	mViewport.Y = 0;
	mViewport.Width = (DWORD)mWidth;
	mViewport.Height = (DWORD)mHeight;
	mViewport.MinZ = 0.0f;
	mViewport.MaxZ = 1.0f;

	HRESULT hr = D3DXCreateTexture(gDevice, (UINT)mWidth, (UINT)mHeight, 0, D3DUSAGE_RENDERTARGET | D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT,
		&mTexture);
	if (hr == D3D_OK)
	{
		hr = D3DXCreateRenderToSurface(gDevice, (DWORD)mWidth, (DWORD)mHeight, D3DFMT_A8R8G8B8, true, D3DFMT_D24X8, &mRTS);
		if (hr == D3D_OK)
		{
			mTexture->GetSurfaceLevel(0, &mSurface);
		}
	}
}

void RenderTarget::beginScene()
{
	mRTS->BeginScene(mSurface, &mViewport);

	gDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, mBgColor, 1.0f, 0);

	gDevice->BeginScene();

}
void RenderTarget::endScene()
{
	mRTS->EndScene(D3DX_FILTER_POINT);
}

void RenderTarget::setDimensions(int width, int height)
{
	mWidth = width;
	mHeight = height;
	recreateTargets();
}
int RenderTarget::getWidth()
{
	return mWidth;
}
int RenderTarget::getHeight()
{
	return mHeight;
}
void RenderTarget::setColor(D3DXCOLOR bgColor)
{
	mBgColor = bgColor;
}
D3DXCOLOR RenderTarget::getColor()
{
	return mBgColor;
}
LPDIRECT3DTEXTURE9 RenderTarget::getTexture()
{
	return mTexture;
}