// RenderTarget.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// A class for rendering to a surface
#include "d3dUtil.h"

#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

namespace MonkeyPlayer
{
	class RenderTarget
	{
	public:
		RenderTarget(int width, int height, D3DXCOLOR bgColor, bool ownTexture);
		~RenderTarget();

		void onDeviceLost();
		void onDeviceReset();

		void recreateTargets();
		void beginScene();
		void endScene();

		void setDimensions(int width, int height);
		int getWidth();
		int getHeight();
		void setColor(D3DXCOLOR bgColor);
		D3DXCOLOR getColor();
		LPDIRECT3DTEXTURE9 getTexture();

	private:
		int mWidth, mHeight;
		bool mOwnTexture;
		LPDIRECT3DTEXTURE9 mTexture;
		LPDIRECT3DSURFACE9 mSurface;
		LPDIRECT3DSURFACE9 mBackBuffer;
		ID3DXRenderToSurface* mRTS;
		D3DVIEWPORT9 mViewport;
		D3DXCOLOR mBgColor;

	};
}
#endif