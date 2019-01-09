;******************************************************************************
;
;  Copyright (C) 1997 Marius Greuel. All rights reserved.
;
;  Title:       mydebug.asm
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

        .386p

;******************************************************************************
;                             I N C L U D E S
;******************************************************************************

        .xlist
        include VMM.inc
        .list

;******************************************************************************
;                           L O C A L   D A T A
;******************************************************************************

MAX_STRING_LEN  equ 80

VxD_LOCKED_DATA_SEG

szCR            db 0dh, 0
TextBuffer      db ( MAX_STRING_LEN + 8 + 1 ) dup(?)

VxD_LOCKED_DATA_ENDS

;******************************************************************************
;                         D E B U G   R O U T I N E S
;******************************************************************************

VxD_CODE_SEG

        public MYDEBUG_OutString
        public MYDEBUG_OutCR

StoreHex8 proc near
        ror     edx,8
        mov     ch,2
        jmp     StoreHex
StoreHex8 endp

StoreHex16 proc near
        ror     edx,16
        mov     ch,4
        jmp     StoreHex
StoreHex16 endp

StoreHex32 proc near
        mov     ch,8
StoreHex32 endp

StoreHex proc
SH32_1: rol     edx, 4
        mov     al, dl
        and     al, 0fh
        add     al, '0'
        cmp     al, '9'
        jbe     SH32_2
        add     al, 'A' - '9' - 1
SH32_2: stosb
        dec     ch
        jnz     SH32_1
        ret
StoreHex endp

Base    equ 2*4
GetRegValue proc near
        mov edx,[esp+Base+7*4]
        cmp al,'a'
        jz G1
        mov edx,[esp+Base+4*4]
        cmp al,'b'
        jz G1
        mov edx,[esp+Base+6*4]
        cmp al,'c'
        jz G1
        mov edx,[esp+Base+5*4]
        cmp al,'d'
        jz G1
        mov edx,[esp+Base+0*4]
        cmp al,'D'
        jz G1
        mov edx,[esp+Base+1*4]
        cmp al,'S'
        jz G1
        mov edx,[esp+Base+2*4]
        cmp al,'B'
        jz G1
        mov edx,-1
G1:     ret
GetRegValue endp

BeginProc MYDEBUG_OutString
        cld
        mov     cl, MAX_STRING_LEN
        mov     edi, offset TextBuffer
        jmp     L2
L1:     stosb
        or      al, al
        jz      L3
        dec     cl
        jz      L3
L2:     lodsb
        cmp     al, '#'
        jnz     L1
        lodsb
        or      al,20h
        cmp     al,'e'
        jz      Is_Extended
        mov     ah,[esi]
        inc     esi
        or      ah,20h
        cmp     ah,'x'
        jz      Disp_Word
        cmp     ah,'h'
        jz      Disp_HighByte
        cmp     ah,'l'
        jz      Disp_LowByte
        and     al, not 20h
        jmp     Disp_Word
Is_Extended:
        lodsb
        or      al,20h
        mov     ah,[esi]
        inc     esi
        or      ah,20h
        cmp     ah,'x'
        jz      Disp_Extended
        and     al,not 20h
Disp_Extended:
        call    GetRegValue
        call    StoreHex32
        jmp     L2
Disp_Word:
        call    GetRegValue
        call    StoreHex16
        jmp     L2
Disp_HighByte:
        call    GetRegValue
        mov     dl,dh
        call    StoreHex8
        jmp     L2
Disp_LowByte:
        call    GetRegValue
        call    StoreHex8
        jmp     L2
L3:     mov     esi,offset TextBuffer
        VxDCall Out_Debug_String
        ret
EndProc MYDEBUG_OutString

BeginProc MYDEBUG_OutCR
        mov     esi,offset szCR
        VxDCall Out_Debug_String
        ret
EndProc MYDEBUG_OutCR

VxD_CODE_ENDS

        end

