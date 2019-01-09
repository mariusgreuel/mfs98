//
// instdrv.cpp : register RCCom device
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "scmanager.h"

static LPCTSTR szName           = TEXT("rccom");
static LPCTSTR szDisplayName    = TEXT("R/C Communication Driver");
static LPCTSTR szPath           = TEXT("\\SystemRoot\\system32\\drivers\\rccom.sys");

void eprint(LPCSTR pszText)
{
  LPSTR         lpLastError;

  FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(), 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpLastError,
        0,
        NULL);

  printf("%s: %s\n", pszText, lpLastError);

  if (lpLastError != NULL)
    LocalFree(lpLastError);
}

int main(int argc, char *argv[])
{
    BOOL        bInstall = TRUE;

    CSCManager  SCManager(szName );

    if (argc <= 1) {
		printf("Usage: InstDrv [a][r]\n");
		return 2;
	}
	if(*argv[1] == 'r')
        bInstall = FALSE;

    if (!SCManager.Open()) {
        eprint("Could not open SCManager");
        return 2;
    }

    if (bInstall) {
        if (!SCManager.InstallDriver(szDisplayName, szPath, SERVICE_AUTO_START)) {
            eprint("Could not install driver");
            return 2;
        }
        if (!SCManager.StartDriver()) {
            eprint("Could not start driver");
            return 2;
        }
        printf("%s installed successfully\n", szName);
    } else {
        if (!SCManager.StopDriver()) {
            eprint("Could not stop driver");
        }
        if (!SCManager.RemoveDriver()) {
            eprint("Could not remove driver");
            return 2;
        }
        printf("%s removed successfully\n", szName);
    }

    if (!SCManager.Close()) {
        eprint("Could not close SCManager");
        return 2;
    }

    return 0;
}
