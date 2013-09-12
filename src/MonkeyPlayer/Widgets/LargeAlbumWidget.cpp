// LargeAlbumWidget.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// Window of large albums

#include "LargeAlbumWidget.h"
#include "d3dApp.h"
#include "DatabaseManager.h"
#include "Settings.h"
#include "Vertex.h"

#include <MonkeyInput.h>

const float LargeAlbumWidget::ALBUM_WIDTH = 4.0f;
const float LargeAlbumWidget::ALBUM_HEIGHT = 3.88f;
const float LargeAlbumWidget::ALBUM_DEPTH = .2f;

// used for synchronization
CCriticalSection LargeAlbumWidget::mCritSection;

LargeAlbumWidget::LargeAlbumWidget(float x, float y, float width, float height)
{
	std::string fxPath = Settings::instance()->getStringValue(Settings::CONTENT_DIR, "") + "\\Effects\\default.fx";

	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectPool(&mPool));
	(D3DXCreateEffectFromFile(gDevice, fxPath.c_str(), 
		0, 0, D3DXSHADER_DEBUG, 0, &mEffect, &errors));

	if (errors)
	{
		MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);
		PostQuitMessage(1);
	}

	mTechnique = mEffect->GetTechniqueByName("Phong");
	mTextureHandle = mEffect->GetParameterByName(0, "Texture");
	mViewHandle = mEffect->GetParameterByName(0, "view");
	mWorldHandle = mEffect->GetParameterByName(0, "world");
	mDiffuseHandle = mEffect->GetParameterByName(0, "diffuseColor");
	mProjectionHandle = mEffect->GetParameterByName(0, "projection");
	mLightColorHandle = mEffect->GetParameterByName(0, "lightColor");
	mAmbientHandle = mEffect->GetParameterByName(0, "ambient");
	mCamPosHandle = mEffect->GetParameterByName(0, "camPos");
	mSpecularPowerHandle = mEffect->GetParameterByName(0, "specularPower");
	mSpecularColorHandle = mEffect->GetParameterByName(0, "specularColor");
	mLightPosHandle = mEffect->GetParameterByName(0, "lightPos");
	mFogColorHandle = mEffect->GetParameterByName(0, "fogColor");
	mFogEnabledHandle = mEffect->GetParameterByName(0, "fogEnabled");

	D3DXMatrixIdentity(&mWorld);
	mDiffuseColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	mCamPos = D3DXVECTOR3(0, 0, -10.0f);
	mLightColor = D3DXVECTOR4(.5f, .5f, .5f, 1.0f);
	mAmbient = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpecularPower = 33.0f;
	mSpecularColor = D3DXVECTOR3(0.8f, 0.8f, 0.8f);
	mLightPos = D3DXVECTOR3(0.0f, 2.0f, -20.0f);
	mFogColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
	mFogEnabled = false;

	mTimeInterval = .5f;

	mPositions.push_back(AlbumPos(false, -6.0f, -D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, -5.5f, -D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, -4.0f, -D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, 0.0f, 0, mTimeInterval));
	mPositions.push_back(AlbumPos(true, 4.0f, D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(true, 5.0f, D3DX_PI*.5f, mTimeInterval));
	mPositions.push_back(AlbumPos(false, 6.0f, D3DX_PI*.5f, mTimeInterval));

	mAlbumIndex = 0;
	vector<Album*> albums = DatabaseManager::instance()->getAllAlbums();

	for (unsigned int i = 0; i < albums.size(); i++)
	{
		mLargeAlbums.push_back(snew LargeAlbumItem(*albums[i]));
		mLargeAlbums[i]->setPosition(mPositions[mPositions.size() - 1].xPos, mPositions[mPositions.size() - 1].yRotation);
		delete albums[i];
	}
	unsigned int midPos = mPositions.size() / 2;
	for (unsigned int i = midPos; i < mPositions.size() && i < albums.size() - midPos; i++)
	{
		mLargeAlbums[i - midPos]->setPosition(mPositions[i].xPos, mPositions[i].yRotation);
		mLargeAlbums[i - midPos]->setVisible(true);
	}

	D3DXVECTOR3 at(0,0,0);
	D3DXVECTOR3 up(0,1,0);
	D3DXMatrixLookAtLH(&mView, &mCamPos, &at, &up);
	D3DXMatrixPerspectiveFovLH(&mProjection, 3.14159f*.25f, mWidth / mHeight, .1f, 100.0f);

	mResized = true;

	mX = mY;
	mWidth = 100.0f;
	mHeight = 500.0f;

	mTarget = snew RenderTarget((int)mWidth, (int)mHeight, D3DCOLOR_XRGB(0, 0, 0), false);

	mTargetSprite = NULL;

	createBuffers();
}
LargeAlbumWidget::~LargeAlbumWidget()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		delete mSprites[i];
	}
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		delete mLargeAlbums[i];
	}
	ReleaseCOM(mVertexBuffer);
	ReleaseCOM(mIndexBuffer);
	delete mTarget;
	lock.Unlock();
}

void LargeAlbumWidget::onDeviceLost()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceLost();
	}
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->onDeviceLost();
	}
	HR(mEffect->OnLostDevice());
	mTarget->onDeviceLost();
	lock.Unlock();
}
void LargeAlbumWidget::onDeviceReset()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mSprites.size(); i++)
	{
		mSprites[i]->onDeviceReset();
	}
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->onDeviceReset();
	}
	HR(mEffect->OnResetDevice());
	mTarget->onDeviceReset();
	recreateTargets();
	lock.Unlock();
}

void LargeAlbumWidget::recreateTargets()
{
	if (mTargetSprite == NULL)
	{
		mTargetSprite = snew Sprite(mTarget->getTexture(), mX, mY, mWidth, mHeight);
		mSprites.push_back(mTargetSprite);
	}
	else
	{
		mTarget->setDimensions((int)mWidth, (int)mHeight);
		mTargetSprite->setDest(mX, mY, mWidth, mHeight);
		mTargetSprite->replaceCurrentTexture(mTarget->getTexture(), false);
	}
}

void LargeAlbumWidget::update(float dt)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	for (unsigned int i = 0; i < mAlbumsToAdd.size(); i++)
	{
		doAddAlbum(mAlbumsToAdd[i]);
		delete mAlbumsToAdd[i];
	}
	mAlbumsToAdd.clear();
	for (unsigned int i = 0; i < mTracksToAdd.size(); i++)
	{
		doAddTrack(mTracksToAdd[i]);
		delete mTracksToAdd[i];
	}
	mTracksToAdd.clear();

	lock.Unlock();
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->update(dt);
	}
	if (gInput->keyPressed(VK_LEFT))
	{
		int mid = mPositions.size() / 2;
		mAlbumIndex--;
		for (unsigned int i = 0; i < mPositions.size(); i++)
		{
			int index = mAlbumIndex - mid + i;
			if (index >= 0 && index < (int)mLargeAlbums.size())
			{
				mLargeAlbums[index]->setDest(mPositions[i].xPos, mPositions[i].yRotation, mPositions[i].visible, mTimeInterval);
			}
		}
	}
	else if (gInput->keyPressed(VK_RIGHT))
	{
		int mid = mPositions.size() / 2;
		mAlbumIndex++;
		for (unsigned int i = 0; i < mPositions.size(); i++)
		{
			int index = mAlbumIndex - mid + i;
			if (index >= 0 && index < (int)mLargeAlbums.size())
			{
				mLargeAlbums[index]->setDest(mPositions[i].xPos, mPositions[i].yRotation, mPositions[i].visible, mTimeInterval);
			}
		}
	}
}
void LargeAlbumWidget::createBuffers()
{
		// Obtain a pointer to a new vertex buffer.
	HR(gDevice->CreateVertexBuffer(24 * sizeof(Vertex::VertexPosTexNormal), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED, &mVertexBuffer, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// cube's vertex data.

	Vertex::VertexPosTexNormal* v = 0;
	HR(mVertexBuffer->Lock(0, 0, (void**)&v, 0));

	float half_width = ALBUM_WIDTH * .5f;
	float half_height = ALBUM_HEIGHT * .5f;
	float half_depth = ALBUM_DEPTH * .5f;
	// front
	v[0] = Vertex::VertexPosTexNormal(-half_width, -half_height, -half_depth, 0.0f, 0.0f, 0, 0, -1.0f); // bottom-left
	v[1] = Vertex::VertexPosTexNormal(-half_width, half_height, -half_depth, 0.0f, 1.0f, 0, 0, -1.0f); // top-left
	v[2] = Vertex::VertexPosTexNormal(half_width, half_height, -half_depth, 1.0f, 1.0f, 0, 0, -1.0f);  //top-right
	v[3] = Vertex::VertexPosTexNormal(half_width, -half_height, -half_depth, 1.0f, 0.0f, 0, 0, -1.0f); // bottom right

	// left
	v[4] = Vertex::VertexPosTexNormal(-half_width, -half_height, half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f); // bottom-left
	v[5] = Vertex::VertexPosTexNormal(-half_width, half_height, half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f); // top-left
	v[6] = Vertex::VertexPosTexNormal(-half_width, half_height, -half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f);  //top-right
	v[7] = Vertex::VertexPosTexNormal(-half_width, -half_height, -half_depth, 0.0f, 0.0f, -1.0f, 0, 0.0f); // bottom right

	// back
	v[8] = Vertex::VertexPosTexNormal(-half_width, half_height, half_depth, 0.0f, 1.0f, 0, 0, 1.0f); // top-left
	v[9] = Vertex::VertexPosTexNormal(-half_width, -half_height, half_depth, 0.0f, 0.0f, 0, 0, 1.0f); // bottom-left
	v[10] = Vertex::VertexPosTexNormal(half_width, -half_height, half_depth, 1.0f, 0.0f, 0, 0, 1.0f); // bottom right
	v[11] = Vertex::VertexPosTexNormal(half_width, half_height, half_depth, 1.0f, 1.0f, 0, 0, 1.0f);  //top-right

	// right
	v[12] = Vertex::VertexPosTexNormal(half_width, half_height, half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f); // top-left
	v[13] = Vertex::VertexPosTexNormal(half_width, -half_height, half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f); // bottom-left
	v[14] = Vertex::VertexPosTexNormal(half_width, -half_height, -half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f); // bottom right
	v[15] = Vertex::VertexPosTexNormal(half_width, half_height, -half_depth, 0.0f, 0.0f, 1.0f, 0, 0.0f);  //top-right

	// top
	v[16] = Vertex::VertexPosTexNormal(-half_width, half_height, -half_depth, 0.0f, 0.0f, 0, 0, 1.0f); // bottom-left
	v[17] = Vertex::VertexPosTexNormal(-half_width, half_height, half_depth, 0.0f, 1.0f, 0, 0, 1.0f); // top-left
	v[18] = Vertex::VertexPosTexNormal(half_width, half_height, half_depth, 1.0f, 1.0f, 0, 0, 1.0f);  //top-right
	v[19] = Vertex::VertexPosTexNormal(half_width, half_height, -half_depth, 1.0f, 0.0f, 0, 0, 1.0f); // bottom right

	// bottom
	v[20] = Vertex::VertexPosTexNormal(-half_width, -half_height, half_depth, 0.0f, 1.0f, 0, 0, 1.0f); // top-left
	v[21] = Vertex::VertexPosTexNormal(-half_width, -half_height, -half_depth, 0.0f, 0.0f, 0, 0, 1.0f); // bottom-left
	v[22] = Vertex::VertexPosTexNormal(half_width, -half_height, -half_depth, 1.0f, 0.0f, 0, 0, 1.0f); // bottom right
	v[23] = Vertex::VertexPosTexNormal(half_width, -half_height, half_depth, 1.0f, 1.0f, 0, 0, 1.0f);  //top-right

	HR(mVertexBuffer->Unlock());

	// Obtain a pointer to a new index buffer.
	HR(gDevice->CreateIndexBuffer(36 * sizeof(WORD), D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16, D3DPOOL_MANAGED, &mIndexBuffer, 0));

	// Now lock it to obtain a pointer to its internal data, and write the
	// cube's index data.

	WORD* k = 0;

	HR(mIndexBuffer->Lock(0, 0, (void**)&k, 0));

	// Front face.
	k[0] = 0; k[1] = 1; k[2] = 2;
	k[3] = 0; k[4] = 2; k[5] = 3;

	// Left
	k[6] = 4; k[7] = 5; k[8] = 6;
	k[9] = 4; k[10] = 6; k[11] = 7;

	// Back face.
	k[12] = 8; k[13] = 9; k[14] = 10;
	k[15] = 8; k[16] = 10; k[17] = 11;

	// Right
	k[18] = 12; k[19] = 13; k[20] = 14;
	k[21] = 12; k[22] = 14; k[23] = 15;

	// top
	k[24] = 16; k[25] = 17; k[26] = 18;
	k[27] = 16; k[28] = 18; k[29] = 19;

	// top
	k[30] = 20; k[31] = 21; k[32] = 22;
	k[33] = 20; k[34] = 22; k[35] = 23;

	HR(mIndexBuffer->Unlock());
}

void LargeAlbumWidget::preRender()
{
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		mLargeAlbums[i]->preRender();
	}
	mTarget->beginScene();
	HR(mEffect->SetTechnique(mTechnique));

	mNumTriangles = 0;

	UINT numPasses = 0;
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		if (mLargeAlbums[i]->getVisible())
		{
			HR(mEffect->Begin(&numPasses, 0));
			for (UINT j = 0; j < numPasses; j++)
			{
				HR(mEffect->BeginPass(j));

				HR(mEffect->SetTexture(mTextureHandle, mLargeAlbums[i]->getTexture()));
				HR(mEffect->SetMatrix(mViewHandle, &mView));
				HR(mEffect->SetMatrix(mWorldHandle, &mLargeAlbums[i]->getWorld()));
				HR(mEffect->SetMatrix(mProjectionHandle, &mProjection));
				HR(mEffect->SetVector(mLightColorHandle, &mLightColor));
				HR(mEffect->SetVector(mDiffuseHandle, &mDiffuseColor));
				HR(mEffect->SetVector(mAmbientHandle, &mAmbient));
				HR(mEffect->SetValue(mCamPosHandle, &mCamPos, sizeof(D3DXVECTOR3)));
				HR(mEffect->SetFloat(mSpecularPowerHandle, mSpecularPower));
				HR(mEffect->SetValue(mSpecularColorHandle, &mSpecularColor, sizeof(D3DXVECTOR3)));
				HR(mEffect->SetValue(mLightPosHandle, &mLightPos, sizeof(D3DXVECTOR3)));
				HR(mEffect->SetVector(mFogColorHandle, &mFogColor));
				HR(mEffect->SetBool(mFogEnabledHandle, mFogEnabled));

				HR(mEffect->CommitChanges());

				HR(gDevice->SetStreamSource(0, mVertexBuffer, 0, sizeof(Vertex::VertexPosTexNormal)));
				HR(gDevice->SetIndices(mIndexBuffer));
				HR(gDevice->SetVertexDeclaration(Vertex::VertexPosTexNormal::Decl));

				HR(gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 36, 0, 14));
					
				mNumTriangles += 1;

				HR(mEffect->EndPass());
			}
			HR(mEffect->End());
		}
	}
	mTarget->endScene();
}

void LargeAlbumWidget::setPos(float x, float y, float width, float height)
{
	mX = x;
	mY = y;
	if (width != -999.0f)
	{
		mWidth = width;
	}

	if (height != -999.0f)
	{
		mHeight = height;
	}

	D3DXVECTOR3 at(0,0,0);
	D3DXVECTOR3 up(0,1,0);
	D3DXMatrixLookAtLH(&mView, &mCamPos, &at, &up);
	D3DXMatrixPerspectiveFovLH(&mProjection, 3.14159f*.25f, mWidth / mHeight, .1f, 100.0f);
	recreateTargets();
}

std::vector<Sprite*> LargeAlbumWidget::getSprites()
{
	return mSprites;
}
int LargeAlbumWidget::getNumTriangles()
{
	return 2;
}
bool LargeAlbumWidget::onMouseEvent(MouseEvent e)
{
	return false;
}
void LargeAlbumWidget::refresh()
{
}
void LargeAlbumWidget::goToChar(char c)
{
	int index = -1;
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		if (mLargeAlbums[i]->getAlbum().Artist[0] == c)
		{
			index = (int)i;
			break;
		}
	}

	if (index >= 0 && index != mAlbumIndex)
	{
		int currAlbum = mAlbumIndex;
		mAlbumIndex = index;

		int mid = (int)mPositions.size() / 2;
		if (index > currAlbum)
		{
			for (int i = currAlbum - mid + 1; i < index - mid + 1 && i < (int)mLargeAlbums.size(); i++)
			{
				for (int j = i; j < index + mid && j < (int)mLargeAlbums.size(); j++)
				{
					int posIndex = max(0, min((int)mPositions.size() - 1, j - i));
					if (j >= 0)
					{
						mLargeAlbums[j]->addDest(mPositions[posIndex].xPos,
							mPositions[posIndex].yRotation,
							mPositions[posIndex].visible,
							mPositions[posIndex].time * .03f);
					}
				}
			}
		}
		else
		{
			for (int i = currAlbum + mid; i > index + mid && i >= 0; i--)
			{
				for (int j = i; j > index - mid && j >= 0; j--)
				{
					int posIndex = max(0, min((int)mPositions.size() - 1, (int)mPositions.size() - i + j));
					if (j < mLargeAlbums.size())
					{
						mLargeAlbums[j]->addDest(mPositions[posIndex].xPos,
							mPositions[posIndex].yRotation,
							mPositions[posIndex].visible,
							mPositions[posIndex].time * .03f);
					}
				}
			}
		}
	}
}

bool LargeAlbumWidget::isPointInside(int x, int y)
{
	return false;
}
void LargeAlbumWidget::addAlbum(Album *album)
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	mAlbumsToAdd.push_back(snew Album(album->Id, album->NumTracks, album->Title, album->Year, album->Artist));
	lock.Unlock();
}
void LargeAlbumWidget::doAddAlbum(Album *album)
{
	int insertIndex = 0;
	vector<LargeAlbumItem*>::iterator iter = mLargeAlbums.begin();
	for (unsigned int i = 0; i < mLargeAlbums.size(); i++)
	{
		if (mLargeAlbums[i]->getAlbum().Id == album->Id)
		{
			insertIndex = -1;
			break;
		}
		else if (mLargeAlbums[i]->getAlbum().Artist == album->Artist)
		{
			insertIndex = i;
			for (unsigned int j = i; j < mLargeAlbums.size() && mLargeAlbums[j]->getAlbum().Artist == album->Artist; j++)
			{
				if (mLargeAlbums[j]->getAlbum().Title == album->Title)
				{
					insertIndex = -1;
					break;
				}
				else if (mLargeAlbums[j]->getAlbum().Title > album->Title)
				{
					insertIndex = j;
					break;
				}
				iter++;
			}
			break;
		}
		else if (mLargeAlbums[i]->getAlbum().Artist >= album->Artist)
		{
			insertIndex = i;
			break;
		}
		iter++;
	}

	if (insertIndex >= 0)
	{
		int diff = (insertIndex - mAlbumIndex);
		if (diff == 0)
		{
			iter++;
			insertIndex++;
		}
		mLargeAlbums.insert(iter, snew LargeAlbumItem(*album));

		unsigned int midPos = mPositions.size() / 2;
		for (unsigned int i = 0; i < mPositions.size(); i++)
		{
			int index = i - midPos + mAlbumIndex;
			if (index >= 0 && index < (int)mLargeAlbums.size())
			{
				mLargeAlbums[index]->setPosition(mPositions[i].xPos, mPositions[i].yRotation);
				mLargeAlbums[index]->setVisible(true);
			}
		}
	}
}

void LargeAlbumWidget::addTrack(Track* track)
{
/*	CSingleLock lock(&mCritSection);
	lock.Lock();

	if (track->AlbumId > 0)
	{
		bool foundAlbum = false;
		for (unsigned int i = 0; i < mSmallItems.size(); i++)
		{
			if (mSmallItems[i]->getAlbum().Id == track->AlbumId)
			{
				foundAlbum = true;
			}
		}
		if (!foundAlbum)
		{
			Album album;
			DatabaseManager::instance()->getAlbum(track->AlbumId, &album);
			if (album.Id > 0)
			{
				mAlbumsToAdd.push_back(snew Album(album.Id, album.NumTracks, album.Title, album.Year, album.Artist));
			}
		}
		mTracksToAdd.push_back(snew Track(*track));
	}
	lock.Unlock();
*/
}
void LargeAlbumWidget::doAddTrack(Track* track)
{
/*	int insertIndex = 0;
	for (unsigned int i = 0; i < mSmallItems.size(); i++)
	{
		if (mSmallItems[i]->getAlbum().Id == track->AlbumId)
		{
			insertIndex = i;
			break;
		}
	}

	if (insertIndex >= 0)
	{
		mSmallItems[insertIndex]->addTrack(track);
		mSmallItems[insertIndex]->onDeviceReset();
	}
*/
}
