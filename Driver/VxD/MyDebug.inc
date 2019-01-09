;******************************************************************************
;
;  Copyright (C) 1997 Marius Greuel. All rights reserved.
;
;  Title:       mydebug.inc
;
;  Version:     1.00
;
;  Date:        20-Jan-97
;
;  Author:      MG
;
;------------------------------------------------------------------------------
;
;   Change log:
;
;      DATE     REV                 DESCRIPTION
;   ----------- --- -----------------------------------------------------------
;   20-Jan-1997 MG      initial implementation
;------------------------------------------------------------------------------

IFDEF DEBUG
        extrn MYDEBUG_OutString : proc
        extrn MYDEBUG_OutCR : proc
ENDIF


Trace_Out MACRO S, nocrlf
        LOCAL   Str_Off
IFDEF DEBUG
_LDATA SEGMENT
Str_Off db      S
IFB <nocrlf>
        db      0Dh,0Ah
ENDIF
        db      0
_LDATA ENDS
        pushfd
        pushad
        mov     esi, OFFSET32 Str_Off
        call    MYDEBUG_OutString
        popad
        popfd
ENDIF
        ENDM

Trace_Out_CR MACRO 
IFDEF DEBUG
        pushfd
        pushad
        call    MYDEBUG_OutCR
        popad
        popfd
ENDIF
        ENDM
