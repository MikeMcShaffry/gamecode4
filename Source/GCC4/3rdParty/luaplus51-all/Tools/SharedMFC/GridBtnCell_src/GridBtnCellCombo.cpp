/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#include "stdafx.h"
#include "GridBtnCellCombo.h"

#include "InPlaceList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CGridBtnCellCombo, CGridBtnCell)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridBtnCellCombo::CGridBtnCellCombo()
    : CGridBtnCell()
{

}

CGridBtnCellCombo::~CGridBtnCellCombo()
{

}

void CGridBtnCellCombo::Reset()
{
    CGridBtnCell::Reset();

    m_dwComboStyle = CBS_SIMPLE;
    m_StringArrayCombo.RemoveAll();
}

void CGridBtnCellCombo::OnClick( CPoint PointCellRelative)
{
    // immediately edit if user clicked on scroll down button picture

    int iCtlHit = RelPointInCtl( PointCellRelative);  // Relative point coords
    // returns:  Index of control that this point is within bounds of or -1 if no control matches

    BOOL bHitScrollDown = FALSE;
    if( iCtlHit >= 0)
    {
        // if user clicked on scroll down button picture, then show
        //  the drop-down, too

        UINT uiType = GetDrawCtlType( iCtlHit);
        UINT uiState = GetDrawCtlState( iCtlHit);

        bHitScrollDown =    ( uiType == DFC_SCROLL)
                            && uiState & DFCS_SCROLLDOWN;
    }
    if( bHitScrollDown)
    {
        // invoke the edit, now -- similar to CGridCtl::OnEditCell() logic
        CGridCtrl* pGrid = GetGrid();
        ASSERT( pGrid != NULL);

        CRect rect;
        if (!pGrid->GetCellRect( m_iRow, m_iCol, rect))
            return;

        SendMessageToParent(m_iRow, m_iCol, GVN_BEGINLABELEDIT);

        Edit( m_iRow, m_iCol, rect, PointCellRelative, IDC_INPLACE_CONTROL, VK_LBUTTON);
        return;
    }

    CGridBtnCell::OnClick( PointCellRelative);
}

/*****************************************************************************
May mean end of a button click sequence or to edit text

*****************************************************************************/
BOOL CGridBtnCellCombo::Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar)
{
    int iCtlHit = RelPointInCtl( point);  // Relative point coords
    // returns:  Index of control that this point is within bounds of or -1 if no control matches

    BOOL bShowDropDownNow = FALSE;
    if( iCtlHit >= 0)
    {
        // if user clicked on scroll down button picture, then show
        //  the drop-down, too

        UINT uiType = GetDrawCtlType( iCtlHit);
        UINT uiState = GetDrawCtlState( iCtlHit);

        bShowDropDownNow =  ( uiType == DFC_SCROLL)
                            && uiState & DFCS_SCROLLDOWN;
    }
    BOOL bHitNonComboButton =   ( iCtlHit >= 0)
                                && !bShowDropDownNow;

    if( HasCellText()
        && !bHitNonComboButton )
    {
        m_ucEditing = TRUE;

        // InPlaceList auto-deletes itself
        CGridCtrl* pGrid = GetGrid();
        CInPlaceList* pInPlaceList = new CInPlaceList(  pGrid, rect, m_dwComboStyle, nID, nRow, nCol,
                                                        m_StringArrayCombo, GetText(), nChar);

        if( bShowDropDownNow)
        {
            if( m_dwComboStyle & CBS_DROPDOWN
                || m_dwComboStyle & CBS_DROPDOWNLIST)
            {
                pInPlaceList->ShowDropDown( TRUE);
            }
        }

        m_pBtnDataBase->SetEditWnd( pInPlaceList);
        return TRUE;
    }
    // call base class, otherwise
    return CGridBtnCell::Edit( nRow, nCol, rect, point, nID, nChar);
}

void CGridBtnCellCombo::EndEdit()
{
    ASSERT( m_pBtnDataBase != NULL);

    CWnd* pWnd = m_pBtnDataBase->GetEditWnd();

    if (pWnd)
        ((CInPlaceList*)pWnd)->EndEdit();

}

void CGridBtnCellCombo::OnEndEdit()
{
    ASSERT( m_pBtnDataBase != NULL);

    m_ucEditing = FALSE;
    m_pBtnDataBase->SetEditWnd( NULL);

    // make sure that all pushbuttons appear in "up" state
    const int iCtlNbr = GetDrawCtlNbr();
    for( int i1=0; i1 < iCtlNbr; i1++)
    {
        UINT uiType = GetDrawCtlType( i1);
        UINT uiState = GetDrawCtlState( i1);

        BOOL bIsScrollDown =    ( uiType == DFC_SCROLL)
                                && uiState & DFCS_SCROLLDOWN;
        BOOL bIsPushButton =    ( uiType == DFC_BUTTON)
                                && uiState & DFCS_BUTTONPUSH;

        if( bIsScrollDown
            || bIsPushButton)
        {
            ClickedCellCtl( WM_LBUTTONUP,   // Command that invoked.  e.g. WM_LBUTTONDOWN
                            i1);    // zero-based index of image to draw
        }
    }


}

