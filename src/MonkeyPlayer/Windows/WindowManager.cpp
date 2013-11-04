// WindowManager.cpp
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// handles several windows

#include "d3dApp.h"
#include "MusicLibrary.h"
#include "Settings.h"
#include "WindowManager.h"

#include "../Winforms/SearchForm.h"

#include <vector>

using namespace MonkeyPlayer;

WindowManager::WindowManager()
{
	std::string fxPath = Settings::instance()->getStringValue(Settings::CONTENT_DIR, "") + "\\Effects\\SimpleSprite.fx";

	ID3DXBuffer* errors = 0;
	HR(D3DXCreateEffectPool(&mPool));
	(D3DXCreateEffectFromFile(gDevice, fxPath.c_str(), 
		0, 0, D3DXSHADER_DEBUG, 0, &mEffect, &errors));

	if (errors)
	{
		::MessageBox(0, (char*)errors->GetBufferPointer(), 0, 0);
		PostQuitMessage(1);
	}

	mTechnique = mEffect->GetTechniqueByName("SpriteTechnique");
	mScreenWidth = mEffect->GetParameterByName(0, "SCREEN_WIDTH");
	mScreenHeight = mEffect->GetParameterByName(0, "SCREEN_HEIGHT");
	mRectHandle = mEffect->GetParameterByName(0, "spriteRect");
	mTexture = mEffect->GetParameterByName(0, "tex0");
	mSpriteColor = mEffect->GetParameterByName(0, "spriteColor");

	mProgressBar = snew ProgressBar(0, 0, 100.0f, 100.0f);
	mProgressBar->setVisible(false);

	mContextMenu = snew ItemListBox(-999.0f, -999.0f, 150.0f, 50.0, contextMenu_callback, this, D3DCOLOR_XRGB(116, 116, 64));
	mContextMenu->setAllowSingleClickSelection(true);
	mContextMenuOwner = NULL;

	mNumTriangles = 0;

	gInput->addMouseCallback(this, &mouseEventCallback);
	mFocusWindow = NULL;
	mCollectionWin = NULL;

	mResized = true;

	mSearchThread = NULL;
}
WindowManager::~WindowManager()
{
	for (unsigned int i = 0; i < mWindows.size(); i++)
	{
		delete mWindows[i];
	}
	delete mProgressBar;
	delete mContextMenu;
	if (mSearchThread != NULL)
	{
		WaitForSingleObject(mThread, 2000);
		delete mSearchThread;
		delete mThread;
		mSearchThread = NULL;
	}

	ReleaseCOM(mEffect);
}
void WindowManager::onDeviceLost()
{
	for (unsigned int i = 0; i < mWindows.size(); i++)
	{
		mWindows[i]->onDeviceLost();
	}
	mProgressBar->onDeviceLost();
	mContextMenu->onDeviceLost();
	HR(mEffect->OnLostDevice());
}
void WindowManager::onDeviceReset()
{
	for (unsigned int i = 0; i < mWindows.size(); i++)
	{
		mWindows[i]->onDeviceReset();
	}
	mProgressBar->onDeviceReset();
	mContextMenu->onDeviceReset();
	HR(mEffect->OnResetDevice());

	mResized = true;
}


void WindowManager::addWindow(IWindow *win)
{
	if (mFocusWindow == NULL)
	{
		requestFocusedWindow(win);
	}
	mWindows.push_back(win);
}

void WindowManager::update(float dt)
{
	if (gInput->keyPressed(0x46 /* F */))
	{
		if (gInput->isKeyDown(VK_CONTROL) || gInput->keyReleased(VK_CONTROL))
		{
			openSearch();
		}
	}
	
	for (unsigned int i = 0; i < mWindows.size(); i++)
	{
		mWindows[i]->update(dt);
	}
	
	if (mSearchThread != NULL && mSearchThread->mFinished)
	{
		if (MonkeyPlayer::SearchForm::instance()->doPlay())
		{
			Track t;
			DatabaseManager::instance()->getTrack(MonkeyPlayer::SearchForm::instance()->getSelectedTrackID(), &t);
			if (t.Id >= 0)
			{
				MusicLibrary::instance()->playSong(t.Filename);
			}
		}
		
		if (MonkeyPlayer::SearchForm::instance()->doGoTo() && mCollectionWin != NULL)
		{
			Track t;
			DatabaseManager::instance()->getTrack(MonkeyPlayer::SearchForm::instance()->getSelectedTrackID(), &t);
			if (t.Id >= 0)
			{
				mCollectionWin->goToSong(t.Filename);
			}
		}
		WaitForSingleObject(mThread, 2000);
		delete mSearchThread;
		delete mThread;
		mSearchThread = NULL;
		if (mFocusWindow != NULL)
		{
			mFocusWindow->onFocus();
		}
	}
	mContextMenu->update(dt);
	if (mResized)
	{
		RECT r;
		GetClientRect(gApp->getMainWnd(), &r);

		mProgressBar->setPos(0, (float)(r.bottom - 20), (float)getMainContentWidth(), 20.0f);
		mResized = false;
	}
	mProgressBar->update(dt);
}

void WindowManager::preRender()
{
	// draw bottom to top
	for (int i = (int)mWindows.size() - 1; i >= 0; i--)
	{
		mWindows[i]->preRender();
	}
	mContextMenu->preRender();
}
void WindowManager::display()
{
	HR(mEffect->SetTechnique(mTechnique));

	mNumTriangles = 0;

	UINT numPasses = 0;
	HR(mEffect->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; i++)
	{
		HR(mEffect->BeginPass(i));

		HR(mEffect->SetFloat(mScreenWidth, (float)gApp->getWidth()));
		HR(mEffect->SetFloat(mScreenHeight, (float)gApp->getHeight()));
		// backwards because "bottom" windows' sizes 
		// might depend on others
		for (int j = (int)mWindows.size() - 1; j >= 0; j--)
		{
			drawSprites(mWindows[j]->getSprites());
			drawWidgets(mWindows[j]->getWidgets());
			mNumTriangles += mWindows[j]->getNumTriangles();
		}
		drawSprites(mProgressBar->getSprites());
		drawSprites(mContextMenu->getSprites());
		mNumTriangles += mProgressBar->getNumTriangles();
		mNumTriangles += mContextMenu->getNumTriangles();

		HR(mEffect->EndPass());
	}
	HR(mEffect->End());
}

void WindowManager::drawSprites(std::vector<Sprite*> sprites)
{
	for (unsigned int k = 0; k < sprites.size(); k++)
	{
		HR(mEffect->SetVector(mRectHandle, &D3DXVECTOR4(sprites[k]->getX(),
			sprites[k]->getY(),
			sprites[k]->getWidth(),
			sprites[k]->getHeight())));

		HR(mEffect->SetTexture(mTexture, sprites[k]->getTexture()));
		HR(mEffect->SetVector(mSpriteColor, &sprites[k]->getColor()));

		HR(mEffect->CommitChanges());

		HR(gDevice->SetStreamSource(0, sprites[k]->getVertexBuffer(), 0, sprites[k]->getVertexStride()));
		HR(gDevice->SetIndices(sprites[k]->getIndexBuffer()));
		HR(gDevice->SetVertexDeclaration(sprites[k]->getVertexDeclaration()));

		HR(gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, sprites[k]->getNumVertices(), 0, 
			sprites[k]->getNumTriangles()));
	}
}
// should be called between Begin() and End()
void WindowManager::drawSprite(Sprite* sprite, float width, float height)
{
	HR(mEffect->SetFloat(mScreenWidth, width));
	HR(mEffect->SetFloat(mScreenHeight, height));
	
	HR(mEffect->SetVector(mRectHandle, &D3DXVECTOR4(sprite->getX(),
	sprite->getY(),
	sprite->getWidth(),
	sprite->getHeight())));

	HR(mEffect->SetTexture(mTexture, sprite->getTexture()));
	HR(mEffect->SetVector(mSpriteColor, &sprite->getColor()));

	HR(mEffect->SetTechnique(mTechnique));
	HR(mEffect->CommitChanges());	

	UINT numPasses = 0;
	HR(mEffect->Begin(&numPasses, 0));
	for (UINT i = 0; i < numPasses; i++)
	{
		HR(mEffect->BeginPass(i));

		HR(gDevice->SetStreamSource(0, sprite->getVertexBuffer(), 0, sprite->getVertexStride()));
		HR(gDevice->SetIndices(sprite->getIndexBuffer()));
		HR(gDevice->SetVertexDeclaration(sprite->getVertexDeclaration()));

		HR(gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, sprite->getNumVertices(), 0, 
			sprite->getNumTriangles()));
		HR(mEffect->EndPass());
	}
	HR(mEffect->End());
}
void WindowManager::drawWidgets(std::vector<IWidget*> widgets)
{
	for (unsigned int k = 0; k < widgets.size(); k++)
	{
		drawSprites(widgets[k]->getSprites());
	}
	for (unsigned int k = 0; k < widgets.size(); k++)
	{
		drawWidgets(widgets[k]->getWidgets());
	}
}
int WindowManager::getNumTriangles()
{
	return mNumTriangles;
}
bool WindowManager::requestFocusedWindow(IWindow* win)
{
	if (mFocusWindow != NULL)
	{
		mFocusWindow->onBlur();
	}
	mFocusWindow = win;
	if (mFocusWindow != NULL)
	{
		mFocusWindow->onFocus();
	}
	return true;
}
IWindow* WindowManager::getFocusWindow()
{
	return mFocusWindow;
}

void WindowManager::mouseEventCallback(void* obj, MouseEvent e)
{
	((WindowManager*)obj)->onMouseEvent(e);
}
void WindowManager::onMouseEvent(MouseEvent e)
{
	mContextMenu->onMouseEvent(e);
	if (((!mContextMenu->isPointInside(e.getX(), e.getY()) && 
		(e.getEvent() == MouseEvent::LBUTTONDOWN || e.getEvent() == MouseEvent::RBUTTONDOWN))) &&
		e.getEvent() != MouseEvent::MOUSEWHEEL)
	{
		mContextMenu->setPos(-999.0f, -999.0f, 50.0f, 50.0f);
		mContextMenu->blur();
		mContextMenuOwner = NULL;
		if (mFocusWindow != NULL)
		{
			mFocusWindow->onFocus();
		}
	}
	if (mContextMenu->onMouseEvent(e))
	{
		e.setConsumed(true);
	}
	if (mWindows.size() > 0)
	{
		for (unsigned int i = 0; i < mWindows.size(); i++)
		{
			if (mWindows[i]->onMouseEvent(e))
			{
				if (mFocusWindow != mWindows[i])
				{
					requestFocusedWindow(mWindows[i]);
				}
				e.setConsumed(true);
			}
		}
	}
}
int WindowManager::getMainContentWidth()
{
	int width = gApp->getWidth();
	for (unsigned int i = 0; i < mWindowsBesideMain.size(); i++)
	{
		width -= mWindowsBesideMain[i]->getWidth();
	}
	return width;
}
int WindowManager::getMainContentTop()
{
	int height = 0;
	for (unsigned int i = 0; i < mWindowsAboveMain.size(); i++)
	{
		height += mWindowsAboveMain[i]->getHeight();
	}
	return height;
}
int WindowManager::getMainContentBottom()
{
	int height = gApp->getHeight();
	for (unsigned int i = 0; i < mWindowsBelowMain.size(); i++)
	{
		height -= mWindowsBelowMain[i]->getHeight();
	}
	return height;
}
void WindowManager::addWindowBesideMain(IWindow* win)
{
	mWindowsBesideMain.push_back(win);
}
void WindowManager::addWindowAboveMain(IWindow* win)
{
	mWindowsAboveMain.push_back(win);
}
void WindowManager::addWindowBelowMain(IWindow* win)
{
	mWindowsBelowMain.push_back(win);
}

void WindowManager::openContextMenu(float mouseX, float mouseY, vector<ListItem*> items, IDrawable* owner)
{
	mContextMenu->setItems(items);
	mContextMenu->setSelectedIndex(-1);
	float width = mContextMenu->getWidthToFit();
	float height = mContextMenu->getHeightToFit();

	float x = mouseX;
	if ((gApp->getWidth() - mouseX) < width)
	{
		x = mouseX - width;
	}
	
	float y = mouseY;
	float maxHeight = gApp->getHeight() - mouseY;
	if (maxHeight < height)
	{
		y = mouseY - (height - maxHeight);
	}
	mContextMenu->setPos(x, y, width, height);
	for (unsigned int i = 0; i < mWindows.size(); i++)
	{
		mWindows[i]->onBlur();
	}
	mContextMenu->focus();
	mContextMenuOwner = owner;
}
/*static*/ void WindowManager::contextMenu_callback(void* obj, ItemListBox* listBox)
{
	WindowManager* win = static_cast<WindowManager*>(obj);
	if (win)
	{
		if (win->mContextMenuOwner != NULL)
		{
			win->mContextMenuOwner->onContextMenuSelected(listBox);
		}
		win->mContextMenu->setPos(-999.0f, -999.0f, 50.0f, 50.0f);
		win->mContextMenuOwner = NULL;
		win->mContextMenu->blur();
		if (win->mFocusWindow != NULL)
		{
			win->mFocusWindow->onFocus();
		}
	}
}

void WindowManager::openSearch()
{
	if (mFocusWindow != NULL)
	{
		mFocusWindow->onBlur();
	}
	if (mSearchThread == NULL)
	{
		mSearchThread = snew SearchFormThread();
		mThread = AfxBeginThread(&searchThread, mSearchThread, 0, 0, CREATE_SUSPENDED);
		mThread->m_bAutoDelete = FALSE;
		mThread->ResumeThread();
	}
	else
	{
		mSearchThread->resetSearchForm();
	}
}
/*static*/ UINT WindowManager::searchThread(LPVOID pParam)
{
	SearchFormThread* search = (SearchFormThread*)pParam;

	bool reOpen = false;

	do
	{
		search->mRestart = false;
		MonkeyPlayer::SearchForm::openSearch(); 
		
		CSingleLock lock(&search->mCritSection);
		reOpen = search->mRestart;
		lock.Lock();
		
		lock.Unlock();
	} while (reOpen);

	search->mFinished = true;
	return 0;
}

void SearchFormThread::closeSearchForm()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	MonkeyPlayer::SearchForm::instance()->threadClose(); 
	MonkeyPlayer::SearchForm::instance()->reset();
	mRestart = false;
	lock.Unlock();
}
void SearchFormThread::resetSearchForm()
{
	CSingleLock lock(&mCritSection);
	lock.Lock();
	
	MonkeyPlayer::SearchForm::instance()->threadClose(); 
	MonkeyPlayer::SearchForm::instance()->reset(); 
	mRestart = true;
	lock.Unlock();
}
