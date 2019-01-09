/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        airplane.cpp

  Version:      1.00

  Date:         18-Feb-97

  Author:       MG

\***************************************************************************/

#include "stdafx.h"

#include "3dmath.h"
#include "variable.h"

/////////////////////////////////////////////////////////////////////////////
// Defines...

extern CVarList         VarList;

#pragma init_seg(user)
CVarSection VarSectionAirplane(VarList, "Airplane");

static CFVar Sensitivity[] = {
  CFVar(VarSectionAirplane, "RollSensitivity",     6.0, 0.0, 5.0),
  CFVar(VarSectionAirplane, "GierSensitivity",     20.0, 0.0, 100.0),
  CFVar(VarSectionAirplane, "NickSensitivity",     6.0, 0.0, 5.0),
};

