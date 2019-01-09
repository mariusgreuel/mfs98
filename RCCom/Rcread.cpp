//
// rcread : displays the rc test box
//

#include "stdafx.h"

#include "rccom.h"

int ErrMsgBox(LPCSTR pszMsg, LPCSTR pszLastError)
{
    char    szBuf[512];
    LPSTR   lpLastError;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(), 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpLastError,
        0,
        NULL
       );

    wsprintf(szBuf, "%s\n%s\n%s", pszMsg, pszLastError, lpLastError);

    if (lpLastError != NULL)
        LocalFree(lpLastError);

    MessageBox(NULL, szBuf, "RCCOM", MB_OK | MB_ICONEXCLAMATION);

    return FALSE;
}


#ifndef __WATCOMC__
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
    CRCBox          RCBox;

    if (!RCBox.OpenDevice())
        return ErrMsgBox("Kann das Gerät RCCOM nicht öffnen:", RCBox.GetLastErrorString());

    RCBox.Initialize();

    RCBox.Dialog(NULL, 0);

    RCBox.Terminate();

    if (!RCBox.CloseDevice())
        return ErrMsgBox("Kann das Gerät RCCOM nicht schließen:", RCBox.GetLastErrorString());

    return TRUE;
}



