/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     variable.h, dynamic variables

 Version:   1.00

 Date:      18-Jan-97

 Author:    MG

\***************************************************************************/


#ifndef __VARIABLE_H_INCLUDED__
#define __VARIABLE_H_INCLUDED__

#include <stdio.h>
#include <stdlib.h>
#include <fstream.h>

#include "mglist.h"

extern char szVarBuf[];

/////////////////////////////////////////////////////////////////////////////
// class definitions...

class CVarSection;
class CVarList;

/////////////////////////////////////////////////////////////////////////////
// class CVar definition

class CVarKey : public CISLink<CVarKey> {
public:
    CVarKey( CVarSection &Section, LPCSTR szName );
    LPCSTR GetName() const      { return m_szName; }

    virtual void SetValue( LPCSTR szValue ) = 0;
    virtual LPCSTR GetValue() const = 0;

private:
    LPCSTR              m_szName;
};

/////////////////////////////////////////////////////////////////////////////
// class CFVar definition

class CFVar : public CVarKey {
public:
    CFVar( CVarSection &Sec, LPCSTR szName, float fDefault, float fMin, float fMax );

    void SetValue( LPCSTR szValue )     { m_fValue = (float)atof( szValue ); }
    LPCSTR GetValue() const             { sprintf( szVarBuf, "%0.2f", m_fValue ); return szVarBuf; }

    void operator = ( float fValue )    { m_fValue = fValue; }
    operator float() const              { return m_fValue; }

    float GetMin() const                { return m_fMin; }
    float GetMax() const                { return m_fMax; }
    float GetDefault() const            { return m_fDefault; }

    float GetRealValue( int iPercent );
    int GetPercentValue( float fValue );

private:
    float               m_fValue;
    float               m_fDefault;
    float               m_fMin;
    float               m_fMax;
};

/////////////////////////////////////////////////////////////////////////////
// class CVarSection definition

class CVarSection : public CISLink<CVarSection> {
public:
    CVarSection( CVarList &List, LPCSTR szName );
    LPCSTR GetName() const { return m_szName; }

    void AddKey( CVarKey *pKey );
    CVarKey *SearchKey( LPCSTR szName ) const;
    int Count() { return m_KeyList.Count(); }

private:
    CISList<CVarKey>    m_KeyList;
    LPCSTR              m_szName;

    friend class CVarKeyIter;
};

/////////////////////////////////////////////////////////////////////////////
// class CVarList definition

class CVarList {
public:
    void AddSection( CVarSection *pSection );
    CVarSection *SearchSection( LPCSTR szName ) const;
    int Count() { return m_SectionList.Count(); }

private:
    CISList<CVarSection> m_SectionList;

    friend class CVarSectionIter;
};

/////////////////////////////////////////////////////////////////////////////
// class iterators

class CVarKeyIter : public CISIter<CVarKey> {
public:
    CVarKeyIter( const CVarSection *lpSection ) :
        CISIter<CVarKey>( lpSection->m_KeyList ) {}
};

class CVarSectionIter : public CISIter<CVarSection> {
public:
    CVarSectionIter( const CVarList *lpVarList ) :
        CISIter<CVarSection>( lpVarList->m_SectionList ) {}
};

#endif // __VARIABLE_H_INCLUDED__


