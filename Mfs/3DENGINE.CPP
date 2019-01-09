/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 3dengine.cpp

 Version:   1.00

 Date:	  18-Jan-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "variable.h"
#include "display.h"
#include "3dengine.h"
#include "gfx8.h"

/////////////////////////////////////////////////////////////////////////////
// Definitions...

#define DEFAUT_SCREEN_ASPECT_RATIO  1.333
#define ROUNDOFF					(10E-4)

/////////////////////////////////////////////////////////////////////////////
// Locals...

//MODULE(__FILE__);

/////////////////////////////////////////////////////////////////////////////
// Globals...

C3DEngine			My3DEngine;

M3DRECT				ClipWindow;
M3DPOINT			ViewportCenter;

HDC					g_hdc;						// for GDI drawing

C3DBspObject		*Airplane;

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern CDisplay		g_Display;

extern DWORD		g_bUseGDI;
extern PALETTEENTRY	g_upe[];
extern DWORD		g_mpe[];

extern void Disp(LPSTR fmt, ...);

/////////////////////////////////////////////////////////////////////////////
// Parameter...

extern CVarList	VarList;
extern CVarSection VarSectionCommon;

#pragma init_seg(user)
CFVar
	GridLines( 	   VarSectionCommon, "GridLines", 20.0, 0.0, 100.0),
	GridDensity(	  VarSectionCommon, "GridDensity", 30.0, 5.0, 255.0),
	ScreenAspectRatio(VarSectionCommon, "ScreenAspectRatio", 1.333, 0.5, 2.0),
	ViewPosX(		 VarSectionCommon, "Pos X", -10.0, -50.0, 50.0),
	ViewPosY(		 VarSectionCommon, "Pos Y",   1.8,   0.0, 10.0),
	ViewPosZ(		 VarSectionCommon, "Pos Z", -20.0, -50.0, 50.0);

/////////////////////////////////////////////////////////////////////////////
// Clipping

M3DVALUE				BackPlaneZ = 1.0;

static int			  nPointInBuffer;
static POINT			PointBuffer[32];
static LPPOINT		  lppb;

static void SetClipWindow(LPRECT lprc)
{
	ClipWindow.x1 = lprc->left;
	ClipWindow.y1 = lprc->top;
	ClipWindow.x2 = lprc->right-1;
	ClipWindow.y2 = lprc->bottom-1;
}

/////////////////////////////////////////////////////////////////////////////
// Line clipping

static inline void StoreLPoints(int p1x, int p1y, int p2x, int p2y)
{
	PointBuffer[0].x = p1x;
	PointBuffer[0].y = p1y;
	PointBuffer[1].x = p2x;
	PointBuffer[1].y = p2y;
	nPointInBuffer = 2;
}

static void LClipBottom(M3DPOINT &p1, M3DPOINT &p2)
{
	int tx;

	if (p1.y < ClipWindow.y2) {
		if (p2.y < ClipWindow.y2) {
			// both points are in the clipping area
			StoreLPoints(p1.x, p1.y, p2.x, p2.y);
		} else {
			// draw from inner to outer area
			tx = p2.x + (ClipWindow.y2 - p2.y) * (p1.x - p2.x) / (p1.y - p2.y);
			StoreLPoints(p1.x, p1.y, tx, ClipWindow.y2);
		}
	} else {
		if (p2.y < ClipWindow.y2) {
			// draw from outer to inner area
			tx = p2.x + (ClipWindow.y2 - p2.y) * (p1.x - p2.x) / (p1.y - p2.y);
			StoreLPoints(tx, ClipWindow.y2, p2.x, p2.y);
		} // else both points are out of the clipping area
	}
}

static void LClipTop(M3DPOINT &p1, M3DPOINT &p2)
{
	M3DPOINT tp;

	if (p1.y > ClipWindow.y1) {
		if (p2.y > ClipWindow.y1) {
			// both points are in the clipping area
			LClipBottom(p1, p2);
		} else {
			// draw from inner to outer area
			tp.y = ClipWindow.y1;
			tp.x = p2.x + (tp.y - p2.y) * (p1.x - p2.x) / (p1.y - p2.y);
			LClipBottom(p1, tp);
		}
	} else {
		if (p2.y > ClipWindow.y1) {
			// draw from outer to inner area
			tp.y = ClipWindow.y1;
			tp.x = p2.x + (tp.y - p2.y) * (p1.x - p2.x) / (p1.y - p2.y);
			LClipBottom(tp, p2);
		} else {
			// both points are out of the clipping area
		}
	}
}

static void LClipRight(M3DPOINT &p1, M3DPOINT &p2)
{
	M3DPOINT tp;

	if (p1.x < ClipWindow.x2) {
		if (p2.x < ClipWindow.x2) {
			// both points are in the clipping area
			LClipTop(p1, p2);
		} else {
			// draw from inner to outer area
			tp.x = ClipWindow.x2;
			tp.y = p2.y + (tp.x - p2.x) * (p1.y - p2.y) / (p1.x - p2.x);
			LClipTop(p1, tp);
		}
	} else {
		if (p2.x < ClipWindow.x2) {
			// draw from outer to inner area
			tp.x = ClipWindow.x2;
			tp.y = p2.y + (tp.x - p2.x) * (p1.y - p2.y) / (p1.x - p2.x);
			LClipTop(tp, p2);
		} // else both points are out of the clipping area
	}
}

static void LClipLeft(M3DPOINT &p1, M3DPOINT &p2)
{
	M3DPOINT tp;

	if (p1.x > ClipWindow.x1) {
		if (p2.x > ClipWindow.x1) {
			// both points are in the clipping area
			LClipRight(p1, p2);
		} else {
			// draw from inner to outer area
			tp.x = ClipWindow.x1;
			tp.y = p2.y + (tp.x - p2.x) * (p1.y - p2.y) / (p1.x - p2.x);
			LClipRight(p1, tp);
		}
	} else {
		if (p2.x > ClipWindow.x1) {
			// draw from outer to inner area
			tp.x = ClipWindow.x1;
			tp.y = p2.y + (tp.x - p2.x) * (p1.y - p2.y) / (p1.x - p2.x);
			LClipRight(tp, p2);
		} // else both points are out of the clipping area
	}
}

static void LClipBack(M3DVERTEX &v1, M3DPOINT &p1, M3DVERTEX &v2, M3DPOINT &p2)
{
	M3DPOINT tp;
	M3DVALUE dz;

	if (v1.z > BackPlaneZ) {
		if (v2.z > BackPlaneZ) {
			// both points are in the clipping area
			// pass transformed point
			LClipLeft(p1, p2);
		} else {
			// draw from inner to outer area
			dz = (BackPlaneZ - v2.z) / (v1.z - v2.z);
			tp.x = (v2.x + (v1.x - v2.x) * dz) / BackPlaneZ + ViewportCenter.x;
			tp.y = (v2.y + (v1.y - v2.y) * dz) / BackPlaneZ + ViewportCenter.y;
			LClipLeft(p1, tp);
		}
	} else {
		if (v2.z > BackPlaneZ) {
			// draw from outer to inner area
			dz = (BackPlaneZ - v2.z) / (v1.z - v2.z);
			tp.x = (v2.x + (v1.x - v2.x) * dz) / BackPlaneZ + ViewportCenter.x;
			tp.y = (v2.y + (v1.y - v2.y) * dz) / BackPlaneZ + ViewportCenter.y;
			LClipLeft(tp, p2);
		} // else both points are out of the clipping area
	}
}

static int StoreLPoints(LPM3DVERTEXSET lpvs1, LPM3DVERTEXSET lpvs2)
{
	nPointInBuffer = 0;
	LClipBack(lpvs1->t, lpvs1->p, lpvs2->t, lpvs2->p);
	return nPointInBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// Polygon clipping

#define FIRST_BACK	  1
#define FIRST_LEFT	  0x10
#define FIRST_RIGHT	 0x20
#define FIRST_TOP	   0x40
#define FIRST_BOTTOM	0x80

static BYTE			 ClipFlag;
static BYTE			 ClipMoveCount;
static M3DPOINT		 FirstBack;
static M3DPOINT		 FirstLeft, FirstRight, FirstTop, FirstBottom;
static M3DVERTEX		PosBack;
static M3DPOINT		 PosLeft, PosRight, PosTop, PosBottom;

static inline void StorePPoint(M3DPOINT &p)
{
	if (ClipMoveCount == 0) {
		lppb->x = p.x;
		lppb->y = p.y;
		lppb++;
		nPointInBuffer++;
	}
}

static void PClipBottom(M3DPOINT &p)
{
	M3DPOINT tp;

	if (PosBottom.y < ClipWindow.y2) {
		if (p.y < ClipWindow.y2) {
			// both points are in the clipping area
			StorePPoint(p);
		} else {
			// draw from inner to outer area
			tp.y = ClipWindow.y2;
			tp.x = p.x + (tp.y - p.y) * (PosBottom.x - p.x) / (PosBottom.y - p.y);
			if (ClipFlag & FIRST_BOTTOM || ClipMoveCount != 0) {
				StorePPoint(tp);
				// close polygon
				StorePPoint(FirstBottom);
			} else {
				ClipFlag |= FIRST_BOTTOM;
				StorePPoint(tp);
			}
		}
	} else {
		if (p.y < ClipWindow.y2) {
			// draw from outer to inner area
			tp.y = ClipWindow.y2;
			tp.x = p.x + (tp.y - p.y) * (PosBottom.x - p.x) / (PosBottom.y - p.y);
			if (ClipFlag & FIRST_BOTTOM) {
				// close polygon
				StorePPoint(tp);
				StorePPoint(p);
			} else {
				if (ClipMoveCount == 0) {
					ClipFlag |= FIRST_BOTTOM;
					// save first intersection
					FirstBottom = tp;
				}
				// set move command
				//ClipMoveCount++;
				//StorePPoint(tp);
				//ClipMoveCount--;
				StorePPoint(p);
			}
		} // else both points are out of the clipping area
	}
	PosBottom = p;
}

static void PClipTop(M3DPOINT &p)
{
	M3DPOINT tp;

	if (PosTop.y > ClipWindow.y1) {
		if (p.y > ClipWindow.y1) {
			// both points are in the clipping area
			PClipBottom(p);
		} else {
			// draw from inner to outer area
			tp.y = ClipWindow.y1;
			tp.x = p.x + (tp.y - p.y) * (PosTop.x - p.x) / (PosTop.y - p.y);
			if (ClipFlag & FIRST_TOP || ClipMoveCount != 0) {
				PClipBottom(tp);
				// close polygon
				PClipBottom(FirstTop);
			} else {
				ClipFlag |= FIRST_TOP;
				PClipBottom(tp);
			}
		}
	} else {
		if (p.y > ClipWindow.y1) {
			// draw from outer to inner area
			tp.y = ClipWindow.y1;
			tp.x = p.x + (tp.y - p.y) * (PosTop.x - p.x) / (PosTop.y - p.y);
			if (ClipFlag & FIRST_TOP) {
				// close polygon
				PClipBottom(tp);
				PClipBottom(p);
			} else {
				if (ClipMoveCount == 0) {
					ClipFlag |= FIRST_TOP;
					// save first intersection
					FirstTop = tp;
				}
				// set move command
				ClipMoveCount++;
				PClipBottom(tp);
				ClipMoveCount--;
				PClipBottom(p);
			}
		} // else both points are out of the clipping area
	}
	PosTop = p;
}

static void PClipRight(M3DPOINT &p)
{
	M3DPOINT tp;

	if (PosRight.x < ClipWindow.x2) {
		if (p.x < ClipWindow.x2) {
			// both points are in the clipping area
			PClipTop(p);
		} else {
			// draw from inner to outer area
			tp.x = ClipWindow.x2;
			tp.y = p.y + (tp.x - p.x) * (PosRight.y - p.y) / (PosRight.x - p.x);
			if (ClipFlag & FIRST_RIGHT || ClipMoveCount != 0) {
				PClipTop(tp);
				// close polygon
				PClipTop(FirstRight);
			} else {
				ClipFlag |= FIRST_RIGHT;
				PClipTop(tp);
			}
		}
	} else {
		if (p.x < ClipWindow.x2) {
			// draw from outer to inner area
			tp.x = ClipWindow.x2;
			tp.y = p.y + (tp.x - p.x) * (PosRight.y - p.y) / (PosRight.x - p.x);
			if (ClipFlag & FIRST_RIGHT) {
				// close polygon
				PClipTop(tp);
				PClipTop(p);
			} else {
				if (ClipMoveCount == 0) {
					ClipFlag |= FIRST_RIGHT;
					// save first intersection
					FirstRight = tp;
				}
				// set move command
				ClipMoveCount++;
				PClipTop(tp);
				ClipMoveCount--;
				PClipTop(p);
			}
		} // else both points are out of the clipping area
	}
	PosRight = p;
}

static void PClipLeft(M3DPOINT &p)
{
	M3DPOINT tp;

	if (PosLeft.x > ClipWindow.x1) {
		if (p.x > ClipWindow.x1) {
			// both points are in the clipping area
			PClipRight(p);
		} else {
			// draw from inner to outer area
			tp.x = ClipWindow.x1;
			tp.y = p.y + (tp.x - p.x) * (PosLeft.y - p.y) / (PosLeft.x - p.x);
			if (ClipFlag & FIRST_LEFT || ClipMoveCount != 0) {
				PClipRight(tp);
				// close polygon
				PClipRight(FirstLeft);
			} else {
				ClipFlag |= FIRST_LEFT;
				PClipRight(tp);
			}
		}
	} else {
		if (p.x > ClipWindow.x1) {
			// draw from outer to inner area
			tp.x = ClipWindow.x1;
			tp.y = p.y + (tp.x - p.x) * (PosLeft.y - p.y) / (PosLeft.x - p.x);
			if (ClipFlag & FIRST_LEFT) {
				// close polygon
				PClipRight(tp);
				PClipRight(p);
			} else {
				if (ClipMoveCount == 0) {
					ClipFlag |= FIRST_LEFT;
					// save first intersection
					FirstLeft = tp;
				}
				// set move command
				ClipMoveCount++;
				PClipRight(tp);
				ClipMoveCount--;
				PClipRight(p);
			}
		} // else both points are out of the clipping area
	}
	PosLeft = p;
}

static void PClipBack(M3DVERTEX &v, M3DPOINT &p)
{
	M3DPOINT tp;
	M3DVALUE dz;

	if (PosBack.z > BackPlaneZ) {
		if (v.z > BackPlaneZ) {
			// both points are in the clipping area
			// pass transformed point
			PClipLeft(p);
		} else {
			// draw from inner to outer area
			dz = (BackPlaneZ - v.z) / (PosBack.z - v.z);
			tp.x = (v.x + (PosBack.x - v.x) * dz) / BackPlaneZ + ViewportCenter.x;
			tp.y = (v.y + (PosBack.y - v.y) * dz) / BackPlaneZ + ViewportCenter.y;
			if (ClipFlag & FIRST_BACK) {
				PClipLeft(tp);
				// close polygon
				PClipLeft(FirstBack);
			} else {
				ClipFlag |= FIRST_BACK;
				PClipLeft(tp);
			}
		}
	} else {
		if (v.z > BackPlaneZ) {
			// draw from outer to inner area
			dz = (BackPlaneZ - v.z) / (PosBack.z - v.z);
			tp.x = (v.x + (PosBack.x - v.x) * dz) / BackPlaneZ + ViewportCenter.x;
			tp.y = (v.y + (PosBack.y - v.y) * dz) / BackPlaneZ + ViewportCenter.y;
			if (ClipFlag & FIRST_BACK) {
				// close polygon
				PClipLeft(tp);
				PClipLeft(p);
			} else {
				ClipFlag |= FIRST_BACK;
				// save first intersection
				FirstBack = tp;
				// set move command
				ClipMoveCount++;
				PClipLeft(tp);
				ClipMoveCount--;
				PClipLeft(p);
			}
		} // else both points are out of the clipping area
	}
	PosBack = v;
}

static int StorePolygonsPoints(LPM3DVERTEXSET *lplpvs, int Points)
{
	lppb = PointBuffer;
	nPointInBuffer = 0;
	ClipFlag = ClipMoveCount = 0;

	PosBack = lplpvs[Points-1]->t;
	PosLeft = PosRight = PosTop = PosBottom = lplpvs[Points-1]->p;

	for(int i = 0; i<Points; i++) {
		PClipBack(lplpvs[i]->t, lplpvs[i]->p);
	}
	return nPointInBuffer;
}

/////////////////////////////////////////////////////////////////////////////
// BSP routines

// checks if the face shows its front side
// cross product of the first three points, (v1 x v2).z > 0
static BOOL IsFrontSide(LPM3DBSPFACE lpfc)
{
	LPM3DVERTEXSET v0 = lpfc->vs[0];
	LPM3DVERTEXSET v1 = lpfc->vs[1];
	LPM3DVERTEXSET v2 = lpfc->vs[2];
	return (v1->p.x - v0->p.x) * (v2->p.y - v0->p.y) -
				 (v1->p.y - v0->p.y) * (v2->p.x - v0->p.x) > 0.0 ? 1 : 0;
}

// calculate the face normal with the first three points
// N = v1 x v2, h = P0*N
static C3DHVector GetNormalVector(LPM3DBSPFACE lpfc)
{
	C3DVector v1 = C3DVector(lpfc->vs[1]->o) - C3DVector(lpfc->vs[0]->o);
	C3DVector v2 = C3DVector(lpfc->vs[2]->o) - C3DVector(lpfc->vs[0]->o);
	C3DVector vp = cross(v1, v2);
	return C3DHVector(vp, dot(C3DVector(lpfc->vs[0]->o), vp));
}

// locate the points of a face with a reference plane
// P*N-P0*N > 0 => point in front of the plane
static int LocatePoints(LPM3DBSPFACE lpfc, const C3DHVector &refvec)
{
	int nBefore = 0, nBehind = 0;
	for(int i = 0; i < lpfc->nVertices; i++) {
		float diff = dot(C3DVector(lpfc->vs[i]->o), refvec) - refvec.h;
		if (diff < -ROUNDOFF)
			nBefore++;
		else if (diff > ROUNDOFF)
			nBehind++;
	}
	if (nBehind == 0 && nBefore == 0)
		return 0;					   // all points in the ref plane
	if (nBehind == 0)
		return 2;					   // all points before the ref plane
	if (nBefore == 0)
		return -2;					  // all points behind the ref plane
	if (nBefore > nBehind)
		return 1;
	else
		return -1;
}

// set the face to the root of the tree
static void SetToRoot(LPM3DBSPFACE lpfc, LPLPM3DBSPFACE root)
{
	// if already on root return
	if (lpfc == *root)
		return;
	LPM3DBSPFACE lpfcrun = *root;
	while (lpfcrun->Before != lpfc)
		lpfcrun = lpfcrun->Before;
	lpfcrun->Before = lpfc->Before;
	lpfc->Before = *root;
	*root = lpfc;
}

// look for the faces that is the best one for splitting the bsp tree
static LPM3DBSPFACE SearchSplitPlane(LPM3DBSPFACE root)
{
	unsigned int Bonus, Faults, MaxBonus, MinFaults;
	LPM3DBSPFACE BestPlane, CmpPlane;

	MaxBonus = 0;
	MinFaults = -1;
	BestPlane = CmpPlane = root;
	while (CmpPlane) {					 // run through the planes
		C3DHVector CmpNormal = GetNormalVector(CmpPlane);
		Bonus = 0;
		Faults = 0;
		LPM3DBSPFACE Plane = root;
		while (Plane) {
			int i = LocatePoints(Plane, CmpNormal);
			if (i == 2 || i == -2)		 // if this plane is before or behind the cmpplane
				Bonus++;					// add a bonus for the cmpplane
			else if (i == 1 || i == -1)	// if this plane intersects the cmpplane
				Faults++;				   // add a fault
			Plane = Plane->Before;
		}
		if (Bonus > MaxBonus ||			 // if the cmpplane is better than the bestplane
				Bonus == MaxBonus && Faults < MinFaults) {
			MaxBonus = Bonus;
			MinFaults = Faults;
			BestPlane = CmpPlane;		   // take the cmpplane for the bestplane
		}
		CmpPlane = CmpPlane->Before;
	}
	return BestPlane;
}

// erstellt aus der Polygonliste gemäß dem
// BinarySpacePartitioning-Algorithmus (nach Henry Fuchs)
// einen einfach verketteten binären Baum 
static void BuildBspSubTree(LPLPM3DBSPFACE root)
{
	if (*root == NULL)
		return;
	LPM3DBSPFACE RefPlane = SearchSplitPlane(*root);
	SetToRoot(RefPlane, root);
	C3DHVector NormalVector = GetNormalVector(RefPlane);
	LPM3DBSPFACE Plane = RefPlane->Before;
	RefPlane->Before = NULL;
	RefPlane->Behind = NULL;
	while (Plane) {
		LPM3DBSPFACE NextPlane = Plane->Before;
		if (LocatePoints(Plane, NormalVector) < 0) {
			Plane->Before = RefPlane->Before;
			RefPlane->Before = Plane;
		} else {
			Plane->Before = RefPlane->Behind;
			RefPlane->Behind = Plane;
		}
		Plane = NextPlane;
	}
	BuildBspSubTree(&RefPlane->Before);
	BuildBspSubTree(&RefPlane->Behind);
}

/////////////////////////////////////////////////////////////////////////////
// vertex transformation

static void TransformVertices(LPM3DVERTEXSET lpvs, int nVertices, const C3DMatrix &Matrix)
{
	for(int i=0; i<nVertices; i++) {
		for(int j=0; j<3; j++) {
			M3DVALUE d = 0.0;
			for(int k=0; k<3; k++)
				d += Matrix.v[k].v[j] *lpvs->o.v[k];
			d += Matrix.v[H].v[j];
			lpvs->t.v[j] = d;
		}
		if (lpvs->t.z > 1.0) {
			lpvs->p.x = lpvs->t.x / lpvs->t.z + ViewportCenter.x;
			lpvs->p.y = lpvs->t.y / lpvs->t.z + ViewportCenter.y;
		}
		lpvs++;
	}
}

/////////////////////////////////////////////////////////////////////////////
// face drawing

// draw a face with windows GDI
static void DrawFace(M3DCOLOR PenColor, M3DCOLOR BrushColor)
{
	static HPEN	 penFace;
	static HBRUSH   brFace;

	if (BrushColor != COLOR_NULL) {
		if (BrushColor == COLOR_TRANSLUCENT) {
			BrushColor = 10;
		}
		if (PenColor != COLOR_NULL) {
			PALETTEENTRY ppe = g_upe[PenColor];
			PALETTEENTRY bpe = g_upe[BrushColor];
			penFace = CreatePen(PS_SOLID, 0, PALETTERGB(ppe.peRed, ppe.peGreen, ppe.peBlue));
			penFace = SelectPen(g_hdc, penFace);
			brFace = CreateSolidBrush(PALETTERGB(bpe.peRed, bpe.peGreen, bpe.peBlue));
			brFace = SelectBrush(g_hdc, brFace);
			Polygon(g_hdc, PointBuffer, nPointInBuffer);
			brFace = SelectBrush(g_hdc, brFace);
			penFace = SelectPen(g_hdc, penFace);
			DeleteBrush(brFace);
			DeleteBrush(penFace);
		} else {
			PALETTEENTRY bpe = g_upe[BrushColor];
			penFace = CreatePen(PS_SOLID, 0, PALETTERGB(bpe.peRed, bpe.peGreen, bpe.peBlue));
			brFace = CreateSolidBrush(PALETTERGB(bpe.peRed, bpe.peGreen, bpe.peBlue));
			penFace = SelectPen(g_hdc, penFace);
			brFace = SelectBrush(g_hdc, brFace);
			Polygon(g_hdc, PointBuffer, nPointInBuffer);
			brFace = SelectBrush(g_hdc, brFace);
			penFace = SelectPen(g_hdc, penFace);
			DeleteBrush(brFace);
			DeleteBrush(penFace);
		}
	} else if (PenColor != COLOR_NULL) {
		PALETTEENTRY ppe = g_upe[PenColor];
		penFace = CreatePen(PS_SOLID, 0, PALETTERGB(ppe.peRed, ppe.peGreen, ppe.peBlue));
		penFace = SelectPen(g_hdc, penFace);
		SelectBrush(g_hdc, GetStockBrush(NULL_BRUSH));
		Polyline(g_hdc, PointBuffer, nPointInBuffer);
		penFace = SelectPen(g_hdc, penFace);
		DeleteObject(penFace);
	}
}

// run through the BSP tree
static void RunBSPTree(LPM3DBSPFACE lpfc)
{
	if (lpfc == NULL)
		return;
	if (IsFrontSide(lpfc)) {
		RunBSPTree(lpfc->Behind);
		if (StorePolygonsPoints(lpfc->vs, lpfc->nVertices) >= 3)
			DrawFace(lpfc->Color.fgp, lpfc->Color.fgb);
		RunBSPTree(lpfc->Before);
	} else {
		RunBSPTree(lpfc->Before);
		if (StorePolygonsPoints(lpfc->vs, lpfc->nVertices) >= 3)
			DrawFace(lpfc->Color.bgp, lpfc->Color.bgb);
		RunBSPTree(lpfc->Behind);
	}
}

#ifdef ACCEL_GFX8

// draw a face (256 color mode)
static void DrawFace8(M3DCOLOR PenColor, M3DCOLOR BrushColor)
{
	if (BrushColor != COLOR_NULL) {
		if (BrushColor != COLOR_TRANSLUCENT) {
			_SetColor8(g_mpe[ BrushColor ]);
			_Polygon8(PointBuffer, nPointInBuffer);
		} else {
			_TPolygon8(PointBuffer, nPointInBuffer);
		}
		if (PenColor != COLOR_NULL) {
			_SetColor8(g_mpe[ PenColor ]);
			_Polyline8(PointBuffer, nPointInBuffer);
		}
	} else if (PenColor != COLOR_NULL) {
		_SetColor8(g_mpe[ PenColor ]);
		_Polyline8(PointBuffer, nPointInBuffer);
	}
}

// run through the BSP tree (256 color mode)
static void RunBSPTree8(LPM3DBSPFACE lpfc)
{
	if (lpfc == NULL)
		return;
	if (IsFrontSide(lpfc)) {
		RunBSPTree8(lpfc->Behind);
		if (StorePolygonsPoints(lpfc->vs, lpfc->nVertices) >= 3)
			DrawFace8(lpfc->Color.fgp, lpfc->Color.fgb);
		RunBSPTree8(lpfc->Before);
	} else {
		RunBSPTree8(lpfc->Before);
		if (StorePolygonsPoints(lpfc->vs, lpfc->nVertices) >= 3)
			DrawFace8(lpfc->Color.bgp, lpfc->Color.bgb);
		RunBSPTree8(lpfc->Behind);
	}
}

#endif

/////////////////////////////////////////////////////////////////////////////
// C3DBspBuffer

C3DBspBuffer::C3DBspBuffer()
{
	m_nVertices = 0;
	m_nFaces = 0;
	m_lpVertexSet = NULL;
	m_lpBspFaceRoot = NULL;
}

C3DBspBuffer::~C3DBspBuffer()
{
	if (m_lpVertexSet) {
		delete[] m_lpVertexSet;
		m_lpVertexSet = NULL;
	}
}

BOOL C3DBspBuffer::Create(LPCSTR Name, int nVertices)
{
	m_Name = Name;
	m_lpVertexSet = new M3DVERTEXSET[ nVertices ];
	return TRUE;
}

void C3DBspBuffer::AddVertex(C3DVector vec)
{
	m_lpVertexSet[ m_nVertices ].o.x = vec.v[X];
	m_lpVertexSet[ m_nVertices ].o.y = vec.v[Y];
	m_lpVertexSet[ m_nVertices ].o.z = vec.v[Z];
	m_nVertices++;
}

void C3DBspBuffer::AddFace(M3DCOLORSET Color, int nVertices, int Index[])
{
	LPM3DBSPFACE lpfc = new M3DBSPFACE;

	lpfc->Before = m_lpBspFaceRoot;
	lpfc->Behind = NULL;
	lpfc->Color = Color;
	lpfc->nVertices = nVertices;
	for(int i=0; i<nVertices; i++)
		lpfc->vs[i] = &m_lpVertexSet[ Index[i] ];

	m_lpBspFaceRoot = lpfc;
	m_nFaces++;
}

void C3DBspBuffer::BuildBspTree()
{
	::BuildBspSubTree(&m_lpBspFaceRoot);
}

void C3DBspBuffer::TransformVertices(const C3DMatrix &Matrix)
{
	::TransformVertices(m_lpVertexSet, m_nVertices, Matrix);
}

void C3DBspBuffer::RunBSPTree(BOOL bOwnDraw)
{
#ifdef ACCEL_GFX8
	if (bOwnDraw)
		::RunBSPTree8(m_lpBspFaceRoot);
	else
#endif
		::RunBSPTree(m_lpBspFaceRoot);
}

/////////////////////////////////////////////////////////////////////////////
// C3DBspObject

C3DBspObject::C3DBspObject()
{
	m_lpBspBuffer = m_lpShadowBspBuffer = NULL;
	m_Matrix.Init();
//	if (Airplane == NULL)
		Airplane = this;
}

void C3DBspObject::AttachBspBuffer(C3DBspBuffer *lpBspBuffer)
{
	m_lpBspBuffer = lpBspBuffer;
}

void C3DBspObject::AttachShadowBspBuffer(C3DBspBuffer *lpShadowBspBuffer)
{
	m_lpShadowBspBuffer = lpShadowBspBuffer;
}

void C3DBspObject::Move(const C3DVector &vec)
{
	m_Matrix.v[H] += vec;
}

void C3DBspObject::Rotate(const C3DVector &vec)
{
	m_Matrix.RotateInAxe(vec);
}

C3DMatrix C3DBspObject::CalcShadowMatrix()
{
	C3DMatrix Matrix = m_Matrix;
	Matrix.v[H].v[Y] = 0.0;
	return Matrix;
}

void C3DBspObject::Render(const C3DMatrix &ViewMatrix, BOOL bOwnDraw)
{
	if (m_lpShadowBspBuffer != NULL) {
		m_lpShadowBspBuffer->TransformVertices(CalcShadowMatrix() * ViewMatrix);
		m_lpShadowBspBuffer->RunBSPTree(bOwnDraw);
	}
	m_lpBspBuffer->TransformVertices(m_Matrix * ViewMatrix);
	m_lpBspBuffer->RunBSPTree(bOwnDraw);
}

/////////////////////////////////////////////////////////////////////////////
// C3DEngine

C3DEngine::C3DEngine()
{
}

void C3DEngine::Initialize()
{
}

void C3DEngine::FreeAllBuffers()
{
	C3DBspBuffer *buf;
	C3DBspObject *obj;
	while ((buf = m_BspBufferList.Get()) != NULL)
		delete buf;
	while ((obj = m_BspObjectList.Get()) != NULL)
		delete obj;
	Airplane = NULL;
}

C3DBspBuffer *C3DEngine::GetBspBuffer(LPCSTR Name)
{
	for(CIDIter<C3DBspBuffer> Iter(m_BspBufferList); Iter(); Iter++) {
		if (stricmp(Iter()->GetName(), Name) == 0)
			return Iter();
	}
	return NULL;
}

C3DBspBuffer *C3DEngine::CreateBspBuffer(LPCSTR Name, int nVertices)
{
	C3DBspBuffer *buf = new C3DBspBuffer;
	m_BspBufferList.Append(buf);
	buf->Create(Name, nVertices);
	return buf;
}

C3DBspObject *C3DEngine::CreateBspObject()
{
	C3DBspObject *obj = new C3DBspObject;
	m_BspObjectList.Append(obj);
	return obj;
}

void C3DEngine::SetViewport(LPRECT lprc)
{
	SetClipWindow(lprc);
	m_Viewport.left = lprc->left;
	m_Viewport.top = lprc->top;
	m_Viewport.right = lprc->right;
	m_Viewport.bottom = lprc->bottom;
	ViewportCenter.x = lprc->left + (lprc->right - lprc->left) / 2;
	ViewportCenter.y = lprc->top + (lprc->bottom - lprc->top) / 2;
}

void C3DEngine::SetViewpoint(const C3DVector &vec)
{
	m_Viewpoint = vec;
}

void C3DEngine::SetViewvector(const C3DVector &vec)
{
	m_Viewvector = vec;
}

void C3DEngine::CalcViewMatrix()
{
	M3DVALUE dx, dy, Scale;

	dx = m_Viewport.right - m_Viewport.left;
	dy = m_Viewport.bottom - m_Viewport.top;

	Scale = dx / dy > DEFAUT_SCREEN_ASPECT_RATIO ? dy : dx / DEFAUT_SCREEN_ASPECT_RATIO;

	m_Viewmatrix.Init();
	m_Viewmatrix.v[H] -= m_Viewpoint;
	m_Viewmatrix.RotateToAxe(m_Viewvector);
	m_Viewmatrix *= C3DVector(Scale * DEFAUT_SCREEN_ASPECT_RATIO / 0.27, -Scale * ScreenAspectRatio / 0.27, 1);
}

/////////////////////////////////////////////////////////////////////////////

BOOL C3DEngine::DrawBackground()
{
	RECT rc;
	LONG middle;

	rc.left = m_Viewport.left;
	rc.right = m_Viewport.right;

	middle = m_Viewmatrix.v[Z].v[Y] / m_Viewmatrix.v[Z].v[Z];
	middle += m_Viewport.top + (m_Viewport.bottom - m_Viewport.top) / 2;
	if (middle < m_Viewport.top)
		middle = m_Viewport.top;
	else if (middle > m_Viewport.bottom)
		middle = m_Viewport.bottom;

	// draw blue upper half
	if (middle > m_Viewport.top) {
		rc.top = m_Viewport.top;
		rc.bottom = middle;
		if (!g_Display.ColorFill(&rc, g_mpe[ COLOR_SKY ]))
			return FALSE;
	}
	// draw green lower half
	if (middle < m_Viewport.bottom) {
		rc.top = middle;
		rc.bottom = m_Viewport.bottom;
		if (!g_Display.ColorFill(&rc, g_mpe[ COLOR_GROUND ]))
			return FALSE;
	}
	return TRUE;
}

static LPM3DVERTEXSET	 lpvsGrid;

void CreateHall()
{
	int i, j, k;
	int nGridLines = GridLines + 1;

	lpvsGrid = new M3DVERTEXSET[12*nGridLines];
	// set up grid
	M3DVALUE	b = (nGridLines >> 1) * GridDensity;
	for(i=0; i<nGridLines; i++) {
		M3DVALUE	p = ((nGridLines >> 1) - i) * GridDensity;
		for(j=0; j<3; j++) {
			for(k=0; k<4; k++) {
				M3DVERTEX v;
				v.v[(j+X)%3] = p;
				v.v[(j+Y)%3] = (k & 2) ? -b : b;
				v.v[(j+Z)%3] = (k & 1) ? -b : b;
				v.y += b;
				lpvsGrid[12*i+4*j+k].o = v;
			}
		}
	}
}

void C3DEngine::DrawHall()
{
	static HPEN	 hPen;

	if (GridLines == 0)
		return;

	int nGridLines = GridLines + 1;

	CreateHall();
	// transform the points
	TransformVertices(lpvsGrid, 12*nGridLines, m_Viewmatrix);
	// draw grid
	PALETTEENTRY ppe = g_upe[ COLOR_GRIDGROUND ];
	hPen = CreatePen(PS_SOLID, 0, PALETTERGB(ppe.peRed, ppe.peGreen, ppe.peBlue));
	hPen = SelectPen(g_hdc, hPen);
	SelectBrush(g_hdc, GetStockBrush(NULL_BRUSH));
	for(int i=0; i<nGridLines; i++) {
		for(int j=0; j<6; j++) {
			if (StoreLPoints(&lpvsGrid[ 12*i + 2*j ], &lpvsGrid[ 12*i + 2*j + 1 ]) == 2) {
				MoveToEx(g_hdc, PointBuffer[0].x, PointBuffer[0].y, NULL);
				LineTo(g_hdc, PointBuffer[1].x, PointBuffer[1].y);
			}
			if (StoreLPoints(&lpvsGrid[ 12*i + 2*j - (j&1) ], &lpvsGrid[ 12*i + 2*j - (j&1) + 2 ]) == 2) {
				MoveToEx(g_hdc, PointBuffer[0].x, PointBuffer[0].y, NULL);
				LineTo(g_hdc, PointBuffer[1].x, PointBuffer[1].y);
			}
		}
	}
	hPen = SelectPen(g_hdc, hPen);
	DeleteObject(hPen);
	delete[] lpvsGrid;
}

#ifdef ACCEL_GFX8

void C3DEngine::DrawHall8()
{
	if (GridLines == 0)
		return;

	int nGridLines = GridLines + 1;

	CreateHall();
	// transform the points
	TransformVertices(lpvsGrid, 12*nGridLines, m_Viewmatrix);
	// draw grid
	_SetColor8(g_mpe[ COLOR_GRIDGROUND ]);
	for(int i=0; i<nGridLines; i++) {
		for(int j=0; j<6; j++) {
			if (StoreLPoints(&lpvsGrid[ 12*i + 2*j ], &lpvsGrid[ 12*i + 2*j + 1 ]) == 2) {
				_MoveTo8(PointBuffer[0].x, PointBuffer[0].y);
				_LineTo8(PointBuffer[1].x, PointBuffer[1].y);
			}
			if (StoreLPoints(&lpvsGrid[ 12*i + 2*j - (j&1) ], &lpvsGrid[ 12*i + 2*j - (j&1) + 2 ]) == 2) {
				_MoveTo8(PointBuffer[0].x, PointBuffer[0].y);
				_LineTo8(PointBuffer[1].x, PointBuffer[1].y);
			}
		}
	}
	delete[] lpvsGrid;
}

#endif

BOOL C3DEngine::DrawScene()
{
	if (Airplane == NULL)
		return TRUE;
	RECT rc;
	g_Display.GetBackBufferRect(&rc);
	SetViewport(&rc);
	SetViewpoint(C3DVector(ViewPosX, ViewPosY, ViewPosZ));
	SetViewvector(Airplane->m_Matrix.v[H] - m_Viewpoint);
	CalcViewMatrix();

	if (!DrawBackground())
		return FALSE;

	if (g_bUseGDI || !g_Display.IsPaletteMode()) {
		if (!g_Display.GetDC(&g_hdc)) return FALSE;
		DrawHall();
		for(CIDIter<C3DBspObject> ObjectIter(m_BspObjectList); ObjectIter(); ObjectIter++) {
			ObjectIter()->Render(m_Viewmatrix, FALSE);
		}
		if (!g_Display.ReleaseDC(g_hdc)) return FALSE;
	} else {
		if (!g_Display.Lock()) return FALSE;
#ifdef ACCEL_GFX8
		DrawHall8();
#endif
		for(CIDIter<C3DBspObject> ObjectIter(m_BspObjectList); ObjectIter(); ObjectIter++) {
			ObjectIter()->Render(m_Viewmatrix, TRUE);
		}
		if (!g_Display.Unlock()) return FALSE;
	}
	return TRUE;
}

BOOL RenderScene()
{
	return My3DEngine.DrawScene();
}

void FreeAllBuffers()
{
	My3DEngine.FreeAllBuffers();
}


