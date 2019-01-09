
#include "stdafx.h"

#include <stdio.h>
#include "display.h"

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HFONT            AppFont;

extern DWORD            g_dwFrameCount;
extern DWORD            g_dwFrameRate;

extern CDisplay         g_Display;

void DisplayCurrentVar();

/////////////////////////////////////////////////////////////////////////////
// Disp

BOOL DispEnabled = TRUE;
int  DispRow = 0;
char DispBuffer[20][80];

void Disp(LPSTR fmt, ...)
{
  if (DispRow < 20 && DispEnabled) {
    va_list arglist;
    va_start(arglist, fmt);
    vsprintf(DispBuffer[DispRow], fmt, arglist);
    va_end(arglist);
    DispRow++;
  }
}

void InitDisp()
{
  DispEnabled = TRUE;
  DispRow = 0;
}

BOOL DisplayInfo()
{
	HDC hdc;
	HFONT OldFont;

	Disp("Frame: %05d, FPS: %d.%d", g_dwFrameCount, g_dwFrameRate/10, g_dwFrameRate%10);
//	DisplayCurrentVar();

	if (!g_Display.GetDC(&hdc))
	return FALSE;

	SetBkMode(hdc, TRANSPARENT);
	OldFont = (HFONT)SelectObject(hdc, AppFont);

	SetTextColor(hdc, RGB(255, 255, 255));
	for(int i=0; i<DispRow; i++)
		TextOut(hdc, 40, 16*i, DispBuffer[i], lstrlen(DispBuffer[i]));
	DispEnabled = TRUE;

	SelectObject(hdc, OldFont);

	g_Display.ReleaseDC(hdc);

	DispEnabled = FALSE;
	return TRUE;
}


