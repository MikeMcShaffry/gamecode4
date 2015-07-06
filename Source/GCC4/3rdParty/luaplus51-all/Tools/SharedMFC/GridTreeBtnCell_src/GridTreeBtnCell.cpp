/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#include "stdafx.h"
#include "GridTreeBtnCell.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CGridTreeBtnCell, CGridTreeCell)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridTreeBtnCell::CGridTreeBtnCell()
    : CGridTreeCell()
{
    Reset();
}

CGridTreeBtnCell::~CGridTreeBtnCell()
{
}

void CGridTreeBtnCell::Reset()
{
    CGridTreeCell::Reset();

    m_GridBtnCell.Reset();
}

BOOL CGridTreeBtnCell::Draw(CDC* pDC, int nRow, int nCol, CRect rect,
    BOOL bEraseBkgnd /*=TRUE*/)
{
    CRect RectCell( rect);
    if( CGridTreeCell::DrawTreeCell( pDC, nRow, nCol, &RectCell, bEraseBkgnd) )
    {
        if( m_GridBtnCell.DrawBtnCell( pDC, nRow, nCol, &RectCell, bEraseBkgnd) )
        {

            // call button class to finish it off
            return CGridCellBase::Draw( pDC, nRow, nCol, RectCell, bEraseBkgnd);
        }
    }
    return FALSE;
}

BOOL CGridTreeBtnCell::GetTextRect( LPRECT pRect)  // i/o:  i=dims of cell rect; o=dims of text rect
{
    // move text over to allow for level indentation
    pRect->left += GetTreeIndent();

    return m_GridBtnCell.GetTextRect( pRect);
}

CSize CGridTreeBtnCell::GetCellExtent(CDC* pDC)
{
    CSize sizeBase = m_GridBtnCell.GetCellExtent(pDC);

    // move text over to allow for level indentation
    sizeBase.cx += GetTreeIndent();

    return sizeBase;
}

BOOL CGridTreeBtnCell::Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar)
{
    point.x -= GetTreeIndent(); // adjust point clicked to account for tree
    return m_GridBtnCell.Edit( nRow, nCol, rect, point, nID, nChar);
}

void CGridTreeBtnCell::EndEdit()
{
    m_GridBtnCell.EndEdit();
}

void CGridTreeBtnCell::OnEndEdit()
{
    m_GridBtnCell.OnEndEdit();
}

void CGridTreeBtnCell::OnClick( CPoint PointCellRelative)
{
    PointCellRelative.x -= GetTreeIndent(); // adjust point clicked to account for tree
    m_GridBtnCell.OnClick( PointCellRelative);
}

void CGridTreeBtnCell::OnClickDown( CPoint PointCellRelative)
{
    ASSERT( m_pTreeColumn != NULL);

    if( TreeHitPlusMinus(  PointCellRelative) ) // relative coordinates of mouse click
    {
        // returns:  T=hit a plus or minus;  F=Missed it or tree cell has no plus or minus
        m_pTreeColumn->TreeExpandCollapseToggle( m_iRow); // Grid row of node to toggle
        return;
    }

    PointCellRelative.x -= GetTreeIndent(); // adjust point clicked to account for tree
    m_GridBtnCell.OnClickDown( PointCellRelative);
}

void CGridTreeBtnCell::OnRClick( CPoint PointCellRelative)
{
    PointCellRelative.x -= GetTreeIndent(); // adjust point clicked to account for tree
    m_GridBtnCell.OnRClick( PointCellRelative);
}

void CGridTreeBtnCell::OnDblClick( CPoint PointCellRelative)
{
    ASSERT( m_pTreeColumn != NULL);

    if( GetState() & GVIS_READONLY
        || !m_pTreeColumn->GetGrid()->IsEditable() )
    {
        if( !TreeHitPlusMinus(  PointCellRelative) ) // relative coordinates of mouse click
        {
            // returns:  T=hit a plus or minus;  F=Missed it or tree cell has no plus or minus
            m_pTreeColumn->TreeExpandCollapseToggle( m_iRow); // Grid row of node to toggle
            return;
        }
    }
    PointCellRelative.x -= GetTreeIndent(); // adjust point clicked to account for tree
    m_GridBtnCell.OnDblClick( PointCellRelative);
}

void CGridTreeBtnCell::SetText(LPCTSTR szText)
{
    // copy string to both cell representations
    CGridTreeCell::SetText( szText);
    m_GridBtnCell.SetText( szText);
}
