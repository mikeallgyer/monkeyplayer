// WindowManager.h
//
// (C) 2013 Mike Allgyer.  All Rights Reserved.
//
// handles several windows


#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "CollectionWindow.h"
#include "d3dUtil.h"
#include "IDrawable.h"
#include "ItemListBox.h"
#include "IWindow.h"
#include "ProgressBar.h"
#include <MonkeyInput.h>

#include <vector>

using namespace std;

namespace MonkeyPlayer
{
	class SearchFormThread
	{
	public:
		SearchFormThread()
		{
			mFinished = false;
			mRestart = false;
		}

		void closeSearchForm();
		void resetSearchForm();
		bool mFinished;
		bool mRestart;

		// synchronization
		CCriticalSection mCritSection;
	};

	class WindowManager : public IDrawable
	{
	public:
		WindowManager();
		~WindowManager();

		void onDeviceLost();
		void onDeviceReset();

		void addWindow(IWindow *win);

		void update(float dt);

		void preRender();
		void display();

		void drawSprite(Sprite* sprite, float width, float height);
		int getNumTriangles();

		static void mouseEventCallback(void* obj, MouseEvent e);
		void onMouseEvent(MouseEvent e);
		bool requestFocusedWindow(IWindow* win);
		IWindow* getFocusWindow();
		int getMainContentWidth();
		int getMainContentTop();
		int getMainContentBottom();
		void addWindowBesideMain(IWindow* win);
		void addWindowAboveMain(IWindow* win);
		void addWindowBelowMain(IWindow* win);
		ProgressBar* getProgressBar() { return mProgressBar; }
		CollectionWindow* getCollectionWindow() { return mCollectionWin; }
		void setCollectionWindow(CollectionWindow*  win) { mCollectionWin = win; }

		void openContextMenu(float mouseX, float mouseY, vector<ListItem*> items, IDrawable* owner);
		static void contextMenu_callback(void* obj, ItemListBox* listBox);

		void openSearch();
		static UINT searchThread(LPVOID pParam);

	private:
		void drawSprites(std::vector<Sprite*> sprites);
		void drawWidgets(std::vector<IWidget*> widgets);

		vector<IWindow*> mWindows;
		vector<IWindow*> mWindowsBesideMain;
		vector<IWindow*> mWindowsAboveMain;
		vector<IWindow*> mWindowsBelowMain;

		ID3DXEffectPool* mPool;
		ID3DXEffect* mEffect;
		D3DXHANDLE mTechnique;
		D3DXHANDLE mScreenWidth;
		D3DXHANDLE mScreenHeight;
		D3DXHANDLE mRectHandle;
		D3DXHANDLE mTexture;
		D3DXHANDLE mSpriteColor;

		int mNumTriangles;

		IWindow* mFocusWindow;
		std::vector<void (*)(void* ptrObj, MouseEvent e) > mMouseCallbacks;
		std::vector<void*> mMouseCallbackObj; 

		ProgressBar* mProgressBar;
		CollectionWindow* mCollectionWin;

		ItemListBox* mContextMenu;
		IDrawable* mContextMenuOwner;
		bool mResized;

		SearchFormThread* mSearchThread;
		CWinThread* mThread;
	};
}
#endif
