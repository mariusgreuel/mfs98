/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        types.h, some types used in global vars

  Version:      1.00

  Date:         9-Jan-97

  Author:       MG

-----------------------------------------------------------------------------

   Change log:

      DATE     REV                 DESCRIPTION
   ----------- --- ----------------------------------------------------------
   9-Jan-1997  MG  initial implementation
-----------------------------------------------------------------------------
\***************************************************************************/

#ifndef _TYPES_H_INCLUDED
#define _TYPES_H_INCLUDED

typedef struct {
  RECT  rcPlacement;
  SIZE  szResolution;
  WORD  wBpp;
  BOOL  bFullscreen;
}WNDPLACEMENT, *LPWNDPLACEMENT;

#endif // _TYPES_H_INCLUDED

