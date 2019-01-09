;
; ctlcode.inc, subset of CTL definition
;

FILE_DEVICE_BEEP                equ 00000001h
FILE_DEVICE_CD_ROM              equ 00000002h
FILE_DEVICE_CD_ROM_FILE_SYSTEM  equ 00000003h
FILE_DEVICE_CONTROLLER          equ 00000004h
FILE_DEVICE_DATALINK            equ 00000005h
FILE_DEVICE_DFS                 equ 00000006h
FILE_DEVICE_DISK                equ 00000007h
FILE_DEVICE_DISK_FILE_SYSTEM    equ 00000008h
FILE_DEVICE_FILE_SYSTEM         equ 00000009h
FILE_DEVICE_INPORT_PORT         equ 0000000ah
FILE_DEVICE_KEYBOARD            equ 0000000bh
FILE_DEVICE_MAILSLOT            equ 0000000ch
FILE_DEVICE_MIDI_IN             equ 0000000dh
FILE_DEVICE_MIDI_OUT            equ 0000000eh
FILE_DEVICE_MOUSE               equ 0000000fh
FILE_DEVICE_MULTI_UNC_PROVIDER  equ 00000010h
FILE_DEVICE_NAMED_PIPE          equ 00000011h
FILE_DEVICE_NETWORK             equ 00000012h
FILE_DEVICE_NETWORK_BROWSER     equ 00000013h
FILE_DEVICE_NETWORK_FILE_SYSTEM equ 00000014h
FILE_DEVICE_NULL                equ 00000015h
FILE_DEVICE_PARALLEL_PORT       equ 00000016h
FILE_DEVICE_PHYSICAL_NETCARD    equ 00000017h
FILE_DEVICE_PRINTER             equ 00000018h
FILE_DEVICE_SCANNER             equ 00000019h
FILE_DEVICE_SERIAL_MOUSE_PORT   equ 0000001ah
FILE_DEVICE_SERIAL_PORT         equ 0000001bh
FILE_DEVICE_SCREEN              equ 0000001ch
FILE_DEVICE_SOUND               equ 0000001dh
FILE_DEVICE_STREAMS             equ 0000001eh
FILE_DEVICE_TAPE                equ 0000001fh
FILE_DEVICE_TAPE_FILE_SYSTEM    equ 00000020h
FILE_DEVICE_TRANSPORT           equ 00000021h
FILE_DEVICE_UNKNOWN             equ 00000022h
FILE_DEVICE_VIDEO               equ 00000023h
FILE_DEVICE_VIRTUAL_DISK        equ 00000024h
FILE_DEVICE_WAVE_IN             equ 00000025h
FILE_DEVICE_WAVE_OUT            equ 00000026h
FILE_DEVICE_8042_PORT           equ 00000027h
FILE_DEVICE_NETWORK_REDIRECTOR  equ 00000028h
FILE_DEVICE_BATTERY             equ 00000029h
FILE_DEVICE_BUS_EXTENDER        equ 0000002ah

; Macro definition for defining IOCTL and FSCTL function control codes. Note
; that function codes 0-2047 are reserved for Microsoft Corporation, and
; 2048-4095 are reserved for customers.

CTL_CODE MACRO Procedure, DeviceType, Function, Method, Access
Procedure = (DeviceType shl 16) or (Access shl 14) or (Function shl 2) or Method
	ENDM

METHOD_BUFFERED                 equ 0
METHOD_IN_DIRECT                equ 1
METHOD_OUT_DIRECT               equ 2
METHOD_NEITHER                  equ 3

; Define the access check value for any access
;
; The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
; ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
; constants *MUST* always be in sync.

FILE_ANY_ACCESS                 equ 0
FILE_READ_ACCESS                equ 0001        ; file & pipe
FILE_WRITE_ACCESS               equ 0002        ; file & pipe

