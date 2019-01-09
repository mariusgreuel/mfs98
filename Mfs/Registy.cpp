/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 registry.cpp

 Version:   1.00

 Date:	  20-Jan-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "types.h"
#include "mgstring.h"
#include "mgregkey.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern WNDPLACEMENT	g_WndInfo;

extern DWORD		g_bShowToolbar;
extern DWORD		g_bShowStatusbar;
extern DWORD		g_bUseGDI;

extern DWORD		g_dwCtrlDeviceType;
extern DWORD		g_dwCtrlDeviceMode;
extern DWORD		g_bCtrlDeviceFTBack;

/////////////////////////////////////////////////////////////////////////////

static LPCSTR   szWndPlace		  = "WindowPlacement";
static LPCSTR   szShowToolbar	   = "ShowToolbar";
static LPCSTR   szShowStatusbar	 = "ShowStatusbar";
static LPCSTR   szUseGDI			= "UseGDIDrawingFunctions";
static LPCSTR   szCtrlDeviceType	= "ControllerDeviceType";
static LPCSTR   szCtrlDeviceMode	= "ControllerDeviceMode";
static LPCSTR   szCtrlDeviceFTBK	= "FullThrottleBack";

static void InitializeRegistrySettings()
{
	g_WndInfo.rcPlacement.left = CW_USEDEFAULT;
	g_WndInfo.rcPlacement.top = CW_USEDEFAULT;
	g_WndInfo.rcPlacement.right = CW_USEDEFAULT;
	g_WndInfo.rcPlacement.bottom = CW_USEDEFAULT;
	g_WndInfo.szResolution.cx = 640;
	g_WndInfo.szResolution.cy = 480;
	g_WndInfo.wBpp = 8;
	g_WndInfo.bFullscreen = FALSE;

	g_bShowToolbar = TRUE;
	g_bShowStatusbar = TRUE;
	g_bUseGDI = FALSE;

	g_dwCtrlDeviceType = 0;
	g_dwCtrlDeviceMode = 1;
	g_bCtrlDeviceFTBack = TRUE;
}

BOOL LoadRegistrySettings()
{
	CMGRegKey Reg;

	CMGString strKey = CMGString(IDS_REGK_MFS) + '\\' + CMGString(IDS_REGK_MFS_VER);

	if (!Reg.Open(HKCU, strKey) ||
		!Reg.QueryValue(&g_WndInfo, sizeof(g_WndInfo), szWndPlace) ||
		!Reg.QueryValue(g_bShowToolbar, szShowToolbar) ||
		!Reg.QueryValue(g_bShowStatusbar, szShowStatusbar) ||
		!Reg.QueryValue(g_bUseGDI, szUseGDI) ||
		!Reg.QueryValue(g_dwCtrlDeviceType, szCtrlDeviceType) ||
		!Reg.QueryValue(g_dwCtrlDeviceMode, szCtrlDeviceMode) ||
		!Reg.QueryValue(g_bCtrlDeviceFTBack, szCtrlDeviceFTBK)
	) {
		InitializeRegistrySettings();
		return FALSE;
	}

	return TRUE;
}

BOOL SaveRegistrySettings()
{
	CMGRegKey Reg;

	CMGString strKey = CMGString(IDS_REGK_MFS) + '\\' + CMGString(IDS_REGK_MFS_VER);

	if (!Reg.Create(HKCU, strKey) ||
		!Reg.SetValue(&g_WndInfo, sizeof(g_WndInfo), szWndPlace) ||
		!Reg.SetValue(g_bShowToolbar, szShowToolbar) ||
		!Reg.SetValue(g_bShowStatusbar, szShowStatusbar) ||
		!Reg.SetValue(g_bUseGDI, szUseGDI) ||
		!Reg.SetValue(g_dwCtrlDeviceType, szCtrlDeviceType) ||
		!Reg.SetValue(g_dwCtrlDeviceMode, szCtrlDeviceMode) ||
		!Reg.SetValue(g_bCtrlDeviceFTBack, szCtrlDeviceFTBK)
	 )
		return FALSE;

	return TRUE;
}


