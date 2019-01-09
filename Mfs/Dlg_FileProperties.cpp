/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     Dlg_FileProperties.cpp

 Version:   1.00

 Date:      3-Feb-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "mgstring.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE        g_hInst;            // defined in winmain.cpp
extern HWND             g_hWnd;             // defined in winmain.cpp

extern CMGString          g_strSceneryName;
extern CMGString          g_strSceneryModel;
extern CMGString          g_strSceneryWorld;

/////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK PropertiesDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM )
{
    OPENFILENAME    ofn;
    char            szFile[256] = "";
    CMGString         strTitle;
    char            strFilter[256] = "Modelldateien (*.ifm)\0*.IFM\0Alle Dateien (*.*)\0*.*\0";

    switch( uMsg ) {
        case WM_INITDIALOG: {
            SetDlgItemText( hwndDlg, IDC_PROP_TITLE, g_strSceneryName );
            SetDlgItemText( hwndDlg, IDC_PROP_MODEL, g_strSceneryModel );
            SetDlgItemText( hwndDlg, IDC_PROP_WORLD, g_strSceneryWorld );
            break;
        }
        case WM_COMMAND: {
            switch( LOWORD( wParam ) ) {
                case IDOK: {
                    CHAR cBuf[256];
                    GetDlgItemText( hwndDlg, IDC_PROP_TITLE, cBuf, 256 );
                    g_strSceneryName = cBuf;
                    GetDlgItemText( hwndDlg, IDC_PROP_MODEL, cBuf, 256 );
                    g_strSceneryModel = cBuf;
                    GetDlgItemText( hwndDlg, IDC_PROP_WORLD, cBuf, 256 );
                    g_strSceneryWorld = cBuf;
/*                    FreeAllBuffers();
                    LoadScriptSets();*/
                    EndDialog( hwndDlg, TRUE );
                    break;
                }
                case IDCANCEL: {
                    EndDialog( hwndDlg, TRUE );
                    break;
                }
                case IDC_PROP_MODEL_BROWSE: {
                    strTitle = "Öffnen";
//                    strFilter = "Modelldateien (*.ifm)\0*.IFM\0Alle Dateien (*.*)\0*.*\0";

                    ofn.lStructSize         = sizeof(ofn);
                    ofn.hwndOwner           = hwndDlg;
                    ofn.lpstrFilter         = strFilter;
                    ofn.lpstrCustomFilter   = NULL;
                    ofn.nFilterIndex        = 1L;
                    ofn.lpstrFile           = szFile;
                    ofn.nMaxFile            = 256;
                    ofn.lpstrFileTitle      = NULL;
                    ofn.lpstrInitialDir     = CMGString( IDS_PATH_MODEL );
                    ofn.lpstrTitle          = strTitle;
                    ofn.Flags = OFN_LONGNAMES|OFN_HIDEREADONLY;
                    ofn.nFileOffset         = 0;
                    ofn.nFileExtension      = 0;
                    ofn.lpstrDefExt         = NULL;
                    ofn.lpfnHook            = NULL;
                    if( !GetOpenFileName( &ofn ) )
                        break;
                    SetDlgItemText( hwndDlg, IDC_PROP_MODEL, szFile );
                    break;
                }
            }
        }
        default:
            return FALSE;
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Properties dialog

BOOL ShowDlg_FileProperties()
{
    DialogBox( g_hInst, MAKEINTRESOURCE( IDD_PROPERTIES ), g_hWnd, (DLGPROC)PropertiesDialogProc );
    return TRUE;
}


