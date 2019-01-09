/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     rccom.cpp : rcbox communication virtual device driver

 Version:   1.00

 Date:      15-Jan-97

 Author:    MG

****************************************************************************/

#include "stdafx.h"

#include "mgstring.h"
#include "mgregkey.h"
#include "resource.h"
#include "rccom.h"

/////////////////////////////////////////////////////////////////////////////
// Globals...

LPCSTR                  g_szDeviceName95    = "\\\\.\\rccom.vxd";
LPCSTR                  g_szDeviceNameNT    = "\\\\.\\rccom";
LPCSTR                  g_szHelpFile        = "mfs.hlp";

HINSTANCE               g_hInst = NULL;

CALDATA                 g_CalData;

RCCOM_INFO              rci;

RCCOM_HARDWARE_INFO rchi = {

    0, 0, 0, 0, 0,
    { DEFAULT_SYNC_LEN, DEFAULT_CENTER_LEN, DEFAULT_DEVIATION_LEN,
      DEFAULT_MAX_SYNC_LEN, DEFAULT_IRQ_TIMEOUT, DEFAULT_SYNC_TIMEOUT },
    { DEFAULT_SYNC_LEN, DEFAULT_CENTER_LEN, DEFAULT_DEVIATION_LEN,
      DEFAULT_MAX_SYNC_LEN, DEFAULT_IRQ_TIMEOUT, DEFAULT_SYNC_TIMEOUT },
    0

};

static LPCSTR   szREGK_RCCOM        = "RCCom";
static LPCSTR   szREG_COMPORT       = "ComPort";
static LPCSTR   szREG_COMIRQ        = "ComIRQ";
static LPCSTR   szREG_INVPULSE      = "Inverted_Pulse";
static LPCSTR   szREG_SYNCLEN       = "PulseWidth_Sync";
static LPCSTR   szREG_CENTERLEN     = "PulseWidth_Center";
static LPCSTR   szREG_DEVIATIONLEN  = "PulseWidth_Deviation";
static LPCSTR   szREG_MAXSYNCLEN    = "PulseWidth_SyncMax";
static LPCSTR   szREG_IRQTIMEOUT    = "TimeOut_IRQ";
static LPCSTR   szREG_SYNCTIMEOUT   = "TimeOut_Sync";
static LPCSTR   szREG_CALDATA       = "CalibrationData";

/////////////////////////////////////////////////////////////////////////////

void InitCalData(PCALDATA pCalData)
{
    LONG k = -MAX_STICK_VALUE * 65536 / (LONG)rchi.ttv.uDeviationLen;

    for(int i=0; i<MAX_CHANNELS; i++) {
        rci.RawValue[i] = 0;
        pCalData->CHStick[i] = i;
        pCalData->Zero[i] = rchi.ttv.uCenterLen;
        pCalData->NFac[i] = k;
        pCalData->PFac[i] = k;
    }
}

/////////////////////////////////////////////////////////////////////////////
// registry stuff

BOOL ValidateConfig()
{
    BOOL    bConfigChanged = FALSE;

    if (rchi.uComPort > 15) {
        rchi.uComPort = 0;
        bConfigChanged = TRUE;
    }

    if (rchi.uComIrq == 1 || rchi.uComIrq > 15) {
        rchi.uComIrq = 0;
        bConfigChanged = TRUE;
    }

    if (rchi.tv.uSyncLen < DEFAULT_SYNC_LEN_RANGE_MIN ||
        rchi.tv.uSyncLen > DEFAULT_SYNC_LEN_RANGE_MAX) {

        rchi.tv.uSyncLen = DEFAULT_SYNC_LEN;
        bConfigChanged = TRUE;
    }

    if (rchi.tv.uCenterLen < DEFAULT_CENTER_LEN_RANGE_MIN ||
        rchi.tv.uCenterLen > DEFAULT_CENTER_LEN_RANGE_MAX) {

        rchi.tv.uCenterLen = DEFAULT_CENTER_LEN;
        bConfigChanged = TRUE;
    }

    if (rchi.tv.uDeviationLen < DEFAULT_DEVIATION_LEN_RANGE_MIN ||
        rchi.tv.uDeviationLen > DEFAULT_DEVIATION_LEN_RANGE_MAX) {

        rchi.tv.uDeviationLen = DEFAULT_DEVIATION_LEN;
        bConfigChanged = TRUE;
    }

    if (bConfigChanged) {
        rchi.tv.uMaxSyncLen = DEFAULT_MAX_SYNC_LEN;
        rchi.tv.uIrqTimeOut = DEFAULT_IRQ_TIMEOUT;
        rchi.tv.uSyncTimeOut = DEFAULT_SYNC_TIMEOUT;
        return FALSE;
    }

    return TRUE;
}

// read and vaidate registry data
BOOL ReadRegistryData()
{
    CMGRegKey Reg;

    CMGString strKey = CMGString(IDS_REGK_MFS) + '\\' +
                     CMGString(IDS_REGK_MFS_VER) + '\\' +
                     CMGString(szREGK_RCCOM);

    if (Reg.Open(HKCU, strKey) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.uComPort, szREG_COMPORT) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.uComIrq, szREG_COMIRQ) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.bInvertedPulse, szREG_INVPULSE) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.tv.uSyncLen, szREG_SYNCLEN) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.tv.uCenterLen, szREG_CENTERLEN) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.tv.uDeviationLen, szREG_DEVIATIONLEN) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.tv.uMaxSyncLen, szREG_MAXSYNCLEN) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.tv.uIrqTimeOut, szREG_IRQTIMEOUT) != ERROR_SUCCESS ||
        Reg.QueryValue(rchi.tv.uSyncTimeOut, szREG_SYNCTIMEOUT) != ERROR_SUCCESS ||
        Reg.QueryValue(&g_CalData, sizeof(CALDATA), szREG_CALDATA) != ERROR_SUCCESS)
	   return FALSE;

    if (!ValidateConfig())
        return FALSE;

    return TRUE;
}

VOID SaveRegistryData()
{
    CMGRegKey Reg;

    CMGString strKey = CMGString(IDS_REGK_MFS) + '\\' +
                     CMGString(IDS_REGK_MFS_VER) + '\\' +
                     CMGString(szREGK_RCCOM);

    if (Reg.Create(HKCU, strKey) != ERROR_SUCCESS)
        return;

    Reg.SetValue(rchi.uComPort, szREG_COMPORT);
    Reg.SetValue(rchi.uComIrq, szREG_COMIRQ);
    Reg.SetValue(rchi.bInvertedPulse, szREG_INVPULSE);
    Reg.SetValue(rchi.tv.uSyncLen, szREG_SYNCLEN);
    Reg.SetValue(rchi.tv.uCenterLen, szREG_CENTERLEN);
    Reg.SetValue(rchi.tv.uDeviationLen, szREG_DEVIATIONLEN);
    Reg.SetValue(rchi.tv.uMaxSyncLen, szREG_MAXSYNCLEN);
    Reg.SetValue(rchi.tv.uIrqTimeOut, szREG_IRQTIMEOUT);
    Reg.SetValue(rchi.tv.uSyncTimeOut, szREG_SYNCTIMEOUT);
    Reg.SetValue(&g_CalData, sizeof(CALDATA), szREG_CALDATA);
}

/////////////////////////////////////////////////////////////////////////////
// RCCom implementation

BOOL ValidateChannelData(ULONG cv)
{
    ULONG minval = rchi.ttv.uCenterLen - rchi.ttv.uDeviationLen - rchi.ttv.uDeviationLen / 8;
    ULONG maxval = rchi.ttv.uCenterLen + rchi.ttv.uDeviationLen + rchi.ttv.uDeviationLen / 8;

    if (cv == INVALID_CHANNEL_VALUE || cv < minval || cv > maxval)
        return FALSE;

    return TRUE;
}

LONG ConvertChannelData(BYTE ch)
{
    static LONG     LastCH[ MAX_CHANNELS ];

    if (!ValidateChannelData(rci.RawValue[ch]))
        return 0;

    LONG f = (LONG)rci.RawValue[ch] - (LONG)g_CalData.Zero[ch];

    if (f >= 0)
        f = f * g_CalData.PFac[ch] + 32768 >> 16;
    else
        f = f * g_CalData.NFac[ch] + 32768 >> 16;

    // apply filter for small values
    if (-5 <= f    && f <= 5) {

        LastCH[ch] = LastCH[ch] * 7 / 8 + f;
        f = LastCH[ch] / 8;

    } else {

        // for large value -> set filter value to zero
        if (f > 0)
            LastCH[ch] = 5*8;
        else
            LastCH[ch] = -5*8;

    }

    // limit data range
    if (f < -MAX_STICK_VALUE)
        return -MAX_STICK_VALUE;
    else if (f > MAX_STICK_VALUE)
        return MAX_STICK_VALUE;

    return f;
}

/////////////////////////////////////////////////////////////////////////////
// RCCom exported functions

__declspec(dllexport) CRCBox::CRCBox()
{
    m_hDevice       = INVALID_HANDLE_VALUE;
    m_bInitialized  = FALSE;

    m_dwLastError   = RCERR_SUCCESS;
}

__declspec(dllexport) CRCBox::~CRCBox()
{
}

__declspec(dllexport) BOOL CRCBox::OpenDevice()
{
    BOOL        bStatus;
    DWORD       nBytesRet;
    ULONG       uVersion;
    BOOL        bNeedToInitCalData;

    if (m_hDevice != INVALID_HANDLE_VALUE) {
        m_dwLastError = RCERR_CANT_LOAD_DEVICE;
        return FALSE;
    }

    bNeedToInitCalData = !ReadRegistryData();

    // check version: Windows 95/NT required
    OSVERSIONINFO vi;	
    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&vi);

    if (vi.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS &&
        vi.dwPlatformId != VER_PLATFORM_WIN32_NT) {

        m_dwLastError = RCERR_NOWIN32;
        return FALSE;
    }

    if (vi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        m_hDevice = CreateFile(
			g_szDeviceName95,
			0,
			0,
			NULL,
			CREATE_NEW,
			FILE_FLAG_DELETE_ON_CLOSE,
			NULL);
    } else {
        m_hDevice = CreateFile(
			g_szDeviceNameNT,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_DELETE_ON_CLOSE,
			NULL);
    }

    if (m_hDevice == INVALID_HANDLE_VALUE) {
        m_dwLastError = RCERR_CANT_LOAD_DEVICE;
        return FALSE;
    }

    // get version code
    bStatus = DeviceIoControl(
                  m_hDevice,
                  IOCTL_RCCOM_GET_VERSION,
                  NULL,
                  0,
                  &uVersion,
                  sizeof(uVersion),
                  &nBytesRet,
                  NULL
                 );

    if (!bStatus) {
        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
        m_dwLastError = RCERR_UNKNOWN;
        return FALSE;
    }

    // check version code
    if (uVersion != RCCOM_VERSION) {
        m_dwLastError = RCERR_BAD_VERSION;
        return FALSE;
    }

    bStatus = SetHardwareInfo(&rchi);

    if (!bStatus) {
        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
        m_dwLastError = RCERR_UNKNOWN;
        return FALSE;
    }

    bStatus = GetHardwareInfo(&rchi);

    if (!bStatus) {
        CloseHandle(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
        m_dwLastError = RCERR_UNKNOWN;
        return FALSE;
    }

    if (bNeedToInitCalData)
        InitCalData(&g_CalData);

    m_dwLastError = RCERR_SUCCESS;
    
	return TRUE;
}

__declspec(dllexport) BOOL CRCBox::CloseDevice()
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    CloseHandle(m_hDevice);
    m_hDevice = INVALID_HANDLE_VALUE;

    SaveRegistryData();

    m_dwLastError = RCERR_SUCCESS;

    return TRUE;
}

__declspec(dllexport) BOOL CRCBox::Initialize()
{
    BOOL        bStatus;
    DWORD       nBytesRet;

    if (m_hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    // if device already loaded return success
    if (m_bInitialized) {
        m_dwLastError = RCERR_SUCCESS;
        return TRUE;
    }

    bStatus = DeviceIoControl(
		m_hDevice,
		IOCTL_RCCOM_INITIALIZE,
		NULL,
		0,
		NULL,
		0,
		&nBytesRet,
		NULL);

    if (!bStatus) {
        m_dwLastError = RCERR_UNKNOWN;
        return FALSE;
    }

    m_dwLastError = ReadLastError();

    if (m_dwLastError != RCERR_SUCCESS) {
		Sleep(100);
        return FALSE;
	}

    m_bInitialized = TRUE;

    return TRUE;
}

__declspec(dllexport) BOOL CRCBox::Terminate()
{
    return TRUE;
}

__declspec(dllexport) BOOL CRCBox::GetHardwareInfo(PRCCOM_HARDWARE_INFO prchi)
{
    BOOL        bStatus;
    DWORD       nBytesRet;

    // get hardware info
    bStatus = DeviceIoControl(
		m_hDevice,
		IOCTL_RCCOM_GET_HARDWARE_INFO,
		NULL,
		0,
		prchi,
		sizeof(rchi),
		&nBytesRet,
		NULL);

    if (bStatus == FALSE || nBytesRet != sizeof(rchi)) {
        m_dwLastError = RCERR_UNKNOWN;
        return FALSE;
    }

    m_dwLastError = ReadLastError();

    if (m_dwLastError != RCERR_SUCCESS)
        return FALSE;

    return TRUE;
}

__declspec(dllexport) BOOL CRCBox::SetHardwareInfo(PRCCOM_HARDWARE_INFO prchi)
{
    BOOL        bStatus;
    DWORD       nBytesRet;

    // set hardware info
    bStatus = DeviceIoControl(
                  m_hDevice,
                  IOCTL_RCCOM_SET_HARDWARE_INFO,
                  prchi,
                  sizeof(rchi),
                  NULL,
                  0,
                  &nBytesRet,
                  NULL
                 );

    if (bStatus == FALSE) {
        m_dwLastError = RCERR_UNKNOWN;
        return FALSE;
    }

    m_dwLastError = ReadLastError();

    if (m_dwLastError != RCERR_SUCCESS)
        return FALSE;

    return TRUE;
}

__declspec(dllexport) BOOL CRCBox::Read()
{
    BOOL        bStatus;
    DWORD       nBytesRet;

    if (m_hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    if (!m_bInitialized && !Initialize())
        return FALSE;

    // read channel data
    bStatus = DeviceIoControl(
             m_hDevice,
             IOCTL_RCCOM_READ,
             NULL,
             0,
             &rci,
             sizeof(RCCOM_INFO),
             &nBytesRet,
             NULL);

    if (!bStatus || nBytesRet != sizeof(RCCOM_INFO)) {
        m_dwLastError = RCERR_UNKNOWN;
        return FALSE;
    }

    m_dwLastError = ReadLastError();

    if (m_dwLastError != RCERR_SUCCESS)
        return FALSE;

    return TRUE;
}

__declspec(dllexport) LONG CRCBox::GetPos(UINT Stick)
{
    if (!m_bInitialized)
        return FALSE;

    m_dwLastError = RCERR_SUCCESS;

    // get channel that is assigned to the stick
    return ConvertChannelData(g_CalData.CHStick[ Stick ]);
}


__declspec(dllexport) DWORD CRCBox::GetLastError()
{
    return m_dwLastError;
}

__declspec(dllexport) LPCSTR CRCBox::GetLastErrorString()
{
    static CMGString  strErrorText;

    switch (m_dwLastError) {
        case RCERR_NOT_FOUND_ON_PORTX:
        case RCERR_PORT_NOT_AVAILABLE:
        case RCERR_PORT_ALREADY_CLAIMED:
        case RCERR_PORT_ACQUIRE_ERROR:
        case RCERR_IRQ_VIRTUALIZE_ERROR:
            strErrorText.Format(IDS_RCERR_OK + m_dwLastError,
                rchi.uComPort, rchi.uPhysicalPort, rchi.uPhysicalIrq);
            break;
        default:
            strErrorText = CMGString(IDS_RCERR_OK + m_dwLastError);
    }

    return strErrorText;
}

DWORD CRCBox::ReadLastError()
{
    BOOL        bStatus;
    DWORD       nBytesRet;
    DWORD       dwLastError;

    if (m_hDevice == INVALID_HANDLE_VALUE)
        return RCERR_UNKNOWN;

    // read last error
    bStatus = DeviceIoControl(
                  m_hDevice,
                  IOCTL_RCCOM_GET_LAST_ERROR,
                  NULL,
                  0,
                  &dwLastError,
                  sizeof(dwLastError),
                  &nBytesRet,
                  NULL);

    if (!bStatus || nBytesRet != sizeof(dwLastError)) {
        return RCERR_UNKNOWN;
    }

    return dwLastError;
}

/////////////////////////////////////////////////////////////////////////////
// DllMain

BOOL PASCAL DllMain(HINSTANCE hDLLInst, DWORD fdwReason, LPVOID)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hDLLInst;
		CMGString::SetModuleHandle(hDLLInst);
	}

    return 1;         // success
}


