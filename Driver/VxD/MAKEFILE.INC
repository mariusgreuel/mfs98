############################################################################
#
#  File:    makefile.inc
#  Content: NT build file for building the parts that SOURCES can't build
#
############################################################################

!ifndef DDB_NAME
DDB_NAME = $(NAME)_DDB
!endif

#ifdef $(NAME)_DEVICE_ID
VER_FILESUBTYPE = $(NAME)_DEVICE_ID
#else
VER_FILESUBTYPE = UNDEFINED_DEVICE_ID
#endif

#ifdef $(PASS0ONLY)
#   Avoid building the link components on the first pass
$(NAME).def \
$(NAME).rc \
$O\$(NAME).res:

#endif

$(NAME).def: makefile.inc sources
    copy <<$(NAME).DEF nul
;   THIS FILE IS GENERATED FROM MAKEFILE.INC; MAKE CHANGES THERE
VXD $(NAME) DYNAMIC
DESCRIPTION '$(DESCRIPTION)'
EXETYPE         DEV386
PROTMODE

SEGMENTS
    _LPTEXT         CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    _LTEXT          CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    _LDATA          CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    _TEXT           CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    _DATA           CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    CONST           CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    _TLS            CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    _BSS            CLASS 'LCODE'   PRELOAD NONDISCARDABLE
    _LMSG           CLASS 'MCODE'   PRELOAD NONDISCARDABLE IOPL
    _IMSG           CLASS 'MCODE'   PRELOAD DISCARDABLE IOPL
    _ITEXT          CLASS 'ICODE'   DISCARDABLE
    _IDATA          CLASS 'ICODE'   DISCARDABLE
    _PTEXT          CLASS 'PCODE'   NONDISCARDABLE
    _PMSG           CLASS 'MCODE'   NONDISCARDABLE IOPL
    _PDATA          CLASS 'PDATA'   NONDISCARDABLE SHARED
    _STEXT          CLASS 'SCODE'   RESIDENT
    _SDATA          CLASS 'SCODE'   RESIDENT
    _DBOSTART       CLASS 'DBOCODE' PRELOAD NONDISCARDABLE CONFORMING
    _DBOCODE        CLASS 'DBOCODE' PRELOAD NONDISCARDABLE CONFORMING
    _DBODATA        CLASS 'DBOCODE' PRELOAD NONDISCARDABLE CONFORMING
    _16ICODE        CLASS '16ICODE' PRELOAD DISCARDABLE
    _RCODE          CLASS 'RCODE'
EXPORTS
    $(DDB_NAME) @1
<<KEEP

$(NAME).rc: makefile.inc sources
    copy <<$(NAME).RC nul
/* THIS FILE IS GENERATED FROM MAKEFILE.INC; MAKE CHANGES THERE */

#define Not_VxD
#include <windows.h>
#include "version.h"

#define VER_FILEDESCRIPTION_STR     "R/C communication device"
#define VER_INTERNALNAME_STR        "rccom.drv"
#define VER_FILETYPE                VFT_DRV
#define VER_FILESUBTYPE             0
#define VER_FILEVERSION             4,0,VER_MAJOR,VER_MINOR
#define VER_FILEVERSION_STR         VER_STRING

#include "..\..\common\mfs.ver"

<<KEEP
