/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:         toolbox.cpp

 Version:       1.00

 Date:          13-Jan-97

 Author:        MG

\***************************************************************************/

#include "stdafx.h"

#include "mgregkey.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////

LPCSTR GetLastErrorString()
{
    static LPSTR pszLastError = NULL;

    DWORD   dwError = GetLastError();

    if (pszLastError != NULL) {
        LocalFree(pszLastError);
        pszLastError = NULL;
    }

    if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwError, 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&pszLastError,
            0,
            NULL) > 0)
        return pszLastError;
    else
        return "\0";
}

LPCSTR GetProgramDirectory()
{
    static CHAR acBuffer[ MAX_PATH ];

    int len = GetModuleFileName(NULL, acBuffer, sizeof acBuffer);

    if (len > 0 && acBuffer[0] != '\0') {
        for(int i=len; i>0; i--) {
            if (acBuffer[i] == '\\') {
                acBuffer[i] = '\0';
                return acBuffer;
            }
        }
    }

    CMGRegKey Reg;
    if (Reg.Open(HKLM, IDS_REGK_MFS) && Reg.QueryValue(acBuffer, sizeof(acBuffer), "Path"))
        return acBuffer;

    return NULL;
}

