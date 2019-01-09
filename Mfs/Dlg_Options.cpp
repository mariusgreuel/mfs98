/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 Dlg_Options.cpp

 Version:   1.00

 Date:	  3-Feb-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "..\rccom\rccom.h"
#include "types.h"
#include "display.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Externals...

BOOL SetDisplayMode(DWORD dwWidth, DWORD dwHeight, WORD bpp, BOOL bFullscreen);

extern LPCTSTR		  g_szHelpFile;

extern HINSTANCE		g_hInst;
extern HWND			 g_hWnd;
extern WNDPLACEMENT	 g_WndInfo;

extern CDisplay		 g_Display;
extern CRCBox		   g_RCBox;

extern DWORD		 g_bUseGDI;

extern DWORD			g_dwCtrlDeviceType;	 // defined in calcmod.cpp
extern DWORD			g_dwCtrlDeviceMode;	 // defined in calcmod.cpp
extern DWORD		 g_bCtrlDeviceFTBack;	// defined in calcmod.cpp

/////////////////////////////////////////////////////////////////////////////

class CDialogControl {
public:
	static LRESULT CALLBACK DialogProc(HWND m_hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnInitDialog();
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void OnHelp(LPHELPINFO lphi);

private:
	HWND			m_hWnd;
};

class CDialogPrefer {
public:
	static LRESULT CALLBACK DialogProc(HWND m_hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnInitDialog();
	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void OnHelp(LPHELPINFO lphi);

private:
	HWND			m_hWnd;
	BOOL			m_bModeChanged;
};

/////////////////////////////////////////////////////////////////////////////
// Control dialog

BOOL CDialogControl::OnInitDialog()
{
	CheckRadioButton(m_hWnd, IDC_CTRL_USERC, IDC_CTRL_USEJOYSTICK, IDC_CTRL_USERC + g_dwCtrlDeviceType);
	CheckRadioButton(m_hWnd, IDC_CTRL_MODE1, IDC_CTRL_MODE4, IDC_CTRL_MODE1 + g_dwCtrlDeviceMode);
	Button_SetCheck(GetDlgItem(m_hWnd, IDC_CTRL_FTBACK), g_bCtrlDeviceFTBack);
	return TRUE;
}

BOOL CDialogControl::OnCommand(WPARAM wParam, LPARAM)
{
	BOOL bChanged = FALSE;

	switch (LOWORD(wParam)) {
		case IDC_CTRL_SETTINGS:
			if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_USERC) == BST_CHECKED)
				g_RCBox.Dialog(m_hWnd, 0);
			else if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_USEJOYSTICK) == BST_CHECKED)
				WinExec("control joy.cpl", SW_SHOW);
			break;
		case IDC_CTRL_USERC:
		case IDC_CTRL_USEJOYSTICK:
		case IDC_CTRL_MODE1:
		case IDC_CTRL_MODE2:
		case IDC_CTRL_MODE3:
		case IDC_CTRL_MODE4:
		case IDC_CTRL_FTBACK:
			if (HIWORD(wParam) == BN_CLICKED)
				bChanged = TRUE;
			break;
	}

	if (bChanged)
		PropSheet_Changed(GetParent(m_hWnd), m_hWnd);

	return 0;
}

BOOL CDialogControl::OnNotify(WPARAM, LPARAM lParam, LRESULT* )
{
	switch (((LPNMHDR)lParam)->code) {
		case PSN_SETACTIVE:
			SetWindowLong(m_hWnd, DWL_MSGRESULT, 0);
			break;
		case PSN_APPLY: {
			if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_USERC) == BST_CHECKED)
				g_dwCtrlDeviceType = 0;
			else if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_USEJOYSTICK) == BST_CHECKED)
				g_dwCtrlDeviceType = 1;
			if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_MODE1) == BST_CHECKED)
				g_dwCtrlDeviceMode = 0;
			else if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_MODE2) == BST_CHECKED)
				g_dwCtrlDeviceMode = 1;
			else if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_MODE3) == BST_CHECKED)
				g_dwCtrlDeviceMode = 2;
			else if (IsDlgButtonChecked(m_hWnd, IDC_CTRL_MODE4) == BST_CHECKED)
				g_dwCtrlDeviceMode = 3;
			g_bCtrlDeviceFTBack = IsDlgButtonChecked(m_hWnd, IDC_CTRL_FTBACK) == BST_CHECKED ? TRUE : FALSE; 
			break;
		}
	}

	return TRUE;
}

void CDialogControl::OnHelp(LPHELPINFO lphi)
{
	WinHelp((HWND)lphi->hItemHandle, g_szHelpFile, HELP_CONTEXTPOPUP, lphi->dwContextId);
}

LRESULT CALLBACK CDialogControl::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CDialogControl	 *pDialog = NULL;

	if (uMsg == WM_INITDIALOG) {
		pDialog = (CDialogControl*)((LPPROPSHEETPAGE)lParam)->lParam;
		if (pDialog == NULL)
			return FALSE;
		pDialog->m_hWnd = hWnd;
		return pDialog->OnInitDialog();
	}

	if (pDialog == NULL)
		return FALSE;

	switch (uMsg) {
		case WM_COMMAND:
			return pDialog->OnCommand(wParam, lParam);
		case WM_NOTIFY:
			return pDialog->OnNotify(wParam, lParam, NULL);
		case WM_HELP:
			pDialog->OnHelp((LPHELPINFO)lParam);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Preferences dialog

BOOL CDialogPrefer::OnInitDialog()
{
	HWND	hcbModes;
	CHAR	szBuffer[32];

	m_bModeChanged = FALSE;

	hcbModes = GetDlgItem(m_hWnd, IDC_SCREEN_MODES);

	g_Display.FillModesComboBox(hcbModes);
	wsprintf(szBuffer, "%dx%dx%d",
			g_WndInfo.szResolution.cx, g_WndInfo.szResolution.cy, g_WndInfo.wBpp);

	int n = ComboBox_FindStringExact(hcbModes, 0, szBuffer);

	if (n == CB_ERR) {

		wsprintf(szBuffer, "%dx%dx%d", 640, 480, 8);
		n = ComboBox_FindStringExact(hcbModes, 0, szBuffer);
		if (n == CB_ERR)
			n = 0;

	}

	ComboBox_SetCurSel(hcbModes, n);
	Button_SetCheck(GetDlgItem(m_hWnd, IDC_USEOPTDRAW), g_bUseGDI ? BST_UNCHECKED : BST_CHECKED);

	return TRUE;
}

BOOL CDialogPrefer::OnCommand(WPARAM wParam, LPARAM)
{
	BOOL bChanged = FALSE;

	switch (LOWORD(wParam)) {
		case IDC_SOUND_VOLUME:
			WinExec("sndvol32.exe", SW_SHOW);
			break;
		case IDC_SCREEN_MODES:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				m_bModeChanged = TRUE;
				bChanged = TRUE;
			}
			break;
		case IDC_USEOPTDRAW:
			if (HIWORD(wParam) == BN_CLICKED)
				bChanged = TRUE;
			break;
	}
	if (bChanged)
		PropSheet_Changed(GetParent(m_hWnd), m_hWnd);

	return TRUE;
}

BOOL CDialogPrefer::OnNotify(WPARAM, LPARAM lParam, LRESULT* )
{
	LPNMHDR pnm = (LPNMHDR)lParam;

	switch (pnm->code)	{
		case PSN_SETACTIVE:
			SetWindowLong(m_hWnd, DWL_MSGRESULT, 0);
			break;
		case PSN_APPLY: {
			HWND hcbModes = GetDlgItem(m_hWnd, IDC_SCREEN_MODES);
			int n = ComboBox_GetCurSel(hcbModes);
			DWORD dwData = SendMessage(hcbModes, CB_GETITEMDATA, n, 0);
			g_WndInfo.szResolution.cx = dwData & 0xFFF;
			g_WndInfo.szResolution.cy = (dwData >> 12) & 0xFFF;
			g_WndInfo.wBpp = (WORD)(dwData >> 24) & 0xFFF;
			g_bUseGDI = IsDlgButtonChecked(m_hWnd, IDC_USEOPTDRAW) == BST_CHECKED ? FALSE : TRUE;
			if (g_Display.IsFullscreenMode() && m_bModeChanged)
				SetDisplayMode(g_WndInfo.szResolution.cx, g_WndInfo.szResolution.cy, g_WndInfo.wBpp, TRUE);
			break;
		}
	}

	return TRUE;
}

void CDialogPrefer::OnHelp(LPHELPINFO lphi)
{
	WinHelp((HWND)lphi->hItemHandle, g_szHelpFile, HELP_CONTEXTPOPUP, lphi->dwContextId);
}

LRESULT CALLBACK CDialogPrefer::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CDialogPrefer	 *pDialog = NULL;

	switch (uMsg) {
		case WM_INITDIALOG:
			pDialog = (CDialogPrefer*)((LPPROPSHEETPAGE)lParam)->lParam;
			if (pDialog == NULL)
				return FALSE;
			pDialog->m_hWnd = hWnd;
			return pDialog->OnInitDialog();
		case WM_COMMAND:
			if (pDialog == NULL)
				return FALSE;
			return pDialog->OnCommand(wParam, lParam);
		case WM_NOTIFY:
			if (pDialog == NULL)
				return FALSE;
			return pDialog->OnNotify(wParam, lParam, NULL);
		case WM_HELP:
			if (pDialog == NULL)
				return FALSE;
			pDialog->OnHelp((LPHELPINFO)lParam);
			return TRUE;
		default:
			return FALSE;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Options dialog

BOOL ShowDlg_Options(int nStartPage)
{
	CDialogControl  DialogControl;
	CDialogPrefer   DialogPrefer;

	static struct _PAGE {
		WORD		idDlg;
		DLGPROC	 pfnDlgProc;
		LPARAM	  lParam;
	}Page[] = {
		IDD_PS_OPTIONS1, (DLGPROC)CDialogControl::DialogProc, (LPARAM)&DialogControl,
		IDD_PS_OPTIONS2, (DLGPROC)CDialogPrefer::DialogProc, (LPARAM)&DialogPrefer,
	};

	#define PAGES   (sizeof(Page)/sizeof(_PAGE))

	PROPSHEETHEADER psh;
	PROPSHEETPAGE   psp[ PAGES ];

	if (nStartPage >= PAGES)
		nStartPage = 0;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE;
	psh.hwndParent = g_hWnd;
	psh.hInstance = g_hInst;
	psh.pszCaption = MAKEINTRESOURCE(IDS_OPTIONS);
	psh.nPages = PAGES;
	psh.nStartPage = nStartPage;
	psh.ppsp = psp;

	for(int i=0; i<PAGES; i++) {
		psp[i].dwSize = sizeof(PROPSHEETPAGE);
		psp[i].dwFlags = PSP_DEFAULT;
		psp[i].hInstance = g_hInst;
		psp[i].pszTemplate = MAKEINTRESOURCE(Page[i].idDlg);
		psp[i].pfnDlgProc = Page[i].pfnDlgProc;
		psp[i].lParam = Page[i].lParam;
	}

	return PropertySheet(&psh);

	#undef PAGES
}


