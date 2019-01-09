/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 dialog.cpp : rccom property sheet dialogs

 Version:   1.00

 Date:	  15-Jan-97

 Author:	MG

****************************************************************************/

#include "stdafx.h"
#include <stdlib.h>

#include "cstmctrl.h"
#include "mgstring.h"
#include "resource.h"
#include "rccom.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

static CRCBox				RCBox;
static BOOL					g_bUpdated = FALSE;

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE			g_hInst;

extern LPCSTR				g_szHelpFile;
extern DWORD				g_aHelpIDs[];

extern RCCOM_INFO			rci;
extern RCCOM_HARDWARE_INFO  rchi;

extern CALDATA				g_CalData;

BOOL ValidateConfig();
void InitCalData(PCALDATA pCalData);
BOOL ValidateChannelData(ULONG cv);
LONG ConvertChannelData(BYTE ch);

/////////////////////////////////////////////////////////////////////////////
// Dialog TestRC

class CDialogTestRC {
public:
	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnInitDialog();

	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	BOOL OnPaint();

	void OnTimer();
	void OnHelp(LPHELPINFO lphi);
	void OnContextMenu(WPARAM wParam);

private:
	HWND			m_hWnd;

	CCrossHair		m_CrossHair_Left;
	CCrossHair		m_CrossHair_Right;
	CShiftBar		m_ShiftBar_Left;
	CShiftBar		m_ShiftBar_Right;

	DWORD			dwLastErr;
};

BOOL CDialogTestRC::OnInitDialog()
{
	m_CrossHair_Left.Create(GetDlgItem(m_hWnd, ID_TEST_LEFT_JOY));
	m_CrossHair_Right.Create(GetDlgItem(m_hWnd, ID_TEST_RIGHT_JOY));
	m_ShiftBar_Left.Create(GetDlgItem(m_hWnd, ID_TEST_LEFT_BAR));
	m_ShiftBar_Right.Create(GetDlgItem(m_hWnd, ID_TEST_RIGHT_BAR));

	dwLastErr = 0xff;

	return TRUE;
}

BOOL CDialogTestRC::OnNotify(WPARAM, LPARAM lParam, LRESULT*)
{
	switch (((LPNMHDR)lParam)->code) {
		case PSN_SETACTIVE:
			if (SetTimer(m_hWnd, 1, 20, NULL) == 0)
				return FALSE;
			break;
		case PSN_KILLACTIVE:
			KillTimer(m_hWnd, 1);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

BOOL CDialogTestRC::OnPaint()
{
	PAINTSTRUCT ps;

	BeginPaint(m_hWnd, &ps);

	m_CrossHair_Left.Invalidate();
	m_CrossHair_Right.Invalidate();
	m_ShiftBar_Left.Invalidate();
	m_ShiftBar_Right.Invalidate();

	EndPaint(m_hWnd, &ps);

	return 0;
}

void CDialogTestRC::OnTimer()
{
	POINT		   p;

	if (RCBox.Read()) {
		if (rci.bUpdated) {
			p.x = RCBox.GetPos(0);
			p.y = RCBox.GetPos(1);
			m_CrossHair_Left.SetPos(p);
			p.x = RCBox.GetPos(2);
			p.y = RCBox.GetPos(3);
			m_CrossHair_Right.SetPos(p);
			m_ShiftBar_Left.SetPos(RCBox.GetPos(4));
			m_ShiftBar_Right.SetPos(RCBox.GetPos(5));

			LONG k = MulDiv(1000000, 65536, rchi.CounterFrequency.LowPart);
			for(int i=0; i<MAX_CHANNELS; i++) {
				CMGString Text;
				if (ValidateChannelData(rci.RawValue[i])) {
					Text.printf("%ld/%ld", ConvertChannelData(i), MulDiv(rci.RawValue[i], k, 65536));
					SetDlgItemText(m_hWnd, ID_TEST_CHANNEL1 + i, Text);
				} else {
					SetDlgItemText(m_hWnd, ID_TEST_CHANNEL1 + i, "n/a");
				}
			}
		}
	} else {
		p.x = p.y = 0;
		m_CrossHair_Left.SetPos(p);
		m_CrossHair_Right.SetPos(p);
		m_ShiftBar_Left.DrawDisabledBar();
		m_ShiftBar_Right.DrawDisabledBar();
		if (dwLastErr != RCBox.GetLastError())
			for(int i=0; i<MAX_CHANNELS; i++)
				SetDlgItemText(m_hWnd, ID_TEST_CHANNEL1 + i, "?");
	}
	if (dwLastErr != RCBox.GetLastError() || g_bUpdated) {
		SetDlgItemText(m_hWnd, ID_TEST_STATUS, RCBox.GetLastErrorString());
		dwLastErr = RCBox.GetLastError();
		g_bUpdated = FALSE;
	}
}

void CDialogTestRC::OnHelp(LPHELPINFO lphi)
{
	WinHelp((HWND)lphi->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)g_aHelpIDs);
}

void CDialogTestRC::OnContextMenu(WPARAM wParam)
{
	WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)g_aHelpIDs);
}

LRESULT CALLBACK CDialogTestRC::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CDialogTestRC	*pDialog = NULL;

	if (uMsg == WM_INITDIALOG) {
		pDialog = (CDialogTestRC*)((LPPROPSHEETPAGE)lParam)->lParam;
		if (pDialog == NULL)
			return FALSE;
		pDialog->m_hWnd = hWnd;
		return pDialog->OnInitDialog();
	}

	if (pDialog == NULL)
		return FALSE;

	switch (uMsg) {
		case WM_NOTIFY:
			return pDialog->OnNotify(wParam, lParam, NULL);
		case WM_PAINT:
			return pDialog->OnPaint();
		case WM_TIMER:
			pDialog->OnTimer();
			break;
		case WM_HELP:
			pDialog->OnHelp((LPHELPINFO)lParam);
			break;
		case WM_CONTEXTMENU:
			pDialog->OnContextMenu(wParam);
			break;
		case WM_DESTROY:
			pDialog = NULL;
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CalDialogProc

class CDialogCalRC {
public:
	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnInitDialog();

	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	BOOL OnPaint();

	void OnTimer();
	void OnHelp(LPHELPINFO lphi);
	void OnContextMenu(WPARAM wParam);

private:
	void UpdateDialog();
	int  UpdateStep();

private:
	HWND			m_hWnd;

	CShiftBar	   ShiftBar[ MAX_CHANNELS ];
	CALDATA		 TempInfo;
	int			 DialogStep;
};

BOOL CDialogCalRC::OnInitDialog()
{
	DialogStep = -2;

	for(int i=0; i<MAX_CHANNELS; i++)
		ShiftBar[i].Create(GetDlgItem(m_hWnd, ID_CAL_BAR1 + i));

	UpdateDialog();

	return TRUE;
}

BOOL CDialogCalRC::OnCommand(WPARAM wParam, LPARAM)
{
	int	 Result;

	switch (LOWORD(wParam)) {
		case IDNEXT:
			if (DialogStep == 8) {
				DialogStep = -2;
				g_CalData = TempInfo;
				PropSheet_Changed(GetParent(m_hWnd), m_hWnd);
				break;
			}
			while ((Result = UpdateStep()) == IDRETRY);
			if (Result == IDABORT)
				DialogStep = -2;
			else
				DialogStep++;
			break;
		case IDPREV:
			DialogStep--;
			break;
		default:
			return 1;
	}

	UpdateDialog();

	return 0;
}

BOOL CDialogCalRC::OnNotify(WPARAM, LPARAM lParam, LRESULT*)
{
	switch (((LPNMHDR)lParam)->code)	{
		case PSN_SETACTIVE:
			if (SetTimer(m_hWnd, 1, 20, NULL) == 0)
				return FALSE;
			break;
		case PSN_KILLACTIVE:
			KillTimer(m_hWnd, 1);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

BOOL CDialogCalRC::OnPaint()
{
	PAINTSTRUCT ps;

	BeginPaint(m_hWnd, &ps);

	for(int i=0; i<MAX_CHANNELS; i++)
		ShiftBar[i].Invalidate();

	EndPaint(m_hWnd, &ps);

	return 0;
}

void CDialogCalRC::OnTimer()
{
	if (RCBox.Read()) {
		if (rci.bUpdated) {
			LONG k = -MAX_STICK_VALUE * 65536 / (LONG)rchi.ttv.uDeviationLen;
			for(int i=0; i<MAX_CHANNELS; i++) {
				if (ValidateChannelData(rci.RawValue[i]))
					ShiftBar[i].SetPos(((LONG)rci.RawValue[i] - (LONG)rchi.ttv.uCenterLen) * k + 32768 >> 16);
				else
					ShiftBar[i].DrawDisabledBar();
			}
		}
	} else {
		for(int i=0; i<MAX_CHANNELS; i++)
			ShiftBar[i].DrawDisabledBar();
	}
}

void CDialogCalRC::OnHelp(LPHELPINFO lphi)
{
	WinHelp((HWND)lphi->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)g_aHelpIDs);
}

void CDialogCalRC::OnContextMenu(WPARAM wParam)
{
	WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)g_aHelpIDs);
}

LRESULT CALLBACK CDialogCalRC::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CDialogCalRC	*pDialog = NULL;

	if (uMsg == WM_INITDIALOG) {
		pDialog = (CDialogCalRC*)((LPPROPSHEETPAGE)lParam)->lParam;
		if (pDialog == NULL)
			return FALSE;
		pDialog->m_hWnd = hWnd;
		return pDialog->OnInitDialog();
	}

	if (pDialog == NULL)
		return FALSE;

	switch (uMsg) {
		case WM_COMMAND:
			pDialog->OnCommand(wParam, lParam);
			break;
		case WM_NOTIFY:
			pDialog->OnNotify(wParam, lParam, NULL);
			break;
		case WM_PAINT:
			pDialog->OnPaint();
			break;
		case WM_TIMER:
			pDialog->OnTimer();
			break;
		case WM_HELP:
			pDialog->OnHelp((LPHELPINFO)lParam);
			break;
		case WM_CONTEXTMENU:
			pDialog->OnContextMenu(wParam);
			break;
		case WM_DESTROY:
			pDialog = NULL;
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

void CDialogCalRC::UpdateDialog()
{
	CMGString Text;

	if (DialogStep == -2) {
		// "press cal to start"
		Text = CMGString(IDS_CAL_START);
		SetDlgItemText(m_hWnd, ID_CAL_COMMAND, Text);
		Button_Enable(GetDlgItem(m_hWnd, IDPREV), 0);
		Button_Enable(GetDlgItem(m_hWnd, IDNEXT), 1);
		Text = CMGString(IDS_CALIBRATE_BUTTON);
		SetDlgItemText(m_hWnd, IDNEXT, Text);
	} else if (DialogStep == -1) {
		// "center sticks"
		Text = CMGString(IDS_CAL_CENTER);
		Text += CMGString(IDS_CAL_PRESSCONTINUE);
		SetDlgItemText(m_hWnd, ID_CAL_COMMAND, Text);
		Button_Enable(GetDlgItem(m_hWnd, IDPREV), 0);
		Button_Enable(GetDlgItem(m_hWnd, IDNEXT), 1);
		Text = CMGString(IDS_CONTINUE_BUTTON);
		SetDlgItemText(m_hWnd, IDNEXT, Text);
	} else if (DialogStep >= 8) {
		// "cal finished, press finish to save settings"
		Text = CMGString(IDS_CAL_PRESSFINISH);
		SetDlgItemText(m_hWnd, ID_CAL_COMMAND, Text);
		Button_Enable(GetDlgItem(m_hWnd, IDPREV), 0);
		Text = CMGString(IDS_FINISH_BUTTON);
		SetDlgItemText(m_hWnd, IDNEXT, Text);
	} else {
		// "put the s1 stick to the s2 stop"
		CMGString Fmt, Side1, Side2;
		Fmt = CMGString(IDS_CAL_STICK);
		Side1 = CMGString(IDS_CAL_SIDE0 + (DialogStep < 4 ? 0 : 1));
		Side2 = CMGString(IDS_CAL_SIDE0 + (DialogStep & 3));
		Text.printf(Fmt, DialogStep + 1, (LPCTSTR)Side1, (LPCTSTR)Side2);
		Text += CMGString(IDS_CAL_PRESSCONTINUE);
		SetDlgItemText(m_hWnd, ID_CAL_COMMAND, Text);
		Button_Enable(GetDlgItem(m_hWnd, IDPREV), 1);
	}
}

int CDialogCalRC::UpdateStep()
{
	int	 Result = IDOK;
	BOOL	Ignore = FALSE;

	if (DialogStep == -2) {
		if (RCBox.GetLastError() != RCERR_SUCCESS) {
			CMGString strText;
			strText = CMGString(IDS_CAL_BADDATA);
			strText += RCBox.GetLastErrorString();
			MessageBox(m_hWnd, strText, CMGString(IDS_CAL_ERROR), MB_OK | MB_ICONEXCLAMATION);
			return IDABORT;
		}
	} else if (DialogStep == -1) {
		InitCalData(&TempInfo);
		RCBox.Read();
		for(int i=0; i<MAX_CHANNELS; i++) {
			if (!ValidateChannelData(rci.RawValue[i]) && i >= 4)
				continue;
			if (!Ignore && abs((LONG)rci.RawValue[i] - (LONG)rchi.ttv.uCenterLen) > (LONG)rchi.ttv.uDeviationLen / 4) {
				CMGString Text;
				Text = CMGString(IDS_CAL_WARN_CENTER);
				Text += CMGString(IDS_CAL_WARN_CORRECT);
				Result = MessageBox(m_hWnd,
						Text,
						CMGString(IDS_CAL_ERROR),
						MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | MB_DEFBUTTON2);
				if (Result == IDRETRY || Result == IDABORT)
					return Result;
				Ignore = TRUE;
			}
			TempInfo.Zero[i] = rci.RawValue[i];
		}
	} else {
		int  MaxChannel;			// channel with the largest value
		LONG MaxValue1 = 0;		 // largest value
		LONG MaxValue2 = 0;		 // 2nd largest value
		RCBox.Read();
		for(int i=0; i<MAX_CHANNELS; i++) {
			if (!ValidateChannelData(rci.RawValue[i]))
				continue;
			LONG d = abs((LONG)rci.RawValue[i] - (LONG)TempInfo.Zero[i]);
			if (d > MaxValue1) {
				MaxChannel = i;
				MaxValue2 = MaxValue1;
				MaxValue1 = d;
			} else if (d > MaxValue2) {
				MaxValue2 = d;
			}
		}
		if (!Ignore && MaxValue2 / 64 >= MaxValue1 / 128) {
			CMGString Text, Fmt, Side1, Side2;
			Fmt = CMGString(IDS_CAL_WARN_STICK1);
			Side1 = CMGString(IDS_CAL_SIDE0 + (DialogStep < 4 ? 0 : 1));
			Side2 = CMGString(IDS_CAL_SIDE0 + (DialogStep	& 3));
			Text.printf(Fmt, (LPCTSTR)Side1, (LPCTSTR)Side2);
			Text += CMGString(IDS_CAL_WARN_CORRECT);
			Result = MessageBox(m_hWnd,
						 Text,
						 CMGString(IDS_CAL_ERROR),
						 MB_ABORTRETRYIGNORE | MB_ICONEXCLAMATION | MB_DEFBUTTON2);
			if (Result == IDRETRY || Result == IDABORT)
				return Result;
			Ignore = TRUE;
		}
		int n = DialogStep >> 1;
		TempInfo.CHStick[ n ] = MaxChannel;
		LONG f = (LONG)rci.RawValue[ MaxChannel ] - (LONG)TempInfo.Zero[ MaxChannel ];
		if (f > 0) {
			if ((DialogStep & 1) == 0)
				f = -f;
			TempInfo.PFac[ MaxChannel ] = LONG(MAX_STICK_VALUE * 65536 / f);
		} else if (f < 0) {
			if ((DialogStep & 1) == 1)
				f = -f;
			TempInfo.NFac[ MaxChannel ] = LONG(MAX_STICK_VALUE * 65536 / -f);
		}
	}

	return IDOK;
}

/////////////////////////////////////////////////////////////////////////////
// ExtDialogProc

class CDialogExtended {
public:
	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnInitDialog();

	BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	void OnHelp(LPHELPINFO lphi);
	void OnContextMenu(WPARAM wParam);

private:
	void UpdateDialog();

private:
	HWND			m_hWnd;
};

BOOL CDialogExtended::OnInitDialog()
{
	TCHAR szBuffer[16];

	UpdateDialog();

	Button_SetCheck(GetDlgItem(m_hWnd, ID_EXT_INVPULSE), rchi.bInvertedPulse);

	HWND hComPort = GetDlgItem(m_hWnd, ID_EXT_COM_PORT);
	ComboBox_AddString(hComPort, "Auto");
	for(int i=1; i<=16; i++) {
		wsprintf(szBuffer, "COM%u", i);
		ComboBox_AddString(hComPort, szBuffer);
	}
	ComboBox_SetCurSel(hComPort, rchi.uComPort);

	HWND hComIRQ = GetDlgItem(m_hWnd, ID_EXT_COM_IRQ);
	ComboBox_AddString(hComIRQ, "Auto");
	for(i=2; i<=15; i++) {
		wsprintf(szBuffer, "%u", i);
		ComboBox_AddString(hComIRQ, szBuffer);
	}
	ComboBox_SetCurSel(hComIRQ, rchi.uComIrq == 0 ? 0 : rchi.uComIrq - 1);

	return TRUE;
}

BOOL CDialogExtended::OnCommand(WPARAM wParam, LPARAM)
{
	BOOL bChanged = FALSE;
	switch (LOWORD(wParam)) {
		case ID_EXT_PULSE_SYNC:
		case ID_EXT_PULSE_MIDDLE:
		case ID_EXT_PULSE_MAX:
			if (HIWORD(wParam) == EN_CHANGE)
				bChanged = TRUE;
			break;
		case ID_EXT_INVPULSE:
			if (HIWORD(wParam) == BN_CLICKED)
				bChanged = TRUE;
			break;
		case ID_EXT_COM_PORT:
		case ID_EXT_COM_IRQ:
			if (HIWORD(wParam) == CBN_SELCHANGE)
				bChanged = TRUE;
			break;
		default:
			return 1;
	}

	if (bChanged)
		PropSheet_Changed(GetParent(m_hWnd), m_hWnd);

	return 0;
}

BOOL CDialogExtended::OnNotify(WPARAM, LPARAM lParam, LRESULT*)
{
	switch (((LPNMHDR)lParam)->code)	{
		case PSN_APPLY: {
			TCHAR szBuffer[32];
			Edit_GetText(GetDlgItem(m_hWnd, ID_EXT_PULSE_SYNC), szBuffer, sizeof(szBuffer));
			rchi.tv.uSyncLen = (ULONG)atoi(szBuffer);
			Edit_GetText(GetDlgItem(m_hWnd, ID_EXT_PULSE_MIDDLE), szBuffer, sizeof(szBuffer));
			rchi.tv.uCenterLen = (ULONG)atoi(szBuffer);
			Edit_GetText(GetDlgItem(m_hWnd, ID_EXT_PULSE_MAX), szBuffer, sizeof(szBuffer));
			rchi.tv.uDeviationLen = (ULONG)atoi(szBuffer);
			rchi.uComPort = (ULONG)ComboBox_GetCurSel(GetDlgItem(m_hWnd, ID_EXT_COM_PORT));
			rchi.uComIrq = (ULONG)ComboBox_GetCurSel(GetDlgItem(m_hWnd, ID_EXT_COM_IRQ));
			rchi.bInvertedPulse = IsDlgButtonChecked(m_hWnd, ID_EXT_INVPULSE) == BST_CHECKED ? TRUE : FALSE;

			if (rchi.uComIrq > 0)
				rchi.uComIrq++;

			if (!ValidateConfig()) {
				UpdateDialog();
			}

			RCBox.SetHardwareInfo(&rchi);
			RCBox.GetHardwareInfo(&rchi);

			g_bUpdated = TRUE;

			break;
		}
	}

	return TRUE;
}

void CDialogExtended::OnHelp(LPHELPINFO lphi)
{
	WinHelp((HWND)lphi->hItemHandle, g_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)g_aHelpIDs);
}

void CDialogExtended::OnContextMenu(WPARAM wParam)
{
	WinHelp((HWND)wParam, g_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)g_aHelpIDs);
}

LRESULT CALLBACK CDialogExtended::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static CDialogExtended	*pDialog = NULL;

	if (uMsg == WM_INITDIALOG) {
		pDialog = (CDialogExtended*)((LPPROPSHEETPAGE)lParam)->lParam;
		if (pDialog == NULL)
			return FALSE;
		pDialog->m_hWnd = hWnd;
		return pDialog->OnInitDialog();
	}

	if (pDialog == NULL)
		return FALSE;

	switch (uMsg) {
		case WM_COMMAND:
			pDialog->OnCommand(wParam, lParam);
			break;
		case WM_NOTIFY:
			pDialog->OnNotify(wParam, lParam, NULL);
			break;
		case WM_HELP:
			pDialog->OnHelp((LPHELPINFO)lParam);
			break;
		case WM_CONTEXTMENU:
			pDialog->OnContextMenu(wParam);
			break;
		case WM_DESTROY:
			pDialog = NULL;
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

void CDialogExtended::UpdateDialog()
{

	TCHAR szBuffer[16];

	wsprintf(szBuffer, "%u", DWORD(rchi.tv.uSyncLen));

	SendMessage(
		GetDlgItem(m_hWnd, ID_EXT_UD_PSYNC),
		UDM_SETRANGE,
		0,
		MAKELONG(DEFAULT_SYNC_LEN_RANGE_MAX, DEFAULT_SYNC_LEN_RANGE_MIN)
   );

	Edit_SetText(GetDlgItem(m_hWnd, ID_EXT_PULSE_SYNC), szBuffer);

	wsprintf(szBuffer, "%u", DWORD(rchi.tv.uCenterLen));

	SendMessage(
		GetDlgItem(m_hWnd, ID_EXT_UD_PMIDDLE),
		UDM_SETRANGE,
		0,
		MAKELONG(DEFAULT_CENTER_LEN_RANGE_MAX, DEFAULT_CENTER_LEN_RANGE_MIN)
	 );

	Edit_SetText(GetDlgItem(m_hWnd, ID_EXT_PULSE_MIDDLE), szBuffer);

	wsprintf(szBuffer, "%u", DWORD(rchi.tv.uDeviationLen));

	SendMessage(
		GetDlgItem(m_hWnd, ID_EXT_UD_PMAX),
		UDM_SETRANGE,
		0,
		MAKELONG(DEFAULT_DEVIATION_LEN_RANGE_MAX, DEFAULT_DEVIATION_LEN_RANGE_MIN)
	 );

	Edit_SetText(GetDlgItem(m_hWnd, ID_EXT_PULSE_MAX), szBuffer);
}

/////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) BOOL CRCBox::Dialog(HWND hWnd, int nStartPage)
{
	CDialogTestRC	   DialogTestRC;
	CDialogCalRC		DialogCalRC;
	CDialogExtended	 DialogExtended;

	struct _PAGE {
		WORD		idDlg;
		DLGPROC	 pfnDlgProc;
		LPARAM	  lParam;
	}Page[] = {
		IDD_TEST,   (DLGPROC)CDialogTestRC::DialogProc, (LPARAM)&DialogTestRC,
		IDD_CAL,	(DLGPROC)CDialogCalRC::DialogProc, (LPARAM)&DialogCalRC,
		IDD_EXT,	(DLGPROC)CDialogExtended::DialogProc, (LPARAM)&DialogExtended,
	};

	#define PAGES   (sizeof(Page)/sizeof(_PAGE))

	PROPSHEETHEADER psh;
	PROPSHEETPAGE   psp[ PAGES ];

	RCBox.OpenDevice();

	if (nStartPage >= PAGES)
		nStartPage = 0;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_DEFAULT | PSH_PROPSHEETPAGE | PSH_PROPTITLE;
	psh.hwndParent = hWnd;
	psh.hInstance = g_hInst;
	psh.pszCaption = MAKEINTRESOURCE(IDS_REMOTECONTROL);
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

	PropertySheet(&psh);

	RCBox.CloseDevice();

	return TRUE;
}
