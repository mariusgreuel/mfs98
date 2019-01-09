/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 mfs.cpp

 Version:   1.00

 Date:	  13-Jan-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "mgjoystick.h"
#include "mgstring.h"
#include "mgtimer.h"
#include "..\rccom\rccom.h"
#include "types.h"
#include "display.h"
#include "toolbox.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

MGMODULE(__FILE__);

static BOOL		 bStopProcessing;		// DefWindowProc should not be processed
static BOOL		 bSysMoveImpending;	  // window will be minimized or restored
static BOOL		 bSizeMove;			  // inside the size-move loop


/////////////////////////////////////////////////////////////////////////////
// Globals...

LPCSTR				g_szAppName = "MFS";
LPCSTR				g_szAppClass = "MFS_Class";
LPCSTR				g_szAppTitle = "MFS Modellflugsimulator";
LPCSTR				g_szHelpFile = "mfs.hlp";
LPCSTR				g_szLogFile = "mfslog.txt";
LPCSTR				g_szScriptFile = "default.ifs";

CMGDebug				  g_Debug;
CMGTimer			g_Timer;
CMGJoystick			g_Joystick;
CRCBox				g_RCBox;
CDisplay			g_Display;

HINSTANCE			g_hInst;						// handle of app instance
HACCEL				g_hAccel;					   // handle of app accelerator
HWND				g_hWnd;						 // handle of app window

BOOL				g_bInitialized = FALSE;

BOOL				g_bAppHalted = FALSE;		   // app is halted (timer killed)
BOOL				g_bAppPaused = FALSE;		   // app is paused by user
BOOL				g_bAppActive = FALSE;		   // main window is active
BOOL				g_bAppMinimized = FALSE;		// main window is minimized
BOOL				g_bAppInMenuLoop = FALSE;	   // in menu loop

BOOL				g_bExitWithError = FALSE;	   // fatal error has occurred

DWORD				g_bUseGDI = FALSE;			  // GDI drawing is recommend
DWORD				g_bShowToolbar = TRUE;		  // display the toolbar
DWORD				g_bShowStatusbar = TRUE;		// display the statusbar

CMGString			g_strSceneryName;
CMGString			g_strSceneryModel;
CMGString			g_strSceneryWorld;

BOOL				g_bSceneryModified = FALSE;

WNDPLACEMENT		g_WndInfo;

float				fCalculationPeriod = 0.05;	  // in seconds

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HWND g_hWndToolbar, g_hWndStatusbar;

BOOL CreateToolbar(HWND hwndParent, BOOL bVisible);
LRESULT MsgNotifyToolbar(HWND hwnd, WPARAM wparam, LPARAM lparam);

BOOL CreateStatusbar(HWND hwndParent, BOOL bVisible);
LRESULT MsgMenuSelectStatusbar(HWND hwnd, UINT nID, UINT nFlags, HMENU hMenu);

BOOL LoadRegistrySettings();
BOOL SaveRegistrySettings();

BOOL DisplaySplashWindow();
BOOL DestroySplashWindow();

BOOL OnMenuCommand(UINT id);
BOOL OnKeyCommand(UINT id);

extern BOOL LoadScenery(LPCSTR szScriptFile);
extern BOOL SaveScenery(LPCSTR  szScriptFile);

extern void CalcFrameRate();

extern void InitScene();
extern void CalcScene();
extern void MoveScene();
extern BOOL RenderScene();

extern BOOL DisplayInfo();

/////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerProc(UINT, UINT, DWORD, DWORD, DWORD)
{
	CalcScene();
}

void SetHaltState()
{
	static  bTimerActive = FALSE;

	if (!g_bInitialized)
		return;

	g_bAppHalted = !g_bAppActive || g_bAppPaused || g_bAppInMenuLoop || g_bAppMinimized;

	if (!g_bAppHalted && !bTimerActive) {
		if (!g_Timer.Start()) {
			MGDBG0(DMSG_ERROR, "Could not start mmtimer\n");
			return;
		}
		bTimerActive = TRUE;
	} else if (g_bAppHalted && bTimerActive) {
		if (!g_Timer.Kill()) {
			MGDBG0(DMSG_ERROR, "Could not stop mmtimer\n");
			return;
		}
		bTimerActive = FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////

void SetCaptionText(LPSTR fmt = NULL, ...)
{
	CMGString strText;

	if (g_hWnd == NULL)
		return;

	strText = g_szAppTitle;

	if (fmt == NULL) {
		if (g_strSceneryName != "") {
			strText += " - ";
			if (g_bSceneryModified)
				strText += '*';
			strText += g_strSceneryName;
		}
	} else {
		CMGString strBuffer;

		va_list arglist;
		va_start(arglist, fmt);
		strBuffer.vprintf(fmt, arglist);
		va_end(arglist);
		strText += TEXT(" - ");
		strText += strBuffer;
	}
	if (SetWindowText(g_hWnd, strText) == 0) {
		MGDBG0(DMSG_APIERROR, "SetWindowText failed\n");
	}
}

void DispSizeMoveText()
{
	if (!bSizeMove || g_Display.IsFullscreenMode())
		return;
	RECT rc;
	GetWindowRect(g_hWnd, &rc);
	rc.right -= rc.left; rc.bottom -= rc.top;
	SetCaptionText("(%d,%d) (%d,%d)", rc.left, rc.top, rc.right, rc.bottom);
}

/////////////////////////////////////////////////////////////////////////////

void CalcClientRect(LPRECT lprc)
{
	RECT rc;

	if (g_Display.IsFullscreenMode() && !g_bAppHalted) {
		GetWindowRect(g_hWnd, lprc);
	} else {
		GetClientRect(g_hWnd, lprc);
		if (g_bShowToolbar) {
			GetWindowRect(g_hWndToolbar, &rc);
			ScreenToClient(g_hWnd, (LPPOINT)&rc + 1);
			lprc->top = rc.bottom;
		}
		if (g_bShowStatusbar) {
			GetWindowRect(g_hWndStatusbar, &rc);
			ScreenToClient(g_hWnd, (LPPOINT)&rc);
			lprc->bottom = rc.top;
		}
	}
}

void SetClientRect()
{
	RECT rc;

	CalcClientRect(&rc);

//debug	g_Debug.SetWindowPos(&rc);
	g_Display.SetBufferPos(rc.left, rc.top);
	g_Display.SetBufferSize(rc.right - rc.left, rc.bottom - rc.top);
}

// set display mode (either windowed or fullscreen mode)
BOOL SetDisplayMode(DWORD dwWidth, DWORD dwHeight, WORD bpp, BOOL bFullscreen)
{
	if (bFullscreen) {
		if (!g_WndInfo.bFullscreen) {
			// if previous mode was windowed mode, save window position
			GetWindowRect(g_hWnd, &g_WndInfo.rcPlacement);
			g_WndInfo.rcPlacement.right -= g_WndInfo.rcPlacement.left;
			g_WndInfo.rcPlacement.bottom -= g_WndInfo.rcPlacement.top;
		}
		g_WndInfo.bFullscreen = TRUE;
		if (dwWidth == 0 || dwHeight == 0 || bpp == 0) {
			// use saved mode info if parms are zero
			dwWidth = g_WndInfo.szResolution.cx;
			dwHeight = g_WndInfo.szResolution.cy;
			bpp = g_WndInfo.wBpp;
		} else {
			// otherwise save mode info
			g_WndInfo.szResolution.cx = dwWidth;
			g_WndInfo.szResolution.cy = dwHeight;
			g_WndInfo.wBpp = bpp;
		}
		g_Display.SetMode(g_WndInfo.szResolution.cx, g_WndInfo.szResolution.cy, g_WndInfo.wBpp, TRUE);
	} else {
	  g_Display.SetMode(0,0,0, FALSE);
	}

	SetClientRect();

	return TRUE;
}

BOOL RenderFrame()
{
	if (!RenderScene() || !DisplayInfo()) {
		MGDBG0(DMSG_FATAL, "Render frame failed.\n");
		return FALSE;
	}
	g_Display.ShowBackBuffer();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Windows messages handlers

BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
	CreateToolbar(hwnd, g_bShowToolbar);
	CreateStatusbar(hwnd, g_bShowStatusbar);
//debug	g_Debug.CreateLogWindow(hwnd);

	return 0;
}

void OnDestroy(HWND hwnd)
{
	WinHelp(hwnd, g_szHelpFile, HELP_QUIT, 0);
	PostQuitMessage(g_bExitWithError ? 1 : 0);
	g_bInitialized = FALSE;
}

void OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
	g_bAppActive = state != WA_INACTIVE ? TRUE : FALSE;
	SetHaltState();
}

void OnSysCommand(HWND hwnd, UINT uCmdType, int x, int y)
{
	switch (uCmdType & 0xfff0) {
		case SC_MINIMIZE:
		case SC_RESTORE:
			bSysMoveImpending = TRUE;
			break;
		case SC_MAXIMIZE:
			// if the window was minimized, restore its position
			if (IsIconic(hwnd))
				ShowWindow(hwnd, SW_RESTORE);
			// switch to fullscreen mode
			PostMessage(hwnd, WM_COMMAND, IDM_VIEW_FULLSCREEN, 0L);
			bStopProcessing = TRUE;
	}
}

void OnPaint(HWND hwnd)
{
	if (g_bInitialized) {
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
//debug		if (!g_Debug.EventLogIsActive())
			RenderFrame();
		EndPaint(hwnd, &ps);
		bStopProcessing = TRUE;
	}
}

void OnNCPaint(HWND hwnd, HRGN)
{
}

BOOL OnEraseBkGnd(HWND hwnd, HDC hDC)
{
	if (g_bInitialized)
		bStopProcessing = TRUE;

	return TRUE;
}

BOOL OnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
	if (g_Display.IsFullscreenMode() && !g_bAppHalted) {
		// dont show the cursor in fullscreen mode
		SetCursor(NULL);
		bStopProcessing = TRUE;
		return TRUE;
	}
	return FALSE;
}

BOOL OnWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos)
{
	if (bSysMoveImpending) {
		// system moves are allowed (either SC_MINIMIZE or SC_RESTORE)
		bSysMoveImpending = FALSE;
	} else if (g_bInitialized && !g_Display.IsMoveAllowed()) {
		// ensure that the window won't move or resize in fullscreen mode
		lpwpos->flags |= SWP_NOMOVE | SWP_NOSIZE;
		return FALSE;
	}
	return TRUE;
}

void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	// ensure the window won't get too small or too big
	lpMinMaxInfo->ptMinTrackSize.x = 300;
	lpMinMaxInfo->ptMinTrackSize.y = 200;
	lpMinMaxInfo->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
	lpMinMaxInfo->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN);
}

void OnMove(HWND hwnd, int, int)
{
	RECT rc;

	if (g_Display.IsFullscreenMode())
		return;

	GetWindowRect(hwnd, &g_WndInfo.rcPlacement);
	g_WndInfo.rcPlacement.right -= g_WndInfo.rcPlacement.left;
	g_WndInfo.rcPlacement.bottom -= g_WndInfo.rcPlacement.top;

	if (!g_bInitialized)
		return;

	DispSizeMoveText();

	CalcClientRect(&rc);

//debug	g_Debug.SetWindowPos(&rc);
	g_Display.SetBufferPos(rc.left, rc.top);
}

void OnSize(HWND hwnd, UINT nType, int cx, int cy)
{
	if (nType == SIZE_MINIMIZED)
		return;

	SendMessage(g_hWndToolbar, WM_SIZE, nType, MAKELONG(cy, cx));
	SendMessage(g_hWndStatusbar, WM_SIZE, nType, MAKELONG(cy, cx));

	if (g_Display.IsFullscreenMode())
		return;

	GetWindowRect(hwnd, &g_WndInfo.rcPlacement);
	g_WndInfo.rcPlacement.right -= g_WndInfo.rcPlacement.left;
	g_WndInfo.rcPlacement.bottom -= g_WndInfo.rcPlacement.top;

	if (!g_bInitialized)
		return;

	DispSizeMoveText();

	SetClientRect();
}

void OnPaletteChanged(HWND hwnd, HWND hwndPaletteChange)
{
	if (g_bInitialized && !g_Display.IsFullscreenMode() && !IsIconic(hwnd)) {
		g_Display.PaletteChanged();
	}
}

BOOL OnQueryNewPalette(HWND hwnd)
{
	if (g_bInitialized && !g_Display.IsFullscreenMode() && !IsIconic(hwnd)) {
		g_Display.PaletteChanged();
	}
	return TRUE;
}

void OnInitMenu(HWND hwnd, HMENU hMenu)
{
	CheckMenuItem(hMenu, IDM_SIM_PAUSE, g_bAppPaused ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_VIEW_TOOLBAR, g_bShowToolbar ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_VIEW_STATUSBAR, g_bShowStatusbar ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_VIEW_FULLSCREEN, g_Display.IsFullscreenMode() ? MF_CHECKED : MF_UNCHECKED);
//debug	g_Debug.InitMenu(hwnd, hMenu);
}

void OnEnterMenuLoop(HWND hwnd, BOOL bIsTrackPopupMenu)
{
	g_bAppInMenuLoop = TRUE;
	SetHaltState();
	SetClientRect();
	if (g_bAppActive && g_Display.IsFullscreenMode()) {
		g_Display.SwitchToMenuMode();
		DrawMenuBar(hwnd);
		RedrawWindow(hwnd, NULL, NULL, RDW_FRAME);
	}
}

void OnExitMenuLoop(HWND hwnd, BOOL bIsTrackPopupMenu)
{
	g_bAppInMenuLoop = FALSE;
	SetHaltState();
	SetClientRect();
}


void OnNotify(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	if (g_bShowToolbar)
		MsgNotifyToolbar(hwnd, wParam, lParam);
}

void OnMenuSelect(HWND hwnd, UINT nID, UINT nFlags, HMENU hMenu)
{
	if (g_bShowStatusbar)
		MsgMenuSelectStatusbar(hwnd, nID, nFlags, hMenu);
}

void OnCommand(HWND, int id, HWND hwndCtl, UINT codeNotify)
{
	OnMenuCommand(id);
//	SetCaptionText();
	SetClientRect();
}

void OnKeyDown(HWND, UINT vk, BOOL fDown, int cRepeat, UINT)
{
	OnKeyCommand(vk);

	if (!g_bInitialized)
		return;

//	SetCaptionText();
	SetClientRect();
}

/////////////////////////////////////////////////////////////////////////////
// Windows message cracker

LRESULT PASCAL MyWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		HANDLE_MSG(hwnd, WM_COMMAND,		   OnCommand);
		HANDLE_MSG(hwnd, WM_KEYDOWN,		   OnKeyDown);

		HANDLE_MSG(hwnd, WM_CREATE,			OnCreate);
		HANDLE_MSG(hwnd, WM_DESTROY,		   OnDestroy);
		HANDLE_MSG(hwnd, WM_ACTIVATE,		  OnActivate);
		HANDLE_MSG(hwnd, WM_SYSCOMMAND,		OnSysCommand);
		HANDLE_MSG(hwnd, WM_PAINT,			 OnPaint);
		HANDLE_MSG(hwnd, WM_NCPAINT,		   OnNCPaint);
		HANDLE_MSG(hwnd, WM_ERASEBKGND,		OnEraseBkGnd);
		HANDLE_MSG(hwnd, WM_SETCURSOR,		 OnSetCursor);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, OnWindowPosChanging);
		HANDLE_MSG(hwnd, WM_GETMINMAXINFO,	 OnGetMinMaxInfo);
		HANDLE_MSG(hwnd, WM_MOVE,			  OnMove);
		HANDLE_MSG(hwnd, WM_SIZE,			  OnSize);
		HANDLE_MSG(hwnd, WM_PALETTECHANGED,	OnPaletteChanged);
		HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE,   OnQueryNewPalette);
		HANDLE_MSG(hwnd, WM_INITMENU,		  OnInitMenu);

		case WM_NOTIFY:
			OnNotify(hwnd, wParam, lParam);
			break;

		case WM_MENUSELECT:
			OnMenuSelect(hwnd, (UINT)LOWORD(wParam), (UINT)HIWORD(wParam), (HMENU)lParam);
			break;
		
		case WM_ENTERMENULOOP:
			OnEnterMenuLoop(hwnd, (BOOL)wParam);
			break;
		
		case WM_EXITMENULOOP:
			OnExitMenuLoop(hwnd, (BOOL)wParam);
			break;
		
		case WM_ENTERSIZEMOVE:
			bSizeMove = TRUE;
			DispSizeMoveText();
			break;
		
		case WM_EXITSIZEMOVE:
			bSizeMove = FALSE;
			SetCaptionText();
			break;
	}
	return 0L;
}

LRESULT PASCAL MfsWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bStopProcessing = FALSE;
	LRESULT res = MyWndProc(hwnd, message, wParam, lParam);
	if (bStopProcessing)
		return res;
	return DefWindowProc(hwnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////
// InitApplication : Create the main window

BOOL InitApplication(HINSTANCE hInst, int nCmdShow)
{
	WNDCLASS	wndClass;

	wndClass.style			= CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc	= MfsWndProc;
	wndClass.cbClsExtra		= 0;
	wndClass.cbWndExtra		= 0;
	wndClass.hInstance		= hInst;
	wndClass.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_MFS));
	wndClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground  = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wndClass.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU) ;
	wndClass.lpszClassName  = g_szAppClass;

	if (!RegisterClass(&wndClass)) {
		MGDBG0(DMSG_APIERROR, "Could not register class\n");
		return FALSE;
	}

	RECT &rc = g_WndInfo.rcPlacement;
	g_hWnd = CreateWindowEx(
				 WS_EX_APPWINDOW,
				 g_szAppClass, 
				 g_szAppTitle,
				 WS_OVERLAPPEDWINDOW,
				 rc.left, rc.top, rc.right, rc.bottom,
				 NULL,
				 NULL,
				 hInst,
				 NULL);

	if (g_hWnd == NULL) {
		MGDBG0(DMSG_APIERROR, "CreateWindowEx failed\n");
		return FALSE;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// MfsMain : Entry point to application

int MfsMain(int nCmdShow)
{
	SetCurrentDirectory(GetProgramDirectory());

	g_Debug.CreateLogFile(g_szLogFile);

	LoadRegistrySettings();

	if (!InitApplication(g_hInst, nCmdShow)) {
		MGDBG0(DMSG_FATAL, "Initialization of main window failed\n");
		goto exit_with_error;
	}

	DisplaySplashWindow();

	if (!LoadScenery(g_szScriptFile)) {
		MGDBG0(DMSG_FATAL, "Could not load scenery file\n");
//		goto exit_with_error;
	}
	if (!g_Display.Initialize()) {
		MGDBG0(DMSG_FATAL, "Direct Draw initialization failed\n");
		goto exit_with_error;
	}
	if (!SetDisplayMode(0, 0, 0, g_Display.IsFullscreenMode())) {
		MGDBG0(DMSG_FATAL, "Direct Draw mode setting failed\n");
		goto exit_with_error;
	}
	if (!g_Timer.Create(50, 50, TimerProc, 0, TIME_PERIODIC)) {
		MGDBG0(DMSG_FATAL, "Could not create multimedia timer\n");
//		goto exit_with_error;
	}
	if (!g_RCBox.OpenDevice()) {
		MGDBG0(DMSG_FATAL, "Could not open the RCCOM device\n");
//		goto exit_with_error;
	}
	if (!g_Joystick.Initialize()) {
		MGDBG0(DMSG_FATAL, "Could not initialize joystick\n");
//		goto exit_with_error;
	}

	OutputDebugString("MFS: Entering message loop");

	g_bInitialized = TRUE;

	SetHaltState();

	SetCaptionText();
	InitScene();
	CalcScene();

	g_hAccel = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDR_ACCEL));

	DestroySplashWindow();

	// main message loop
	MSG msg;
	for(;;) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;
			if (g_hAccel && (msg.hwnd == g_hWnd))
				TranslateAccelerator(g_hWnd, g_hAccel, &msg);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else if (!g_bAppHalted) {
			CalcFrameRate();
			MoveScene();
			RenderFrame();
		} else {
			WaitMessage();
		}
	}

	if (g_bExitWithError)
		goto exit_with_error;

	SaveRegistrySettings();
//	SaveScenery(g_szScriptFile);

	g_Display.Terminate();

	if (!g_RCBox.CloseDevice()) {
		MGDBG0(DMSG_FATAL, "Could not close RCCOM device\n");
	}

	return msg.wParam;

exit_with_error:

//debug	g_Debug.ShowWindow(TRUE);

	// i think we have to clear the message queue,
	// otherwise sometimes the following mbox doesn't appear
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT)
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	MessageBox(g_hWnd, CMGString(IDS_FATALERROR), g_szAppTitle, MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);

	return 0;
}

void UpdateClientWindows()
{
	SetClientRect();
	ShowWindow(g_hWndToolbar, g_bShowToolbar ? SW_SHOW : SW_HIDE);
	ShowWindow(g_hWndStatusbar, g_bShowStatusbar ? SW_SHOW : SW_HIDE);
}





