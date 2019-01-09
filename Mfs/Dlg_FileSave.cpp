/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 Dlg_FileSave.cpp

 Version:   1.00

 Date:	  3-Feb-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "mgstring.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

#define LOGERROR MGDBG0

//MODULE(__FILE__);

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE		g_hInst;			// defined in winmain.cpp
extern HWND			 g_hWnd;			 // defined in winmain.cpp

extern CMGString		  g_strSceneryName;
extern BOOL			 g_bSceneryModified;

BOOL SaveScenery(LPCTSTR szScriptFile);

/////////////////////////////////////////////////////////////////////////////

BOOL ShowDlg_FileSave(BOOL bSaveAs)
{
	CMGString strFilter;
	CMGString strFilename;

strFilter = CMGString(IDS_FILTER_SCENE);

	if (bSaveAs || g_strSceneryName == "") {
		OPENFILENAME	ofn;
		char			szFilename[MAX_PATH] = "";
	
		ofn.lStructSize		 = sizeof(ofn);
		ofn.hwndOwner		   = g_hWnd;
		ofn.hInstance		   = g_hInst;
		ofn.lpstrFilter		 = "Szenarien (*.ifs)\0*.ifs\0Alle Dateien (*.*)\0*.*\0";
		ofn.lpstrCustomFilter   = (LPTSTR)NULL;
		ofn.nMaxCustFilter	  = 0L;
		ofn.nFilterIndex		= 1L;
		ofn.lpstrFile		   = szFilename;
		ofn.nMaxFile			= sizeof(szFilename);
		ofn.lpstrFileTitle	  = NULL;
		ofn.nMaxFileTitle	   = 0;
		ofn.lpstrInitialDir	 = NULL;
		ofn.lpstrTitle		  = "Speichern";
		ofn.nFileOffset		 = 0;
		ofn.nFileExtension	  = 0;
		ofn.lpstrDefExt		 = "*.ifs";
		ofn.lpfnHook			= NULL;
		ofn.Flags = OFN_LONGNAMES|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;

		if (!GetSaveFileName(&ofn)) {
			return FALSE;
		}
		strFilename = szFilename;
	} else {
		strFilename = g_strSceneryName;
	}
	
	if (!SaveScenery(strFilename)) {
		LOGERROR("Can't save scenery '%s'.\n", (LPCSTR)strFilename);
		return FALSE;
	}

	g_strSceneryName = strFilename;
	g_bSceneryModified = FALSE;

	return TRUE;
}


