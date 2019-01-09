;******************************************************************************
;
;  Copyright (C) 1997 Marius Greuel. All rights reserved.
;
;  Title:       rccom.asm : rcbox communication virtual device driver
;
;  Version:     1.00
;
;  Date:        9-Jan-97
;
;  Author:      MG
;
;------------------------------------------------------------------------------

        .386p

;******************************************************************************
;                             I N C L U D E S
;******************************************************************************

        .xlist
        include VMM.inc
        include VPICD.inc
        include VTD.inc
        include VCD.inc
        include VWin32.inc
        include Ctlcode.inc

        include MyDebug.inc
        include Uart.inc
        include Rccom.inc
        .list

        extern UART_GetState : proc
        extern UART_SetState : proc

COUNTER_FREQUENCY       equ 1193000

;******************************************************************************
;                V I R T U A L   D E V I C E   D E C L A R A T I O N
;******************************************************************************

Declare_Virtual_Device RCCOM, RCCOM_MAJ_VER, RCCOM_MIN_VER, \
        RCCOM_ControlProc, RCCOM_DEV_ID

;******************************************************************************
;                           L O C A L   D A T A
;******************************************************************************

VxD_LOCKED_DATA_SEG

IRQ_Descriptor  VPICD_IRQ_Descriptor <0,VPICD_Opt_Can_Share,OFFSET32 RCCOM_IrqHandler>

COM_Handle      dd 0
IRQ_Handle      dd 0

Initialized     dd 0
OpenCount       dd 0

LastError       dd 0            ; error code (0=Success)
IrqCounter      dd 0            ; # of catched irqs (all)
MyIrqCounter    dd 0            ; # of catched irqs (good ones)

UartState       UART_STATE<>

Port            dd 0
Irq             dd 0

rchi            RCCOM_HARDWARE_INFO<>
rci             RCCOM_INFO<>

TimeOfLastIrq   dq ?
TimeOfLastSync  dq ?

sz_RCCOM        db "RCCOM",0
sz_Hello        db "Copyright © Marius Greuel 1998. All rights reserved.",0dh,0ah,0

VxD_LOCKED_DATA_ENDS

VxD_IDATA_SEG

VxD_IDATA_ENDS

;******************************************************************************
;                  I N I T I A L I Z A T I O N   C O D E
;******************************************************************************

VxD_ICODE_SEG

;==============================================================================
;
; RCCOM_SysDynamicDeviceInit
;
; DESCRIPTION: Handler for SYS_DYNAMIC_DEVICE_INIT
;
; ENTRY: EBX = VM handle
;
; EXIT: carry flag cleared if success
;
;------------------------------------------------------------------------------

BeginProc RCCOM_SysDynamicDeviceInit

        Trace_Out "RCCOM: SysDynamicDeviceInit"

        mov     [rchi.tv.SyncLen],      DEFAULT_SYNC_LEN
        mov     [rchi.tv.CenterLen],    DEFAULT_CENTER_LEN
        mov     [rchi.tv.DeviationLen], DEFAULT_DEVIATION_LEN
        mov     [rchi.tv.MaxSyncLen],   DEFAULT_MAX_SYNC_LEN
        mov     [rchi.tv.IrqTimeOut],   DEFAULT_IRQ_TIMEOUT
        mov     [rchi.tv.SyncTimeOut],  DEFAULT_SYNC_TIMEOUT

        call    TranslateTimingValues

        xor     eax, eax
SDDI_ClearChannels:
        mov     [rci.RawValue + eax*4], INVALID_CHANNEL_VALUE
        inc     eax
        cmp     eax, MAX_CHANNELS
        jb      SDDI_ClearChannels

        inc     [OpenCount]

        clc
        ret

EndProc RCCOM_SysDynamicDeviceInit

VxD_ICODE_ENDS

VxD_LOCKED_CODE_SEG

;==============================================================================
;
; RCCOM_SysDynamicDeviceExit
;
; DESCRIPTION: Handler for SYS_DYNAMIC_DEVICE_EXIT
;
; EXIT: carry flag cleared if success
;
;------------------------------------------------------------------------------

BeginProc RCCOM_SysDynamicDeviceExit

        Trace_Out "RCCOM: SysDynamicDeviceExit"

IFDEF DEBUG
        mov     eax, 4
        VxDCall VPICD_Get_IRQ_Complete_Status
        Trace_Out "RCCOM: Status=#ecx"

        mov     eax, [IrqCounter]
        mov     edx, [MyIRQCounter]
        sub     eax, edx
        Trace_Out "RCCOM: SysDynamicDeviceExit: #edx/!#eax Irqs occured"
ENDIF
        cmp     [Initialized], 0
        jz      DDE_Finished
        cmp     [OpenCount], 0
        jz      DDE_Finished

        dec     [OpenCount]

        cmp     [OpenCount], 0
        jnz     DDE_Finished

        call    RCCOM_Terminate

DDE_Finished:
        clc
        ret

EndProc RCCOM_SysDynamicDeviceExit

;==============================================================================
;
; RCCOM_ControlProc
;
; DESCRIPTION: Routine to dispatch control messages to handlers
;
;------------------------------------------------------------------------------

BeginProc RCCOM_ControlProc

        Control_Dispatch SYS_DYNAMIC_DEVICE_INIT, RCCOM_SysDynamicDeviceInit
        Control_Dispatch SYS_DYNAMIC_DEVICE_EXIT, RCCOM_SysDynamicDeviceExit
        Control_Dispatch W32_DEVICEIOCONTROL,     RCCOM_DeviceIOControl
        clc
        ret

EndProc RCCOM_ControlProc

;******************************************************************************
;                              D E V I C E   I O
;******************************************************************************

;==============================================================================
;
; RCCOM_DeviceIOControl
;
; DESCRIPTION: Handler for W32_DEVICEIOCONTROL
;
; ENTRY: ESI DIOCParams 
;
;------------------------------------------------------------------------------

BeginProc RCCOM_DeviceIOControl

        mov     eax, [esi.dwIoControlCode]      ; get control code

        cmp     eax, DIOC_OPEN
        jz      DIO_Sucess
        cmp     eax, DIOC_CLOSEHANDLE
        jz      DIO_Sucess

        cmp     eax, IOCTL_RCCOM_GET_VERSION
        jz      DIO_GetVersion
        cmp     eax, IOCTL_RCCOM_GET_LAST_ERROR
        jz      DIO_GetLastError
        cmp     eax, IOCTL_RCCOM_INITIALIZE
        jz      DIO_Initialize
        cmp     eax, IOCTL_RCCOM_TERMINATE
        jz      DIO_Terminate
        cmp     eax, IOCTL_RCCOM_GET_HARDWARE_INFO
        jz      DIO_GetHardwareInfo
        cmp     eax, IOCTL_RCCOM_SET_HARDWARE_INFO
        jz      DIO_SetHardwareInfo
        cmp     eax, IOCTL_RCCOM_READ
        jz      DIO_Read

        jmp     DIO_FailedUnkown

DIO_GetVersion:
        mov     eax, sizeof dword
        cmp     [esi.cbOutBuffer], eax          ; enough bytes in out buffer?
        jb      DIO_FailedBufferTooSmall
        cmp     [esi.lpvOutBuffer],0            ; non zero ptr given?
        jz      DIO_FailedInvalidArgs

        mov     edi, [esi.lpcbBytesReturned]
        mov     [edi], eax                      ; store # of returned bytes

        mov     edi, [esi.lpvOutBuffer]         ; load ptr of out buffer
        mov     dword ptr [edi], RCCOM_VERSION

        mov     eax, RCERR_SUCCESS
        jmp     DIO_Sucess

DIO_GetLastError:
        mov     eax, sizeof dword
        cmp     [esi.cbOutBuffer], eax          ; enough bytes in out buffer?
        jb      DIO_FailedBufferTooSmall
        cmp     [esi.lpvOutBuffer],0            ; non zero ptr given?
        jz      DIO_FailedInvalidArgs

        mov     edi, [esi.lpcbBytesReturned]
        mov     [edi], eax                      ; store # of returned bytes

        mov     edi, [esi.lpvOutBuffer]         ; load ptr of out buffer
        mov     eax, [LastError]
        mov     [edi], eax

        jmp     DIO_Sucess

DIO_Initialize:
        mov     eax, RCERR_SUCCESS

        cmp     [Initialized], 0
        jnz     DIO_Sucess

        call    RCCOM_Initialize

        jmp     DIO_Sucess

DIO_Terminate:
        mov     eax, RCERR_SUCCESS

        cmp     [Initialized], 0
        jnz     DIO_Sucess

        call    RCCOM_Initialize

        jmp     DIO_Sucess

DIO_GetHardwareInfo:
        mov     eax, sizeof RCCOM_HARDWARE_INFO
        cmp     [esi.cbOutBuffer], eax          ; enough bytes in out buffer?
        jb      DIO_FailedBufferTooSmall
        cmp     [esi.lpvOutBuffer],0            ; non zero ptr given?
        jz      DIO_FailedInvalidArgs

        mov     edi, [esi.lpcbBytesReturned]
        mov     [edi], eax                      ; store # of returned bytes

        mov     eax, [Port]
        mov     [rchi.PhysicalPort], eax
        mov     eax, [Irq]
        mov     [rchi.PhysicalIrq], eax

        mov     edi, [esi.lpvOutBuffer]         ; load ptr of out buffer
        mov     esi, OFFSET32 rchi
        mov     ecx, sizeof RCCOM_HARDWARE_INFO
        cld
        rep     movsb

        mov     eax, RCERR_SUCCESS
        jmp     DIO_Sucess

DIO_SetHardwareInfo:
        mov     eax, sizeof RCCOM_HARDWARE_INFO
        cmp     [esi.cbInBuffer], eax           ; enough bytes in inbuffer?
        jb      DIO_FailedBufferTooSmall
        cmp     [esi.lpvInBuffer],0             ; non zero ptr given?
        jz      DIO_FailedInvalidArgs

        mov     esi, [esi.lpvInBuffer]          ; load ptr of inbuffer
        mov     edi, OFFSET32 rchi
        mov     ecx, sizeof RCCOM_HARDWARE_INFO
        cld
        rep     movsb

        call    TranslateTimingValues

        mov     eax, RCERR_SUCCESS
        jmp     DIO_Sucess

DIO_Read:
        mov     eax, sizeof RCCOM_INFO
        cmp     [esi.cbOutBuffer], eax          ; enough bytes in out buffer?
        jb      DIO_FailedBufferTooSmall
        cmp     [esi.lpvOutBuffer],0            ; non zero ptr given?
        jz      DIO_FailedInvalidArgs

        mov     edi, [esi.lpcbBytesReturned]
        mov     [edi], eax                      ; store # of returned bytes

        mov     edi, [esi.lpvOutBuffer]         ; load ptr of out buffer
        mov     ecx, sizeof RCCOM_INFO
        cld
        mov     al, 0
        rep     stosb

        cmp     [Initialized], 0
        jz      DIO_FailedUnkown

        call    CheckTimeOuts
        jc      DIO_Sucess

        mov     edi, [esi.lpvOutBuffer]         ; load ptr of out buffer
        mov     esi, OFFSET32 rci
        mov     ecx, sizeof RCCOM_INFO
        cld
        rep     movsb

        mov     [rci.Updated], 0

        xor     eax, eax

DIO_Sucess:

        mov     [LastError], eax

        xor     eax, eax
        clc
        ret

DIO_FailedInvalidArgs:
DIO_FailedBufferTooSmall:
DIO_FailedUnkown:
        mov     eax, RCERR_UNKNOWN

DIO_Failed:
        mov     [LastError], eax

        mov     eax, -1
        stc
        ret

EndProc RCCOM_DeviceIOControl

;******************************************************************************
;                                 R C C O M
;******************************************************************************

;==============================================================================
;
; RCCOM_Initialize
;
; DESCRIPTION:
;
;   This routine does all necessary initialization
;
;------------------------------------------------------------------------------

BeginProc RCCOM_Initialize

        call    AutoDetectInterface
        jc      RI_Failed

        call    AllocateResources
        jc      RI_Failed

        mov     eax, [COM_Handle]
        mov     edx, [eax.VCD_IObase]
        mov     esi, OFFSET32 UartState
        call    UART_GetState

        mov     eax, [COM_Handle]
        mov     edx, [eax.VCD_IObase]
        BasePort

        SetPort ACE_LCR
        mov     al, 0
        out     dx, al                  ; DLAB = 0

        SetPort ACE_IER
        mov     al, IER_MS
        out     dx, al                  ; enable modem status

        SetPort ACE_MCR
        mov     al, ACE_RTS + ACE_OUT2
        out     dx, al                  ; RTS = HIGH, enable ints

        mov     [Initialized], 1

        mov     eax, RCERR_SUCCESS
        clc
        ret

RI_Failed:
        Trace_Out "RCCOM: Initialize failed: EAX=#eax"
        stc
        ret

EndProc RCCOM_Initialize

;==============================================================================
;
; RCCOM_Terminate
;
; DESCRIPTION:
;
;   This routine cleans up our used resources
;
;------------------------------------------------------------------------------

BeginProc RCCOM_Terminate

        mov     eax, [COM_Handle]
        mov     edx, [eax.VCD_IObase]
        mov     esi, OFFSET32 UartState
        call    UART_SetState

        call    FreeResources
        jc      RT_Failed

        mov     [Initialized], 0

        mov     eax, RCERR_SUCCESS
        clc
        ret

RT_Failed:
        Trace_Out "RCCOM: Terminate failed: EAX=#eax"
        stc
        ret

EndProc RCCOM_Terminate

;==============================================================================
;
; AutoDetectInterface
;
; DESCRIPTION:
;
;   This routine detects the port of the interface
;
;------------------------------------------------------------------------------

BeginProc AutoDetectInterface

        mov     eax, [rchi.ComPort]
        or      eax, eax
        jz      ADI_Auto

        mov     [Port], eax

        call    IsInterfacePresent
        jnc     ADI_Found

        cmp     eax, RCERR_NOT_FOUND
        jnz     ADI_NotFound

        mov     eax, RCERR_NOT_FOUND_ON_PORTX
        jmp     ADI_NotFound

ADI_Auto:
        mov     [Port], 1
        call    IsInterfacePresent
        jnc     ADI_Found
        mov     [Port], 2
        call    IsInterfacePresent
        jnc     ADI_Found
        mov     [Port], 3
        call    IsInterfacePresent
        jnc     ADI_Found
        mov     [Port], 4
        call    IsInterfacePresent
        jnc     ADI_Found

        mov     eax, RCERR_NOT_FOUND

ADI_NotFound:
        Trace_Out "RCCOM: AutoDetect failed"
        stc
        ret

ADI_Found:
        mov     eax, RCERR_SUCCESS
        clc
        ret

EndProc AutoDetectInterface

;==============================================================================
;
; IsInterfacePresent
;
; DESCRIPTION: checks if the interface is connected to the specified port
;
; EXTRY:
;
; EXIT: carry flag cleared if found
;
;------------------------------------------------------------------------------

BeginProc IsInterfacePresent

        call    AllocateResources
        jc      IIP_Failed

        mov     eax, [COM_Handle]
        mov     edx, [eax.VCD_IObase]
        mov     esi, OFFSET32 UartState
        call    UART_GetState

        mov     eax, [COM_Handle]
        mov     edx, [eax.VCD_IObase]
        BasePort

        xor     ebx, ebx

        SetPort ACE_MCR
        mov     al, 0
        out     dx, al          ; RTS = 0
        call    Wait10us
        SetPort ACE_MSR
        in      al, dx
        test    al, ACE_DSR     ; DSR must be 0
        jz      IIP_Good1
        inc     ebx

IIP_Good1:
        SetPort ACE_MCR
        mov     al, ACE_RTS
        out     dx, al          ; RTS = 1
        call    Wait10us
        SetPort ACE_MSR
        in      al, dx
        test    al, ACE_DSR     ; DSR must be 1
        jnz     IIP_Good2
        inc     ebx

IIP_Good2:
        mov     eax, [COM_Handle]
        mov     edx, [eax.VCD_IObase]
        mov     esi, OFFSET32 UartState
        call    UART_SetState

        call    FreeResources
        jc      IIP_Failed

        or      ebx, ebx
        jnz     IIP_Failed_NotFound

        mov     eax, [Port]
        Trace_Out "RCCOM: Interface detected at port #al"

        mov     eax, RCERR_SUCCESS
        clc
        ret

IIP_Failed:
        stc
        ret

IIP_Failed_NotFound:
        mov     eax, [Port]
        Trace_Out "RCCOM: Can't detect interface at port #al"

        mov     eax, RCERR_NOT_FOUND
        stc
        ret

EndProc IsInterfacePresent

;******************************************************************************
;                         R e s o u r c e   h a n d l i n g
;******************************************************************************

BeginProc AllocateResources

        mov     eax, [Port]
        Trace_Out "RCCOM: Allocate resources for port #al: ", nocrlf
        VxDCall VCD_Get_Focus           ; get handle in ebx
        jc      AR_Failed1
        or      ebx, ebx
        jnz     AR_Failed2

        mov     eax, [Port]
        xor     ecx, ecx
        mov     edi, OFFSET32 sz_RCCOM
        VxDCall VCD_Acquire_Port        ; acquire Port
        jc      AR_Failed3
        or      eax, eax
        jz      AR_Failed3

        mov     [COM_Handle], eax

        mov     edx, [eax.VCD_IObase]
        Trace_Out "Base is #dx, ", nocrlf

        mov     eax, [COM_Handle]
        movzx   eax, [eax.VCD_IRQN]
        Trace_Out "IRQ is #al"

IFDEF DEBUG
        push    eax
        VxDcall VPICD_Get_Virtualization_Count
        Trace_Out "RCCOM: IRQ has been virtualized #al times"
        pop     eax
ENDIF

        mov     edi, OFFSET32 IRQ_Descriptor
        mov     [edi.VID_IRQ_Number], ax
        VxDcall VPICD_Virtualize_IRQ
        jc      AR_Failed4

        mov     [IRQ_Handle], eax

        VxDCall VPICD_Physically_Unmask

        mov     eax, RCERR_SUCCESS
        clc
        ret

AR_Failed1:
        Trace_Out "Not available. EAX=#eax"
        mov     eax, RCERR_PORT_NOT_AVAILABLE
        stc
        ret

AR_Failed2:
        Trace_Out "Already claimed by #ebx"
        mov     eax, RCERR_PORT_ALREADY_CLAIMED
        stc
        ret

AR_Failed3:
        Trace_Out "Can't acquire Port. EAX=#eax"
        mov     eax, RCERR_PORT_ACQUIRE_ERROR
        stc
        ret

AR_Failed4:
        Trace_Out "Can't virtualize interrupt. EAX=#eax"

        mov     eax, [COM_Handle]
        VxDCall VCD_Free_Port

        mov     eax, RCERR_IRQ_VIRTUALIZE_ERROR
        stc
        ret

EndProc AllocateResources


BeginProc FreeResources

        mov     eax, [IRQ_Handle]
        VxDCall VPICD_Force_Default_Behavior

        mov     eax, [COM_Handle]
        VxDCall VCD_Free_Port

        mov     eax, RCERR_SUCCESS
        clc
        ret

EndProc FreeResources


;******************************************************************************
;                                    M I S C
;******************************************************************************

BeginProc TranslateTimingValues

        mov     ebx, COUNTER_FREQUENCY
        mov     dword ptr [rchi.CounterFrequency], ebx
        mov     dword ptr [rchi.CounterFrequency+4], 0

        mov     ecx, 1000000

        mov     eax, [rchi.tv.SyncLen]
        mul     ebx
        div     ecx
        mov     [rchi.ttv.SyncLen], eax

        mov     eax, [rchi.tv.CenterLen]
        mul     ebx
        div     ecx
        mov     [rchi.ttv.CenterLen], eax

        mov     eax, [rchi.tv.DeviationLen]
        mul     ebx
        div     ecx
        mov     [rchi.ttv.DeviationLen], eax

        mov     eax, [rchi.tv.MaxSyncLen]
        mul     ebx
        div     ecx
        mov     [rchi.ttv.MaxSyncLen], eax

        mov     eax, [rchi.tv.IrqTimeOut]
        mul     ebx
        div     ecx
        mov     [rchi.ttv.IrqTimeOut], eax

        mov     eax, [rchi.tv.SyncTimeOut]
        mul     ebx
        div     ecx
        mov     [rchi.ttv.SyncTimeOut], eax

        ret

EndProc TranslateTimingValues

BeginProc CheckTimeOuts

        mov     eax, dword ptr [TimeOfLastIrq]
        or      eax, dword ptr [TimeOfLastIrq+4]
        jz      CTO_Failed1

        VxDCall VTD_Get_Real_Time
        mov     ebx, eax
        mov     ecx, edx

        sub     eax, dword ptr [TimeOfLastIrq]
        sbb     edx, dword ptr [TimeOfLastIrq+4]
        jnz     CTO_Failed1
        cmp     eax, [rchi.ttv.IrqTimeOut]
        ja      CTO_Failed1

        mov     eax, dword ptr [TimeOfLastSync]
        or      eax, dword ptr [TimeOfLastSync+4]
        jz      CTO_Failed2

        sub     ebx, dword ptr [TimeOfLastSync]
        sbb     ecx, dword ptr [TimeOfLastSync+4]
        jnz     CTO_Failed2
        cmp     ebx, [rchi.ttv.SyncTimeOut]
        ja      CTO_Failed2

        clc
        ret

CTO_Failed1:
        mov     eax, RCERR_NO_DATA
        stc
        ret

CTO_Failed2:
        mov     eax, RCERR_BAD_DATA
        stc
        ret

EndProc CheckTimeOuts


;==============================================================================
;
; Wait10us
;
; DESCRIPTION:
;
;   Wait for about 10us
;
;------------------------------------------------------------------------------
BeginProc Wait10us

        push    eax
        push    ebx
        push    ecx
        push    edx
        mov     ecx, 1000
        VxDCall VTD_Get_Real_Time
        mov     ebx,eax
WaitLoop:
        dec     ecx
        jz      TimeOut
        VxDCall VTD_Get_Real_Time
        sub     eax, ebx
        cmp     eax, 12
        jb      WaitLoop
TimeOut:
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        ret

EndProc Wait10us

;******************************************************************************
;            H A R D W A R E   I N T E R R U P T   R O U T I N E
;******************************************************************************

;==============================================================================
;
; RCCOM_IrqHandler
;
; DESCRIPTION:
;
;   This is the default handler for interrupt processing
;
; ENTRY: EAX = IRQ handle
;        EBX = VM handle
;
;------------------------------------------------------------------------------

BeginProc RCCOM_IrqHandler, High_Freq

IFDEF DEBUG
        inc     [IrqCounter]
ENDIF
        push    eax

        mov     eax, [COM_Handle]
        mov     edx, [eax.VCD_IObase]
        BasePort

        SetPort ACE_IIDR
        in      al, dx                  ; check if it is our IRQ
        test    al, IIR_NONE
        jnz     IH_NotMyIRQ

        SetPort ACE_MSR
        in      al, dx                  ; DCTS has to be changed
        test    al, ACE_DCTS
        jz      IH_NotMyIRQ

IFDEF DEBUG
        inc     [MyIrqCounter]
ENDIF

        cmp     [rchi.InvertedPulse], 0
        jz      PosSlope
        not     al
PosSlope:

        test    al, ACE_CTS                     ; check CTS
        pushfd
        VxDCall VTD_Get_Real_Time               ; get current time in EDX:EAX
        popfd
        jnz      IH_Finished

        mov     dword ptr [TimeOfLastIrq+4], edx
        xchg    dword ptr [TimeOfLastIrq], eax  ; set new starting value and get last one
        neg     eax
        add     eax, dword ptr [TimeOfLastIrq]  ; calc difference

        cmp     eax, [rchi.ttv.MaxSyncLen]           ; longer than a sync impuls can ever be?
        ja      IH_IsDisturbance

        cmp     eax, [rchi.ttv.SyncLen]              ; enought to be sync impuls?
        ja      IH_IsSync

        mov     edx, [rci.CurrentChannel]
        cmp     edx, MAX_CHANNELS               ; read max. # of channels
        jae     IH_Finished
        mov     [rci.RawValue+edx*4], eax        ; store channel value
        inc     [rci.CurrentChannel]            ; set to next channel
IH_Finished:

        pop     eax
        VxDCall VPICD_Phys_EOI
        clc                                     ; tell VPICD that we've handled the irq
        ret

IH_NotMyIRQ:
        pop     eax
        VxDCall VPICD_Phys_EOI
        stc                             ; tell VPICD that this irq was not handled
        ret

IH_IsSync:
        mov     eax, dword ptr [TimeOfLastIrq]   ; restore eax (edx still valid)
        mov     dword ptr [TimeOfLastSync], eax  ; save time of last sync
        mov     dword ptr [TimeOfLastSync+4], edx
        mov     edx, [rci.CurrentChannel]
IH_FillBuffer:
        cmp     edx, MAX_CHANNELS       ; mark unused space as invalid
        jae     IH_BufferFilled
        mov     [rci.RawValue+edx*4], INVALID_CHANNEL_VALUE
        inc     edx
        jmp     IH_FillBuffer
IH_BufferFilled:
        mov     [rci.CurrentChannel], 0     ; start with channel 0
        mov     [rci.Updated], 1
        jmp     IH_Finished

; we have received a disturbance impuls
IH_IsDisturbance:
IFDEF DEBUG
        Trace_Out "RCCOM: Disturbance: len=#EAX";
ENDIF
        xor     eax, eax
        mov     dword ptr [TimeOfLastSync], eax  ; set it to invalid
        mov     dword ptr [TimeOfLastSync+4], eax
        mov     [rci.CurrentChannel],MAX_CHANNELS
        jmp     IH_Finished

EndProc RCCOM_IrqHandler

VxD_LOCKED_CODE_ENDS

        end

