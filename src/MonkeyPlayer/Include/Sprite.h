// Sprite.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Manages a single image (at a time) to draw on the screen
#include <map>

#include <d3dx9.h>

#ifndef SPRITE_H
#define SPRITE_H

class Sprite
{
public:
	Sprite(const char* textureFile, float x, float y, float width, float height);
	Sprite(const char* textureFile, float x, float y, float width, float height, D3DXVECTOR4 color);
	Sprite(IDirect3DTexture9* texture, float x, float y, float width, float height);
	~Sprite();

	void onDeviceLost();
	void onDeviceReset();

	void createBuffers();
	void setDest(float x, float y, float width, float height);
	void setDest(int x, int y, int width, int height);
	void setDest(float dx, float dy);
	void setColor(D3DXVECTOR4 color);

	float getX();
	float getY();
	float getWidth();
	float getHeight();
	D3DXVECTOR4 getColor();

	UINT getVertexStride();
	int getNumTriangles();
	int getNumVertices();

	IDirect3DVertexDeclaration9* getVertexDeclaration();
	D3DXMATRIX getWorld();
	IDirect3DTexture9* getTexture();

	int getCurrentIndex();
	void addTexture(int index, IDirect3DTexture9* tex, bool useNow, bool deleteOld = true);
	void addTexture(int index, const char* filename, bool useNow, bool deleteOld = true);
	void setTextureIndex(int index);
	void replaceCurrentTexture(IDirect3DTexture9* tex, bool dispose);

	IDirect3DVertexBuffer9* getVertexBuffer();
	IDirect3DIndexBuffer9*  getIndexBuffer();

	void setTextureOwned(bool owned);

private:

	std::map<int, IDirect3DTexture9*> mTextures;

	IDirect3DVertexBuffer9* mVertexBuffer;
	IDirect3DIndexBuffer9*  mIndexBuffer;

	D3DXMATRIX mWorld;

	float mX;
	float mY;
	float mWidth;
	float mHeight;

	D3DXVECTOR4 mColor;

	int mCurrIndex;

	bool mOwnSprite;
};

#endif

