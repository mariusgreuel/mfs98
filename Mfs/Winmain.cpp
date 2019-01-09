/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     winmain.cpp

 Version:   1.00

 Date:      13-Jan-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE        g_hInst;
extern LPCTSTR          g_szAppClass;
extern LPCTSTR          g_szAppTitle;

int MfsMain(int nCmdShow);

/////////////////////////////////////////////////////////////////////////////
// WinMain

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{
    // set global handle
    g_hInst = hInst;

    // switch to Mfs window, if already started
    HWND hWnd;
    if ((hWnd = FindWindow(g_szAppClass, NULL)) != NULL) {
        if (SetForegroundWindow(hWnd)) {
            ShowWindow(hWnd, SW_NORMAL);
            return 0;
        }
    }

    return MfsMain(nCmdShow);
}


