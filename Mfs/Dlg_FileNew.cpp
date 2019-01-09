/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 Dlg_FileNew.cpp

 Version:   1.00

 Date:	  3-Feb-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

static HBITMAP		  g_hBitmap = NULL;

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE		g_hInst;			// defined in winmain.cpp
extern HWND			 g_hWnd;			 // defined in winmain.cpp

/////////////////////////////////////////////////////////////////////////////

static void DrawBitmap(HWND hwndDlg)
{			
	HDC		 hdcSrc, hdcDest;
	PAINTSTRUCT ps;

	hdcDest = BeginPaint(hwndDlg, &ps);
	if (hdcDest == NULL)
		return;
	hdcSrc = CreateCompatibleDC(hdcDest);
	if (hdcSrc == NULL)
		return;
	g_hBitmap = (HBITMAP)SelectObject(hdcSrc, g_hBitmap);
	BitBlt(hdcDest, 0, 0, 150, 320, hdcSrc, 0,0, SRCCOPY);
	g_hBitmap = (HBITMAP)SelectObject(hdcSrc, g_hBitmap);
	DeleteDC(hdcSrc);
	EndPaint(hwndDlg, &ps);
}

static BOOL EnumerateFiles(HWND hwndComboBox, LPCSTR szFileType)
{
	WIN32_FIND_DATA	 fd;

	HANDLE hff = FindFirstFile(szFileType, &fd);
	if (hff == NULL) {
		if (GetLastError() == ERROR_NO_MORE_FILES)
			return TRUE;
		else
			return FALSE;
	}

	ComboBox_AddString(hwndComboBox, fd.cFileName);

	while (FindNextFile(hff, &fd))
		if (fd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL ||
				fd.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE)
		ComboBox_AddString(hwndComboBox, fd.cFileName);

	FindClose(hff);

	if (GetLastError() == ERROR_NO_MORE_FILES)
		return TRUE;
	else
		return FALSE;
}

static BOOL CALLBACK ChooseModelClass(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_PAINT: {
			DrawBitmap(hwndDlg);
			break;
		}
		case WM_NOTIFY: {
			switch (((LPNMHDR)lParam)->code)	{
				case PSN_SETACTIVE:
					PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
					break;
				case PSN_QUERYCANCEL:
					return FALSE;
			}
		}
		default:
			return FALSE;
	}
	return TRUE;
}

static BOOL CALLBACK ChooseModelLook(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG: {
			EnumerateFiles(GetDlgItem(hwndDlg, IDC_NEW_MODEL), "*.ifm");
			EnumerateFiles(GetDlgItem(hwndDlg, IDC_NEW_WORLD), "*.ifw");
			break;
		}
		case WM_NOTIFY: {
			switch (((LPNMHDR)lParam)->code)	{
				case PSN_SETACTIVE:
					PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
					break;
				case PSN_WIZNEXT:
					break;
				case PSN_QUERYCANCEL:
					return FALSE;
			}
		}
		case WM_PAINT: {
			DrawBitmap(hwndDlg);
			break;
		}
		default:
			return FALSE;
	}
	return TRUE;
}

static BOOL CALLBACK ChooseModelParms(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG: {
			EnumerateFiles(GetDlgItem(hwndDlg, IDC_NEW_PARMS), "*.ifm");
			EnumerateFiles(GetDlgItem(hwndDlg, IDC_NEW_WIND), "*.ifw");
			EnumerateFiles(GetDlgItem(hwndDlg, IDC_NEW_COMMON), "*.ifw");
			break;
		}
		case WM_NOTIFY: {
			switch (((LPNMHDR)lParam)->code)	{
				case PSN_SETACTIVE:
					PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
					break;
				case PSN_WIZNEXT:
					break;
				case PSN_QUERYCANCEL:
					return FALSE;
			}
		}
		case WM_PAINT: {
			DrawBitmap(hwndDlg);
			break;
		}
		default:
			return FALSE;
	}
	return TRUE;
}

static BOOL CALLBACK Finished(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_NOTIFY: {
			switch (((LPNMHDR)lParam)->code)	{
				case PSN_SETACTIVE:
					PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK|PSWIZB_FINISH);
					break;
				case PSN_WIZNEXT:
					break;
				case PSN_QUERYCANCEL:
					return FALSE;
			}
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case 0: {
					break;
				}
			}
		}
		case WM_PAINT: {
			DrawBitmap(hwndDlg);
			break;
		}
		default:
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

BOOL ShowDlg_FileNew()
{
	static struct _PAGE {
		WORD		idDlg;
		DLGPROC	 pfnDlgProc;
	}Page[] = {
		IDD_WZ_NEW1, (DLGPROC)ChooseModelClass,
		IDD_WZ_NEW2, (DLGPROC)ChooseModelLook,
		IDD_WZ_NEW3, (DLGPROC)ChooseModelParms,
		IDD_WZ_NEW4, (DLGPROC)Finished,
	};

	PROPSHEETHEADER	 psh;
	PROPSHEETPAGE	   psp[sizeof(Page)/sizeof(_PAGE)];

	g_hBitmap = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_NEW));

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD;
	psh.hwndParent = g_hWnd;
	psh.hInstance = g_hInst;
	psh.pszCaption = NULL;
	psh.nPages = sizeof(Page)/sizeof(_PAGE);
	psh.nStartPage = 0;
	psh.ppsp = psp;

	for(int i=0; i<sizeof(Page)/sizeof(_PAGE); i++) {
		psp[i].dwSize = sizeof(PROPSHEETPAGE);
		psp[i].dwFlags = PSP_DEFAULT;
		psp[i].hInstance = g_hInst;
		psp[i].pszTemplate = MAKEINTRESOURCE(Page[i].idDlg);
		psp[i].pfnDlgProc = Page[i].pfnDlgProc;
		psp[i].lParam = 0;
	}

	return PropertySheet(&psh);
}


