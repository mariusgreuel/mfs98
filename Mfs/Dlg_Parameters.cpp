/***************************************************************************\

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:     Dlg_Parameters.cpp

 Version:   1.00

 Date:      11-Jul-97

 Author:    MG

\***************************************************************************/

#include "stdafx.h"

#include "mgdebug.h"
#include "variable.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// Statics...

//MGMODULE( __FILE__ );

#define LOGERROR MGDBG0

/////////////////////////////////////////////////////////////////////////////
// Externals...

extern HINSTANCE        g_hInst;
extern HWND             g_hWnd;

extern CVarList         VarList;

BOOL LoadParameterFile( LPCSTR pszFilename, LPCSTR pszSection );
BOOL SaveParameterFile( LPCSTR pszFilename, LPCSTR pszSection );

/////////////////////////////////////////////////////////////////////////////
// Parameter dialog

class CDialogParameter {
public:
    CDialogParameter( UINT uStartPage );
    BOOL Create( UINT nIDTemplate, HWND hParentWnd = NULL );

protected:
    BOOL OnInitDialog();
    BOOL OnCommand( WPARAM wParam, LPARAM lParam );
    BOOL OnNotify( WPARAM wParam, LPARAM lParam, LRESULT* pResult );
    BOOL OnHScroll();

    static LRESULT CALLBACK DialogProc( HWND m_hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:
    void FillParameterListView( CVarSection *m_pSelSection );
    void UpdateItem();
    BOOL LoadParameter();
    BOOL SaveParameter();

    HWND            m_hWnd;
    HWND            m_hWndTabCtrl, m_hWndListView, m_hWndTrackBar;

    UINT            m_uStartPage;

    BOOL            m_bFillMode;

    int             m_iSelItem;
    CVarSection     *m_pSelSection;
    CFVar           *m_pSelKey;
    int             m_iVarValue;
};


/////////////////////////////////////////////////////////////////////////////

BOOL ShowDlg_Parameter( UINT uStartPage )
{
    CDialogParameter    Dialog( uStartPage );

    return Dialog.Create( IDD_PARAMETERS, g_hWnd );
}


/////////////////////////////////////////////////////////////////////////////

CDialogParameter::CDialogParameter( UINT uStartPage )
{
    m_uStartPage = uStartPage;

    m_bFillMode = FALSE;
}


BOOL CDialogParameter::Create( UINT nIDTemplate, HWND hParentWnd )
{
    return DialogBoxParam(
            g_hInst,
            MAKEINTRESOURCE( nIDTemplate ),
            hParentWnd,
            (DLGPROC)DialogProc,
            (LPARAM)this );
}


BOOL CDialogParameter::OnInitDialog()
{
    if( ( m_hWndTabCtrl  = GetDlgItem( m_hWnd, IDC_PARAM_TABCTRL  ) ) == NULL ||
        ( m_hWndListView = GetDlgItem( m_hWnd, IDC_PARAM_LISTVIEW ) ) == NULL ||
        ( m_hWndTrackBar = GetDlgItem( m_hWnd, IDC_PARAM_TRACKBAR ) ) == NULL ) {
        LOGERROR( "Couldn't get dialog items\n" );
        return FALSE;
    }

    // initialize tab control
    int i = 0;
    for( CVarSectionIter m_pSelSectionIter( &VarList ); m_pSelSectionIter(); m_pSelSectionIter++ ) {
        TC_ITEM     tcI;
        tcI.mask = TCIF_TEXT | TCIF_PARAM;
        tcI.pszText = (LPSTR)m_pSelSectionIter()->GetName();
        tcI.lParam = (LPARAM)m_pSelSectionIter();
        TabCtrl_InsertItem( m_hWndTabCtrl, i, &tcI );
        if( i == m_uStartPage )
            m_pSelSection = m_pSelSectionIter();
        i++;
    }
    TabCtrl_SetCurSel( m_hWndTabCtrl, m_uStartPage );

    // create column headers
    RECT rc;
    GetWindowRect( m_hWndListView, &rc );
    rc.right -= GetSystemMetrics( SM_CXVSCROLL ) + GetSystemMetrics( SM_CXFRAME );
    LV_COLUMN       lvC;
    lvC.mask        = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    lvC.fmt         = LVCFMT_LEFT;
    lvC.cx          = (rc.right-rc.left) - (rc.right-rc.left) / 4;
    lvC.pszText     = "Name";
    lvC.iSubItem    = 0;
    ListView_InsertColumn( m_hWndListView, 0, &lvC );

    lvC.fmt         = LVCFMT_RIGHT;
    lvC.cx          = (rc.right-rc.left) / 4;
    lvC.pszText     = "Value";
    lvC.iSubItem    = 1;
    ListView_InsertColumn( m_hWndListView, 1, &lvC );
    FillParameterListView( m_pSelSection );

    // initialize trackbar control
    SendMessage( m_hWndTrackBar, TBM_SETRANGE, 0, MAKELONG( 0, 100 ) );
    SendMessage( m_hWndTrackBar, TBM_SETTICFREQ, 10, 0 );
    SendMessage( m_hWndTrackBar, TBM_SETLINESIZE, 0, 1 );
    SendMessage( m_hWndTrackBar, TBM_SETPAGESIZE, 0, 10 );
    Button_Enable( GetDlgItem( m_hWnd, IDAPPLY ), 0 );

    return TRUE;
}


BOOL CDialogParameter::OnCommand( WPARAM wParam, LPARAM )
{
    char cBuf[32];

    switch( LOWORD( wParam ) ) {
        case IDOK: {
            if( m_pSelKey != NULL )
                *((CFVar*)m_pSelKey) = ((CFVar*)m_pSelKey)->GetRealValue( m_iVarValue );
            EndDialog( m_hWnd, TRUE );
            break;
        }
        case IDCANCEL: {
            EndDialog( m_hWnd, TRUE );
            break;
        }
        case IDAPPLY: {
            if( m_pSelKey != NULL ) {
                *((CFVar*)m_pSelKey) = ((CFVar*)m_pSelKey)->GetRealValue( m_iVarValue );
                UpdateItem();
            }
            SendMessage( m_hWnd, WM_NEXTDLGCTL, (WPARAM)GetDlgItem( m_hWnd, IDOK ), 1 );
            Button_Enable( GetDlgItem( m_hWnd, IDAPPLY ), 0 );
            break;
        }
        case IDC_PARAM_VALUE: {
            if( HIWORD( wParam ) == EN_CHANGE && m_pSelKey != NULL ) {
                GetDlgItemText( m_hWnd, IDC_PARAM_VALUE, cBuf, sizeof( cBuf ) );
                float f = atof( cBuf );
                int i = ((CFVar*)m_pSelKey)->GetPercentValue( f );
                if( abs( f - ((CFVar*)m_pSelKey)->GetRealValue( i ) ) > 1e-4 ) {
                    sprintf( cBuf, "%.2f", ((CFVar*)m_pSelKey)->GetRealValue( i ) );
                    SetDlgItemText( m_hWnd, IDC_PARAM_VALUE, cBuf );
                }
                if( m_iVarValue != i ) {
                    m_iVarValue = i;
                    SendMessage( m_hWndTrackBar, TBM_SETPOS, TRUE, m_iVarValue );
                    Button_Enable( GetDlgItem( m_hWnd, IDAPPLY ), 1 );
                }
            }
            break;
        }
        case IDC_PARAM_LOAD:
            LoadParameter();
            break;
        case IDC_PARAM_SAVE:
            SaveParameter();
            break;
        default:
            return FALSE;
    }
    return TRUE;
}


BOOL CDialogParameter::OnNotify( WPARAM wParam, LPARAM lParam, LRESULT*  )
{
    TC_ITEM     tcI;
    CHAR        cBuf[16];

    switch( wParam ) {
        case IDC_PARAM_TABCTRL: {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if( pnm->code == TCN_SELCHANGE ) {
                tcI.mask = TCIF_PARAM;
                TabCtrl_GetItem( m_hWndTabCtrl, TabCtrl_GetCurSel( m_hWndTabCtrl ), &tcI );
                m_pSelSection = (CVarSection*)tcI.lParam;
                FillParameterListView( m_pSelSection );
            }
            break;
        }
        case IDC_PARAM_LISTVIEW: {
            LPNM_LISTVIEW pnm = (LPNM_LISTVIEW)lParam;
            if( pnm->hdr.code == LVN_ITEMCHANGED && !m_bFillMode ) {
                m_iSelItem = pnm->iItem;
                m_pSelKey = (CFVar*)pnm->lParam;

                m_iVarValue = m_pSelKey->GetPercentValue( *m_pSelKey );
                SetDlgItemText( m_hWnd, IDC_PARAM_NAME, (LPSTR)m_pSelKey->GetName() );
                sprintf( cBuf, "%0.2f", m_pSelKey->GetMin() );
                SetDlgItemText( m_hWnd, IDC_PARAM_MIN, cBuf );
                SetDlgItemText( m_hWnd, IDC_PARAM_MIN2, cBuf );
                sprintf( cBuf, "%0.2f", m_pSelKey->GetMax() );
                SetDlgItemText( m_hWnd, IDC_PARAM_MAX, cBuf );
                SetDlgItemText( m_hWnd, IDC_PARAM_MAX2, cBuf );
                sprintf( cBuf, "%0.2f", m_pSelKey->GetDefault() );
                SetDlgItemText( m_hWnd, IDC_PARAM_DEFAULT, cBuf );
                SetDlgItemText( m_hWnd, IDC_PARAM_VALUE, (LPSTR)m_pSelKey->GetValue() );
                SendMessage( m_hWndTrackBar, TBM_SETPOS, TRUE, m_iVarValue );
            }
            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}


BOOL CDialogParameter::OnHScroll()
{
    char cBuf[32];

    if( m_pSelKey != NULL ) {
        m_iVarValue = SendMessage( m_hWndTrackBar, TBM_GETPOS, 0, 0 );
        sprintf( cBuf, "%.2f", ((CFVar*)m_pSelKey)->GetRealValue( m_iVarValue ) );
        SetDlgItemText( m_hWnd, IDC_PARAM_VALUE, cBuf );
        Button_Enable( GetDlgItem( m_hWnd, IDAPPLY ), 1 );
    }

    return TRUE;
}


LRESULT CALLBACK CDialogParameter::DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static CDialogParameter     *pDialog = NULL;

    switch( uMsg ) {
        case WM_INITDIALOG:
            pDialog = (CDialogParameter*)lParam;
            if( pDialog == NULL )
                return FALSE;
            pDialog->m_hWnd = hWnd;
            return pDialog->OnInitDialog();
        case WM_COMMAND:
            if( pDialog == NULL )
                return FALSE;
            return pDialog->OnCommand( wParam, lParam );
        case WM_NOTIFY:
            if( pDialog == NULL )
                return FALSE;
            return pDialog->OnNotify( wParam, lParam, NULL );
        case WM_HSCROLL:
            if( pDialog == NULL )
                return FALSE;
            return pDialog->OnHScroll();
        default:
            return FALSE;
    }
}


void CDialogParameter::FillParameterListView( CVarSection *m_pSelSection )
{
    int     i=0;

    m_bFillMode = TRUE;

    ListView_DeleteAllItems( m_hWndListView );
    ListView_SetItemCount( m_hWndListView, m_pSelSection->Count() );

    for( CVarKeyIter KeyIter( m_pSelSection ); KeyIter(); KeyIter++ ) {
        LV_ITEM         lvI;
        lvI.mask        = LVIF_TEXT | LVIF_PARAM;
        lvI.iItem       = i;
        lvI.iSubItem    = 0;
        lvI.pszText     = "";
        lvI.cchTextMax  = 32;
        lvI.lParam = (LPARAM)KeyIter();
        i = ListView_InsertItem( m_hWndListView, &lvI );
        ListView_SetItemText( m_hWndListView, i, 0, (LPSTR)KeyIter()->GetName() );
        ListView_SetItemText( m_hWndListView, i, 1, (LPSTR)KeyIter()->GetValue() );
        i++;
    }
    m_bFillMode = FALSE;

    ListView_EnsureVisible( m_hWndListView, 0, FALSE );
    ListView_SetItemState( m_hWndListView, 0, CDIS_DEFAULT, CDIS_DEFAULT );
}


void CDialogParameter::UpdateItem()
{
    ListView_SetItemText( m_hWndListView, m_iSelItem, 1, (LPSTR)m_pSelKey->GetValue() );
}


BOOL CDialogParameter::LoadParameter()
{
    OPENFILENAME    ofn;
    char            szFilename[MAX_PATH] = "";

    ofn.lStructSize         = sizeof(ofn);
    ofn.hwndOwner           = m_hWnd;
    ofn.hInstance           = g_hInst;
    ofn.lpstrFilter         = "Parametersatz (*.ifp)\0*.ifp\0Alle Dateien (*.*)\0*.*\0";
    ofn.lpstrCustomFilter   = (LPTSTR)NULL;
    ofn.nMaxCustFilter      = 0L;
    ofn.nFilterIndex        = 1L;
    ofn.lpstrFile           = szFilename;
    ofn.nMaxFile            = 256;
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = "Öffnen";
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = "*.ifp";
    ofn.lpfnHook            = NULL;
    ofn.Flags = OFN_LONGNAMES|OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

    if( !GetOpenFileName( &ofn ) )
        return FALSE;

    if( !LoadParameterFile( szFilename, m_pSelSection->GetName() ) )
        return FALSE;

    return TRUE;
}


BOOL CDialogParameter::SaveParameter()
{
    OPENFILENAME    ofn;
    char            szFilename[MAX_PATH] = "";

    ofn.lStructSize         = sizeof(ofn);
    ofn.hwndOwner           = m_hWnd;
    ofn.hInstance           = g_hInst;
    ofn.lpstrFilter         = "Parametersatz (*.ifp)\0*.ifp\0Alle Dateien (*.*)\0*.*\0";
    ofn.lpstrCustomFilter   = (LPTSTR)NULL;
    ofn.nMaxCustFilter      = 0L;
    ofn.nFilterIndex        = 1L;
    ofn.lpstrFile           = szFilename;
    ofn.nMaxFile            = sizeof( szFilename );
    ofn.lpstrFileTitle      = NULL;
    ofn.nMaxFileTitle       = 0;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = "Speichern unter";
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = "*.ifp";
    ofn.lpfnHook            = NULL;
    ofn.Flags = OFN_LONGNAMES|OFN_PATHMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;

    if( !GetSaveFileName( &ofn ) ) {
        return FALSE;
    }

    if( !SaveParameterFile( szFilename, m_pSelSection->GetName() ) )
        return FALSE;

    return TRUE;
}

