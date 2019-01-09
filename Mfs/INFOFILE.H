/***************************************************************************\

  Copyright (C) 1997 Marius Greuel. All rights reserved.

  Title:        infofile.h, information file routines

  Version:      1.00

  Date:         18-Jan-97

  Author:       MG

-----------------------------------------------------------------------------

   Change log:

      DATE     REV                 DESCRIPTION
   ----------- --- ----------------------------------------------------------
   18-Jan-1997 MG  initial implementation
-----------------------------------------------------------------------------
\***************************************************************************/

#ifndef __INFOFILE_H_INCLUDED__
#define __INFOFILE_H_INCLUDED__

#include <fstream.h>
#include "mgstring.h"
#include "mglist.h"

class CInfValue : public CISLink<CInfValue> {
public:
  CInfValue(LPCSTR szValue) : m_Value(szValue) {}
  const CMGString &GetValue() const { return m_Value; }

private:
  const CMGString m_Value;
};

class CInfKey : public CISLink<CInfKey> {
public:
  CInfKey(LPCSTR szName) : m_Name(szName) {}
  ~CInfKey() { DestroyValues(); }
  void AddValue(LPCSTR szValue);
  const CMGString &GetName() const { return m_Name; }
  const CMGString &GetValue(int nValue = 0) const;
  int CountValues() const { return ValueList.Count(); }
  friend class CInfValueIter;
  friend class CInfSection;

private:
  CISList<CInfValue> ValueList;
  const CMGString m_Name;

protected:
  void DestroyValues();
};

class CInfSection : public CISLink<CInfSection> {
public:
  CInfSection(LPCSTR szName) : m_Name(szName) {}
  ~CInfSection() { DestroyKeys(); }
  void AddValue(LPCSTR szKey, LPCSTR szValue);
  CInfKey *AddKey(LPCSTR szKey);
  CInfKey *SearchKey(LPCSTR szKey) const;
  const CMGString &GetName() const { return m_Name; }
  const CMGString &GetValue(LPCSTR szKeyName, int nValue = 0) const;
  int CountKeys() const { return KeyList.Count(); }
  friend class CInfKeyIter;

private:
  CISList<CInfKey> KeyList;
  const CMGString m_Name;

protected:
  void DestroyKeys();
};

class CInfList {
public:
  ~CInfList() { DestroySections(); }
  void AddValue(LPCSTR szSection, LPCSTR szKey, LPCSTR szValue);
  CInfKey *AddKey(LPCSTR szSection, LPCSTR szKey);
  CInfSection *AddSection(LPCSTR szSection);
  CInfSection *SearchSection(LPCSTR szSection) const;
  const CMGString &GetValue(LPCSTR szSectionName, LPCSTR szKeyName, int nValue = 0) const;
  friend class CInfSectionIter;

protected:
  CISList<CInfSection> SectionList;

protected:
  void DestroySections();
};

class CInfFile : public CInfList {
public:
  void AddFile(LPCSTR szFile);
  BOOL ProcessFiles(LPCSTR szFile);

private:
  ifstream strm;

protected:
  BOOL SkipWhiteSpace();
  BOOL SkipChar(char SkipChar);
  CMGString GetString();
  BOOL ProcessInputStream();
};

class CInfValueIter : public CISIter<CInfValue> {
public:
  CInfValueIter(const CInfKey *Key, int Start = 0) : CISIter<CInfValue>(Key->ValueList, Start) {}
};

class CInfKeyIter : public CISIter<CInfKey> {
public:
  CInfKeyIter(const CInfSection *Section) : CISIter<CInfKey>(Section->KeyList) {}
};

class CInfSectionIter : public CISIter<CInfSection> {
public:
  CInfSectionIter(const CInfList *InfoList) : CISIter<CInfSection>(InfoList->SectionList) {}
};

ostream &operator << (ostream &strm, const CInfValue &Value);
ostream &operator << (ostream &strm, const CInfKey &Key);
ostream &operator << (ostream &strm, const CInfSection &Section);
ostream &operator << (ostream &strm, const CInfList &InfoList);

istream &operator >> (istream const &istrm, CInfFile &Infofile);

#endif // __INFOFILE_H_INCLUDED__


