/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:         toolbar.cpp

 Version:       1.00

 Date:          13-Jan-97

 Author:        MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

MGMODULE(__FILE__);

static TBBUTTON tbButton[] =
{
    { 0, IDM_FILE_NEW,          TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 1, IDM_FILE_OPEN,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 2, IDM_FILE_SAVE,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 0, 0,                     TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0 },
    { 3, IDM_FILE_PROPERTIES,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 0, 0,                     TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0 },
    { 4, IDM_SIM_PAUSE,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 5, IDM_VIEW_FULLSCREEN,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 0, 0,                     TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0 },
    { 6, IDM_EXTRAS_RCCOM,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 8, IDM_EXTRAS_VIDREC,     TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_BUTTON, 0, 0 },
    { 7, IDM_EXTRAS_SOUND,      TBSTATE_ENABLED|TBSTATE_HIDDEN, TBSTYLE_CHECK,  0, 0 },
    { 0, 0,                     TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0 },
    { 9, IDM_HELP_ABOUT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
    { 10, IDM_HELP_ARROW,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
};

/////////////////////////////////////////////////////////////////////////////
// Globals...

HWND                    g_hWndToolbar;


/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE        g_hInst;
extern BOOL             g_bAppPaused;


/////////////////////////////////////////////////////////////////////////////

BOOL CreateToolbar(HWND hwndParent, BOOL bVisible)
{
    g_hWndToolbar = CreateToolbarEx(
            hwndParent,
            WS_CHILD | TBSTYLE_TOOLTIPS | (bVisible ? WS_VISIBLE : 0),
            IDR_TOOLBAR,
            sizeof(tbButton)/sizeof(TBBUTTON),
            g_hInst,
            IDB_TOOLBAR,
            tbButton,
            sizeof(tbButton)/sizeof(TBBUTTON),
            0,0,18,17,
            sizeof(TBBUTTON));
    if (g_hWndToolbar == NULL) {
        MGDBG0(DMSG_APIERROR, "CreateToolbarEx failed\n");
        return FALSE;
    }
    return TRUE;
}


LRESULT MsgNotifyToolbar(HWND, WPARAM, LPARAM lParam)
{
    LPTOOLTIPTEXT lpToolTipText;

    lpToolTipText = (LPTOOLTIPTEXT)lParam;
    if (lpToolTipText->hdr.code == TTN_NEEDTEXT) {
        // string ID == command ID
        lpToolTipText->lpszText = (LPTSTR)lpToolTipText->hdr.idFrom;
        lpToolTipText->hinst = g_hInst;
    }
    return 0;
}


void UpdateToolbar()
{
    SendMessage(
            g_hWndToolbar,
            TB_CHANGEBITMAP,
            IDM_SIM_PAUSE,
            MAKELONG((!g_bAppPaused ? 4 : 11), 0));
}


