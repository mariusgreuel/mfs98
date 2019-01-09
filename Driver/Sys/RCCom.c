/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:			rccom.c : rcbox communication device driver

 Version:		1.00

 Date:			17-Aug-97

 Author:		MG

 Environment:	NT Kernel mode only

****************************************************************************/

#include <ntddk.h>
#include "uart.h"
#include "..\..\rccom\rccom.h"

/////////////////////////////////////////////////////////////////////////////

#define NT_DEVICE_NAME		L"\\Device\\rccom"
#define DOS_DEVICE_NAME		L"\\DosDevices\\rccom"

/////////////////////////////////////////////////////////////////////////////

typedef struct _LOCAL_DEVICE_INFO
{
	PDRIVER_OBJECT		DriverObject;
	PDEVICE_OBJECT		DeviceObject;
	PKINTERRUPT			InterruptObject;

	BOOLEAN				bInitialized;
	ULONG				OpenCount;

	ULONG				LastError;
	ULONG				IrqCounter;

	UART_STATE			UartState;

	ULONG				Port;
	ULONG				Irq;
	PUCHAR				PhysPort;

	RCCOM_HARDWARE_INFO rchi;
	RCCOM_INFO			rci;

	LARGE_INTEGER		TimeOfLastIrq;
	LARGE_INTEGER		TimeOfLastSync;
}LOCAL_DEVICE_INFO, *PLOCAL_DEVICE_INFO;

/////////////////////////////////////////////////////////////////////////////
// Prototypes

NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	);

VOID RCCOM_Unload(
	IN PDRIVER_OBJECT DriverObject
	);

NTSTATUS RCCOM_Dispatch(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP pIrp
	);

NTSTATUS RCCOM_Initialize(
	IN PDRIVER_OBJECT DriverObject
	);
NTSTATUS RCCOM_Terminate(
	IN PDRIVER_OBJECT DriverObject
	);

NTSTATUS AutoDetectInterface(
	IN PDRIVER_OBJECT DriverObject
	);
NTSTATUS IsInterfacePresent(
	IN PDRIVER_OBJECT DriverObject
	);

NTSTATUS AllocateResources(
	IN PDRIVER_OBJECT DriverObject
	);
NTSTATUS FreeResources(
	IN PDRIVER_OBJECT DriverObject
	);

NTSTATUS ReportResourceUsage(
	IN PDRIVER_OBJECT DriverObject,
	IN PHYSICAL_ADDRESS PortAddress,
	IN ULONG Irq
	);

BOOLEAN RccomInterruptServiceRoutine(
	IN PKINTERRUPT pInterrupt,
	IN OUT PVOID pContext
	);

VOID InitTimingValues(
	PLOCAL_DEVICE_INFO pLDI
	);
VOID TranslateTimingValues(
	PLOCAL_DEVICE_INFO pLDI
	);
BOOLEAN CheckTimeOuts(
	PLOCAL_DEVICE_INFO pLDI
	);


/////////////////////////////////////////////////////////////////////////////
// DriverEntry

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	)
{
	PDEVICE_OBJECT		DeviceObject = NULL;
	NTSTATUS			ntStatus;
	UNICODE_STRING		uniNtNameString;
	UNICODE_STRING		uniWin32NameString;
	PLOCAL_DEVICE_INFO	pLDI;
	int					i;

	KdPrint(("RCCOM: Entering DriverEntry.\n"));

	DriverObject->MajorFunction[IRP_MJ_CREATE] =
	DriverObject->MajorFunction[IRP_MJ_CLOSE] =
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RCCOM_Dispatch;
	DriverObject->DriverUnload = RCCOM_Unload;

	RtlInitUnicodeString(&uniNtNameString, NT_DEVICE_NAME);

	ntStatus = IoCreateDevice(
		DriverObject,
		sizeof(LOCAL_DEVICE_INFO),
		&uniNtNameString,
		FILE_DEVICE_RCCOM,
		0,
		FALSE,
		&DeviceObject
		);

	if (!NT_SUCCESS(ntStatus)) {
		KdPrint(("RCCOM: Couldn't create the device\n"));
		return ntStatus;
	}

	RtlInitUnicodeString(&uniWin32NameString, DOS_DEVICE_NAME);

	ntStatus = IoCreateSymbolicLink(&uniWin32NameString, &uniNtNameString);

	if (!NT_SUCCESS(ntStatus)) {
		KdPrint(("RCCOM: Couldn't create the symbolic link\n"));
		IoDeleteDevice(DriverObject->DeviceObject);
		return ntStatus;
	}

	KdPrint(("RCCOM: just about ready!\n"));

	pLDI = (PLOCAL_DEVICE_INFO)DeviceObject->DeviceExtension;
	RtlZeroMemory(pLDI, sizeof(LOCAL_DEVICE_INFO));

	pLDI->DriverObject = DriverObject;

	InitTimingValues(pLDI);
	TranslateTimingValues(pLDI);

	// clear channels
	for(i=0; i<MAX_CHANNELS; i++)
		pLDI->rci.RawValue[ i ] = (ULONG)INVALID_CHANNEL_VALUE;

	return ntStatus;

}


VOID RCCOM_Unload(
	IN PDRIVER_OBJECT DriverObject
	)
{
	UNICODE_STRING		uniWin32NameString;
	PLOCAL_DEVICE_INFO	pLDI;

	KdPrint(("RCCOM: Unload\n"));

	pLDI = (PLOCAL_DEVICE_INFO)DriverObject->DeviceObject->DeviceExtension;

	if (pLDI->bInitialized && pLDI->OpenCount > 0)
		RCCOM_Terminate(DriverObject);

	RtlInitUnicodeString(&uniWin32NameString, DOS_DEVICE_NAME);
	IoDeleteSymbolicLink(&uniWin32NameString);

	IoDeleteDevice(DriverObject->DeviceObject);
}


NTSTATUS
RCCOM_Dispatch(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP pIrp
	)
{
	PIO_STACK_LOCATION	irpStack;
	PLOCAL_DEVICE_INFO	pLDI;
	PVOID				ioBuffer;
	ULONG				ioControlCode;
	NTSTATUS			ntStatus;

	irpStack = IoGetCurrentIrpStackLocation(pIrp);

	pLDI = (PLOCAL_DEVICE_INFO)DeviceObject->DeviceExtension;

	ioBuffer = pIrp->AssociatedIrp.SystemBuffer;

	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;

	switch (irpStack->MajorFunction) {

	case IRP_MJ_CREATE:
		pLDI->OpenCount++;
		break;

	case IRP_MJ_CLOSE:
		if (pLDI->OpenCount == 0) {
			pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			break;
		}
		pLDI->OpenCount--;
		if (pLDI->bInitialized && pLDI->OpenCount == 0)
			RCCOM_Terminate(pLDI->DriverObject);
		break;

	case IRP_MJ_DEVICE_CONTROL:
		ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
		switch (ioControlCode) {
		
		case IOCTL_RCCOM_GET_VERSION:
			KdPrint(("RCCOM: GetVersion\n"));
			if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {
				pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				pLDI->LastError = RCERR_UNKNOWN;
				break;
			}
			pIrp->IoStatus.Information = sizeof(ULONG);
			*(PULONG)ioBuffer = RCCOM_VERSION;
			pLDI->LastError = RCERR_SUCCESS;
			break;

		case IOCTL_RCCOM_GET_LAST_ERROR:
			KdPrint(("RCCOM: GetLastError\n"));
			if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG)) {
				pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				pLDI->LastError = RCERR_UNKNOWN;
				break;
			}
			pIrp->IoStatus.Information = sizeof(ULONG);
			*(PULONG)ioBuffer = pLDI->LastError;
			break;

		case IOCTL_RCCOM_INITIALIZE:
			KdPrint(("RCCOM: Init\n"));
			pLDI->LastError = RCERR_SUCCESS;
			if (pLDI->bInitialized)
				break;
			pLDI->LastError = RCCOM_Initialize(pLDI->DriverObject);
			break;

		case IOCTL_RCCOM_TERMINATE:
			KdPrint(("RCCOM: Terminate\n"));
			pLDI->LastError = RCERR_SUCCESS;
			if (!pLDI->bInitialized || pLDI->OpenCount > 1)
				break;
			pLDI->LastError = RCCOM_Terminate(pLDI->DriverObject);
			break;

		case IOCTL_RCCOM_GET_HARDWARE_INFO:
			KdPrint(("RCCOM: GetHardwareInfo\n"));
			if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
				sizeof(RCCOM_HARDWARE_INFO)) {
				pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				pLDI->LastError = RCERR_UNKNOWN;
				break;
			}
			pLDI->rchi.uPhysicalPort = (ULONG)pLDI->PhysPort;
			pLDI->rchi.uPhysicalIrq = (ULONG)pLDI->Irq;
			pIrp->IoStatus.Information = sizeof(RCCOM_HARDWARE_INFO);
			RtlCopyMemory(ioBuffer, &pLDI->rchi, sizeof(RCCOM_HARDWARE_INFO));
			pLDI->LastError = RCERR_SUCCESS;
			break;

		case IOCTL_RCCOM_SET_HARDWARE_INFO:
			KdPrint(("RCCOM: SetHardwareInfo\n"));
			if (irpStack->Parameters.DeviceIoControl.InputBufferLength < sizeof(RCCOM_HARDWARE_INFO)) {
				pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				pLDI->LastError = RCERR_UNKNOWN;
				break;
			}
			RtlCopyMemory(&pLDI->rchi, ioBuffer, sizeof(RCCOM_HARDWARE_INFO));
			TranslateTimingValues(pLDI);
			pLDI->LastError = RCERR_SUCCESS;
			break;

		case IOCTL_RCCOM_READ:
			KdPrint(("RCCOM: Read\n"));
			if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(RCCOM_INFO)) {
				pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				pLDI->LastError = RCERR_UNKNOWN;
				break;
			}
			pIrp->IoStatus.Information = sizeof(RCCOM_INFO);
			RtlZeroMemory(ioBuffer, sizeof(RCCOM_INFO));
			if (!pLDI->bInitialized) {
				pLDI->LastError = RCERR_UNKNOWN;
				break;
			}
			pLDI->LastError = CheckTimeOuts(pLDI);
			if (pLDI->LastError != RCERR_SUCCESS)
				break;
			RtlCopyMemory(ioBuffer, &pLDI->rci, sizeof(RCCOM_INFO));
			pLDI->rci.bUpdated = FALSE;
			pLDI->LastError = RCERR_SUCCESS;
			break;

		default:
			KdPrint(("RCCOM: unknown IO_DEVICE_CONTROL\n"));
			pIrp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			pLDI->LastError = RCERR_UNKNOWN;
			break;
		}
		break;
	}
	ntStatus = pIrp->IoStatus.Status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}

/////////////////////////////////////////////////////////////////////////////
// Initialize the device

NTSTATUS
RCCOM_Initialize(
	IN PDRIVER_OBJECT DriverObject
	)
{
	PLOCAL_DEVICE_INFO	pLDI;
	UART_STATE			UartState;
	ULONG				uError;

	pLDI = (PLOCAL_DEVICE_INFO)DriverObject->DeviceObject->DeviceExtension;

	uError = AutoDetectInterface(DriverObject);

	if (uError != RCERR_SUCCESS)
		return uError;

	uError = AllocateResources(DriverObject);

	if (uError != RCERR_SUCCESS)
		return uError;

	// save Uart state
	UART_GetState(pLDI->PhysPort, &pLDI->UartState);

	UartState.LineCtrl		= 0;
	UartState.ModemCtrl	 = ACE_RTS + ACE_OUT2;
	UartState.InterruptCtrl = ACE_EDSSI;

	UART_SetState(pLDI->PhysPort, &UartState);

	pLDI->LastError = RCERR_SUCCESS;

	pLDI->bInitialized = TRUE;

	return RCERR_SUCCESS;
}

NTSTATUS
RCCOM_Terminate(
	IN PDRIVER_OBJECT DriverObject
	)
{
	PLOCAL_DEVICE_INFO	pLDI;
	ULONG				uError;

	pLDI = (PLOCAL_DEVICE_INFO)DriverObject->DeviceObject->DeviceExtension;

	// restore Uart state
	UART_SetState(pLDI->PhysPort, &pLDI->UartState);

	uError = FreeResources(DriverObject);

	if (uError != RCERR_SUCCESS)
		return uError;

	pLDI->bInitialized = FALSE;

	return RCERR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//	stuff for searching the interface

NTSTATUS
AutoDetectInterface(
	IN PDRIVER_OBJECT DriverObject
	)
{
	PLOCAL_DEVICE_INFO	pLDI;
	ULONG				uError;

	int	i;

	static ULONG PortList[] = {
		0x3f8, 0x2f8, 0x3e8, 0x2e8,
		0x3f8, 0x2f8, 0x3e8, 0x2e8,
		0x3f8, 0x2f8, 0x3e8, 0x2e8,
		0x3f8, 0x2f8, 0x3e8, 0x2e8 };

	static ULONG IrqList[] = {
		4, 3, 4, 3,
		4, 3, 4, 3,
		4, 3, 4, 3,
		4, 3, 4, 3 };

	pLDI = (PLOCAL_DEVICE_INFO)DriverObject->DeviceObject->DeviceExtension;

	if (pLDI->rchi.uComPort >= 1 && pLDI->rchi.uComPort <= 16) {
		pLDI->Port = PortList[pLDI->rchi.uComPort-1];
		if (pLDI->rchi.uComIrq >= 2) {
			pLDI->Irq = pLDI->rchi.uComIrq;
		} else {
			pLDI->Irq = IrqList[pLDI->rchi.uComPort-1];
		}

		uError = IsInterfacePresent(DriverObject);
		if (uError == RCERR_NOT_FOUND)
			return RCERR_NOT_FOUND_ON_PORTX;
		if (uError != RCERR_SUCCESS)
			return uError;
		return RCERR_SUCCESS;
	} else {
		for(i=0; i<4; i++) {
			pLDI->Port = PortList[i];
			pLDI->Irq = IrqList[i];

			uError = IsInterfacePresent(DriverObject);

			if (uError == RCERR_SUCCESS)
				return RCERR_SUCCESS;
		}
		return RCERR_NOT_FOUND;
	}
}

NTSTATUS
	IsInterfacePresent(
	IN PDRIVER_OBJECT DriverObject
	)
{
	PLOCAL_DEVICE_INFO	pLDI;
	UART_STATE			UartState;
	BOOLEAN			 bInterfaceFound;
	ULONG				uError;

	int				 i;

	pLDI = (PLOCAL_DEVICE_INFO)DriverObject->DeviceObject->DeviceExtension;

	uError = AllocateResources(DriverObject);

	if (uError != RCERR_SUCCESS)
		return uError;

	bInterfaceFound = TRUE;

	// save Uart state
	UART_GetState(pLDI->PhysPort, &UartState);

	for(i=0; i<2; i++) {
		UART_SetModemCtrl(pLDI->PhysPort, 0);
		KeStallExecutionProcessor(10);

		if ((UART_GetModemStatus(pLDI->PhysPort) & ACE_CTS) == ACE_CTS)
			bInterfaceFound = FALSE;
	
		UART_SetModemCtrl(pLDI->PhysPort, ACE_RTS);
		KeStallExecutionProcessor(10);

		if ((UART_GetModemStatus(pLDI->PhysPort) & ACE_CTS) == 0)
			bInterfaceFound = FALSE;
	}

	// restore Uart state
	UART_SetState(pLDI->PhysPort, &UartState);

	uError = FreeResources(DriverObject);

	if (uError != RCERR_SUCCESS)
		return RCERR_PORT_ACQUIRE_ERROR;

	if (!bInterfaceFound) {
		KdPrint(("RCCOM: Can't detect interface at 0x%04x\n", pLDI->Port));
		return RCERR_NOT_FOUND;
	}

	KdPrint(("RCCOM: Interface detected at 0x%04x\n", pLDI->Port));

	return RCERR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// Resource handling

// claim all necessary resources
NTSTATUS
AllocateResources(
	IN PDRIVER_OBJECT DriverObject
	)
{
	ULONG				MappedVector, AddressSpace = 1;
	KIRQL				irql;
	KAFFINITY			Affinity;
	PHYSICAL_ADDRESS	InPortAddr, OutPortAddr;
	PLOCAL_DEVICE_INFO	pLDI;
	NTSTATUS			ntStatus, ioConnectStatus;

	pLDI = (PLOCAL_DEVICE_INFO)DriverObject->DeviceObject->DeviceExtension;

	pLDI->rchi.uPhysicalPort = pLDI->rchi.uPhysicalIrq = 0;

	MappedVector = HalGetInterruptVector(
		Isa,
		0,
		pLDI->Irq,
		pLDI->Irq,
		&irql,
		&Affinity);

	InPortAddr.LowPart = pLDI->Port;
	InPortAddr.HighPart = 0;

	if (!HalTranslateBusAddress(
			 Isa,
			 0,
			 InPortAddr,
			 &AddressSpace,
			 &OutPortAddr
			)) {
		KdPrint(("RCCOM: HalTranslateBusAddress failed\n"));
		return RCERR_PORT_ACQUIRE_ERROR;
	}

	pLDI->PhysPort = (PUCHAR)(PVOID)OutPortAddr.LowPart;

	ntStatus = ReportResourceUsage(
		DriverObject,
		OutPortAddr,
		pLDI->Irq);

	if (ntStatus != STATUS_SUCCESS) {
		KdPrint(("RCCOM: Couldn't get resources: 0x%x, 0x%02x\n",
				 pLDI->Port, pLDI->Irq));
		return RCERR_PORT_ALREADY_CLAIMED;
	}

	ioConnectStatus = IoConnectInterrupt(
		&pLDI->InterruptObject,
		RccomInterruptServiceRoutine,
		DriverObject->DeviceObject,
		NULL,
		MappedVector,
		irql,
		irql,
		Latched,
		TRUE,	 // share it
		Affinity,
		FALSE);

	if (!NT_SUCCESS(ioConnectStatus)) {
		KdPrint(("RCCOM: couldn't connect interrupt 0x%02x\n", pLDI->Irq));
		return RCERR_IRQ_VIRTUALIZE_ERROR;
	}

	return RCERR_SUCCESS;
}

// free all resources and restore previous state
NTSTATUS
FreeResources(
	IN PDRIVER_OBJECT DriverObject
	)
{
	PLOCAL_DEVICE_INFO	pLDI;
	CM_RESOURCE_LIST	NullResourceList;
	BOOLEAN			 ResourceConflict;

	pLDI = (PLOCAL_DEVICE_INFO)DriverObject->DeviceObject->DeviceExtension;

	IoDisconnectInterrupt(pLDI->InterruptObject);

	RtlZeroMemory(&NullResourceList, sizeof(NullResourceList));

	IoReportResourceUsage(
		NULL,
		DriverObject,
		&NullResourceList,
		sizeof(ULONG),
		NULL,
		NULL,
		0,
		FALSE,
		&ResourceConflict);

	return RCERR_SUCCESS;
}

// report used resources to NT
NTSTATUS ReportResourceUsage(
	IN PDRIVER_OBJECT DriverObject,
	IN PHYSICAL_ADDRESS PortAddress,
	IN ULONG Irq
	)
{
	PCM_RESOURCE_LIST				pResourceList;
	PCM_FULL_RESOURCE_DESCRIPTOR	pFRD;
	PCM_PARTIAL_RESOURCE_DESCRIPTOR pPRD;
	ULONG							SizeOfResourceList;
	BOOLEAN							ResourceConflict;
	NTSTATUS						ntStatus;

	SizeOfResourceList = FIELD_OFFSET(CM_RESOURCE_LIST, List[0]);
	SizeOfResourceList += sizeof(CM_FULL_RESOURCE_DESCRIPTOR);
	SizeOfResourceList += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

	pResourceList = (PCM_RESOURCE_LIST)ExAllocatePool(PagedPool, SizeOfResourceList);

	if (pResourceList == NULL)
		return STATUS_INSUFFICIENT_RESOURCES;

	RtlZeroMemory(pResourceList, SizeOfResourceList);

	pResourceList->Count = 1;

	pFRD = &pResourceList->List[0];
	pFRD->InterfaceType = Isa;
	pFRD->BusNumber = 0;
	pFRD->PartialResourceList.Count = 2;

	pPRD = &pFRD->PartialResourceList.PartialDescriptors[0];

	pPRD->Type = CmResourceTypePort;
	pPRD->ShareDisposition = CmResourceShareDriverExclusive;
	pPRD->Flags = CM_RESOURCE_PORT_IO;
	pPRD->u.Port.Start = PortAddress;
	pPRD->u.Port.Length = 8;

	pPRD++;

	pPRD->Type = CmResourceTypeInterrupt;
	pPRD->ShareDisposition = CmResourceShareDriverExclusive;
	pPRD->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
	pPRD->u.Interrupt.Level = Irq;
	pPRD->u.Interrupt.Vector = Irq;
	pPRD->u.Interrupt.Affinity = 0;

	ntStatus = IoReportResourceUsage(
		NULL,
		DriverObject,
		pResourceList,
		SizeOfResourceList,
		NULL,
		NULL,
		0,
		TRUE,
		&ResourceConflict);

	ExFreePool(pResourceList);

	if (ntStatus != STATUS_SUCCESS)
		return ntStatus;

	if (ResourceConflict) {
		CM_RESOURCE_LIST	NullResourceList;
	
		RtlZeroMemory(&NullResourceList, sizeof(NullResourceList));

		IoReportResourceUsage(
			NULL,
			DriverObject,
			&NullResourceList,
			sizeof(ULONG),
			NULL,
			NULL,
			0,
			FALSE,
			&ResourceConflict
			);
			return STATUS_INSUFFICIENT_RESOURCES;
	}
	return STATUS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// Miscellaneous stuff

VOID InitTimingValues(PLOCAL_DEVICE_INFO pLDI)
{
	pLDI->rchi.tv.uSyncLen		= DEFAULT_SYNC_LEN;
	pLDI->rchi.tv.uCenterLen	= DEFAULT_CENTER_LEN;
	pLDI->rchi.tv.uDeviationLen = DEFAULT_DEVIATION_LEN;
	pLDI->rchi.tv.uMaxSyncLen	= DEFAULT_MAX_SYNC_LEN;
	pLDI->rchi.tv.uIrqTimeOut	= DEFAULT_IRQ_TIMEOUT;
	pLDI->rchi.tv.uSyncTimeOut	= DEFAULT_SYNC_TIMEOUT;
}

VOID TranslateTimingValues(PLOCAL_DEVICE_INFO pLDI)
{
	LARGE_INTEGER OneMega, tmp;

	OneMega.LowPart = 1000000L;
	OneMega.HighPart = 0L;

	KeQueryPerformanceCounter(&pLDI->rchi.CounterFrequency);

	tmp = RtlLargeIntegerDivide(
		RtlEnlargedUnsignedMultiply(
			pLDI->rchi.CounterFrequency.LowPart,
			pLDI->rchi.tv.uSyncLen), OneMega, &tmp);
	pLDI->rchi.ttv.uSyncLen = tmp.LowPart;

	tmp = RtlLargeIntegerDivide(
		RtlEnlargedUnsignedMultiply(
			pLDI->rchi.CounterFrequency.LowPart,
			pLDI->rchi.tv.uCenterLen), OneMega, &tmp);
	pLDI->rchi.ttv.uCenterLen = tmp.LowPart;

	tmp = RtlLargeIntegerDivide(
		RtlEnlargedUnsignedMultiply(
			pLDI->rchi.CounterFrequency.LowPart,
			pLDI->rchi.tv.uDeviationLen), OneMega, &tmp);
	pLDI->rchi.ttv.uDeviationLen = tmp.LowPart;

	tmp = RtlLargeIntegerDivide(
		RtlEnlargedUnsignedMultiply(
			pLDI->rchi.CounterFrequency.LowPart,
			pLDI->rchi.tv.uMaxSyncLen), OneMega, &tmp);
	pLDI->rchi.ttv.uMaxSyncLen = tmp.LowPart;

	tmp = RtlLargeIntegerDivide(
		RtlEnlargedUnsignedMultiply(
			pLDI->rchi.CounterFrequency.LowPart,
			pLDI->rchi.tv.uIrqTimeOut), OneMega, &tmp);
	pLDI->rchi.ttv.uIrqTimeOut = tmp.LowPart;

	tmp = RtlLargeIntegerDivide(
		RtlEnlargedUnsignedMultiply(
			pLDI->rchi.CounterFrequency.LowPart,
			pLDI->rchi.tv.uSyncTimeOut), OneMega, &tmp);
	pLDI->rchi.ttv.uSyncTimeOut = tmp.LowPart;
}

BOOLEAN CheckTimeOuts(PLOCAL_DEVICE_INFO pLDI)
{
	TIME			Time, TimeDiff;
	LARGE_INTEGER	CounterFrequency;

	if (RtlLargeIntegerEqualToZero(pLDI->TimeOfLastIrq))
		return RCERR_NO_DATA;

	Time = KeQueryPerformanceCounter(&CounterFrequency);

	TimeDiff = RtlLargeIntegerSubtract(Time, pLDI->TimeOfLastIrq);

	if (TimeDiff.HighPart != 0 || TimeDiff.LowPart > pLDI->rchi.ttv.uIrqTimeOut)
		return RCERR_NO_DATA;

	if (RtlLargeIntegerEqualToZero(pLDI->TimeOfLastSync))
		return RCERR_BAD_DATA;

	TimeDiff = RtlLargeIntegerSubtract(Time, pLDI->TimeOfLastSync);

	if (TimeDiff.HighPart != 0 || TimeDiff.LowPart > pLDI->rchi.ttv.uSyncTimeOut)
		return RCERR_BAD_DATA;

	return RCERR_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
// InterruptServiceRoutine

BOOLEAN RccomInterruptServiceRoutine(
		IN PKINTERRUPT	pInterrupt,
		IN OUT PVOID	pContext
		)
{
	PDEVICE_OBJECT		DeviceObject;
	PLOCAL_DEVICE_INFO	pLDI;
	TIME				Time;
	int					i;

	DeviceObject = (PDEVICE_OBJECT)pContext;
	pLDI = (PLOCAL_DEVICE_INFO)DeviceObject->DeviceExtension;

	pLDI->IrqCounter++;

	// if no interrupt pending -> exit
	if ((READ_PORT_UCHAR(pLDI->PhysPort+ACE_IIDR) & ACE_IIP) == 1)
		return TRUE;

	// DCTS has to be changed, exit otherwise
	if ((READ_PORT_UCHAR(pLDI->PhysPort+ACE_MSR) & ACE_DCTS) == 0)
		return TRUE;

	// wait for positive slope of the CTS signal

	if ((READ_PORT_UCHAR(pLDI->PhysPort+ACE_MSR) & ACE_CTS) == 0) {
		if (!pLDI->rchi.bInvertedPulse)
			return TRUE;
	} else {
		if (pLDI->rchi.bInvertedPulse)
			return TRUE;
	}

	// remember current time and calculate pulse width
	Time = pLDI->TimeOfLastIrq;
	pLDI->TimeOfLastIrq = KeQueryPerformanceCounter(&pLDI->rchi.CounterFrequency);
	Time = RtlLargeIntegerSubtract(pLDI->TimeOfLastIrq, Time);

	// filter out very long pulses
	if (Time.HighPart != 0 || Time.LowPart > pLDI->rchi.ttv.uMaxSyncLen) {
		pLDI->TimeOfLastSync.LowPart = pLDI->TimeOfLastSync.HighPart = 0;
		pLDI->rci.uCurrentChannel = MAX_CHANNELS;
		return TRUE;
	}

	if (Time.LowPart > pLDI->rchi.ttv.uSyncLen) {
		// remember last sync
		pLDI->TimeOfLastSync = pLDI->TimeOfLastIrq;
		// clear unused channels
		for(i=pLDI->rci.uCurrentChannel; i<MAX_CHANNELS; i++)
			pLDI->rci.RawValue[i] = (ULONG)INVALID_CHANNEL_VALUE;
		pLDI->rci.uCurrentChannel = 0;
		pLDI->rci.bUpdated = TRUE;

		return TRUE;
	}

	if (pLDI->rci.uCurrentChannel >= MAX_CHANNELS)
		return TRUE;

	// store channel pulse width
	pLDI->rci.RawValue[ pLDI->rci.uCurrentChannel ] = Time.LowPart;

	pLDI->rci.uCurrentChannel++;

	return TRUE;
}
