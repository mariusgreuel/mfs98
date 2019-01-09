/****************************************************************************

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:		rccom.h : R/C communication device driver

  Version:	  1.00

  Date:		 9-Jan-97

  Author:	   MG

  Remarks:	  all values have to be synchronized with rccom.inc

****************************************************************************/

#ifndef __RCCOM_H_INCLUDED__
#define __RCCOM_H_INCLUDED__

#pragma pack(1)

#ifdef _NTDDK_
#include <devioctl.h>
#else
#include <winioctl.h>
#endif

#define RCCOM_VERSION					0x101

#define MAX_CHANNELS					10
#define MAX_STICK_VALUE					128
#define INVALID_CHANNEL_VALUE			-1

// the following values are pulse lenght in us
#define DEFAULT_SYNC_LEN				3000
#define DEFAULT_SYNC_LEN_RANGE_MIN		2000
#define DEFAULT_SYNC_LEN_RANGE_MAX		8000

#define DEFAULT_CENTER_LEN				1600
#define DEFAULT_CENTER_LEN_RANGE_MIN	(DEFAULT_CENTER_LEN-500)
#define DEFAULT_CENTER_LEN_RANGE_MAX	(DEFAULT_CENTER_LEN+500)

#define DEFAULT_DEVIATION_LEN			550
#define DEFAULT_DEVIATION_LEN_RANGE_MIN (DEFAULT_DEVIATION_LEN-200)
#define DEFAULT_DEVIATION_LEN_RANGE_MAX (DEFAULT_DEVIATION_LEN+200)

#define DEFAULT_MAX_SYNC_LEN			20000
#define DEFAULT_IRQ_TIMEOUT				25000
#define DEFAULT_SYNC_TIMEOUT			200000

#define FILE_DEVICE_RCCOM				0x8FF

#define IOCTL_RCCOM_GET_VERSION			CTL_CODE(FILE_DEVICE_RCCOM,0x0800,METHOD_BUFFERED,FILE_READ_ACCESS)
#define IOCTL_RCCOM_GET_LAST_ERROR		CTL_CODE(FILE_DEVICE_RCCOM,0x0801,METHOD_BUFFERED,FILE_READ_ACCESS)
#define IOCTL_RCCOM_INITIALIZE			CTL_CODE(FILE_DEVICE_RCCOM,0x0802,METHOD_BUFFERED,FILE_READ_ACCESS)
#define IOCTL_RCCOM_TERMINATE			CTL_CODE(FILE_DEVICE_RCCOM,0x0803,METHOD_BUFFERED,FILE_READ_ACCESS)
#define IOCTL_RCCOM_GET_HARDWARE_INFO	CTL_CODE(FILE_DEVICE_RCCOM,0x0804,METHOD_BUFFERED,FILE_READ_ACCESS)
#define IOCTL_RCCOM_SET_HARDWARE_INFO	CTL_CODE(FILE_DEVICE_RCCOM,0x0805,METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_RCCOM_READ				CTL_CODE(FILE_DEVICE_RCCOM,0x0806,METHOD_BUFFERED,FILE_READ_ACCESS)

#define RCERR_SUCCESS					0
#define RCERR_UNKNOWN					1
#define RCERR_CANT_LOAD_DEVICE			2
#define RCERR_BAD_VERSION				3
#define RCERR_NO_DATA					4
#define RCERR_BAD_DATA					5
#define RCERR_NOT_FOUND					6
#define RCERR_NOT_FOUND_ON_PORTX		7
#define RCERR_PORT_NOT_AVAILABLE		8
#define RCERR_PORT_ALREADY_CLAIMED		9
#define RCERR_PORT_ACQUIRE_ERROR		10
#define RCERR_IRQ_VIRTUALIZE_ERROR		11
#define RCERR_NOWIN32					-1

typedef struct _TIMING_VALUES
{
	ULONG		uSyncLen;
	ULONG		uCenterLen;
	ULONG		uDeviationLen;
	ULONG		uMaxSyncLen;
	ULONG		uIrqTimeOut;
	ULONG		uSyncTimeOut;
}TIMING_VALUES;

typedef struct _RCCOM_HARDWARE_INFO
{
	ULONG			uComPort;			// 0=auto, 1-256
	ULONG			uComIrq;			// 0=auto, 2-16
	ULONG			uPhysicalPort;
	ULONG			uPhysicalIrq;
	ULONG			bInvertedPulse;
	TIMING_VALUES   tv;					// in micro seconds
	TIMING_VALUES   ttv;				// translated tv (in timer ticks a` 838ns)
	LARGE_INTEGER	CounterFrequency;
}RCCOM_HARDWARE_INFO, *PRCCOM_HARDWARE_INFO;

typedef struct _RCCOM_INFO
{
	ULONG			bUpdated;
	ULONG			uCurrentChannel;
	ULONG			RawValue[ MAX_CHANNELS ];
}RCCOM_INFO, *PRCCOM_INFO;

// neglect this for the device driver source code
#ifndef _NTDDK_

typedef struct _CALDATA {
	BYTE		CHStick[ MAX_CHANNELS ];	// stick assigned to channel
	ULONG		Zero[ MAX_CHANNELS ];		// pulse lenght at middle posistions
	LONG		NFac[ MAX_CHANNELS ];		// factor for negative pulse lenght
	LONG		PFac[ MAX_CHANNELS ];		// factor for negative pulse lenght
}CALDATA, *PCALDATA;

class CRCBox {
public:
	__declspec(dllexport) CRCBox();
	__declspec(dllexport) ~CRCBox();

	__declspec(dllexport) BOOL OpenDevice();
	__declspec(dllexport) BOOL CloseDevice();

	__declspec(dllexport) BOOL Initialize();
	__declspec(dllexport) BOOL Terminate();

	__declspec(dllexport) BOOL GetHardwareInfo(PRCCOM_HARDWARE_INFO pHWI);
	__declspec(dllexport) BOOL SetHardwareInfo(PRCCOM_HARDWARE_INFO pHWI);

	__declspec(dllexport) BOOL Read();
	__declspec(dllexport) LONG GetPos(UINT uStick);

	__declspec(dllexport) ULONG GetLastError();
	__declspec(dllexport) LPCSTR GetLastErrorString();

	__declspec(dllexport) BOOL Dialog(HWND hWnd = NULL, int nStartPage = 0);

protected:
	ULONG ReadLastError();

private:
	HANDLE		m_hDevice;
	BOOL		m_bInitialized;
	DWORD		m_dwLastError;
};

#endif // _NTDDK_

#pragma pack()

#endif // __RCCOM_H_INCLUDED__


