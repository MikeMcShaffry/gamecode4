/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#include "stdafx.h"
#include "GridTreeCellBase.h"

#include "..\GRIDCTRL_SRC\GridCtrl.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define TREE_BOX_MARGIN     2   // for drawing "+" or "-" box associated with a Tree


IMPLEMENT_DYNCREATE(CGridTreeCellBase, CGridCellBase)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridTreeCellBase::CGridTreeCellBase()
{
    m_pTreeColumn = NULL;
	m_forcePlus = false;
    Reset();
}

CGridTreeCellBase::~CGridTreeCellBase()
{
}

void CGridTreeCellBase::Reset()
{
    m_iRow    = -1;
    m_iCol    = -1;
    m_nState  = 0;
    m_sImage  = -1;
    m_ucEditing = FALSE;
    m_ucLevelAndHide = 0x80;
}

BOOL CGridTreeCellBase::Draw(CDC* pDC, int nRow, int nCol, CRect rect,
    BOOL bEraseBkgnd /*=TRUE*/)
{
    CRect RectCell( rect);
    if( DrawTreeCell( pDC, nRow, nCol, &RectCell, bEraseBkgnd) )
    {
        // call base class to finish it off
        return CGridCellBase::Draw( pDC, nRow, nCol, RectCell, bEraseBkgnd);
    }
    return FALSE;
}

/*****************************************************************************
Allows Tree + Btn object to call drawing member

*****************************************************************************/
BOOL CGridTreeCellBase::DrawTreeCell(CDC* pDC, int nRow, int nCol, CRect* prect,
    BOOL /* bEraseBkgnd */)
{
    ASSERT( m_pTreeColumn != NULL);

    if( !m_pTreeColumn->GetAllowDraw())
        return FALSE;

    CGridCtrl* pGrid = GetGrid();

    if (!pGrid || !pDC)
        return FALSE;

    if( m_pTreeColumn->GetFixedRowCount() != pGrid->GetFixedRowCount()
        || m_pTreeColumn->GetRowCount() > pGrid->GetRowCount() )
    {
        ASSERT( FALSE); // if ASSERT here, this means that you a tree in a column
                        //  but you called CGridCtrl::SetFixedRowCount() or
                        //  or CGridCtrl::SetRowCount() directly.  You can't do this.
                        //  To change the layout of rows when you are using the tree,
                        //  you must call CTreeColumn::Setup().
        return FALSE;
    }

    if( prect->Width() <= 0
        || prect->Height() <= 0)  // prevents imagelist item from drawing even
        return FALSE;           //  though cell is hidden

    // tree drawing graphic logic begins here

    int nSavedDC = pDC->SaveDC();

    if( nCol == m_pTreeColumn->GetColumnWithTree())
    {
        // this column has a tree

        // move text over to allow for level indentation
        if( IsViewable() )
        {
            // if row is showing, draw the tree graphic
            if( m_pTreeColumn->GetTreeLines())
            {
                TreeDrawGraphic(    pDC,      // current CDC
                                    *prect);    // coordinates of bounding cell rectangle
            }
            else if( m_pTreeColumn->GetTreeUsesImages())
            {
                TREE_IMAGE TreeImage = TREE_IMAGE_DOCUMENT;

                // is it not the very last row?
                if( nRow + m_pTreeColumn->GetFixedRowCount() < m_pTreeColumn->GetRowCount())
                {
                    // is it a plus or minus?
                    BOOL bIsPlus;
                    BOOL bIsMinus;
                    BOOL bIsLastLeaf;

                    if( m_pTreeColumn->TreeCellHasPlusMinus(   nRow,        // row of Cell to check
                                                &bIsPlus,    // returns:  T=Is a plus
                                                &bIsMinus,   // returns:  T=Is a minus
                                                &bIsLastLeaf) )// returns:  T=Is Last Leaf
                    {
                        // returns:  T=cell has a plus or minus;  F=not
                        if( bIsPlus)
                            TreeImage = TREE_IMAGE_FOLDER_CLOSED;
                        else
                            TreeImage = TREE_IMAGE_FOLDER_OPEN;
                    }
                }
                SetImage( TreeImage);
            }
        }

        prect->left += GetTreeIndent();

    }
    pDC->RestoreDC(nSavedDC);
    return TRUE;
}

void CGridTreeCellBase::OnClick( CPoint /* PointCellRelative */)
{
}

/*****************************************************************************
Put click exapansion on down rather than up because OnClick() will not
be called if a TitleTip is showing

*****************************************************************************/
void CGridTreeCellBase::OnClickDown( CPoint PointCellRelative)
{
    ASSERT( m_pTreeColumn != NULL);

    if( TreeHitPlusMinus(  PointCellRelative) ) // relative coordinates of mouse click
    {
        // returns:  T=hit a plus or minus;  F=Missed it or tree cell has no plus or minus
        m_pTreeColumn->TreeExpandCollapseToggle( m_iRow); // Grid row of node to toggle
    }

}

void CGridTreeCellBase::OnRClick( CPoint /* PointCellRelative */)
{
}

void CGridTreeCellBase::OnDblClick( CPoint PointCellRelative)
{
    ASSERT( m_pTreeColumn != NULL);

    if( GetState() & GVIS_READONLY
        || !m_pTreeColumn->GetGrid()->IsEditable() )
    {
        if( TreeHitPlusMinus(  PointCellRelative) ) // relative coordinates of mouse click
        {
            // returns:  T=hit a plus or minus;  F=Missed it or tree cell has no plus or minus
            m_pTreeColumn->TreeExpandCollapseToggle( m_iRow); // Grid row of node to toggle
        }
    }

}

BOOL CGridTreeCellBase::GetTextRect( LPRECT pRect)  // i/o:  i=dims of cell rect; o=dims of text rect
{
    // move text over to allow for level indentation
    pRect->left += GetTreeIndent();

    return CGridCellBase::GetTextRect( pRect);
}

CSize CGridTreeCellBase::GetCellExtent(CDC* pDC)
{
    CSize sizeBase = CGridCellBase::GetCellExtent(pDC);

    // move text over to allow for level indentation
    sizeBase.cx += GetTreeIndent();

    return sizeBase;
}

// Simplify by just using drawing logic for printing, too
BOOL CGridTreeCellBase::PrintCell(CDC* pDC, int nRow, int nCol, CRect rect)
{
    return Draw( pDC, nRow, nCol, rect);
}

/*****************************************************************************
For mouse hit test want to know if user clicked on the "+" / "-" box. Also
for drawing the box

*****************************************************************************/
void CGridTreeCellBase::TreeGetBoxRelCoords(CRect* apRect)      // returns: relative coordinates
{
    ASSERT( apRect != NULL);
    ASSERT( m_pTreeColumn != NULL);

    int iMargin = GetMargin();
    int iDefTreeIndent = m_pTreeColumn->GetDefTreeIndent();

    unsigned char ucLevel = GetLevel();
    if( ucLevel < 1)
        return;

    apRect->left = iMargin + (iDefTreeIndent * ( ucLevel - 1) ) + TREE_BOX_MARGIN;
    apRect->right = apRect->left + iDefTreeIndent - TREE_BOX_MARGIN;
    apRect->top = iMargin;
    apRect->bottom = apRect->top + iDefTreeIndent - (2*TREE_BOX_MARGIN);

}

int CGridTreeCellBase::GetTreeIndent()
// returns:  device units to indent within a cell for a tree at this level
{
    ASSERT( m_pTreeColumn != NULL);
    CGridCtrl* pGridCtrl = GetGrid();
    ASSERT( pGridCtrl != NULL);
    unsigned char ucLevel = GetLevel();

    if( ucLevel == 0)
        return 0;

    if( !m_pTreeColumn->GetTreeLines() )
        ucLevel--;

    return (m_pTreeColumn->GetDefTreeIndent() * ucLevel) + (pGridCtrl->GetDefCellMargin() * 2);
}

/*****************************************************************************
When user single clicked on a tree cell, did he happen to click on "+" or "-"
graphic in the tree?

*****************************************************************************/
BOOL CGridTreeCellBase::TreeHitPlusMinus(  CPoint aPointMouse) // relative coordinates of mouse click
// returns:  T=hit a plus or minus;  F=Missed it or tree cell has no plus or minus
{
    ASSERT( m_pTreeColumn != NULL);
    ASSERT( m_iRow >= 0);

    if( !m_pTreeColumn->GetTreeLines() )
        return FALSE;   // can't hit the plus / minus if it's not showing

    BOOL bIsPlus;
    BOOL bIsMinus;
    BOOL bIsLastLeaf;

    if( !m_pTreeColumn->TreeCellHasPlusMinus(  m_iRow,   // row of Cell to check
                                &bIsPlus,    // returns:  T=Is a plus
                                &bIsMinus,   // returns:  T=Is a minus
                                &bIsLastLeaf) )// returns:  T=Is Last Leaf
    {
        return FALSE;
    }

    CRect RectBox;
    TreeGetBoxRelCoords( &RectBox);      // returns: relative coordinates

    // allow a fudge factor so that user doesn't have to be dead-on with his mouse click
    RectBox.right += TREE_BOX_MARGIN;
    RectBox.top -= TREE_BOX_MARGIN;

    if( aPointMouse.x >= RectBox.left
        && aPointMouse.x <= RectBox.right
        && aPointMouse.y >= RectBox.top
        && aPointMouse.y <= RectBox.bottom)
        return TRUE;    // we're in the box

    return FALSE;
}

/*****************************************************************************
Draws "+", "-" and lines that show a tree graphic

*****************************************************************************/
void CGridTreeCellBase::TreeDrawGraphic(CDC* apDC,      // current CDC
                                    CRect aRect)    // coordinates of bounding cell rectangle
{
    ASSERT( apDC != NULL);
    ASSERT( m_pTreeColumn != NULL);
    ASSERT( m_pTreeColumn->GetAllowDraw());

    int iNbrTreeElements = m_pTreeColumn->GetRowCount() - m_pTreeColumn->GetFixedRowCount();

    // get current level
    unsigned char ucLevelCurrent = GetLevel();
    ASSERT( IsViewable() ); // can't deal with hidden rows!
    if( ucLevelCurrent <= 0)
    {
        if( m_pTreeColumn->GetTreeUsesImages())
            SetImage( TREE_IMAGE_DOCUMENT);
        return;     // no + / - box if level 0
    }

    BOOL bIsNextShowing;
    unsigned char ucLevelNext;
    int iLineStop;


    if( m_pTreeColumn->GetDefTreeIndent() < TREE_BOX_MARGIN * 3)
        return; // too small to draw the graphic

    CRect RectRel;
    TreeGetBoxRelCoords( &RectRel);      // returns: relative coordinates
    CRect RectBox( RectRel);
    RectBox.OffsetRect( aRect.left, aRect.top);

    int iXCenter = RectBox.left + ( (RectBox.right - RectBox.left) / 2);
    int iYCenter = RectBox.top + ( (RectBox.bottom - RectBox.top) / 2);

    CPen psPen(PS_SOLID, 1, m_pTreeColumn->GetTreeLineColor() );
    CPen* pOldPen = apDC->SelectObject(&psPen);

    TREE_IMAGE TreeImage = TREE_IMAGE_DOCUMENT;

    // is it the very last row?
    if( !GetForcePlus()  &&  m_iRow >= m_pTreeColumn->GetRowCount() - 1)
    {
        iXCenter -= m_pTreeColumn->GetDefTreeIndent();

        // draw |__ (with long horizontal)
        if( TryMoveTo( aRect, apDC, iXCenter, aRect.top) )
            if( TryLineTo( aRect, apDC, iXCenter, iYCenter) )
                TryLineTo( aRect, apDC, RectBox.right, iYCenter);

        apDC->SelectObject(pOldPen);
        if( m_pTreeColumn->GetTreeUsesImages())
            SetImage( TreeImage);
        return;
    }

    // is it a plus or minus?  Check up to all remaining entries for an answer
    BOOL bIsPlus;
    BOOL bIsMinus;
    BOOL bIsLastLeaf;
    CGridCtrl* pGrid = GetGrid();
    ASSERT( pGrid != NULL);

    int iVertLineBottom = aRect.bottom + 1; // if drawing a horizontal line,
                                            //  overwrite the grid line in case it is
                                            //  not displayed

    BOOL bIsBranch = m_pTreeColumn->TreeCellHasPlusMinus(
                                m_iRow,          // row of Cell to check
                                &bIsPlus,    // returns:  T=Is a plus
                                &bIsMinus,   // returns:  T=Is a minus
                                &bIsLastLeaf);// returns:  T=Is Last Leaf

    if( bIsBranch )
    {
        // returns:  T=cell has a plus or minus;  F=not

        if( bIsPlus)
            TreeImage = TREE_IMAGE_FOLDER_CLOSED;
        else
            TreeImage = TREE_IMAGE_FOLDER_OPEN;

        // draw a square box
        BOOL bCanDrawRectangle =    aRect.PtInRect( CPoint( RectBox.left, RectBox.top ))
                                    && aRect.PtInRect( CPoint( RectBox.right, RectBox.bottom ));

        if( bCanDrawRectangle )
        {
            apDC->Rectangle(    RectBox.left,
                                RectBox.top,
                                RectBox.right,
                                RectBox.bottom);
        }

        // draw a minus sign
        if( TryMoveTo( aRect, apDC, RectBox.left + TREE_BOX_MARGIN, iYCenter) )
            TryLineTo( aRect, apDC, RectBox.right - TREE_BOX_MARGIN, iYCenter);

        // draw small horizontal tick just to the left of the box if not
        //  level 1
        if( ucLevelCurrent > 1)
        {
            // draw -
            if( TryMoveTo( aRect, apDC, iXCenter - m_pTreeColumn->GetDefTreeIndent(), iYCenter) )
                TryLineTo( aRect, apDC, RectBox.left, iYCenter);
        }

        if( bIsPlus)
        {
            // ... make it into a plus sign
            if( TryMoveTo( aRect, apDC, iXCenter, RectBox.top + TREE_BOX_MARGIN) )
                TryLineTo( aRect, apDC, iXCenter, RectBox.bottom - TREE_BOX_MARGIN);
        }
        else
        {
            // it's a minus sign, so draw vertical tick below box center

            // draw |
            if( TryMoveTo( aRect, apDC, iXCenter, RectBox.bottom) )
                TryLineTo( aRect, apDC, iXCenter, iVertLineBottom);
        }

    }
    else if( ucLevelCurrent > 1)
    {
        // it's not a box, it's a leaf of the tree.  Just draw a horizontal line

        CGridTreeCellBase* pGridTreeCellBase = (CGridTreeCellBase*)pGrid->GetCell( m_iRow + 1, m_pTreeColumn->GetColumnWithTree());
        if( pGridTreeCellBase == NULL)
            return;

        bIsNextShowing = pGridTreeCellBase->IsViewable();
        ucLevelNext = pGridTreeCellBase->GetLevel();


        if( ucLevelCurrent > ucLevelNext)
        {
            bIsLastLeaf = TRUE;
        }

        // draw -
        if( TryMoveTo( aRect, apDC, iXCenter - m_pTreeColumn->GetDefTreeIndent(), iYCenter) )
            TryLineTo( aRect, apDC, iXCenter, iYCenter);
    }

    // draw nearest to text vertical lines that appear to the left of the box for all
    //  levels except level 1
    if( ucLevelCurrent > 1)
    {
        iXCenter -= m_pTreeColumn->GetDefTreeIndent();
        if( bIsLastLeaf)
            iLineStop = iYCenter;
        else
            iLineStop = iVertLineBottom;

        // draw |
        if( TryMoveTo( aRect, apDC, iXCenter, aRect.top) )
            TryLineTo( aRect, apDC, iXCenter, iLineStop);
    }

    BOOL bFoundBranch;
    unsigned char ucLevelToCheck = ucLevelCurrent;

    // draw vertical lines that appear to the left of the box for all appropriate
    //  levels except level 1
    for( int i1=ucLevelCurrent - 1; i1 > 1; i1--)
    {
        bFoundBranch = FALSE;

        int iStartPt = m_iRow + 1;

        for( int i2=iStartPt; i2 <= iNbrTreeElements; i2++)
        {
            CGridTreeCellBase* pGridTreeCellBase = (CGridTreeCellBase*)pGrid->GetCell(
                                                i2,
                                                m_pTreeColumn->GetColumnWithTree());
            if( pGridTreeCellBase == NULL)
                return;

            ucLevelNext = pGridTreeCellBase->GetLevel();

            if( ucLevelNext + 1 < ucLevelToCheck)
                break;

            if( ucLevelNext + 1 == ucLevelToCheck)
            {
                bIsNextShowing = pGridTreeCellBase->IsViewable();
                if( bIsNextShowing)
                    bFoundBranch = TRUE;
                break;
            }

        }

        iXCenter -= m_pTreeColumn->GetDefTreeIndent();

        if( bFoundBranch)
        {
            // draw |
            if( TryMoveTo( aRect, apDC, iXCenter, aRect.top) )
                TryLineTo( aRect, apDC, iXCenter, iVertLineBottom);
        }
        ucLevelToCheck--;   // each loop heads to level 1
    }

    // cleanup
    apDC->SelectObject(pOldPen);

    if( m_pTreeColumn->GetTreeUsesImages())
        SetImage( TreeImage);

}
