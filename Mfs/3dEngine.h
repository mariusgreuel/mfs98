/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        3dengine.h

  Version:      1.00

  Date:         20-Jan-97

  Author:       MG

-----------------------------------------------------------------------------

   Change log:

      DATE     REV      DESCRIPTION
   ----------- --- ----------------------------------------------------------
   20-Jan-1997 MG  initial implementation
-----------------------------------------------------------------------------
\***************************************************************************/

#ifndef __3DENGINE_H_INCLUDED__
#define __3DENGINE_H_INCLUDED__

#include "mglist.h"
#include "mgstring.h"
#include "3dtypes.h"
#include "3dmath.h"

class C3DBspBuffer : public CIDLink<C3DBspBuffer> {
public:
	C3DBspBuffer();
	~C3DBspBuffer();
	BOOL Create( LPCSTR Name, int nVertices );
	void AddVertex( C3DVector vec );
	void AddFace( M3DCOLORSET Color, int nVertices, int Index[] );
	void BuildBspTree();
	void TransformVertices( const C3DMatrix &Matrix );
	void RunBSPTree( BOOL bOwnDraw );
	LPCSTR GetName() { return m_Name; }

private:
	CMGString			m_Name;
	int					m_nVertices;
	int					m_nFaces;
	LPM3DVERTEXSET		m_lpVertexSet;
	LPM3DBSPFACE		m_lpBspFaceRoot;
};

class C3DBspObject : public CIDLink<C3DBspObject> {
public:
	C3DBspObject();
	void AttachBspBuffer( C3DBspBuffer *lpBspBuffer );
	void AttachShadowBspBuffer( C3DBspBuffer *lpShadowBspBuffer );
	void Move( const C3DVector &vec );
	void Rotate( const C3DVector &vec );
	void Render( const C3DMatrix &ViewMatrix, BOOL bUseGDI );

protected:
	C3DMatrix CalcShadowMatrix();

private:
	C3DBspBuffer         *m_lpBspBuffer;
	C3DBspBuffer         *m_lpShadowBspBuffer;
public:
	C3DMatrix             m_Matrix;
};

class C3DEngine {
public:
	C3DEngine();
	void Initialize();
	void FreeAllBuffers();
	C3DBspBuffer *GetBspBuffer( LPCSTR Name );
	C3DBspBuffer *CreateBspBuffer( LPCSTR Name, int nVertices );
	C3DBspObject *CreateBspObject();
	void SetViewport( LPRECT lprc );
	void SetViewpoint( const C3DVector &vec );
	void SetViewvector( const C3DVector &vec );
	friend BOOL RenderScene();

protected:
	void DrawHall();
	void DrawHall8();
	void CalcViewMatrix();
	BOOL DrawBackground();
	BOOL DrawScene();

private:
	CIDList<C3DBspBuffer> m_BspBufferList;
	CIDList<C3DBspObject> m_BspObjectList;

	RECT                  m_Viewport;
	C3DVector             m_Viewpoint;
	C3DVector             m_Viewvector;
	C3DMatrix             m_Viewmatrix;
};

#endif // __3DENGINE_H_INCLUDED__


