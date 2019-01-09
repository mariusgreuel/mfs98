/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:	 script.cpp, evaluate script files

 Version:   1.00

 Date:	  18-Jan-97

 Author:	MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "display.h"
#include "infofile.h"
#include "variable.h"
#include "3dengine.h"

/////////////////////////////////////////////////////////////////////////////
// Locals...

//MODULE(__FILE__);

/////////////////////////////////////////////////////////////////////////////
// Globals...

static CInfFile		 *InfoFile;

static LPCTSTR  SECTION_COLOR   = "Colors";
static LPCTSTR  SECTION_OBJECT  = "Objects";
static LPCTSTR  SECTION_POINT   = "Points";
static LPCTSTR  SECTION_PLANE   = "Polygons";

static LPCTSTR  strSectionScenery   = "Scenery";
static LPCTSTR  strKeyModel		 = "Model";
static LPCTSTR  strKeyWorld		 = "World";

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern CMGString		  g_strSceneryName;
extern CMGString		  g_strSceneryModel;
extern CMGString		  g_strSceneryWorld;

extern PALETTEENTRY	 g_upe[ MAX_PALETTEENTRIES ];
extern C3DEngine		My3DEngine;
extern CVarList		 VarList;

void FreeAllBuffers();

/////////////////////////////////////////////////////////////////////////////

static C3DVector GetVector(const CInfKey *Key, int Index = 0)
{
	return C3DVector(
			(float)atof(Key->GetValue(Index)),
			(float)atof(Key->GetValue(Index + 1)),
			(float)atof(Key->GetValue(Index + 2)));
}

static BOOL GetColor(LPCTSTR szName, LPM3DCOLOR lpColor)
{
	CInfSection *Section = InfoFile->SearchSection(SECTION_COLOR);
	if (Section == NULL) {
		MGDBG1(DMSG_ERROR, "Undefined color '%s' (no defined colors).\n", szName);
		return FALSE;
	}
	LPCTSTR szValue = Section->GetValue(szName);
	if (szValue == '\0') {
		MGDBG1(DMSG_ERROR, "Undefined color '%s'.\n", szName);
		return FALSE;
	}
	DWORD Index = atoi(szValue);
	*lpColor = Index;
	return TRUE;
}

// evaluate section 'Color'
static BOOL EvalColorSection()
{
	CInfSection *lpColorSection = InfoFile->SearchSection(SECTION_COLOR);
	if (lpColorSection == NULL) {
		MGDBG1(DMSG_ERROR, "No colors defined in section '%s'.\n", SECTION_COLOR);
		return TRUE;
	}
	for(CInfKeyIter ColorIter(lpColorSection); ColorIter(); ColorIter++) {
		DWORD Index = atoi(ColorIter()->GetValue(0));
		if (Index == COLOR_NULL || Index == COLOR_TRANSLUCENT)
			continue;
		if (Index >= MAX_PALETTEENTRIES) {
			MGDBG1(DMSG_WARNING, "Maximum of %d colors supported.\n", MAX_PALETTEENTRIES);
		} else {
			g_upe[ Index ].peRed	= atoi(ColorIter()->GetValue(1));
			g_upe[ Index ].peGreen  = atoi(ColorIter()->GetValue(2));
			g_upe[ Index ].peBlue   = atoi(ColorIter()->GetValue(3));
			g_upe[ Index ].peFlags  = 0;
		}
	}
	return TRUE;
}

// evaluate section 'Object'
BOOL CreateBspBuffer(LPCTSTR szObjectSection, C3DBspBuffer **lplpBspBuffer)
{
	BOOL bError = FALSE;

	// look if buffer already created
	C3DBspBuffer *lpBspBuffer = My3DEngine.GetBspBuffer(szObjectSection);
	if (lpBspBuffer != NULL) {
		*lplpBspBuffer = lpBspBuffer;
		return TRUE;
	}
	MGDBG1(DMSG_INFO, "Build object '%s': ", szObjectSection);
	CInfSection *lpObjectSection = InfoFile->SearchSection(szObjectSection);
	if (lpObjectSection == NULL) {
		MGDBG1(DMSG_ERROR, "Object section '%s' not found\n", szObjectSection);
		return FALSE;
	}
	LPCTSTR szVertexSection = lpObjectSection->GetValue(SECTION_POINT);
	CInfSection *lpVertexSection = InfoFile->SearchSection(szVertexSection);
	if (lpVertexSection == NULL) {
		MGDBG2(DMSG_ERROR, "Object '%s' has no '%s' section.\n", szObjectSection, SECTION_POINT);
		return FALSE;
	}
	LPCTSTR szFaceSection = lpObjectSection->GetValue(SECTION_PLANE);
	CInfSection *lpFaceSection = InfoFile->SearchSection(szFaceSection);
	if (lpFaceSection == NULL) {
		MGDBG2(DMSG_ERROR, "Object '%s' has no '%s' section\n", szObjectSection, SECTION_PLANE);
		return FALSE;
	}
	int nVertices = lpVertexSection->CountKeys();
	int nFaces = lpFaceSection->CountKeys();
	MGDBG2(DMSG_INFO, "%u vertices, %lu faces\n", nVertices, nFaces);
	lpBspBuffer = My3DEngine.CreateBspBuffer(szObjectSection, nVertices);
	for(CInfKeyIter VtxKeyIter(lpVertexSection); VtxKeyIter(); VtxKeyIter++)
		lpBspBuffer->AddVertex(GetVector(VtxKeyIter()));
	for(CInfKeyIter FaceIter(lpFaceSection); FaceIter(); FaceIter++) {
		M3DCOLORSET Color;
		if (!GetColor(FaceIter()->GetValue(0), &Color.fgp) ||
				!GetColor(FaceIter()->GetValue(1), &Color.fgb) ||
				!GetColor(FaceIter()->GetValue(2), &Color.bgp) ||
				!GetColor(FaceIter()->GetValue(3), &Color.bgb))
			return FALSE;
		int Points = 0;
		int PointList[ 50 ];
		for(CInfValueIter VtxValueIter(FaceIter(), 4); VtxValueIter(); VtxValueIter++) {
			int Index = 0;
			for(CInfKeyIter VtxKeyIter(lpVertexSection); VtxKeyIter(); VtxKeyIter++) {
				if (VtxKeyIter()->GetName() == VtxValueIter()->GetValue())
					break;
				Index++;
			}
			if (VtxKeyIter()) {
				PointList[ Points++ ] = Index;
			} else {
				PointList[ Points++ ] = 0;
				LPCSTR s;
				s = VtxValueIter()->GetValue();
				MGDBG1(DMSG_ERROR, "Point '%s' not found\n", s);
				bError = TRUE;
				break;
			}
		}
		lpBspBuffer->AddFace(Color, Points, PointList);
	}
	if (bError)
		return FALSE;
	lpBspBuffer->BuildBspTree();
	*lplpBspBuffer = lpBspBuffer;
	return TRUE;
}

BOOL EvalObjectSection()
{
	CInfSection *lpSection = InfoFile->SearchSection(SECTION_OBJECT);
	if (lpSection == NULL)
		return TRUE;
	for(CInfKeyIter KeyIter(lpSection); KeyIter(); KeyIter++) {
		CMGString strObjectName = KeyIter()->GetValue();
		CMGString strShadowObjectName = InfoFile->GetValue(strObjectName, "Shadow");
		C3DBspObject *lpObject = My3DEngine.CreateBspObject();
		C3DBspBuffer *lpBspBuffer;
		if (!CreateBspBuffer(strObjectName, &lpBspBuffer))
			return FALSE;
		lpObject->AttachBspBuffer(lpBspBuffer);
		if (strShadowObjectName != "") {
			C3DBspBuffer *lpShadowBspBuffer;
			if (!CreateBspBuffer(strShadowObjectName, &lpShadowBspBuffer))
				return FALSE;
			lpObject->AttachShadowBspBuffer(lpShadowBspBuffer);
		}
		lpObject->Move(GetVector(KeyIter(), 1));
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// evaluate script files

BOOL ScriptError(LPCSTR szFilename, UINT iLine, LPCSTR szErrorMsg)
{
	MGDBG3(DMSG_ERROR, "%s(%u): %s\n", szFilename, iLine, szErrorMsg);
	return FALSE;
}


BOOL LoadScenery(LPCTSTR szFilename)
{
	CInfFile Scenery, Script;

	LPCSTR szModel;
	LPCSTR szWorld;

	if (!Scenery.ProcessFiles(szFilename))
		return FALSE;
 
	FreeAllBuffers();

	CInfSection *lpSection = Scenery.SearchSection(strSectionScenery);
	if (lpSection == NULL)
		return ScriptError(szFilename, 0, "This is not a scenery file.");
	szModel = lpSection->GetValue(strKeyModel);
	if (*szModel == '\0')
		return ScriptError(szFilename, 0, "'Model' entry required.");
	szWorld = lpSection->GetValue(strKeyWorld);
	if (szWorld == '\0')
		return ScriptError(szFilename, 0, "'World' entry required.");

	InfoFile = &Script;
	Script.AddFile(szModel);
	Script.AddFile(szWorld);

	if (!Script.ProcessFiles("mfs.if"))
		return FALSE;

	if (!EvalObjectSection() || !EvalColorSection())
		return FALSE;

	for(CInfSectionIter InfSectionIter(&Scenery); InfSectionIter(); InfSectionIter++) {
		CVarSection *VarSection = VarList.SearchSection(InfSectionIter()->GetName());
		if (VarSection) {
			for(CInfKeyIter InfKeyIter(InfSectionIter()); InfKeyIter(); InfKeyIter++) {
				CVarKey *VarKey = VarSection->SearchKey(InfKeyIter()->GetName());
				if (VarKey) {
					VarKey->SetValue(InfKeyIter()->GetValue());
				} else {
					MGDBG3(DMSG_ERROR, "%s(): Key '%s' in Section '%s' is invalid\n",
							szFilename, (LPCSTR)InfKeyIter()->GetName(), (LPCSTR)InfSectionIter()->GetName());
				}
			}
		} else {
			MGDBG2(DMSG_ERROR, "%s(): Section '%s' is invalid\n",
					szFilename, (LPCSTR)InfSectionIter()->GetName());
		}
	}

	g_strSceneryModel = szModel;
	g_strSceneryWorld = szWorld;

	return TRUE;
}


BOOL SaveScenery(LPCTSTR szFilename)
{
	ofstream			strm;
	CInfList			InfList;

	// add model and world keys to info list
	InfList.AddValue(strSectionScenery, strKeyModel, g_strSceneryModel);
	InfList.AddValue(strSectionScenery, strKeyWorld, g_strSceneryWorld);
	// add variables
	for(CVarSectionIter VarSectionIter(&VarList); VarSectionIter(); VarSectionIter++) {
		CInfSection *InfSection = InfList.AddSection(VarSectionIter()->GetName());
		for(CVarKeyIter VarKeyIter(VarSectionIter()); VarKeyIter(); VarKeyIter++) {
			InfSection->AddValue(VarKeyIter()->GetName(), VarKeyIter()->GetValue());
		}
	}
	// write info file
	strm.open(szFilename);
	if (strm.fail())
		return FALSE;
	strm << ';' << szFilename << '\n';
	strm << InfList;

	return TRUE;
}

BOOL WriteInfoFile(LPCSTR pszFilename, CInfList &InfList)
{
	ofstream	strm;

	// write info file
	strm.open(pszFilename);
	if (strm.fail()) {
		MGDBG1(DMSG_ERROR, "Could not create infofile '%s'\n", pszFilename);
		return FALSE;
	}
	strm << ';' << pszFilename << '\n';
	strm << InfList;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////

BOOL LoadParameterSet(CInfList *pInfList, CVarList *pVarList, LPCSTR pszSection)
{
	CInfSection	 *pInfSection;
	CVarSection	 *pVarSection;
	BOOL			bInvalidKeyFound = FALSE;

	pInfSection = pInfList->SearchSection(pszSection);
	if (pInfSection == NULL) {
		MGDBG1(DMSG_ERROR, "Couldn't find section '%s'.\n", pszSection);
		return FALSE;
	}

	pVarSection = pVarList->SearchSection(pszSection);
	if (pInfSection == NULL) {
		MGDBG1(DMSG_ERROR, "Section '%s' is not a parameter section.\n", pszSection);
		return FALSE;
	}

	for(CInfKeyIter InfKeyIter(pInfSection); InfKeyIter(); InfKeyIter++) {

		CFVar *VarKey = (CFVar*)pVarSection->SearchKey(InfKeyIter()->GetName());

		if (VarKey == NULL) {
			MGDBG1(DMSG_WARNING, "Parameter '%s' is not defined.\n", (LPCSTR)InfKeyIter()->GetName());
			bInvalidKeyFound = TRUE;
			continue;
		}

		float fValue = (float)atof(InfKeyIter()->GetValue());

		if (fValue < VarKey->GetMin() || fValue > VarKey->GetMax()) {
			MGDBG1(DMSG_WARNING, "Parameter '%s' is out of range.\n", (LPCSTR)InfKeyIter()->GetName());
			continue;
		}

		*VarKey = fValue;
	}

	if (bInvalidKeyFound)
		MGDBG1(DMSG_WARNING, "One or more invalid parameter in section '%s'\n", pszSection);

	return TRUE;
}


BOOL SaveParameterSet(CVarList *pVarList, CInfList *pInfList, LPCSTR pszSection)
{
	CVarSection	 *pVarSection;
	CInfSection	 *pInfSection;

	pVarSection = pVarList->SearchSection(pszSection);
	if (pInfSection == NULL) {
		MGDBG1(DMSG_ERROR, "Section '%s' is not a parameter section.\n", pszSection);
		return FALSE;
	}

	pInfSection = pInfList->AddSection(pszSection);
	if (pInfSection == NULL) {
		MGDBG1(DMSG_ERROR, "Couldn't create section '%s'.\n", pszSection);
		return FALSE;
	}

	for(CVarKeyIter VarKeyIter(pVarSection); VarKeyIter(); VarKeyIter++) {
		pInfSection->AddValue(VarKeyIter()->GetName(), VarKeyIter()->GetValue());
	}

	return TRUE;
}


BOOL LoadParameterFile(LPCSTR pszFilename, LPCSTR pszSection)
{
	CInfFile		InfFile;

	MGDBG1(DMSG_INFO, "Loading parameter file '%s'.\n", pszFilename);

	if (!InfFile.ProcessFiles(pszFilename)) {
		MGDBG0(DMSG_ERROR, "Missing or currupt parameter file.\n");
		return FALSE;
	}

	LoadParameterSet(&InfFile, &VarList, pszSection);

	return TRUE;
}


BOOL SaveParameterFile(LPCSTR pszFilename, LPCSTR pszSection)
{
	CInfFile		InfList;
	ofstream		strm;

	MGDBG1(DMSG_INFO, "Saving parameter file '%s'.\n", pszFilename);

	SaveParameterSet(&VarList, &InfList, pszSection);

	strm.open(pszFilename);
	if (strm.fail()) {
		MGDBG0(DMSG_ERROR, "Couldn't create parameter file.\n");
		return FALSE;
	}

	strm << ';' << pszFilename << '\n';
	strm << InfList;

	return TRUE;
}


