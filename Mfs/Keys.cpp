/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     user.cpp, user interface message handler

 Version:   1.00

 Date:      3-Feb-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "variable.h"
#include "3dengine.h"

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE        g_hInst;            // defined in winmain.cpp
extern HWND             g_hWnd;             // defined in winmain.cpp

extern BOOL             g_bSceneryModified;

extern LPCTSTR          g_szHelpFile;

extern CVarList         VarList;
extern C3DBspObject     *Airplane;

void Disp( LPSTR fmt, ... );
void InitScene();

/////////////////////////////////////////////////////////////////////////////
// Statics...

static int      CurrSectionIndex;
static int      CurrVariableIndex;

/////////////////////////////////////////////////////////////////////////////
// Vars...

void DisplayCurrentVar()
{
    CVarSectionIter SectionIter( &VarList );
    CVarSection *Section = SectionIter[CurrSectionIndex];
    if( Section == NULL )
        return;
    CVarKeyIter VariableIter( Section );
    CFVar *Var = (CFVar*)VariableIter[CurrVariableIndex];
    if( Var == NULL )
        return;
    Disp( "%s(%d): %s(%d): %3.3f",
                Section->GetName(), CurrSectionIndex,
                Var->GetName(), CurrVariableIndex, (float)*Var );
}

void SelectNextVarSection()
{
    CVarSectionIter SectionIter( &VarList );
    if( SectionIter[CurrSectionIndex+1] == NULL )
        return;
    CurrSectionIndex++;
    CurrVariableIndex = 0;
}

void SelectPrevVarSection()
{
    CVarSectionIter SectionIter( &VarList );
    if( SectionIter[CurrSectionIndex-1] == NULL )
        return;
    CurrSectionIndex--;
    CurrVariableIndex = 0;
}

void SelectNextVariable()
{
    CVarSectionIter SectionIter( &VarList );
    CVarSection *Section = SectionIter[CurrSectionIndex];
    if( Section == NULL )
        return;
    CVarKeyIter VariableIter( Section );
    if( VariableIter[CurrVariableIndex+1] == NULL )
        return;
    CurrVariableIndex++;
}

void SelectPrevVariable()
{
    CVarSectionIter SectionIter( &VarList );
    CVarSection *Section = SectionIter[CurrSectionIndex];
    if( Section == NULL )
        return;
    CVarKeyIter VariableIter( Section );
    if( VariableIter[CurrVariableIndex-1] == NULL )
        return;
    CurrVariableIndex--;
}

void IncreaseVariable()
{
    CVarSectionIter SectionIter( &VarList );
    CVarSection *Section = SectionIter[CurrSectionIndex];
    if( Section == NULL )
        return;
    CVarKeyIter VariableIter( Section );
    CFVar *Var = (CFVar*)VariableIter[CurrVariableIndex];
    if( Var == NULL )
        return;
    int i = Var->GetPercentValue( *Var );
    if( i < 100 )
        *Var = Var->GetRealValue( i + 1 );
    g_bSceneryModified = TRUE;
}

void DecreaseVariable()
{
    CVarSectionIter SectionIter( &VarList );
    CVarSection *Section = SectionIter[CurrSectionIndex];
    if( Section == NULL )
        return;
    CVarKeyIter VariableIter( Section );
    CFVar *Var = (CFVar*)VariableIter[CurrVariableIndex];
    if( Var == NULL )
        return;
    int i = Var->GetPercentValue( *Var );
    if( i > 0 )
        *Var = Var->GetRealValue( i - 1 );
    g_bSceneryModified = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// OnKeydown...

BOOL OnKeyCommand( UINT nKey )
{
    switch( nKey ) {
        case VK_F1:
            WinHelp( g_hWnd, g_szHelpFile, HELP_FINDER, 0L );
            break;
        case VK_ESCAPE:
            DestroyWindow( g_hWnd );
            break;


        case VK_PRIOR:
            SelectPrevVarSection();
            break;
        case VK_NEXT:
            SelectNextVarSection();
            break;
        case VK_LEFT:
            DecreaseVariable();
            break;
        case VK_RIGHT:
            IncreaseVariable();
            break;
        case VK_UP:
            SelectPrevVariable();
            break;
        case VK_DOWN:
            SelectNextVariable();
            break;

        case 'I':
            InitScene();
            break;

        case 'Q':
            Airplane->m_Matrix.RotateInAxe( C3DVector( 0.1, 0, 0 ) );
            break;
        case 'W':
            Airplane->m_Matrix.RotateInAxe( C3DVector( 0, 0.1, 0 ) );
            break;
        case 'E':
            Airplane->m_Matrix.RotateInAxe( C3DVector( 0, 0, 0.1 ) );
            break;
        case 'A':
            Airplane->m_Matrix.RotateInAxe( C3DVector( -0.1, 0, 0 ) );
            break;
        case 'S':
            Airplane->m_Matrix.RotateInAxe( C3DVector( 0, -0.1, 0 ) );
            break;
        case 'D':
            Airplane->m_Matrix.RotateInAxe( C3DVector( 0, 0, -0.1 ) );
            break;

        default:
            return FALSE;
    }
    // return TRUE to indicate that we have processed this message
    return TRUE;
}


