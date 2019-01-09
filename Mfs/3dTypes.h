/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        3dtypes.h

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

#ifndef __3DTYPES_H_INCLUDED__
#define __3DTYPES_H_INCLUDED__

typedef float M3DVALUE, *LPM3DVALUE;
typedef DWORD M3DCOLOR, *LPM3DCOLOR;

typedef struct tagM3DCOLORSET {
  M3DCOLOR      fgb;            // foreground brush color
  M3DCOLOR      bgb;            // background brush color
  M3DCOLOR      fgp;            // foreground pen color
  M3DCOLOR      bgp;            // background pen color
}M3DCOLORSET;

typedef struct tagM3DPOINT {
  M3DVALUE      x;
  M3DVALUE      y;
}M3DPOINT, *LPM3DPOINT;

typedef struct tagM3DRECT {
  M3DVALUE      x1;
  M3DVALUE      y1;
  M3DVALUE      x2;
  M3DVALUE      y2;
}M3DRECT, *LPM3DRECT;

typedef struct tagM3DVERTEX {
  union {
    struct {
      M3DVALUE  x;
      M3DVALUE  y;
      M3DVALUE  z;
    };
    M3DVALUE    v[3];
  };
}M3DVERTEX, *LPM3DVERTEX;

typedef struct tagM3DVERTEXSET {
  M3DVERTEX     o;              // object vertex
  M3DVERTEX     t;              // transformed vertex
  M3DPOINT      p;              // projected vertex
}M3DVERTEXSET, *LPM3DVERTEXSET;

typedef struct tagM3DBSPFACE {
  tagM3DBSPFACE *Before;
  tagM3DBSPFACE *Behind;
  M3DCOLORSET   Color;
  int           nVertices;
  M3DVERTEXSET  *vs[32];
}M3DBSPFACE, *LPM3DBSPFACE, **LPLPM3DBSPFACE;

#endif // __3DTYPES_H_INCLUDED__


