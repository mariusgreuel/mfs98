/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        vars.cpp - static variables

  Version:      1.00

  Date:         18-Jan-97

  Author:       MG

\***************************************************************************/

#include "stdafx.h"

#include "variable.h"

#pragma init_seg(lib)
CVarList  VarList;

CVarSection VarSectionCommon(VarList, "Common");


