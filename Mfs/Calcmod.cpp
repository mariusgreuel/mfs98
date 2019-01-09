/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:		calcmod.cpp

  Version:	  1.00

  Date:		 18-Feb-97

  Author:	   MG

\***************************************************************************/

#include "stdafx.h"

#include "mgjoystick.h"
#include "types.h"
#include "3dengine.h"
#include "..\rccom\rccom.h"

/////////////////////////////////////////////////////////////////////////////
// Globals...

DWORD			g_dwCtrlDeviceType;		// type of control (0=RCBox, 1=Joystick)
DWORD			g_dwCtrlDeviceMode;		// control mode
DWORD			g_bCtrlDeviceFTBack;	// full throttle at the back

/////////////////////////////////////////////////////////////////////////////
// Statics...

float Stick[4] = { 0,0,0,0 };

BYTE rcModeTable[4][4] = { { 2, 0, 1, 3 }, { 2, 0, 3, 1 }, { 0, 2, 1, 3 }, { 0, 2, 3, 1 } };
BYTE jsModeTable[4][4] = { { 2, 0, 1, 3 }, { 2, 0, 3, 1 }, { 0, 2, 1, 3 }, { 0, 2, 3, 1 } };

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern CMGJoystick		g_Joystick;
extern CRCBox			g_RCBox;

extern float			g_fFrameTime;

extern C3DBspObject	*Airplane;

void InitDisp();

void CalcWind();
void AirplaneInit();
void AirplaneCalc(C3DMatrix &m);

void HeliInit1();
void HeliInit2();
void HeliCalc(C3DMatrix &m);
C3DVector &HeliGetGroundSpeed();
C3DVector &HeliGetRotateSpeed();

/////////////////////////////////////////////////////////////////////////////

BOOL CalcStickValues()
{
  switch (g_dwCtrlDeviceType) {
	case 0: {
	  if (!g_RCBox.Read())
		return FALSE;
	  for(int i=0; i<4; i++)
		Stick[ i ] = g_RCBox.GetPos(rcModeTable[ g_dwCtrlDeviceMode ][i]) / 128.0;
	  Stick[1] = -Stick[1];
	  break;
	}
	case 1: {
	  if (!g_Joystick.Read())
		return FALSE;
	  for(int i=0; i<4; i++)
		Stick[ i ] = g_Joystick.GetPos(jsModeTable[ g_dwCtrlDeviceMode ][i]) / 128.0;
	  break;
	}
	default:
	  return FALSE;
  }

  if (g_bCtrlDeviceFTBack)
	Stick[3] = -Stick[3];

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

void InitScene()
{
  if (Airplane == NULL)
	return;
  Airplane->m_Matrix.Init();
  HeliInit1();
}

void CalcScene()
{
  InitDisp();

  if (Airplane == NULL)
	return;
  if (!CalcStickValues())
	return;

  CalcWind();
  HeliCalc(Airplane->m_Matrix);
}

void MoveScene()
{
  if (Airplane == NULL)
	return;

  Airplane->Move(HeliGetGroundSpeed() * g_fFrameTime);
  Airplane->Rotate(HeliGetRotateSpeed() * g_fFrameTime * _2PI);

  if (Airplane->m_Matrix.v[H].v[Y] < 0.0) {
	Airplane->m_Matrix.v[H].v[Y] = 0.0;
	HeliInit2();
  }
}
