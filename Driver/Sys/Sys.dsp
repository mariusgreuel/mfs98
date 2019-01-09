# Microsoft Developer Studio Project File - Name="Sys" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=Sys - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sys.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sys.mak" CFG="Sys - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sys - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Sys - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "Sys - Win32 Release"

# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Sys___Wi"
# PROP BASE Intermediate_Dir "Sys___Wi"
# PROP BASE Cmd_Line "NMAKE /f Sys.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Sys.exe"
# PROP BASE Bsc_Name "Sys.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Sys___Wi"
# PROP Intermediate_Dir "Sys___Wi"
# PROP Cmd_Line "runbuild free"
# PROP Rebuild_Opt "-cf"
# PROP Target_File "RCCom.sys"
# PROP Bsc_Name "i386\free\RCCom.bsc"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Sys - Win32 Debug"

# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f Sys.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Sys.exe"
# PROP BASE Bsc_Name "Sys.bsc"
# PROP BASE Target_Dir ""
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "runbuild checked"
# PROP Rebuild_Opt "-cf"
# PROP Target_File "RCCom.sys"
# PROP Bsc_Name "i386\checked\RCCom.bsc"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Sys - Win32 Release"
# Name "Sys - Win32 Debug"

!IF  "$(CFG)" == "Sys - Win32 Release"

!ELSEIF  "$(CFG)" == "Sys - Win32 Debug"

!ENDIF 

# Begin Source File

SOURCE=.\RCCom.c
# End Source File
# Begin Source File

SOURCE=..\..\RCCom\Rccom.h
# End Source File
# Begin Source File

SOURCE=.\Uart.c
# End Source File
# Begin Source File

SOURCE=.\Uart.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# End Target
# End Project
