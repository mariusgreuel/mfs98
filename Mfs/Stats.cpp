/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        stats.cpp : statistics, time measurements, ...

  Version:      1.00

  Date:         3-Feb-97

  Author:       MG

\***************************************************************************/

#include "stdafx.h"

#include <mmsystem.h>

/////////////////////////////////////////////////////////////////////////////
// Globals...

float           g_fFrameTime;           // calculated frame time in [s]
DWORD           g_dwFrameRate;          // calculated frame rate in [frames/10s]
DWORD           g_dwFrameCount;         // # of frames

/////////////////////////////////////////////////////////////////////////////
// Locals...

static DWORD    dwCurrTime0;            // time at point zero (frame time)
static DWORD    dwFrameTime0;           // time at point zero (frame rate)
static DWORD    dwFrameCount0;          // # of frames at point zero


/////////////////////////////////////////////////////////////////////////////
// Externals...

void Disp(LPSTR fmt, ...);

/////////////////////////////////////////////////////////////////////////////

// calculate frame rate and frame time
void CalcFrameRate()
{
  DWORD dwCurrTime = timeGetTime();
  g_dwFrameCount++;

  if (dwFrameTime0 == 0)
    dwFrameTime0 = dwCurrTime;

  if (dwCurrTime - dwFrameTime0 > 1000) {
    g_dwFrameRate = (g_dwFrameCount - dwFrameCount0) * 10000 / (dwCurrTime - dwFrameTime0);
    dwFrameTime0 = dwCurrTime;
    dwFrameCount0 = g_dwFrameCount;
  }

  if (dwCurrTime - dwCurrTime0 <= 1 || dwCurrTime - dwCurrTime0 >= 1000)
    g_fFrameTime = (float)0.02;
  else
    g_fFrameTime = float(dwCurrTime - dwCurrTime0) / 1000.0;
  dwCurrTime0 = dwCurrTime;
}


