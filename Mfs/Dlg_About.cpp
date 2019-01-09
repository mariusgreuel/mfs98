/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 dlg_about.cpp : Aboutbox and splash window

 Version:   1.00

 Date:	  14-Jan-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "mgtimer.h"
#include "mgdebug.h"
//#include "string.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

#define LOGERROR(x) MGDBG0(DMSG_ERROR,x)
#define LOGAPIERROR(x) MGDBG0(DMSG_APIERROR,x)

MGMODULE(__FILE__);

static LPCSTR		   szSplashClass = "MFS_SplashClass";

static HWND			 g_hSplashWnd;
static HBITMAP		  g_hbmpSplash;
static BITMAP		   g_bmpSplash;

static CMGTimer		   g_Timer;

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE		g_hInst;
extern HWND			 g_hWnd;

/////////////////////////////////////////////////////////////////////////////
// AboutBox dialog

static LRESULT CALLBACK AppAboutProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg) {
		case WM_INITDIALOG: {
			// fill registered user
			SetDlgItemText(hWnd, IDC_ABOUT_USER, "Nicht registrierte Version");
			
			// fill available memory
			char acBuffer[80];
			MEMORYSTATUS MemStat;
			MemStat.dwLength = sizeof(MEMORYSTATUS);
			GlobalMemoryStatus(&MemStat);
			wsprintf(acBuffer, "%lu KByte", MemStat.dwTotalPhys / 1024L);
			SetDlgItemText(hWnd, IDC_ABOUT_MEMAVAIL, acBuffer);
			// fill system resources
			wsprintf(acBuffer, "%u%%", MemStat.dwMemoryLoad);
			SetDlgItemText(hWnd, IDC_ABOUT_MEMLOAD, acBuffer);
			return TRUE;
		}
		case WM_COMMAND: {
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
				EndDialog(hWnd, TRUE);
			break;
		}
	}
	return 0L;
}

void ShowDlg_About()
{
	if (DialogBox(
			g_hInst,
			MAKEINTRESOURCE(IDD_ABOUTBOX),
			g_hWnd,
			(DLGPROC)AppAboutProc) == -1) {
		LOGAPIERROR("DialogBox failed\n");
	}
}

/////////////////////////////////////////////////////////////////////////////
// SplashWnd dialog

static LRESULT CALLBACK SplashWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_CREATE: {
			return 0;
		}
		case WM_PAINT: {
			RECT rc;
			HDC hdcSrc, hdcDest;
			PAINTSTRUCT ps;

			GetClientRect(hWnd, &rc);

			hdcDest = BeginPaint(hWnd, &ps);
			if (hdcDest == NULL)
				return 0;

			hdcSrc = CreateCompatibleDC(hdcDest);
			if (hdcSrc == NULL)
				return 0;

			SelectObject(hdcSrc, g_hbmpSplash);
			BitBlt(hdcDest, 0,0,450,300, hdcSrc, 0,0, SRCCOPY);
			DeleteDC(hdcSrc);

			EndPaint(hWnd, &ps);
			break;
		}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

BOOL DisplaySplashWindow()
{
	WNDCLASS	wndClass;
	RECT rc;

	g_hbmpSplash = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SPLASH));
	if (g_hbmpSplash == NULL)
		return -1;
	GetObject(g_hbmpSplash, sizeof(BITMAP), &g_bmpSplash);
	GetWindowRect(GetDesktopWindow(), &rc);
	if (rc.right > g_bmpSplash.bmWidth)
		rc.left += (rc.right - rc.left - g_bmpSplash.bmWidth) / 2;
	if (rc.bottom > g_bmpSplash.bmHeight)
		rc.top += (rc.bottom - rc.top - g_bmpSplash.bmHeight) / 2;
	rc.right = g_bmpSplash.bmWidth;
	rc.bottom = g_bmpSplash.bmHeight;
	
	wndClass.style		  = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc	= SplashWndProc;
	wndClass.cbClsExtra	 = 0;
	wndClass.cbWndExtra	 = 0;
	wndClass.hInstance	  = g_hInst;
	wndClass.hIcon		  = NULL;
	wndClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground  = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndClass.lpszMenuName   = NULL;
	wndClass.lpszClassName  = szSplashClass;

	if (!RegisterClass(&wndClass)) {
		LOGAPIERROR("RegisterClass failed\n");
		return FALSE;
	}

	g_hSplashWnd = CreateWindowEx(
			WS_EX_TOPMOST,
			szSplashClass, 
			"",
			WS_VISIBLE | WS_POPUP,
			rc.left, rc.top, rc.right, rc.bottom,
			g_hWnd,
			NULL,
			g_hInst,
			NULL);

	if (g_hSplashWnd == NULL) {
		LOGAPIERROR("CreateWindowEx failed\n");
		return FALSE;
	}

	ShowWindow(g_hSplashWnd, SW_SHOW);
	UpdateWindow(g_hSplashWnd);

	return TRUE;
}

BOOL DestroySplashWindow()
{
	if (g_hSplashWnd) {
		DestroyWindow(g_hSplashWnd);
		g_hSplashWnd = NULL;
	}
	return TRUE;
}


