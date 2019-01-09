/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     menu.cpp, menu function handler

 Version:   1.00

 Date:      3-Feb-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "..\rccom\rccom.h"
#include "types.h"
#include "display.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern CDisplay         g_Display;
extern CRCBox           g_RCBox;

extern HINSTANCE        g_hInst;
extern HWND             g_hWnd;
extern HWND             g_hWndToolbar;
extern HWND             g_hWndStatusbar;

extern BOOL             g_bAppPaused;
extern DWORD            g_bShowToolbar;
extern DWORD            g_bShowStatusbar;
extern DWORD            g_bUseGDI;

extern LPCTSTR          g_szHelpFile;

void InitScene();
void SetHaltState();

void UpdateToolbar();
void UpdateClientWindows();
BOOL SetDisplayMode( DWORD dwWidth, DWORD dwHeight, WORD bpp, BOOL bFullscreen );

void ShowDlg_About();

BOOL ShowDlg_FileNew();
BOOL ShowDlg_FileOpen();
BOOL ShowDlg_FileSave( BOOL SaveAs );
BOOL ShowDlg_FileProperties();

BOOL ShowDlg_Parameter( UINT nStartPage );
BOOL ShowDlg_Options( int nStartPage );

/////////////////////////////////////////////////////////////////////////////
// OnMenuCommand

BOOL OnMenuCommand( UINT nID )
{
    switch( nID ) {
        case IDM_FILE_NEW:
            ShowDlg_FileNew();
            break;
        case IDM_FILE_OPEN:
            ShowDlg_FileOpen();
            break;
        case IDM_FILE_SAVE:
            ShowDlg_FileSave( FALSE );
            break;
        case IDM_FILE_SAVEAS:
            ShowDlg_FileSave( TRUE );
            break;
        case IDM_FILE_PROPERTIES:
            ShowDlg_FileProperties();
            break;
        case IDM_FILE_EXIT:
            DestroyWindow( g_hWnd );
            break;

        case IDM_VIEW_TOOLBAR:
            g_bShowToolbar = !g_bShowToolbar;
            UpdateClientWindows();
            break;
        case IDM_VIEW_STATUSBAR:
            g_bShowStatusbar = !g_bShowStatusbar;
            UpdateClientWindows();
            break;
        case IDM_VIEW_FULLSCREEN:
            SetDisplayMode( 0, 0, 0, !g_Display.IsFullscreenMode() );
            SendMessage(
                g_hWndToolbar,
                TB_SETSTATE,
                IDM_VIEW_FULLSCREEN,
                MAKELONG( TBSTATE_ENABLED | ( g_Display.IsFullscreenMode() ? TBSTATE_CHECKED : 0 ),
                0 ) );
            break;
        case IDM_VIEW_USEGDI:
            g_bUseGDI = !g_bUseGDI;
            break;
        case IDM_VIEW_CHANGERES:
            ShowDlg_Options( 1 );
            break;

        case IDM_SIM_NEW:
            InitScene();
            break;
        case IDM_SIM_PAUSE:
            g_bAppPaused = !g_bAppPaused;
            SetHaltState();
            UpdateToolbar();
            break;

        case IDM_PAR_COMMON:
        case IDM_PAR_WIND:
        case IDM_PAR_AIRPLANE:
        case IDM_PAR_HELI:
            ShowDlg_Parameter( nID - IDM_PAR_COMMON );
            break;

        case IDM_EXTRAS_RCCOM:
            g_RCBox.Dialog( g_hWnd, 0 );
            break;
        case IDM_EXTRAS_OPTIONS:
            ShowDlg_Options( 0 );
            break;

        case IDM_HELP_INDEX:
            WinHelp( g_hWnd, g_szHelpFile, HELP_FINDER, 0L );
            break;
        case IDM_HELP_KEYS:
            WinHelp( g_hWnd, g_szHelpFile, HELP_CONTEXT, 100 );//IDH_KEYS );
            break;
        case IDM_HELP_USINGHELP:
            WinHelp( g_hWnd, "WINHLP32.HLP", HELP_HELPONHELP, 0L );
            break;
        case IDM_HELP_ABOUT:
            ShowDlg_About();
            break;
        default:
//            MessageBox( g_hWnd, "Menüpunkt noch nicht verfügbar!", "MFS", MB_OK );
            return FALSE;
    }
    // return TRUE to indicate that we have processed this message
    return TRUE;
}


