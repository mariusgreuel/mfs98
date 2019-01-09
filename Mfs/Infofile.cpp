/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     infofile.cpp, information file routines

 Version:   1.00

 Date:      18-Jan-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "infofile.h"

#define COMMENT_CHAR            ';'
#define QUOTE_CHAR              '"'
#define SECTION_START_CHAR      '['
#define SECTION_END_CHAR        ']'
#define ASSIGN_CHAR             '='
#define SEQUENCE_CHAR           ','

/////////////////////////////////////////////////////////////////////////////
// Locals...

//MODULE(__FILE__);

static LPCSTR   DefaultSection = TEXT("Default");
static LPCSTR   IncludeSection = TEXT("Include");
static CMGString  NullString = TEXT("");

/////////////////////////////////////////////////////////////////////////////
// class CInfKey

// destroy all arguments
void CInfKey::DestroyValues()
{
    CInfValue *Value;
    while ((Value = ValueList.Get()) != NULL)
        delete Value;
}


// add a value to key
void CInfKey::AddValue(LPCSTR szText)
{
    ValueList.Append(new CInfValue(szText));
}


// return value from key
const CMGString &CInfKey::GetValue(int nValue) const
{
    CInfValueIter ValueIter(this);
    CInfValue *Value = ValueIter[ nValue ];
    if (Value == NULL)
        return NullString;
    return Value->GetValue();
}


/////////////////////////////////////////////////////////////////////////////
// class CInfSection

// destroy all keys
void CInfSection::DestroyKeys()
{
    CInfKey *Key;

    while ((Key = KeyList.Get()) != NULL)
        delete Key;
}


// add a value to key to section
void CInfSection::AddValue(LPCSTR szKey, LPCSTR szValue)
{
    AddKey(szKey)->AddValue(szValue);
}


// add a key to section
CInfKey *CInfSection::AddKey(LPCSTR szKey)
{
    CInfKey *Key;

    if (szKey[0] != '\0' && (Key = SearchKey(szKey)) != NULL) {
        Key->DestroyValues();
    } else {
        Key = new CInfKey(szKey);
        KeyList.Append(Key);
    }

    return Key;
}


// search key in list
CInfKey *CInfSection::SearchKey(LPCSTR szKey) const
{
    for(CInfKeyIter KeyIter(this); KeyIter(); KeyIter++)
        if (KeyIter()->GetName() == szKey)
            break;

    return KeyIter();
}

// return value from key
const CMGString &CInfSection::GetValue(LPCSTR szKeyName, int nValue) const
{
    CInfKey *Key = SearchKey(szKeyName);
    if (Key == NULL)
        return NullString;
    return Key->GetValue(nValue);
}

/////////////////////////////////////////////////////////////////////////////
// class CInfList

// destroy all sections
void CInfList::DestroySections()
{
    CInfSection *Section;

    while ((Section = SectionList.Get()) != NULL)
        delete Section;
}


// add a value to key to section to infofile
void CInfList::AddValue(LPCSTR szSection, LPCSTR szKey, LPCSTR szValue)
{
    AddSection(szSection)->AddKey(szKey)->AddValue(szValue);
}


// add a key to section to infofile
CInfKey *CInfList::AddKey(LPCSTR szSection, LPCSTR szKey)
{
    return AddSection(szSection)->AddKey(szKey);
}


// add a section to infolist
CInfSection *CInfList::AddSection(LPCSTR szSection)
{
    CInfSection *Section = SearchSection(szSection);

    if (Section == NULL) {
        Section = new CInfSection(szSection);
        SectionList.Append(Section);
    }

    return Section;
}


// search section in list
CInfSection *CInfList::SearchSection(LPCSTR szName) const
{
    for(CInfSectionIter SectionIter(this); SectionIter(); SectionIter++)
        if (SectionIter()->GetName() == szName)
            break;
    return SectionIter();
}


// return value from key in section
const CMGString &CInfList::GetValue(LPCSTR szSectionName, LPCSTR szKeyName, int nValue) const
{
    CInfSection *Section = SearchSection(szSectionName);
    if (Section == NULL)
        return NullString;
    return Section->GetValue(szKeyName, nValue);
}


/////////////////////////////////////////////////////////////////////////////
// class InfoFile

// skip whitespace and comments (preceded by a semicolon)
// return TRUE if char found in this line
BOOL CInfFile::SkipWhiteSpace()
{
    char ch;

    while (strm.get(ch) && isspace(ch) && ch != '\n');

    strm.putback(ch);

    if (strm.eof() || ch == '\n' || ch == COMMENT_CHAR)
        return FALSE;

    return TRUE;
}


// skip a given char
// return TRUE if char found
BOOL CInfFile::SkipChar(char SkipChar)
{
    char ch;

    if (SkipWhiteSpace() == FALSE)
        return FALSE;

    strm.get(ch);

    if (ch != SkipChar) {
        strm.putback(ch);
        return FALSE;
    }

    return TRUE;
}


// return a string (with or without quotes)
CMGString CInfFile::GetString()
{
    char ch;
    CMGString Word;
    if (SkipChar(QUOTE_CHAR)) {
        while (strm.get(ch) && ch != '\n' && ch != QUOTE_CHAR)
            Word += ch;
        strm.putback(ch);
        SkipChar(QUOTE_CHAR);
    } else {
        while (strm.get(ch) && (isalnum(ch   ) || ch == '_' || ch == '.' || ch == '+' || ch == '-'))
            Word += ch;
        strm.putback(ch);
    }
    return Word;
}


// scan stream and build info list
BOOL CInfFile::ProcessInputStream()
{
    CInfSection *Section = NULL;
    while (!strm.eof()) {
        if (SkipWhiteSpace()) {
            if (SkipChar(SECTION_START_CHAR)) {
                CMGString SectionName = GetString();
                SkipChar(SECTION_END_CHAR);
                Section = AddSection(SectionName);
            } else {
                if (Section == NULL)
                    Section = AddSection(DefaultSection);
                CInfKey *Key;
                CMGString KeyName = GetString();
                if (SkipChar(ASSIGN_CHAR)) {
                    Key = Section->AddKey(KeyName);
                    Key->AddValue(GetString());
                } else {
                    Key = Section->AddKey("");
                    Key->AddValue(KeyName);
                }
                while (SkipChar(SEQUENCE_CHAR))
                    Key->AddValue(GetString());
            }
        }
        char ch;
        while (strm.get(ch) && ch != '\n');
    }
    return TRUE;
}


// add a file to a Infofiles include list
void CInfFile::AddFile(LPCSTR szFile)
{
    AddValue(IncludeSection, "", szFile);
}


// scan file(s) and build info list
BOOL CInfFile::ProcessFiles(LPCSTR szFile)
{
    BOOL        bError = FALSE;
    CInfKey     LastFiles("");

    AddFile(szFile);

    CInfSection *IncSection = SearchSection(IncludeSection);

    for(CInfKeyIter KeyIter(IncSection); KeyIter(); KeyIter++) {
        for(CInfValueIter ValueIter(KeyIter()); ValueIter(); ValueIter++) {
            CMGString strFileName = ValueIter()->GetValue();
            strFileName.lower();
            for(CInfValueIter LastFilesIter(&LastFiles); LastFilesIter(); LastFilesIter++) {
                if (strFileName == LastFilesIter()->GetValue()) {
                    MGDBG1(DMSG_WARNING, "File '%s' included more than once!\n", (LPCSTR)strFileName);
                }
            }
            if (LastFilesIter() == NULL) {
                LastFiles.AddValue(strFileName);
                strm.open(strFileName);
                if (!strm.fail()) {
                    if (!ProcessInputStream())
                        bError = TRUE;
                    strm.close();
                } else {
                    MGDBG1(DMSG_WARNING, "Can't open script file '%s'\n", (LPCSTR)strFileName);
                    bError = TRUE;
                }
            }
        }
    }

    delete SectionList.Get();

    return !bError;
}


/////////////////////////////////////////////////////////////////////////////
// operator << (stream, class)

BOOL QuotesNeeded(const CMGString str)
{
    for(int i=0; i<str.GetLength(); i++) {
        char ch = str[i];
        if (isalnum(ch) || ch == '_' || ch == '.' || ch == '+' || ch == '-')
            continue;
        return TRUE;
    }
    return FALSE;
}


// write value
ostream &operator << (ostream &strm, const CInfValue &Value)
{
    CMGString strValue = Value.GetValue();

    if (QuotesNeeded(strValue))
        strm << '"' << strValue << '"';
    else
        strm << strValue;

    return strm;
}


// write key with arguments
ostream &operator << (ostream &strm, const CInfKey &Key)
{
    CMGString strKey = Key.GetName();

    if (strKey != "") {
        if (QuotesNeeded(strKey))
            strm << '"' << strKey << '"' << '=';
        else
            strm << strKey << '=';
    }

    CInfValueIter ValueIter(&Key);
    do {
        strm << *ValueIter();
        ValueIter++;
        if (ValueIter())
            strm << ',';
    } while (ValueIter());

    strm << '\n';

    return strm;
}


// write section with keys
ostream &operator << (ostream &strm, const CInfSection &Section)
{
    if (QuotesNeeded(Section.GetName()))
        strm << "[\"" << Section.GetName() << "\"]\n";
    else
        strm << '[' << Section.GetName() << "]\n";
    for(CInfKeyIter KeyIter(&Section); KeyIter(); KeyIter++)
        strm << *KeyIter();

    return strm;
}


// write whole infofile
ostream &operator << (ostream &strm, const CInfList &InfoList)
{
    for(CInfSectionIter SectionIter(&InfoList); SectionIter(); SectionIter++)
        strm << '\n' << *SectionIter();

    return strm;
}



