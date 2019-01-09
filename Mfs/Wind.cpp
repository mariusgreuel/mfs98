/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        VarSectionWind.cpp

  Version:      1.00

  Date:         13-Jan-97

  Author:       MG

\***************************************************************************/

#include "stdafx.h"

#include "3dmath.h"
#include "variable.h"

/////////////////////////////////////////////////////////////////////////////
// Parameter...

extern CVarList         VarList;
extern CVarSection      Render;

#pragma init_seg(user)
CVarSection VarSectionWind(VarList, "Wind");

CFVar
  BaseSpeed(           VarSectionWind, "BaseSpeed",      1.2, 0.0, 100.0),
  SpeedChgFreq(        VarSectionWind, "SpeedChgFreq",   0.0, 0.0, 100.0),
  SpeedChgRange(       VarSectionWind, "SpeedChgRange",  0.0, 0.0, 100.0),
  SpeedRateOfChg(      VarSectionWind, "SpeedRateOfChg", 0.0, 0.0, 100.0),
  SpeedChgDurMin(      VarSectionWind, "SpeedChgDurMin", 0.0, 0.0, 100.0),
  SpeedChgDurMax(      VarSectionWind, "SpeedChgDurMax", 0.0, 0.0, 100.0),
  BaseDir(             VarSectionWind, "Direction",      0.0, 0.0, 360.0),
  DirChgFreq(          VarSectionWind, "DirChgFreq",     0.0, 0.0, 100.0),
  DirChgRange(         VarSectionWind, "DirChgRange",    0.0, 0.0, 100.0),
  DirRateOfChg(        VarSectionWind, "DirRateOfChg",   0.0, 0.0, 100.0),
  DirChgDurMin(        VarSectionWind, "DirChgDurMin",   0.0, 0.0, 100.0),
  DirChgDurMax(        VarSectionWind, "DirChgDurMax",   0.0, 0.0, 100.0),
  GustFreq(            VarSectionWind, "GustFreq",       0.0, 0.0, 100.0),
  GustPeakMin(         VarSectionWind, "GustPeakMin",    0.0, 0.0, 100.0),
  GustPeakMax(         VarSectionWind, "GustPeakMax",    0.0, 0.0, 100.0),
  GustDirMin(          VarSectionWind, "GustDirMin",     0.0, 0.0, 100.0),
  GustDirMax(          VarSectionWind, "GustDirMax",     0.0, 0.0, 100.0),
  GustDurMin(          VarSectionWind, "GustDurMin",     0.0, 0.0, 100.0),
  GustDurMax(          VarSectionWind, "GustDurMax",     0.0, 0.0, 100.0);

/////////////////////////////////////////////////////////////////////////////
// Globals...

C3DVector WindSpeed(0,0,0);

/////////////////////////////////////////////////////////////////////////////

extern void Disp(LPSTR fmt, ...);

void CalcWind()
{
  Disp("Wind %f", (float)BaseSpeed);

/*  float Lenght = BaseSpeed;
  float Angle = BaseDir;
  WindSpeed.v[X] = Lenght * cos(Angle / 360.0 * _2PI);
  WindSpeed.v[Y] = 0.0;
  WindSpeed.v[Z] = Lenght * sin(Angle / 360.0 * _2PI);
*/  WindSpeed.v[X] = 0.0;//Lenght * cos(Angle / 360.0 * _2PI);
  WindSpeed.v[Y] = 0.0;
  WindSpeed.v[Z] = 0.0;//Lenght * sin(Angle / 360.0 * _2PI);
}


