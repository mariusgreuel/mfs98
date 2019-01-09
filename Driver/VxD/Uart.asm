;
; Copyright (C) 1997 Marius Greuel. All rights reserved.
;
; Title:        uart.asm : Hardware support for the serial port
;
; Version:      1.00
;
; Date:         16-Aug-97
;
; Author:       MG
;

        .386

        .xlist
        include vmm.inc
        include uart.inc
        .list

VxD_LOCKED_CODE_SEG

;******************************************************************************
;                            U A R T   R O U T I N E S
;******************************************************************************

;==============================================================================
;
; UART_GetState
;
; DESCRIPTION: Get the complete state of the serial port. May be used for save/restore.
;
; EXTRY: com port base in EDX
;        UART_STATE structure in ESI
;
;------------------------------------------------------------------------------

BeginProc UART_GetState

        BasePort

        SetPort ACE_LCR
        in      al, dx
        mov     [esi.LineCtrl], al

        SetPort ACE_MCR
        in      al, dx
        mov     [esi.ModemCtrl], al

        SetPort ACE_IER
        in      al, dx
        mov     [esi.InterruptCtrl], al

        SetPort ACE_MSR
        in      al, dx          ; clean up
        SetPort ACE_RBR
        in      al, dx          ; clean up

        SetPort 0
        ret

EndProc UART_GetState

;==============================================================================
;
; UART_SetState
;
; DESCRIPTION: Set the complete state of the serial port. May be used for save/restore.
;
; EXTRY: com port base in EDX
;        UART_STATE structure in ESI
;
;------------------------------------------------------------------------------

BeginProc UART_SetState

        BasePort

        SetPort ACE_LCR
        mov     al, 0
        out     dx, al          ; DLAB = 0

        SetPort ACE_MCR
        mov     al, [esi.ModemCtrl]
        out     dx, al
        SetPort ACE_IER
        mov     al, [esi.InterruptCtrl]
        out     dx, al
        SetPort ACE_LCR
        mov     al, [esi.LineCtrl]
        out     dx, al

        SetPort 0
        ret

EndProc UART_SetState

VxD_LOCKED_CODE_ENDS

        end


