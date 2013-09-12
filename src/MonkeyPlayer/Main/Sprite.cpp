// Sprite.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Manages a single image to draw on the screen

#include "d3dUtil.h"
#include "Sprite.h"
#include "Vertex.h"

Sprite::Sprite(const char* textureFile, float x, float y, float width, float height)
{
	Logger::instance()->write(string("Sprite is loading texture: ") + textureFile);

	IDirect3DTexture9* tex;
	HR(D3DXCreateTextureFromFile(gDevice, textureFile, &tex));
	mTextures[0] = tex;
	mCurrIndex = 0;

	createBuffers();
	setDest(x, y, width, height);
	setColor(D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
}

Sprite::Sprite(const char* textureFile, float x, float y, float width, float height, D3DXVECTOR4 color)
{
	Logger::instance()->write(string("Sprite is loading texture: ") + textureFile);
	
	IDirect3DTexture9* tex;
	HR(D3DXCreateTextureFromFile(gDevice, textureFile, &tex));
	mTextures[0] = tex;
	mCurrIndex = 0;

	createBuffers();
	setDest(x, y, width, height);
	setColor(color);
}
Sprite::Sprite(IDirect3DTexture9* texture, float x, float y, float width, float height)
{
	mTextures[0] = texture;
	mCurrIndex = 0;

	createBuffers();
	setDest(x, y, width, height);
	setColor(D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
}
Sprite::~Sprite()
{
	if (mOwnSprite)
	{
		std::map<int, IDirect3DTexture9*>::iterator iter;
		for (iter = mTextures.begin(); iter != mTextures.end(); iter++)
		{
			ReleaseCOM((*iter).second);
		}
		ReleaseCOM(mVertexBuffer);
		ReleaseCOM(mIndexBuffer);
	}
}
void Sprite::createBuffers()
{
		// Obtain a pointer to a new vertex buffer.
	HR(gDevice->CreateVertexBuffer(4 * sizeof(Vertex::VertexPosTex), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED, &mVertexBuffer, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// cube's vertex data.

	Vertex::VertexPosTex* v = 0;
	HR(mVertexBuffer->Lock(0, 0, (void**)&v, 0));

	v[1] = Vertex::VertexPosTex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[0] = Vertex::VertexPosTex(0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	v[3] = Vertex::VertexPosTex(1.0f, 1.0f, 0.0f, 1.0f, 1.0f);
	v[2] = Vertex::VertexPosTex(1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	HR(mVertexBuffer->Unlock());

	// Obtain a pointer to a new index buffer.
	HR(gDevice->CreateIndexBuffer(6 * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIndexBuffer, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// cube's index data.

	WORD* k = 0;

	HR(mIndexBuffer->Lock(0, 0, (void**)&k, 0));

	// Front face.
	k[0] = 0; k[1] = 1; k[2] = 2;
	k[3] = 0; k[4] = 2; k[5] = 3;

	HR(mIndexBuffer->Unlock());
}
void Sprite::onDeviceLost()
{
}
void Sprite::onDeviceReset()
{
}

void Sprite::setDest(float x, float y, float width, float height)
{
	mX = x - .5f;
	mY = y - .5f;
	mWidth = width;
	mHeight = height;
}

void Sprite::setDest(int x, int y, int width, int height)
{
	setDest((float)x, (float)y, (float)width, (float)height);
}
void Sprite::setDest(float dx, float dy)
{
	mX += dx;
	mY += dy;
}
void Sprite::setColor(D3DXVECTOR4 color)
{
	mColor = color;
}
float Sprite::getX()
{
	return mX;
}
float Sprite::getY()
{
	return mY;
}
float Sprite::getWidth()
{
	return mWidth;
}
float Sprite::getHeight()
{
	return mHeight;
}
D3DXVECTOR4 Sprite::getColor()
{
	return mColor;
}

UINT Sprite::getVertexStride()
{
	return sizeof(Vertex::VertexPosTex);
}
int Sprite::getNumTriangles()
{
	return 2;
}
int Sprite::getNumVertices()
{
	return 4;
}

IDirect3DVertexDeclaration9* Sprite::getVertexDeclaration()
{
	return Vertex::VertexPosTex::Decl;
}

D3DXMATRIX Sprite::getWorld()
{
	return mWorld;
}
IDirect3DTexture9* Sprite::getTexture()
{
	return mTextures[mCurrIndex];
}

int Sprite::getCurrentIndex()
{
	return mCurrIndex;
}

void Sprite::addTexture(int index, IDirect3DTexture9* tex, bool useNow, bool deleteOld)
{
	if (deleteOld && mTextures.find(index) != mTextures.end())
	{
		ReleaseCOM(mTextures[index]);
	}
	mTextures[index] = tex;
	if (useNow)
	{
		mCurrIndex = index;
	}
}
void Sprite::addTexture(int index, const char* filename, bool useNow, bool deleteOld)
{
	IDirect3DTexture9* tex;
	HR(D3DXCreateTextureFromFile(gDevice, filename, &tex));
	addTexture(index, tex, useNow, deleteOld);
}
void Sprite::setTextureIndex(int index)
{
	if (mTextures.find(index) != mTextures.end())
	{
		mCurrIndex = index;
	}
}
void Sprite::replaceCurrentTexture(IDirect3DTexture9* tex, bool dispose)
{
	if (dispose)
	{
		ReleaseCOM(mTextures[mCurrIndex]);
	}
	mTextures[mCurrIndex] = tex;
}

IDirect3DVertexBuffer9* Sprite::getVertexBuffer()
{
	return mVertexBuffer;
}
IDirect3DIndexBuffer9* Sprite::getIndexBuffer()
{
	return mIndexBuffer;
}

void Sprite::setTextureOwned(bool owned)
{
	mOwnSprite = owned;
}
