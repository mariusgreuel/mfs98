;
; rccom.inc
;
; Copyright (C) 1997 Marius Greuel. All rights reserved.
;
; all values have to be synchronized with rccom.h
; for description see rccom.h
;

RCCOM_MAJ_VER           equ 1
RCCOM_MIN_VER           equ 1
RCCOM_VERSION           equ RCCOM_MAJ_VER shl 8 + RCCOM_MIN_VER
RCCOM_DEV_ID            equ Undefined_Device_ID

MAX_CHANNELS            equ 10          ; # of channels that are read from the rc box
MAX_STICK_VALUE         equ 128
INVALID_CHANNEL_VALUE   equ -1          ; assigned to a channel, which cannot be read

DEFAULT_SYNC_LEN                equ 3000
DEFAULT_SYNC_LEN_RANGE_MIN      equ 2000
DEFAULT_SYNC_LEN_RANGE_MAX      equ 8000

DEFAULT_CENTER_LEN              equ 1600
DEFAULT_CENTER_LEN_RANGE_MIN    equ (DEFAULT_CENTER_LEN-500)
DEFAULT_CENTER_LEN_RANGE_MAX    equ (DEFAULT_CENTER_LEN+500)

DEFAULT_DEVIATION_LEN           equ 550
DEFAULT_DEVIATION_LEN_RANGE_MIN equ (DEFAULT_DEVIATION_LEN-200)
DEFAULT_DEVIATION_LEN_RANGE_MAX equ (DEFAULT_DEVIATION_LEN+200)

DEFAULT_MAX_SYNC_LEN            equ  20000
DEFAULT_IRQ_TIMEOUT             equ  25000
DEFAULT_SYNC_TIMEOUT            equ 200000

FILE_DEVICE_RCCOM               equ 08FFh

CTL_CODE IOCTL_RCCOM_GET_VERSION,       FILE_DEVICE_RCCOM,0800h,METHOD_BUFFERED,FILE_READ_ACCESS
CTL_CODE IOCTL_RCCOM_GET_LAST_ERROR,    FILE_DEVICE_RCCOM,0801h,METHOD_BUFFERED,FILE_READ_ACCESS
CTL_CODE IOCTL_RCCOM_INITIALIZE,        FILE_DEVICE_RCCOM,0802h,METHOD_BUFFERED,FILE_READ_ACCESS
CTL_CODE IOCTL_RCCOM_TERMINATE,        	FILE_DEVICE_RCCOM,0803h,METHOD_BUFFERED,FILE_READ_ACCESS
CTL_CODE IOCTL_RCCOM_GET_HARDWARE_INFO, FILE_DEVICE_RCCOM,0804h,METHOD_BUFFERED,FILE_READ_ACCESS
CTL_CODE IOCTL_RCCOM_SET_HARDWARE_INFO, FILE_DEVICE_RCCOM,0805h,METHOD_BUFFERED,FILE_WRITE_ACCESS
CTL_CODE IOCTL_RCCOM_READ,              FILE_DEVICE_RCCOM,0806h,METHOD_BUFFERED,FILE_READ_ACCESS

RCERR_SUCCESS                   equ 0
RCERR_UNKNOWN                   equ 1
RCERR_CANT_LOAD_VXD             equ 2
RCERR_BAD_VERSION               equ 3
RCERR_NO_DATA                   equ 4
RCERR_BAD_DATA                  equ 5
RCERR_NOT_FOUND                 equ 6
RCERR_NOT_FOUND_ON_PORTX        equ 7
RCERR_PORT_NOT_AVAILABLE        equ 8
RCERR_PORT_ALREADY_CLAIMED      equ 9
RCERR_PORT_ACQUIRE_ERROR        equ 10
RCERR_IRQ_VIRTUALIZE_ERROR      equ 11

TIMING_VALUES           struc
SyncLen                 dd ?
CenterLen               dd ?
DeviationLen            dd ?
MaxSyncLen              dd ?
IrqTimeOut              dd ?
SyncTimeOut             dd ?
TIMING_VALUES           ends

RCCOM_HARDWARE_INFO     struc
ComPort                 dd ?
ComIrq                  dd ?
PhysicalPort            dd ?
PhysicalIrq             dd ?
InvertedPulse		dd ?
tv                      TIMING_VALUES <>
ttv                     TIMING_VALUES <>
CounterFrequency        dq ?
RCCOM_HARDWARE_INFO     ends

RCCOM_INFO              struc
Updated                 dd ?
CurrentChannel          dd ?
RawValue                dd MAX_CHANNELS dup (?)
RCCOM_INFO              ends


