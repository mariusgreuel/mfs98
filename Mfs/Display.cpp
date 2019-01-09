/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     display.cpp

 Version:   1.00

 Date:      13-Jan-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "gfx8.h"
#include "display.h"

/////////////////////////////////////////////////////////////////////////////
// Defines...

#define RELEASE(x) if (x != NULL) { x->Release(); x = NULL; }

#define TRY(exp) { \
            if (!exp) { \
                m_dded.line = __LINE__; \
                goto exit_with_error;   \
            }                           \
        }

// if exp != DD_OK, fill out m_dded and goto exit_with_error
#define ASSERTDD(exp, text) {         \
            register int res = (exp);   \
            if (res != DD_OK) {        \
                SetLastError(res);    \
                MGDBG0(DMSG_DDAPIERROR, (text));  \
                goto exit_with_error;   \
            }                           \
        }

// same as ASSERTDD, but check for lost surfaces
#define ASSERTDDSF(exp, text) {       \
            register int res = (exp);   \
            if (res == DDERR_SURFACELOST) { \
                if (RestoreSurfaces()) { \
                    res = (exp); \
                } \
            } \
            if (res != DD_OK) {        \
                SetLastError(res);    \
                MGDBG0(DMSG_DDAPIERROR, (text));  \
                goto exit_with_error;   \
            }                           \
        }

typedef HRESULT (WINAPI *DIRECTDRAWCREATE)(GUID *, LPDIRECTDRAW *, IUnknown *);

/////////////////////////////////////////////////////////////////////////////
// Globals...

PALETTEENTRY            g_upe[MAX_PALETTEENTRIES];  // user defined palette entries
DWORD                   g_mpe[MAX_PALETTEENTRIES];  // matched palette entries

HFONT                   AppFont;

/////////////////////////////////////////////////////////////////////////////
// Locals...

MGMODULE(__FILE__);

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HWND             g_hWnd;             // defined in winmain.cpp
extern BOOL             g_bAppHalted;       // defined in winmain.cpp

LPCTSTR MakeDDErrorStr(HRESULT hErr);     // defined in dxerror.cpp

/////////////////////////////////////////////////////////////////////////////
// Direct Draw class...

CDisplay::CDisplay()
{
    m_hDirectDrawDll = NULL;

    m_dd = NULL;
    m_lpFrontBuffer = m_lpBackBuffer = NULL;
    m_lpClipper = NULL;
    m_lpPalette = NULL;

    m_bFullscreen = FALSE;
    m_bMoveAllowed = TRUE;
    m_bPaletteMode = FALSE;

    m_dwWidth  = 640;
    m_dwHeight = 480;
    m_wBpp     = 8;
}


CDisplay::~CDisplay()
{
    if (m_hDirectDrawDll != NULL)
        FreeLibrary(m_hDirectDrawDll);
}


// initialize DirectDraw object
BOOL CDisplay::Initialize()
{
    DIRECTDRAWCREATE    DirectDrawCreate;

    MGDBG0(DMSG_INFO, "Initialize DirectDraw object\n");

    m_hDirectDrawDll = LoadLibrary("DDRAW.DLL");
    if (m_hDirectDrawDll == NULL) {
        MGDBG0(DMSG_APIERROR, "Couldn't load DirectDraw library\n");
        return FALSE;
    }
    DirectDrawCreate = (DIRECTDRAWCREATE)GetProcAddress(m_hDirectDrawDll, "DirectDrawCreate");
    if (DirectDrawCreate == NULL) {
        MGDBG0(DMSG_APIERROR, "Couldn't obtain DirectDrawCreate entry\n");
        return FALSE;
    }

    ASSERTDD(DirectDrawCreate(NULL, &m_dd, NULL), "Creation of IDirectDraw failed");

    return TRUE;

exit_with_error:
    return FALSE;
}

// clean up
void CDisplay::Terminate()
{
    ReleaseObjects();
    RELEASE(m_dd);
}


// set display mode (either windowed or fullscreen mode)
BOOL CDisplay::SetMode(DWORD dwWidth, DWORD dwHeight, WORD wBpp, BOOL bFullscreen)
{
    MGDBG4(DMSG_INFO, "SetMode %s mode: (%ux%ux%u)\n",
        bFullscreen ? "fullscreen" : "windowed",
        dwWidth, dwHeight, wBpp);

    // get rid of old stuff
    ReleaseObjects();

    m_bMoveAllowed = TRUE;

    if (bFullscreen) {

        if (!m_bFullscreen) {
            // previous mode was windowed mode
            m_bFullscreen = TRUE;
            // save window position
            GetWindowRect(g_hWnd, &m_rcPlacement);
            // set cooperative level
            ASSERTDD(
                m_dd->SetCooperativeLevel(g_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN),//| DDSCL_ALLOWMODEX),
                "SetCooperativeLevel(Exclusive) failed");
        }

        if (dwWidth == 0 || dwHeight == 0 || wBpp == 0) {
            dwWidth  = m_dwWidth;
            dwHeight = m_dwHeight;
            wBpp     = m_wBpp;
        }

        // set new display mode
        ASSERTDD(m_dd->SetDisplayMode(dwWidth, dwHeight, wBpp), "SetDisplayMode failed");

        // save mode info
        m_dwWidth  = dwWidth;
        m_dwHeight = dwHeight;
        m_wBpp     = wBpp;

        // stretch window to screen
        MoveWindow(g_hWnd, 0, 0, dwWidth, dwHeight, TRUE);

        m_bMoveAllowed = FALSE;

    } else {

        if (m_bFullscreen) {

            // previous mode was fullscreen mode
            m_bFullscreen = FALSE;
            m_dd->RestoreDisplayMode();

            // restore window pos
            MoveWindow(
                g_hWnd,
                m_rcPlacement.left,
                m_rcPlacement.top,
                m_rcPlacement.right - m_rcPlacement.left,
                m_rcPlacement.bottom - m_rcPlacement.top,
                TRUE);
        }

        // set cooperative level
        ASSERTDD(
            m_dd->SetCooperativeLevel(g_hWnd, DDSCL_NORMAL),
            "SetCooperativeLevel(Normal) failed");

    }

//    ReleaseObjects();
//    CreateObjects();

    return TRUE;

exit_with_error:
    // destroy all objects
    ReleaseObjects();
    // make sure that we leave without exclusive mode
    m_dd->SetCooperativeLevel(g_hWnd, DDSCL_NORMAL);

    return FALSE;
}


// create all objects
BOOL CDisplay::CreateObjects()
{
    TRY(CreateBuffers());
    TRY(CreatePalette());
    TRY(CreateFont());
    TRY(PaletteChanged());

    return TRUE;

exit_with_error:
    return FALSE;
}


// free all objects
void CDisplay::ReleaseObjects()
{
    if (AppFont) {
        DeleteObject(AppFont);
        AppFont = NULL;
    }
    RELEASE(m_lpPalette);
    RELEASE(m_lpBackBuffer);
    RELEASE(m_lpFrontBuffer);
}


// create primary surface and back buffer objects
BOOL CDisplay::CreateBuffers()
{
    DDSURFACEDESC   ddsd;
    DDSCAPS         caps;

    if (m_bFullscreen) {

        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        ddsd.dwBackBufferCount = 2;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
        MGDBG0(DMSG_INFO, "Creating complex flipping surface for fullscreen mode: ");
        m_dded.ddrval = m_dd->CreateSurface(&ddsd, &m_lpFrontBuffer, NULL);
        if (m_dded.ddrval == DDERR_OUTOFMEMORY || m_dded.ddrval == DDERR_OUTOFVIDEOMEMORY) {
            ddsd.dwBackBufferCount = 1;
            m_dded.ddrval = m_dd->CreateSurface(&ddsd, &m_lpFrontBuffer, NULL);
            if (m_dded.ddrval == DDERR_OUTOFMEMORY || m_dded.ddrval == DDERR_OUTOFVIDEOMEMORY) {
                ddsd.ddsCaps.dwCaps = ddsd.ddsCaps.dwCaps & ~DDSCAPS_VIDEOMEMORY | DDSCAPS_SYSTEMMEMORY;
                m_dded.ddrval = m_dd->CreateSurface(&ddsd, &m_lpFrontBuffer, NULL);
            }
        }
        ASSERTDD(m_dded.ddrval, "CreateSurface for fullscreen flipping surface failed");
        if ((ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) == 0)
            MGDBG0(DMSG_INFO, "Got system memory\n");
        else if (ddsd.dwBackBufferCount == 2)
            MGDBG0(DMSG_INFO, "Got triple buffered VRAM\n");
        else 
            MGDBG0(DMSG_INFO, "Got double buffered VRAM\n");
        // get the pointer to the back buffer
        caps.dwCaps = DDSCAPS_BACKBUFFER;
        ASSERTDD(m_lpFrontBuffer->GetAttachedSurface(&caps, &m_lpBackBuffer), "GetAttachedSurface to get back buffer failed");
        // create a clipper for the primary surface
#ifndef WINDOWS_NT
        // clippers in fullscreen don't work with NT...
        ASSERTDD(m_dd->CreateClipper(0, &m_lpClipper, NULL), "CreateClipper for primary window surface failed");
        ASSERTDD(m_lpClipper->SetHWnd(0, g_hWnd), "SetHWnd failed");
        ASSERTDD(m_lpFrontBuffer->SetClipper(m_lpClipper), "SetClipper failed");
#endif // WINDOWS_NT

    } else {

        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        MGDBG0(DMSG_INFO, "Creating primary surface and back buffer: ");
        ASSERTDD(m_dd->CreateSurface(&ddsd, &m_lpFrontBuffer, NULL), "CreateSurface for window front buffer failed");
        // and a back buffer which is an offscreen plane surface
        ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
        ddsd.dwWidth = m_szClient.cx;
        ddsd.dwHeight = m_szClient.cy;
        m_dded.ddrval = m_dd->CreateSurface(&ddsd, &m_lpBackBuffer, NULL);
        if (m_dded.ddrval == DDERR_OUTOFMEMORY || m_dded.ddrval == DDERR_OUTOFVIDEOMEMORY) {
            ddsd.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
            m_dded.ddrval = m_dd->CreateSurface(&ddsd, &m_lpBackBuffer, NULL);
                            
        }
        ASSERTDD(m_dded.ddrval, "CreateSurface for window back buffer failed");
        if ((ddsd.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) == 0)
            MGDBG0(DMSG_INFO, "Got system memory\n");
        else
            MGDBG0(DMSG_INFO, "Got offscreen VRAM\n");
        // create a clipper for the primary surface
        ASSERTDD(m_dd->CreateClipper(0, &m_lpClipper, NULL), "CreateClipper for primary window surface failed");
        ASSERTDD(m_lpClipper->SetHWnd(0, g_hWnd), "SetHWnd failed");
        ASSERTDD(m_lpFrontBuffer->SetClipper(m_lpClipper), "SetClipper failed");

    }
    // determine, if the we are in palette mode
    DDPIXELFORMAT ddpf;
    ZeroMemory(&ddpf, sizeof(ddpf));
    ddpf.dwSize = sizeof(DDPIXELFORMAT);
    ASSERTDD(m_lpBackBuffer->GetPixelFormat(&ddpf), "GetPixelFormat failed");
    m_bPaletteMode = ddpf.dwRGBBitCount == 8 ? TRUE : FALSE;

    return TRUE;

exit_with_error:
    RELEASE(m_lpBackBuffer);
    RELEASE(m_lpFrontBuffer);
    return FALSE;
}


// create a palette, if we are in palette mode
BOOL CDisplay::CreatePalette()
{
    HDC             hdc;
    PALETTEENTRY    ape[256];

    if (!m_bPaletteMode)
        return TRUE;

    MGDBG0(DMSG_INFO, "Creating palette\n");

    GetPaletteEntries((HPALETTE)GetStockObject(DEFAULT_PALETTE), 0,  10, &ape[0]);
    GetPaletteEntries((HPALETTE)GetStockObject(DEFAULT_PALETTE), 10, 10, &ape[246]);

    for(int i=0; i<MAX_PALETTEENTRIES/2; i++) {
        ape[i+10] = g_upe[i];
        ape[i+10+128] = g_upe[i+MAX_PALETTEENTRIES/2];
    }

    ASSERTDD(m_dd->CreatePalette(DDPCAPS_8BIT, ape, &m_lpPalette, NULL), "CreatePalette failed");

    return TRUE;

exit_with_error:
    ::ReleaseDC(NULL, hdc);
    RELEASE(m_lpPalette);
    return FALSE;
}


// create our app font
BOOL CDisplay::CreateFont()
{
    AppFont = ::CreateFont(
            m_szClient.cy < 800 ? 14 : 18,
            0, 0, 0,
            FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            NONANTIALIASED_QUALITY,
            VARIABLE_PITCH,
            "Arial");
    return TRUE;
}


// restore buffers
BOOL CDisplay::RestoreSurfaces()
{
    if (m_lpFrontBuffer->Restore() != DD_OK || m_lpBackBuffer->Restore() != DD_OK) {
        // try to create new surfaces, if the surfaces can't be restored
        ReleaseObjects();
        TRY(CreateObjects());
    }
    return TRUE;

exit_with_error:
    return FALSE;
}


// get the size of the back buffer (in client coords)
void CDisplay::GetBackBufferRect(LPRECT lprc)
{
    SetRect(lprc, 0, 0, m_szClient.cx, m_szClient.cy);
}


// lock the surface
BOOL CDisplay::Lock()
{
    DDSURFACEDESC   ddsd;

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ASSERTDDSF(m_lpBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL), "Lock failed");
#ifdef ACCEL_GFX8
    _InitGfx8(ddsd.lpSurface, ddsd.lPitch);
#endif
    return TRUE;
    
exit_with_error:
    return FALSE;
}


// unlock the surface
BOOL CDisplay::Unlock()
{
    DDSURFACEDESC ddsd;

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ASSERTDDSF(m_lpBackBuffer->Unlock(NULL), "Unlock failed");
    return TRUE;
    
exit_with_error:
    return FALSE;
}


// get DC handle
BOOL CDisplay::GetDC(HDC *lphdc)
{
    ASSERTDDSF(m_lpBackBuffer->GetDC(lphdc), "GetDC failed");
    return TRUE;

exit_with_error:
    return FALSE;
}


// free DC handle
BOOL CDisplay::ReleaseDC(HDC hdc)
{
    ASSERTDDSF(m_lpBackBuffer->ReleaseDC(hdc), "ReleaseDC failed");
    return TRUE;

exit_with_error:
    return FALSE;
}


// show back buffer (either flip or blt)
BOOL CDisplay::ShowBackBuffer()
{
    if (m_bFullscreen && !g_bAppHalted) {
        // if we are in fullscreen mode, flip to the back buffer
        ASSERTDDSF(m_lpFrontBuffer->Flip(NULL, DDFLIP_WAIT), "Flip failed");
    } else {
        // if we are in windowed mode, blt the back buffer in the primary surface
        RECT rcSrc, rcDest;
        SetRect(&rcSrc, 0, 0, m_szClient.cx, m_szClient.cy);
        SetRect(&rcDest, m_ptClient.x, m_ptClient.y, m_ptClient.x + m_szClient.cx, m_ptClient.y + m_szClient.cy);
        ASSERTDDSF(m_lpFrontBuffer->Blt(&rcDest, m_lpBackBuffer, &rcSrc, DDBLT_WAIT, NULL), "Blt failed");
    }
    return TRUE;

exit_with_error:
    return FALSE;
}


// fill rect with certain color
BOOL CDisplay::ColorFill(LPRECT lprc, DWORD Color)
{
    DDBLTFX ddbltfx;

    ZeroMemory(&ddbltfx, sizeof(ddbltfx));
    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = Color;

    ASSERTDDSF(m_lpBackBuffer->Blt(lprc, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx), "Blt failed");

    return TRUE;

exit_with_error:
    return FALSE;
}


BOOL CDisplay::SetBufferPos(int x, int y)
{
    POINT pt;

    pt.x = x;
    pt.y = y;

    ClientToScreen(g_hWnd, &pt);

    m_ptClient.x = pt.x;
    m_ptClient.y = pt.y;

    return TRUE;
}


BOOL CDisplay::SetBufferSize(int cx, int cy)
{
    if (m_szClient.cx != cx || m_szClient.cy != cy) {
        m_szClient.cx = cx;
        m_szClient.cy = cy;
        ReleaseObjects();
        TRY(CreateObjects());
    }
    return TRUE;

exit_with_error:
    return FALSE;
}


BOOL CDisplay::PaletteChanged()
{
    HDC             hdc;
    DDSURFACEDESC   ddsd;
    int             i;

    if (m_bPaletteMode)
        ASSERTDDSF(m_lpFrontBuffer->SetPalette(m_lpPalette), "SetPalette failed");

    // match the colors

    if (m_bPaletteMode && m_bFullscreen) {
        for(i=0; i<MAX_PALETTEENTRIES/2; i++) {
            g_mpe[i] = i+10;
            g_mpe[i+MAX_PALETTEENTRIES/2] = i+10+128;
        }
    } else {
        ASSERTDDSF(m_lpBackBuffer->GetDC(&hdc), "GetDC failed");
    
        for(i=0; i<MAX_PALETTEENTRIES; i++)
            SetPixel(hdc, i, 0, RGB(g_upe[i].peRed, g_upe[i].peGreen, g_upe[i].peBlue));
    
        ASSERTDDSF(m_lpBackBuffer->ReleaseDC(hdc), "ReleaseDC failed");
    
        ZeroMemory(&ddsd, sizeof(ddsd));
        ddsd.dwSize = sizeof(ddsd);
    
        ASSERTDDSF(m_lpBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL), "Lock failed");
    
        for(i=0; i<MAX_PALETTEENTRIES; i++) {
            g_mpe[i] = *(LPDWORD)((LPBYTE)ddsd.lpSurface + i * ddsd.ddpfPixelFormat.dwRGBBitCount / 8);
            if (ddsd.ddpfPixelFormat.dwRGBBitCount != 32)
                g_mpe[i] &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;
        }
    
        ASSERTDDSF(m_lpBackBuffer->Unlock(NULL), "Unlock failed");
    }

    return TRUE;

exit_with_error:
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////

// if we are in a modeX, switch to windowed mode
BOOL CDisplay::SwitchToMenuMode()
{
    DDSCAPS caps;

    if (m_bFullscreen) {
        m_lpFrontBuffer->GetCaps(&caps);
        if (caps.dwCaps & DDSCAPS_MODEX) {
            SetMode(0,0,0, FALSE);
        }
        m_dd->FlipToGDISurface();
    }
    return TRUE;
}


HRESULT CALLBACK ModeCallback(LPDDSURFACEDESC pdds, LPVOID lParam)
{
    HWND hWnd = (HWND)lParam;
    char szBuffer[32];

    int width   = pdds->dwWidth;
    int height  = pdds->dwHeight;
    int wBpp     = pdds->ddpfPixelFormat.dwRGBBitCount;

    wsprintf(szBuffer, "%dx%dx%d", width, height, wBpp);
    ComboBox_AddString(hWnd, szBuffer);

    int n = ComboBox_GetCount(hWnd);
    DWORD dwData = width | (height << 12) | (wBpp << 24);
    SendMessage(hWnd, CB_SETITEMDATA, n-1, dwData);
    return S_FALSE;
}


BOOL CDisplay::FillModesComboBox(HWND hWnd)
{
    ASSERTDD(m_dd->EnumDisplayModes(0, NULL, (LPVOID)hWnd, ModeCallback), "EnumDisplayModes failed");
    return TRUE;

exit_with_error:
    return FALSE;
}



