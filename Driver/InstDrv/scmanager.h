/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     SCManager.h

 Version:   1.00

 Date:      07-Sep-97

 Author:    MG

****************************************************************************/

#ifndef __RCCOM_H_INCLUDED__
#define __RCCOM_H_INCLUDED__

class CSCManager {
public:
    CSCManager(IN LPCTSTR pszDriverName);
    ~CSCManager();

    BOOL Open(
            IN LPCTSTR  lpMachineName = NULL,
            IN LPCTSTR  lpDatabaseName = NULL,
            IN DWORD    dwDesiredAccess = SC_MANAGER_ALL_ACCESS);
    BOOL Close();

    BOOL InstallDriver(
            IN LPCTSTR  pszDisplayName,
            IN LPCTSTR  pszServiceExe,
            IN DWORD    dwStartType);
    BOOL RemoveDriver();

    BOOL StartDriver();
    BOOL StopDriver();

protected:
    BOOL OpenService();
    BOOL CloseService();

private:
    SC_HANDLE       m_schSCManager;
    SC_HANDLE       m_schService;

    LPCTSTR         m_pszDriverName;
};

#endif // __RCCOM_H_INCLUDED__



