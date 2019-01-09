# Microsoft Developer Studio Project File - Name="Mfs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Mfs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Mfs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Mfs.mak" CFG="Mfs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Mfs - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Mfs - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Mfs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../mglib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 mglib.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /machine:I386 /out:"../bin/Mfs.exe" /libpath:"../../mglib"

!ELSEIF  "$(CFG)" == "Mfs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../mglib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mglibd.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../bin/Mfs.exe" /pdbtype:sept /libpath:"../../mglib"

!ENDIF 

# Begin Target

# Name "Mfs - Win32 Release"
# Name "Mfs - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\3dengine.cpp
# End Source File
# Begin Source File

SOURCE=.\3dmath.cpp
# End Source File
# Begin Source File

SOURCE=.\Airplane.cpp
# End Source File
# Begin Source File

SOURCE=.\Calcmod.cpp
# End Source File
# Begin Source File

SOURCE=.\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\Heli.cpp
# End Source File
# Begin Source File

SOURCE=.\Infofile.cpp
# End Source File
# Begin Source File

SOURCE=.\Keys.cpp
# End Source File
# Begin Source File

SOURCE=.\Menu.cpp
# End Source File
# Begin Source File

SOURCE=.\Mfs.cpp
# End Source File
# Begin Source File

SOURCE=.\Misc.cpp
# End Source File
# Begin Source File

SOURCE=.\Registy.cpp
# End Source File
# Begin Source File

SOURCE=.\Script.cpp
# End Source File
# Begin Source File

SOURCE=.\Stats.cpp
# End Source File
# Begin Source File

SOURCE=.\Statusbar.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Toolbar.cpp
# End Source File
# Begin Source File

SOURCE=.\Toolbox.cpp
# End Source File
# Begin Source File

SOURCE=.\Variable.cpp
# End Source File
# Begin Source File

SOURCE=.\Vars.cpp
# End Source File
# Begin Source File

SOURCE=.\Wind.cpp
# End Source File
# Begin Source File

SOURCE=.\Winmain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\3dEngine.h
# End Source File
# Begin Source File

SOURCE=.\3dMath.h
# End Source File
# Begin Source File

SOURCE=.\3dTypes.h
# End Source File
# Begin Source File

SOURCE=.\Display.h
# End Source File
# Begin Source File

SOURCE=.\Gfx8.h
# End Source File
# Begin Source File

SOURCE=.\INFOFILE.H
# End Source File
# Begin Source File

SOURCE=.\Res_strg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Toolbox.h
# End Source File
# Begin Source File

SOURCE=.\Types.h
# End Source File
# Begin Source File

SOURCE=.\Variable.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc"
# Begin Source File

SOURCE=.\res\Joystick.ico
# End Source File
# Begin Source File

SOURCE=.\res\Mfs.ico
# End Source File
# Begin Source File

SOURCE=.\Mfs.rc
# End Source File
# Begin Source File

SOURCE=.\res\New.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Screen.ico
# End Source File
# Begin Source File

SOURCE=.\res\Sound.ico
# End Source File
# Begin Source File

SOURCE=.\res\Splash.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\RCCom.lib
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dlg_About.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg_FileNew.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg_FileOpen.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg_FileProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg_FileSave.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg_Options.cpp
# End Source File
# Begin Source File

SOURCE=.\Dlg_Parameters.cpp
# End Source File
# End Group
# End Target
# End Project
