/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 Dlg_FileOpen.cpp

 Version:   1.00

 Date:	  3-Feb-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "mgstring.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

#define LOGERROR(x) MGDBG0(DMSG_ERROR,x)
#define LOGAPIERROR(x) MGDBG0(DMSG_APIERROR,x)

//MODULE(__FILE__);

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE		g_hInst;			// defined in winmain.cpp
extern HWND			 g_hWnd;			 // defined in winmain.cpp

extern CMGString		  g_strSceneryName;
extern BOOL			 g_bSceneryModified;

BOOL LoadScenery(LPCSTR szScriptFile);

/////////////////////////////////////////////////////////////////////////////

BOOL ShowDlg_FileOpen()
{
	OPENFILENAME	ofn;
	char			szFilename[256] = "";

	ofn.lStructSize		 = sizeof(ofn);
	ofn.hwndOwner		   = g_hWnd;
	ofn.hInstance		   = g_hInst;
	ofn.lpstrFilter		 = "Scenarien (*.ifs)\0*.ifs\0Alle Dateien (*.*)\0*.*\0";
	ofn.lpstrCustomFilter   = (LPTSTR)NULL;
	ofn.nMaxCustFilter	  = 0L;
	ofn.nFilterIndex		= 1L;
	ofn.lpstrFile		   = szFilename;
	ofn.nMaxFile			= 256;
	ofn.lpstrFileTitle	  = NULL;
	ofn.nMaxFileTitle	   = 0;
	ofn.lpstrInitialDir	 = NULL;
	ofn.lpstrTitle		  = "Öffnen";
	ofn.nFileOffset		 = 0;
	ofn.nFileExtension	  = 0;
	ofn.lpstrDefExt		 = "*.ifs";
	ofn.lpfnHook			= NULL;
	ofn.Flags = OFN_LONGNAMES|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

	if (!GetOpenFileName(&ofn)) {
		return FALSE;
	}
	if (!LoadScenery(szFilename)) {
		LOGERROR("Can't open scenery '%s'.\n", szFilename);
		return FALSE;
	}
	g_strSceneryName = szFilename;
	g_bSceneryModified = FALSE;
	return TRUE;
}


