/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     display.h

 Version:   1.00

 Date:      13-Jan-97

 Author:    MG

\***************************************************************************/

#ifndef __DISPLAY_H_INCLUDED__
#define __DISPLAY_H_INCLUDED__

#include <ddraw.h>

/////////////////////////////////////////////////////////////////////////////

//#define ACCEL_GFX8              TRUE

#define MAX_PALETTEENTRIES      128     // # of entries in our user palette

#define COLOR_NULL              -1
#define COLOR_TRANSLUCENT       -2
#define COLOR_GROUND            60
#define COLOR_SKY               61
#define COLOR_GRIDGROUND        62
#define COLOR_GRIDSKY           63

/////////////////////////////////////////////////////////////////////////////
// Structures...

// direct draw error descriptor
typedef struct tagDDERRORDESC {
    HRESULT         ddrval;             // error code
    const char      *str;               // points to additional error description
    int             line;               // line in which the error occured
}DDERRORDESC;

/////////////////////////////////////////////////////////////////////////////
// Direct Draw base class

class CDisplay {
public:
    CDisplay();
    ~CDisplay();

    BOOL Initialize();
    void Terminate();

    BOOL SetMode( DWORD dwWidth, DWORD dwHeight, WORD bpp, BOOL bFullscreen );

    void GetBackBufferRect( LPRECT lprc );

    BOOL Lock();
    BOOL Unlock();
    BOOL GetDC( HDC *hdc );
    BOOL ReleaseDC( HDC hdc );
    BOOL ShowBackBuffer();

    BOOL ColorFill( LPRECT lprc, DWORD Color );

    BOOL SetBufferPos( int x, int y );
    BOOL SetBufferSize( int cx, int cy );
    BOOL PaletteChanged();

    BOOL SwitchToMenuMode();
    BOOL FillModesComboBox( HWND hWnd );

    BOOL IsFullscreenMode() { return m_bFullscreen; }
    BOOL IsMoveAllowed() { return m_bMoveAllowed; }
    BOOL IsPaletteMode() { return m_bPaletteMode; }

protected:
    BOOL CreateObjects();
    void ReleaseObjects();
    BOOL CreateBuffers();
    BOOL CreatePalette();
    BOOL CreateFont();
    BOOL RestoreSurfaces();

private:
    HINSTANCE           m_hDirectDrawDll;

    LPDIRECTDRAW        m_dd;
    LPDIRECTDRAWSURFACE m_lpFrontBuffer;
    LPDIRECTDRAWSURFACE m_lpBackBuffer;
    LPDIRECTDRAWCLIPPER m_lpClipper;
    LPDIRECTDRAWPALETTE m_lpPalette;

    DDERRORDESC         m_dded;

    BOOL                m_bFullscreen;
    BOOL                m_bMoveAllowed;
    BOOL                m_bPaletteMode;

    POINT               m_ptClient;
    SIZE                m_szClient;
    RECT                m_rcPlacement;

    DWORD               m_dwWidth;
    DWORD               m_dwHeight;
    WORD                m_wBpp;

};

#endif //__DISPLAY_H_INCLUDED__


