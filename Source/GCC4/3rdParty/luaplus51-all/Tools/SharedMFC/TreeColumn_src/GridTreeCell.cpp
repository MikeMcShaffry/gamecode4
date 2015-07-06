/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#include "stdafx.h"
#include "GridTreeCell.h"

#include "..\GRIDCTRL_SRC\GridCtrl.h"
#include "..\GRIDCTRL_SRC\InPlaceEdit.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CGridTreeCell, CGridTreeCellBase)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridTreeCell::CGridTreeCell()
    : CGridTreeCellBase()
{
}

CGridTreeCell::~CGridTreeCell()
{
}

void CGridTreeCell::Reset()
{
    m_strText.Empty();
}

BOOL CGridTreeCell::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
    ASSERT( m_pTreeColumn != NULL);
    DWORD dwStyle = ES_LEFT;
    if (GetFormat() & DT_RIGHT)
        dwStyle = ES_RIGHT;
    else if (GetFormat() & DT_CENTER)
        dwStyle = ES_CENTER;

    m_ucEditing = TRUE;

    // InPlaceEdit auto-deletes itself
    CGridCtrl* pGrid = GetGrid();
    m_pTreeColumn->SetEditWnd( new CInPlaceEdit(pGrid, rect, dwStyle, nID, nRow, nCol, GetText(), nChar) );

    return TRUE;
}

void CGridTreeCell::EndEdit()
{
    CWnd* pEditWnd = m_pTreeColumn->GetEditWnd();
    if (pEditWnd)
        ((CInPlaceEdit*)(pEditWnd))->EndEdit();

    m_pTreeColumn->SetEditWnd( NULL);
}

void CGridTreeCell::OnEndEdit()
{
    EndEdit();
    m_ucEditing = FALSE;
}

