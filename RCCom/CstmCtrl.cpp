/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:		cstmctrl.cpp : custom controls: CCrossHair, CShiftBar

 Version:   1.00

 Date:		9-Jan-97

 Author:	MG

****************************************************************************/

#include "stdafx.h"
#include "cstmctrl.h"

/////////////////////////////////////////////////////////////////////////////
// CCrossHair

CCrossHair::CCrossHair()
{
	m_hWnd = NULL;
}

BOOL CCrossHair::Create(HWND hWnd, BYTE bSize)
{
	if (hWnd == NULL)
		return FALSE;

	m_hWnd = hWnd;
	m_bSize = bSize;

	Invalidate();

	return TRUE;
}


void CCrossHair::Invalidate()
{
	m_Pos.x = -1;
}

void CCrossHair::SetPos(POINT p)
{
	RECT	rc;

	if (m_hWnd == NULL)
		return;

	GetClientRect(m_hWnd, &rc);

	if (p.x < -128)
		p.x = -128;
	else if (p.x > 128)
		p.x = 128;
	if (p.y < -128)
		p.y = -128;
	else if (p.y > 128)
		p.y = 128;

	p.x = (rc.right-1)/2 + p.x * (rc.right - m_bSize - m_bSize - 1) / 256;
	p.y = (rc.bottom-1)/2 - p.y * (rc.bottom - m_bSize - m_bSize - 1) / 256;

	if (m_Pos.x == p.x && m_Pos.y == p.y)
		return;

	if (m_Pos.x != -1)
		DrawCrossHair(m_Pos, FALSE);

	DrawCrossHair(p, TRUE);

	m_Pos.x = p.x;
	m_Pos.y = p.y;
}

void CCrossHair::DrawCrossHair(POINT p, BOOL bMode)
{
	HDC	 hdc;
	HGDIOBJ OldObject;

	if (m_hWnd == NULL)
		return;

	hdc = GetDC(m_hWnd);
	if (hdc == NULL)
		return;

	OldObject = SelectObject(hdc, (HPEN)GetStockObject(bMode ? BLACK_PEN : WHITE_PEN));

	MoveToEx(hdc, p.x-m_bSize, p.y, NULL);
	LineTo(hdc, p.x+m_bSize+1, p.y);
	MoveToEx(hdc, p.x, p.y-m_bSize, NULL);
	LineTo(hdc, p.x, p.y+m_bSize+1);

	SelectObject(hdc, OldObject);

	ReleaseDC(m_hWnd, hdc);
}

/////////////////////////////////////////////////////////////////////////////
// CShiftBar

CShiftBar::CShiftBar()
{
	m_hWnd = NULL;
}

BOOL CShiftBar::Create(HWND hWnd)
{
	if (hWnd == NULL)
		return FALSE;

	m_hWnd = hWnd;

	Invalidate();

	return TRUE;
}

void CShiftBar::Invalidate()
{
	m_Pos = -1;
}

void CShiftBar::SetPos(int p)
{
	RECT	rc;

	if (m_hWnd == NULL)
		return;

	if (!GetClientRect(m_hWnd, &rc))
		return;

	if (p < -128)
		p = -128;
	else if (p > 128)
		p = 128;

	p = rc.bottom/2 - p * rc.bottom / 256;

	if (m_Pos == -1) {
		m_Pos = 0;
		DrawBar(p);
		m_Pos = rc.bottom;
		DrawBar(p);
		m_Pos = p;
	}

	if (p == m_Pos)
		return;

	DrawBar(p);

	m_Pos = p;
}

void CShiftBar::DrawBar(int p)
{
	HDC	 hdc;
	HBRUSH  brDark, brLight;
	RECT	rc;

	if (m_hWnd == NULL)
		return;

	hdc = GetDC(m_hWnd);
	if (hdc == NULL)
		return;

	brDark = CreateSolidBrush(RGB(128,0,0));
	if (brDark == NULL) {
		ReleaseDC(m_hWnd, hdc);
		return;
	}

	brLight = CreateSolidBrush(RGB(255,0,0));
	if (brLight == NULL) {
		DeleteObject(brDark);
		ReleaseDC(m_hWnd, hdc);
		return;
	}

	GetClientRect(m_hWnd, &rc);

	if (p > m_Pos) {
		SetRect(&rc, 0, m_Pos, rc.right, p);
		FillRect(hdc, &rc, brDark);
	} else {
		SetRect(&rc, 0, p, rc.right, m_Pos);
		FillRect(hdc, &rc, brLight);
	}

	DeleteObject(brDark);
	DeleteObject(brLight);

	ReleaseDC(m_hWnd, hdc);
}

void CShiftBar::DrawDisabledBar()
{
	HDC	 hdc;
	HBRUSH  brDark;
	RECT	rc;

	if (m_hWnd == NULL)
		return;

	m_Pos = -1;

	hdc = GetDC(m_hWnd);
	if (hdc == NULL)
		return;

	brDark = (HBRUSH)GetStockObject(GRAY_BRUSH);
	if (brDark == NULL) {
		ReleaseDC(m_hWnd, hdc);
		return;
	}

	GetClientRect(m_hWnd, &rc);
	FillRect(hdc, &rc, brDark);

	ReleaseDC(m_hWnd, hdc);
}



