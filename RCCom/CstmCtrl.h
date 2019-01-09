/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     cstmctrl.h : custom controls: CCrossHair, CShiftBar

 Version:   1.00

 Date:      9-Jan-97

 Author:    MG

****************************************************************************/

#ifndef __CSTMCTRL_H_INCLUDED__
#define __CSTMCTRL_H_INCLUDED__

class CCrossHair {
public:
    CCrossHair();
    BOOL Create(HWND hWnd, BYTE bSize = 4);
    void Invalidate();
    void SetPos(POINT p);

protected:
    void DrawCrossHair(POINT p, BOOL MODE);

private:
    HWND        m_hWnd;
    BYTE        m_bSize;
    POINT       m_Pos;
};

class CShiftBar {
public:
    CShiftBar();
    BOOL Create(HWND hWnd);
    void Invalidate();
    void SetPos(int p);
	void DrawDisabledBar();

protected:
    void DrawBar(int p);

private:
    HWND        m_hWnd;
    int         m_Pos;
};

#endif // __CSTMCTRL_H_INCLUDED__


