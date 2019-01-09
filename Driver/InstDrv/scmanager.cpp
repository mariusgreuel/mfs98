/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     SCManager.cpp

 Version:   1.00

 Date:      07-Sep-97

 Author:    MG

****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "scmanager.h"

/////////////////////////////////////////////////////////////////////////////

CSCManager::CSCManager(IN LPCTSTR DriverName)
{
    m_schSCManager = m_schService = NULL;

    m_pszDriverName = DriverName;
}

CSCManager::~CSCManager()
{
    if (m_schSCManager != NULL)
        Close();
}

BOOL
CSCManager::Open(
    IN LPCTSTR  lpMachineName,
    IN LPCTSTR  lpDatabaseName,
    IN DWORD    dwDesiredAccess)
{
    if (m_schSCManager != NULL)
        return FALSE;

    m_schSCManager = OpenSCManager(lpMachineName, lpDatabaseName, dwDesiredAccess);

    if (m_schSCManager == NULL)
        return FALSE;
    
    return TRUE;
}


BOOL
CSCManager::Close()
{
    return CloseServiceHandle(m_schSCManager);
}


BOOL
CSCManager::InstallDriver(
    IN LPCTSTR  pszDisplayName,
    IN LPCTSTR  pszServiceExe,
    IN DWORD    dwStartType)
{
    m_schService = CreateService (
            m_schSCManager,         // SCManager database
            m_pszDriverName,        // name of service
            pszDisplayName,         // name to display
            SERVICE_ALL_ACCESS,     // desired access
            SERVICE_KERNEL_DRIVER,  // service type
            dwStartType,            // start type
            SERVICE_ERROR_NORMAL,   // error control type
            pszServiceExe,          // service's binary
            NULL,                   // no load ordering group
            NULL,                   // no tag identifier
            NULL,                   // no dependencies
            NULL,                   // LocalSystem account
            NULL                    // no password
           );

    if (m_schService == NULL) {
        return FALSE;
    }

    CloseService();

    return TRUE;
}


BOOL
CSCManager::RemoveDriver()
{
    if (!OpenService())
        return FALSE;

    return DeleteService(m_schService);
}


BOOL
CSCManager::StartDriver()
{
    BOOL       ret;

    if (!OpenService())
        return FALSE;

    ret = StartService(m_schService, 0, NULL);

    CloseService();

    return ret;
}



BOOL
CSCManager::StopDriver()
{
    BOOL            ret;
    SERVICE_STATUS  serviceStatus;

    if (!OpenService())
        return FALSE;

    ret = ControlService(m_schService, SERVICE_CONTROL_STOP, &serviceStatus);

    CloseService();

    return ret;
}


BOOL
CSCManager::OpenService()
{
    m_schService = ::OpenService(m_schSCManager, m_pszDriverName, SERVICE_ALL_ACCESS);
    return m_schService != NULL ? TRUE : FALSE;
}


BOOL
CSCManager::CloseService()
{
    return CloseServiceHandle(m_schService);
}


