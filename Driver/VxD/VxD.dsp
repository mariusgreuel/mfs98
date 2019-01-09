# Microsoft Developer Studio Project File - Name="VxD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=VxD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "VxD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "VxD.mak" CFG="VxD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VxD - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "VxD - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "VxD - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f VxD.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "VxD.exe"
# PROP BASE Bsc_Name "VxD.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "runbuild free"
# PROP Rebuild_Opt "-cf"
# PROP Target_File "RCCom.vxd"
# PROP Bsc_Name "VxD.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "VxD - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f VxD.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "VxD.exe"
# PROP BASE Bsc_Name "VxD.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "runbuild checked"
# PROP Rebuild_Opt "-cf"
# PROP Target_File "RCCom.vxd"
# PROP Bsc_Name "VxD.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "VxD - Win32 Release"
# Name "VxD - Win32 Debug"

!IF  "$(CFG)" == "VxD - Win32 Release"

!ELSEIF  "$(CFG)" == "VxD - Win32 Debug"

!ENDIF 

# Begin Source File

SOURCE=.\Mydebug.asm
# End Source File
# Begin Source File

SOURCE=.\MyDebug.inc
# End Source File
# Begin Source File

SOURCE=.\Rccom.asm
# End Source File
# Begin Source File

SOURCE=.\Rccom.def
# End Source File
# Begin Source File

SOURCE=.\Rccom.inc
# End Source File
# Begin Source File

SOURCE=.\Uart.asm
# End Source File
# Begin Source File

SOURCE=.\Uart.inc
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# End Target
# End Project
