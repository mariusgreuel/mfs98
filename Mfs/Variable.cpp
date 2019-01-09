/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:		variable.cpp, static variable routines

 Version:	1.00

 Date:		18-Jan-97

 Author:	MG

 CAUTION:
    all CVar, CVarSection, CVarList and it's names have to be static!

\***************************************************************************/

#include "stdafx.h"

#include "variable.h"

char szVarBuf[32];

/////////////////////////////////////////////////////////////////////////////
// class CVarKey implementation

CVarKey::CVarKey(CVarSection &Section, LPCSTR szName)
{
	m_szName = szName;
	Section.AddKey(this);
}

/////////////////////////////////////////////////////////////////////////////
// class CFVar implementation

CFVar::CFVar(CVarSection &Section, LPCSTR szName, float fDefault, float fMin, float fMax) :
			 CVarKey(Section, szName)
{
	m_fValue = m_fDefault = fDefault;
	m_fMin = fMin;
	m_fMax = fMax;
}

float CFVar::GetRealValue(int iValue)
{
	if (iValue < 0)
		return m_fMin;
	else if (iValue > 100)
		return m_fMax;
	return (float)(m_fMin + iValue * (m_fMax - m_fMin) / 100.0);
}

int CFVar::GetPercentValue(float fValue)
{
	if (fValue < m_fMin)
		return 0;
	else if (fValue > m_fMax)
		return 100;
	return (int)((fValue - m_fMin) * 100.0 / (m_fMax - m_fMin) + 0.5);
}

/////////////////////////////////////////////////////////////////////////////
// class CVarSection implementation

CVarSection::CVarSection(CVarList &List, LPCSTR szName)
{
	m_szName = szName;
	List.AddSection(this);
}

void CVarSection::AddKey(CVarKey *pKey)
{
	m_KeyList.Append(pKey);
}

CVarKey *CVarSection::SearchKey(LPCSTR szName) const
{
	for(CVarKeyIter KeyIter(this); KeyIter(); KeyIter++)
		if (lstrcmp(KeyIter()->GetName(), szName) == 0)
			break;
	return KeyIter();
}

/////////////////////////////////////////////////////////////////////////////
// class VarList

void CVarList::AddSection(CVarSection *pSection)
{
	m_SectionList.Append(pSection);
}

CVarSection *CVarList::SearchSection(LPCSTR szName) const
{
	for(CVarSectionIter SectionIter(this); SectionIter(); SectionIter++)
		if (lstrcmp(SectionIter()->GetName(), szName) == 0)
			break;
	return SectionIter();
}


