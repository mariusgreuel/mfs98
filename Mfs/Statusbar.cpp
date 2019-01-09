/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 statusbar.cpp

 Version:   1.00

 Date:	  13-Jan-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "mgstring.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Globals...

HWND		g_hWndStatusbar;	// handle of statusbar window

/////////////////////////////////////////////////////////////////////////////
// Locals...

MGMODULE(__FILE__);

/////////////////////////////////////////////////////////////////////////////

void UpdateStatusbar(LPCSTR pszText, WORD nPart, WORD wFlags)
{
	SendMessage(g_hWndStatusbar, SB_SETTEXT, nPart | wFlags, (LPARAM)pszText);
}

BOOL CreateStatusbar(HWND hwndParent, BOOL bVisible)
{
	g_hWndStatusbar = CreateStatusWindow(
			WS_CHILD | WS_BORDER | (bVisible ? WS_VISIBLE : 0),
			CMGString(IDS_PRESSF1FORHELP),
			hwndParent,
			IDR_STATUSBAR);
	if (g_hWndStatusbar == NULL) {
		MGDBG0(DMSG_APIERROR, "CreateStatusbar failed\n");
		return FALSE;
	}
	return TRUE;
}

LRESULT MsgMenuSelectStatusbar(HWND, UINT nID, UINT nFlags, HMENU hMenu)
{
	static CMGString  strText;
	UINT			nStringID = 0;

	if (nFlags == 0xffff && hMenu == NULL)
		nStringID = IDS_PRESSF1FORHELP;						 // menu has been closed
	else if (nFlags & MFT_SEPARATOR)
		nStringID = 0;
	else if (nFlags & MF_POPUP) {
		if (nFlags & MF_SYSMENU)
			nStringID = IDS_SYSMENU;
//		else nStringID = ((uCmd < sizeof(idPopup)/sizeof(idPopup[0])) ? idPopup[uCmd] : 0);
	} else {
		// string ID == Command ID
		nStringID = nID;
	}
	if (nStringID != 0) {
		strText = CMGString(nStringID);
		strText += '.';
		UpdateStatusbar(strText, 0, 0);
	}
	return 0;
}
