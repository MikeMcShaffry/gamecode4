/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#include "stdafx.h"
#include "GridBtnCellBase.h"

#include "..\GRIDCTRL_SRC\GridCtrl.h"
#include "..\GRIDCTRL_SRC\InPlaceEdit.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CString CGridBtnCellBase::m_strTipText;

#define MAX_NBR_CTLS_INCELL 16  // maximum number of controls drawn in a cell

#define ALL_BUT_BTN_CHK   (DFCS_BUTTON3STATE | DFCS_BUTTONPUSH | DFCS_BUTTONRADIO | DFCS_BUTTONRADIOIMAGE | DFCS_BUTTONRADIOMASK)
                            // checking for DFCS_BUTTONCHECK
                            //  but it is not a bit, it is 0x0!

#define ALL_BUT_DT_LEFT   (DT_CENTER | DT_RIGHT)
                            // checking for DT_LEFT
                            //  but it is not a bit, it is 0x0!


IMPLEMENT_DYNCREATE(CGridBtnCellBase, CGridCellBase)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridBtnCellBase::CGridBtnCellBase()
{
    m_pBtnDataBase = NULL;
    Reset();
}

CGridBtnCellBase::~CGridBtnCellBase()
{
}

void CGridBtnCellBase::Reset()
{
    m_sNbrDrawCtls = 0;
    m_sLastCtlClicked = -1;
    m_iRow = -1;
    m_iCol = -1;
    m_ucEditing = FALSE;
}

BOOL CGridBtnCellBase::SetupBtns(
                        int aiWhich,        // zero-based index of image to draw
                        UINT auiType,       // type of frame control to draw e.g. DFC_BUTTON
                        UINT auiState,      // like DrawFrameControl()'s nState  e.g. DFCS_BUTTONCHECK
                        CGridBtnCellBase::CTL_ALIGN aAlign,   // horizontal alignment of control image
                        int aiWidth,        // fixed width of control or 0 for size-to-fit
                        BOOL abIsMbrRadioGrp,   // T=btn is member of a radio group
                        const char* apszText)   // Text to insert centered in button; if NULL no text
// returns:  success / failure
{
    if( aiWhich < 0
        || aiWhich >= GetDrawCtlNbrMax() )
    {
        ASSERT( FALSE);
        return FALSE;
    }
    if( aAlign < 0
        || aAlign >= CTL_ALIGN_FENCE)
    {
        ASSERT( FALSE);
        return FALSE;
    }

    if( aAlign == CTL_ALIGN_CENTER
        && aiWhich > 0)
    {
        ASSERT( FALSE); // if centered can have only 1 button
        return FALSE;
    }

    // increase the count of controls, if necessary and possible
    if( aiWhich >= GetDrawCtlNbr() )
    {
        if( aiWhich >= GetDrawCtlNbrMax() )
            return FALSE;   // wanted to add too many images

        ASSERT( aiWhich == GetDrawCtlNbr() );    // if ASSERT, then you are setting
                                                 //  initial image 0, then image 2 -- you've
                                                 //  skipped image 1.  You can't skip an image

        SetDrawCtlNbr( (unsigned char)( aiWhich + 1) );
    }
    SetDrawCtlAlign( aiWhich, aAlign);
    SetDrawCtlWidth( aiWhich, aiWidth);
    SetDrawCtlType( aiWhich, auiType);
    SetDrawCtlState( aiWhich, auiState);
    SetDrawCtlBtnText( aiWhich, apszText);
    SetDrawCtlIsMbrRadioGrp( aiWhich, abIsMbrRadioGrp);

    return TRUE;
}


BOOL CGridBtnCellBase::Draw(CDC* pDC, int nRow, int nCol, CRect rect,
    BOOL bEraseBkgnd /*=TRUE*/)
{
    CRect RectCell( rect);
    if( DrawBtnCell( pDC, nRow, nCol, &RectCell, bEraseBkgnd) )
    {
        // call base class to finish it off
        return CGridCellBase::Draw( pDC, nRow, nCol, RectCell, bEraseBkgnd);
    }
    return FALSE;

}

BOOL CGridBtnCellBase::DrawBtnCell(CDC* pDC, int /* nRow */, int /* nCol */, CRect* prect,
    BOOL /* bEraseBkgnd */)
{
    CGridCtrl* pGrid = GetGrid();

    if (!pGrid || !pDC)
        return FALSE;

    if( prect->Width() <= 0
        || prect->Height() <= 0)  // prevents imagelist item from drawing even
        return FALSE;           //  though cell is hidden

    int nSavedDC = pDC->SaveDC();

    // draw any button images
    ASSERT( MAX_NBR_CTLS_INCELL > GetDrawCtlNbrMax() ); // whoa!
    const int iCtlNbr = GetDrawCtlNbr();
    CRect RectAry[ MAX_NBR_CTLS_INCELL];

    if( iCtlNbr > 0)
    {
        if( GetState() & GVIS_SELECTED
            || GetState() & GVIS_DROPHILITED )
        {
            // draw the rectangle around the grid --
            //  we may be filling cells with controls
            pDC->SelectStockObject(BLACK_PEN);
            pDC->SelectStockObject(NULL_BRUSH);
            pDC->Rectangle(*prect);
        }

        pDC->SetBkMode(TRANSPARENT);
        prect->DeflateRect( GetMargin(), 0);


        CFont* pOldFont = pDC->SelectObject(GetFontObject());

        if( CalcDrawCtlRects(   RectAry,    // returns:  CRects with coordinates
                                            //  last entry has optional leftover rect
                                            //  available for text, etc.
                                MAX_NBR_CTLS_INCELL,// nbr of Rects in above array
                                *prect) )     // cell rectangle to work with
        {
            for( int i1=0; i1 < iCtlNbr; i1++)
            {
                UINT uiType = GetDrawCtlType( i1);
                UINT uiState = GetDrawCtlState( i1);

                pDC->DrawFrameControl(  RectAry[ i1],
                                        uiType,
                                        uiState);

                // if button has text, draw it, too
                LPCTSTR pszBtnText = GetDrawCtlBtnText( i1);
                if( pszBtnText != NULL)
                {
                    COLORREF ColorCurrent = pDC->GetTextColor();

                    if( uiState & DFCS_INACTIVE)
                    {
                        // button is grayed-out

                        // draw the text so that it matches MS's look
                        RectAry[ i1].OffsetRect( 1, 1);
                        pDC->SetTextColor( RGB( 255,255,255) );
                        pDC->DrawText(  pszBtnText,
                                        -1,
                                        RectAry[ i1],
                                        DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

                        RectAry[ i1].OffsetRect( -1, -1);
                        pDC->SetTextColor( ::GetSysColor(COLOR_GRAYTEXT));
                    }
                    else
                    {
                        pDC->SetTextColor( ::GetSysColor(COLOR_BTNTEXT));
                    }

                    pDC->DrawText(  pszBtnText,
                                    -1,
                                    RectAry[ i1],
                                    DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
                    pDC->SetTextColor( ColorCurrent);
                }
            }
            // allowable text area has shrunk up
            *prect = RectAry[ iCtlNbr];
        }

        pDC->SelectObject(pOldFont);

        // maybe there's nothing left to draw
        if( prect->Width() <= 0)
        {
            pDC->RestoreDC(nSavedDC);
            return TRUE;
        }
    }
    pDC->RestoreDC(nSavedDC);
    return TRUE;

}

/*****************************************************************************
3/2001:  Fix:  Pops button back up if mouse clicked on a button, then
moved away from button rectangle before release

*****************************************************************************/
void CGridBtnCellBase::OnClick( CPoint /* PointCellRelative */)
{
    if( m_sLastCtlClicked < 0)
        return;

    ClickedCellCtl( WM_LBUTTONUP,// Command that invoked.  e.g. WM_LBUTTONDOWN
                    m_sLastCtlClicked);    // zero-based index of image to draw
}

/*****************************************************************************
May mean end of a button click sequence or to edit text

*****************************************************************************/
BOOL CGridBtnCellBase::Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar)
{
    ASSERT( m_pBtnDataBase != NULL);

    if( ProcessCtlClick(WM_LBUTTONUP,       // Command that invoked.  e.g. WM_LBUTTONDOWN
                        point) ) // point to check for hit
    {
        SendMessageToParent( m_iRow, m_iCol, NM_CLICK); // tell parent about it
        return FALSE;   // clicked a control rather than edited a value
    }

    const int iCtlNbr = GetDrawCtlNbr();
    if( iCtlNbr > 0 )
    {
        // if there's at least one left-aligned control and user pressed
        //  space bar, activate that control
        if( GetDrawCtlAlign( 0) == CTL_ALIGN_LEFT
            || iCtlNbr == 1)
        {
            if( nChar == ' ' )
            {
                // user hit space bar -- just like clicking a button
                ClickedCellCtl( WM_LBUTTONDOWN,   // Command that invoked.  e.g. WM_LBUTTONDOWN
                                0);    // zero-based index of image to draw
                Sleep( 200);
                ClickedCellCtl( WM_LBUTTONUP,   // Command that invoked.  e.g. WM_LBUTTONDOWN
                                0);    // zero-based index of image to draw

                SendMessageToParent( m_iRow, m_iCol, NM_CLICK); // tell parent about it

                return FALSE;   // clicked a control rather than edited a value
            }
        }
    }

    // if no string text to edit, then typing a character may invoke a button
    if( HasCellText() )
    {
        DWORD dwStyle = ES_LEFT;
        if (GetFormat() & DT_RIGHT)
            dwStyle = ES_RIGHT;
        else if (GetFormat() & DT_CENTER)
            dwStyle = ES_CENTER;

        m_ucEditing = TRUE;

        // InPlaceEdit auto-deletes itself
        CGridCtrl* pGrid = GetGrid();
        CWnd* pWnd = new CInPlaceEdit(pGrid, rect, dwStyle, nID, nRow, nCol, GetText(), nChar);
        m_pBtnDataBase->SetEditWnd( pWnd);
    }
    else
    {
        // since no text to edit, maybe press a button -- check for hot keys
        int iCtl = HotKeyBtnMatch( (char)nChar); // hot key character
        if( iCtl >= 0 )
        {
            // returns:  index of button or -1 if no hot key matches
            ClickedCellCtl( WM_LBUTTONDOWN,   // Command that invoked.  e.g. WM_LBUTTONDOWN
                            iCtl);    // zero-based index of image to draw
            Sleep( 200);
            ClickedCellCtl( WM_LBUTTONUP,   // Command that invoked.  e.g. WM_LBUTTONDOWN
                            iCtl);    // zero-based index of image to draw

            SendMessageToParent( m_iRow, m_iCol, NM_CLICK); // tell parent about it
            return FALSE;   // clicked a control rather than edited a value
        }
    }

    return TRUE;
}

void CGridBtnCellBase::EndEdit()
{
    ASSERT( m_pBtnDataBase != NULL);

    CWnd* pWnd = m_pBtnDataBase->GetEditWnd();

    if (pWnd)
        ((CInPlaceEdit*)pWnd)->EndEdit();
}

void CGridBtnCellBase::OnEndEdit()
{
    ASSERT( m_pBtnDataBase != NULL);

    m_ucEditing = FALSE;
    m_pBtnDataBase->SetEditWnd( NULL);
}



void CGridBtnCellBase::OnClickDown( CPoint PointCellRelative)
{
    ProcessCtlClick(WM_LBUTTONDOWN,     // Command that invoked.  e.g. WM_LBUTTONDOWN
                    PointCellRelative); // point to check for hit
}

void CGridBtnCellBase::OnRClick( CPoint /* PointCellRelative */)
{
}

void CGridBtnCellBase::OnDblClick( CPoint PointCellRelative)
{
    ProcessCtlClick(WM_LBUTTONDBLCLK,   // Command that invoked.  e.g. WM_LBUTTONDOWN
                    PointCellRelative); // point to check for hit
}

BOOL CGridBtnCellBase::GetTextRect( LPRECT pRect)  // i/o:  i=dims of cell rect; o=dims of text rect
{
    CGridCtrl* pGrid = GetGrid();
    ASSERT( pGrid);

    CRect RectCell( *pRect);

    // any button images
    ASSERT( MAX_NBR_CTLS_INCELL > GetDrawCtlNbrMax() ); // whoa!
    CRect RectAry[ MAX_NBR_CTLS_INCELL];

    const int iCtlNbr = GetDrawCtlNbr();
    if( iCtlNbr > 0)
    {
        if( CalcDrawCtlRects(   RectAry,    // returns:  CRects with coordinates
                                            //  last entry has optional leftover rect
                                            //  available for text, etc.
                                MAX_NBR_CTLS_INCELL,// nbr of Rects in above array
                                *pRect) )   // cell rectangle to work with
        {
            // allowable text area has shrunk up
            *pRect = RectAry[ iCtlNbr];
            pRect->left += GetMargin();
            pRect->right -= GetMargin();
        }

    }


    // I've modified the starting point before passing to base
    return CGridCellBase::GetTextRect( pRect);
}

CSize CGridBtnCellBase::GetCellExtent(CDC* pDC)
{
    CSize sizeBase = CGridCellBase::GetCellExtent(pDC);

    // any button images?
    int iCtlNbr = GetDrawCtlNbr();
    if( iCtlNbr > 0)
    {
        int iDefaultWidth = sizeBase.cy;
        int iWidthTotal = 0;
        int iWidth;
        CSize SizeText;

        CFont* pOldFont = pDC->SelectObject(GetFontObject());

        CRect rectText;
        for( int i1=0; i1 < iCtlNbr; i1++)
        {
            iWidth = GetDrawCtlWidth( i1);
            if( iWidth <= 0)
            {
                LPCTSTR pszBtnText = GetDrawCtlBtnText( i1);
                if( pszBtnText == NULL)
                {
                    iWidth = iDefaultWidth;
                }
                else
                {
                    // use width of text in button as a gauge
                    rectText.SetRectEmpty();
                    pDC->DrawText( pszBtnText, _tcslen( pszBtnText), &rectText, GetFormat() | DT_CALCRECT);
                    iWidth = rectText.Width();
                }
            }
            iWidthTotal += iWidth;
        }
        pDC->SelectObject(pOldFont);

        sizeBase += CSize( iWidthTotal + 4*GetMargin(), 0);
    }

    return sizeBase;
}

// Simplify by just using drawing logic for printing, too
BOOL CGridBtnCellBase::PrintCell(CDC* pDC, int nRow, int nCol, CRect rect)
{
    return Draw( pDC, nRow, nCol, rect);
}


//////////////////////////////////////////////////////////

/*****************************************************************************
Called during all the mouse events associated with clicking a control
embedded within a cell.  Override to have more elaborate handling like
implementing radio button logic.

*****************************************************************************/
BOOL CGridBtnCellBase::ClickedCellCtl(  UINT uMsg,      // Command that invoked.  e.g. WM_LBUTTONDOWN
                                        int aiWhich)    // zero-based index of image to draw
// returns:  T=redraw occurred / F=no redraw
{
    if( aiWhich < 0
        || aiWhich >= GetDrawCtlNbrMax() )
    {
        ASSERT( FALSE);
        return FALSE;
    }

    UINT uiState = GetDrawCtlState( aiWhich);
    if( uiState & DFCS_INACTIVE)
        return FALSE;   // button is inactive -- don't do anything

    m_sLastCtlClicked = (short)aiWhich;
    UINT iType = GetDrawCtlType( aiWhich);

    switch( uMsg)
    {
        case WM_LBUTTONDOWN:
            // appears pushed in
            uiState |= DFCS_PUSHED;
            SetDrawCtlState( aiWhich, uiState);
            break;

        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            // appears pushed out
            uiState &= (~DFCS_PUSHED);

            // auto check / uncheck controls, too
            if( iType == DFC_BUTTON )
            {
                BOOL bIsMbrRadioGrp = GetDrawCtlIsMbrRadioGrp( aiWhich);

                if( uiState & DFCS_BUTTONRADIO
                    || bIsMbrRadioGrp )
                {
                    // radio buttons or any button flagged as being part
                    //  of a radio group will be made to look pressed down
                    //  while pushing-up / unchecking all other members
                    //  of the radio group

                    const int iCtlNbr = GetDrawCtlNbr();
                    UINT uiStateRadio;

                    for( int i1=0; i1 < iCtlNbr; i1++)
                    {
                        if( i1 != aiWhich)
                        {
                            uiStateRadio = GetDrawCtlState( i1);
                            bIsMbrRadioGrp = GetDrawCtlIsMbrRadioGrp( i1);

                            if( uiStateRadio & DFCS_BUTTONRADIO
                                || bIsMbrRadioGrp )
                            {
                                uiStateRadio &= (~( DFCS_PUSHED | DFCS_CHECKED)  );
                                                    // push out and uncheck
                                SetDrawCtlState( i1, uiStateRadio);
                            }
                        }
                    }
                    uiState |= DFCS_CHECKED;  // check
                    if( !(uiState & DFCS_BUTTONRADIO) )
                        uiState |= DFCS_PUSHED;  // press in if not real radio button

                }
                else if(  !( uiState & ALL_BUT_BTN_CHK)  )
                {
                    // not a pushbutton -- it's a check box
                    //  (can't check for DFCS_BUTTONCHECK directly since it is bit 0)
                    if( uiState & DFCS_CHECKED)
                        uiState &= (~DFCS_CHECKED); // uncheck
                    else
                        uiState |= DFCS_CHECKED;    // check
                }
            }

            SetDrawCtlState( aiWhich, uiState);
            break;

        default:
            ASSERT( FALSE); // gotta handle new message
            return FALSE;
    }

    CGridCtrl* pGrid = GetGrid();
    ASSERT( pGrid);

    pGrid->RedrawCell( m_iRow, m_iCol);

    return TRUE;
}

/*****************************************************************************
Processes mouse clicks that potentially hit an embedded cell control

*****************************************************************************/
BOOL CGridBtnCellBase::ProcessCtlClick(UINT uMsg,          // Command that invoked.  e.g. WM_LBUTTONDOWN
                                const CPoint& arPoint)  // point to check for hit
// returns:  T=hit a control / F=no control hit
{
    int iCtlHit = RelPointInCtl( arPoint);  // Relative point coords
    // returns:  Index of control that this point is within bounds of or -1 if no control matches

    if( iCtlHit >= 0)
    {
        ClickedCellCtl( uMsg,   // Command that invoked.  e.g. WM_LBUTTONDOWN
                        iCtlHit);    // zero-based index of image to draw
        return TRUE;
    }
    m_sLastCtlClicked = -1;
    return FALSE;
}

int CGridBtnCellBase::RelPointInCtl(    const CPoint& arPoint)  // Relative point coords
// returns:  Index of control that this point is within bounds of or -1 if no control matches
{
    CGridCtrl* pGrid = GetGrid();
    ASSERT( pGrid);

    CRect RectCell;
    if( pGrid->GetCellRect( m_iRow,
                            m_iCol,
                            &RectCell) )
    {
        ASSERT( MAX_NBR_CTLS_INCELL > GetDrawCtlNbrMax() ); // whoa!
        CRect RectAry[ MAX_NBR_CTLS_INCELL];

        if( CalcDrawCtlRects(   RectAry,    // returns:  CRects with coordinates
                                            //  last entry has optional leftover rect
                                            //  available for text, etc.
                                MAX_NBR_CTLS_INCELL,// nbr of Rects in above array
                                RectCell) ) // cell rectangle to work with
        {
            const int iCtlNbr = GetDrawCtlNbr();

            // make point absolute coord
            CPoint pointAbs;
            pointAbs.x = arPoint.x + RectCell.left;
            pointAbs.y = arPoint.y + RectCell.top;

            for( int i1=0; i1 < iCtlNbr; i1++)
            {
                if( pointAbs.x >= RectAry[i1].left
                    && pointAbs.x <= RectAry[i1].right
                    && pointAbs.y >= RectAry[i1].top
                    && pointAbs.y <= RectAry[i1].bottom)
                {
                    return i1;  // found it
                }

            }
        }
    }
    return -1;
}


/*****************************************************************************
Determines bounding rectangle to draw a button in a cell.   Used to draw cells
and for mouse hit testing.

If a fixed-size control can't fit within the cell, the control will shrink in
the required x and / or y dimension.   I do this because I'm not using a
clipping region when I display the cell -- that would be a better solution.

*****************************************************************************/
BOOL CGridBtnCellBase::CalcDrawCtlRects(CRect* apRect,     // returns:  CRects with coordinates
                                                    //  last entry has optional leftover rect
                                                    //  available for text, etc.
                                int aiNbrRectEntries,   // nbr of Rects in above array
                                const CRect& arRectCell)// cell rectangle to work with
// returns:  success / fail
{
    ASSERT( apRect != NULL);

    if( aiNbrRectEntries < GetDrawCtlNbrMax() )
    {
        ASSERT( FALSE); // need to allow for leftover rect
        return FALSE;
    }

    const int iCtlNbr = GetDrawCtlNbr();
    if( iCtlNbr <= 0)
        return FALSE;

    int i1, i2;
    int iSpinBoxDownIdx = -1;       // can have zero or 1 spin box -- no more
                                    //  identifies placment of down arrow of the spin box
    int iWidth = 0;
    CTL_ALIGN CtlAlign;
    UINT uiType;
    UINT uiState;
    CRect* pRectSav = apRect;

    // calculate the width layout of buttons by examining
    //  all of them and noting important info
    int iFixedSum = 0;
    int iSizeToFitCount = 0;
    for( i1=0; i1 < iCtlNbr; i1++)
    {
        CtlAlign = GetDrawCtlAlign( i1);

        if( CtlAlign == CTL_ALIGN_CENTER)
        {
            // forget all calculations if any are centered, all controls
            //  just overwrite each other and expand to fit cell
            for( i2=0; i2 < iCtlNbr; i2++)
            {
                apRect->operator=( arRectCell); // copy initial rectangle
                apRect++;
            }
            apRect->operator=( CRect(0,0,0,0) );    // no text leftover
            return TRUE;
        }

        iWidth = GetDrawCtlWidth( i1);
        if( iWidth > 0)
            iFixedSum += iWidth;
        else
            iSizeToFitCount++;

        // spin box rectangles are stacked on top of each other
        //  thus, avoid doubling spin box width
        if( iSpinBoxDownIdx < 0)
        {
            uiState = GetDrawCtlState( i1);
            if( GetDrawCtlType( i1) == DFC_SCROLL
                && !( uiState & (DFCS_SCROLLCOMBOBOX
                                | DFCS_SCROLLDOWN
                                | DFCS_SCROLLLEFT
                                | DFCS_SCROLLRIGHT
                                | DFCS_SCROLLSIZEGRIP)  )  )  // checking for DFCS_SCROLLUP
                                    // but it is not a bit, it is 0x0!
            {
                if( i1 + 1 < iCtlNbr)
                {
                    // at least 1 more -- see if we got spin box match
                    if( GetDrawCtlType( i1 + 1) == DFC_SCROLL
                        && ( GetDrawCtlState( i1 + 1) & DFCS_SCROLLDOWN)  )
                    {
                        // it's a spin box
                        i1++;   // skip looking at next control
                        iSpinBoxDownIdx = i1;
                    }

                }
            }
        }

    }

    int iSizeToFitWidth = 0;
    int iFitWidthsTotal = arRectCell.Width() - iFixedSum;
    if( iSizeToFitCount > 0)
    {
        iSizeToFitWidth = iFitWidthsTotal / iSizeToFitCount;
    }


    int iSizeToFitCountWrk = iSizeToFitCount;
    int iWidthSoFar = 0;
    for( i1=0; i1 < iCtlNbr; i1++)
    {
        if( iSpinBoxDownIdx == i1)
        {
            // skip down arrow of spin box for calculations
            apRect++;
            continue;
        }

        apRect->operator=( arRectCell); // copy initial rectangle
        apRect->left += iWidthSoFar;
        apRect->right = apRect->left;

        iWidth = GetDrawCtlWidth( i1);
        if( iWidth > 0)
            iWidthSoFar += iWidth; // fixed width
        else
        {
            iSizeToFitCountWrk--;   // found another one

            // may shrink width if control is square -- saves screen real-estate
            uiType = GetDrawCtlType( i1);
            uiState = GetDrawCtlState( i1);
            BOOL bIsRectangle = (   uiType == DFC_BUTTON
                                    && (uiState & DFCS_BUTTONPUSH) == DFCS_BUTTONPUSH);
                                        // all other buttons are square

            if( !bIsRectangle
                && arRectCell.Height() < iSizeToFitWidth)
            {
                // it is square -- make width the cell height
                iWidthSoFar += arRectCell.Height();

                // recalulate size to fit
                iFitWidthsTotal -= arRectCell.Height();

                if( iSizeToFitCountWrk > 0)
                    iSizeToFitWidth = iFitWidthsTotal / iSizeToFitCountWrk;
            }
            else
            {
                iWidthSoFar += iSizeToFitWidth;
                iFitWidthsTotal -= iSizeToFitWidth;
            }
        }

        apRect->right = iWidthSoFar + arRectCell.left;
        apRect++;
    }

    if( iFitWidthsTotal < (int)GetMargin() )
    {
        // no leftover rectangle available
        apRect->SetRectEmpty();
    }
    else
    {
        // calc leftover rectangle
        apRect->operator=( arRectCell);
        apRect->left += iWidthSoFar;
    }

    // I've been assuming that all controls are left-aligned.  Programmer
    //  may have defined controls as right-aligned, too.  Note that left
    //  and right controls can be declared in the array in any order

    // Ok, here's the kludge.  Since I know that the heights of each control
    //  are the same, use these unused height values in the CRect array to
    //  help me calculate the proper order of the widths
    apRect = pRectSav;
    int iSavedTop = arRectCell.left;  // seed first result

    for( i1=0; i1 < iCtlNbr; i1++)
    {
        if( iSpinBoxDownIdx == i1)
        {
            // skip down arrow of spin box for calculations
            apRect++;
            continue;
        }

        // analyzing just Left-aligned controls
        CtlAlign = GetDrawCtlAlign( i1);
        ASSERT( CtlAlign != CTL_ALIGN_CENTER);  // should've taken care of

        if( CtlAlign == CTL_ALIGN_LEFT)
        {
            apRect->top = iSavedTop;
            iSavedTop += apRect->Width();
            apRect->bottom = iSavedTop;
        }
        apRect++;

    }

    // text rectangle appears between left and right aligned controls
    //  and width has been saved in last rectangle
    apRect = pRectSav + iCtlNbr;

    apRect->top = iSavedTop + GetMargin();        // put some margin so doesn't overwrite
    iSavedTop += apRect->Width();
    apRect->bottom = iSavedTop;
    iSavedTop += GetMargin();         // some more margin...

    apRect = pRectSav;
    for( i1=0; i1 < iCtlNbr; i1++)
    {
        if( iSpinBoxDownIdx == i1)
        {
            // skip down arrow of spin box for calculations
            apRect++;
            continue;
        }

        // finally, look at right-aligned controls
        CtlAlign = GetDrawCtlAlign( i1);
        if( CtlAlign == CTL_ALIGN_RIGHT)
        {
            apRect->top = iSavedTop;
            iSavedTop += apRect->Width();
            apRect->bottom = iSavedTop;
        }
        apRect++;
    }


    // flip everything back and we're done
    apRect = pRectSav;
    for( i1=0; i1 <= iCtlNbr; i1++)     // note that I'll get leftover rect, too
    {
        apRect->left = apRect->top;
        if( apRect->left < arRectCell.left )
            apRect->left = arRectCell.left;   // can't go beyond cell
        if( apRect->left > arRectCell.right )
            apRect->left = arRectCell.right;

        apRect->right = apRect->bottom;
        if( apRect->right > arRectCell.right)
            apRect->right = arRectCell.right;
        if( apRect->right < arRectCell.left)
            apRect->right = arRectCell.left;

        apRect->bottom = arRectCell.bottom;
        apRect->top = arRectCell.top;

        apRect++;
    }


    // but wait -- special calculations for the spin box
    if( iSpinBoxDownIdx >= 0)
    {
        CRect* pRectSpinUp = pRectSav + iSpinBoxDownIdx - 1;
        apRect = pRectSav + iSpinBoxDownIdx;

        apRect->left = pRectSpinUp->left;
        apRect->right = pRectSpinUp->right;
        apRect->bottom = pRectSpinUp->bottom;

        int iHalf = apRect->top + ( (apRect->bottom - apRect->top) / 2);
        apRect->top = iHalf;
        pRectSpinUp->bottom = iHalf;
    }

    return TRUE;
}

/*****************************************************************************
Override default title tips so that I'll get a title tip for a cell
filled only with buttons

*****************************************************************************/
LPCTSTR CGridBtnCellBase::GetTipText()
{
    if( HasCellText() )
        return GetText();

    // no string text... maybe there's btns with text
    const int iCtlNbr = GetDrawCtlNbr();
    if( iCtlNbr <= 0)
        return NULL;

    TCHAR szTip[ 256];
    int iNbrChars = 0;

    int i1;
    for( i1=0; i1 < iCtlNbr; i1++)
    {
        // remove any '&' chars
        LPCTSTR pszCtlBtnText = GetDrawCtlBtnText( i1);
        if( pszCtlBtnText != NULL)
        {
            if( i1 > 0)
            {
                szTip[ iNbrChars] = ' ';
                iNbrChars++;
            }

            BOOL bJustGotAmp = FALSE;
            while( iNbrChars < sizeof( szTip) - 2)
            {
                if( *pszCtlBtnText == '\0')
                    break;

                if( *pszCtlBtnText == '&' )
                {
                    if( bJustGotAmp)
                    {
                        bJustGotAmp = FALSE;
                        szTip[ iNbrChars] = *pszCtlBtnText;
                        iNbrChars++;
                    }
                    else
                        bJustGotAmp = TRUE;
                }
                else
                {
                    bJustGotAmp = FALSE;
                    szTip[ iNbrChars] = *pszCtlBtnText;
                    iNbrChars++;
                }
                pszCtlBtnText++;
            }
        }
    }
    szTip[ iNbrChars] = '\0';
    m_strTipText = szTip;

    return m_strTipText;
}

/*****************************************************************************
If no text in the cell, add up all the button text together and use that
as the title tip text

*****************************************************************************/
BOOL CGridBtnCellBase::GetTipTextRect( LPRECT pRect)  // i/o:  i=dims of cell rect; o=dims of text rect
{
    if( HasCellText() )
        return GetTextRect( pRect); // if any text, just use default

    // no string text... maybe there's btns with text
    const int iCtlNbr = GetDrawCtlNbr();
    if( iCtlNbr <= 0)
    {
        pRect->left = 0;
        pRect->top = 0;
        pRect->right = 0;
        pRect->bottom = 0;
    }

    // else, don't modify rect -- rect is full cell size
    return TRUE;
}

BOOL CGridBtnCellBase::HasCellText()
// returns:  F=auto-size buttons, only
{
    CGridCtrl* pGrid = GetGrid();
    ASSERT( pGrid);

    CRect RectCell;
    if( !pGrid->GetCellRect(m_iRow,
                            m_iCol,
                            &RectCell) )
        return FALSE;

    // rather than see if there is text assigned, check if any
    //  space allocated for text
    CRect RectText( RectCell);
    if( !GetTextRect( &RectText) ) // i/o:  i=dims of cell rect; o=dims of text rect
        return FALSE;

    if( RectText.Width() > 0 )
        return TRUE;

    return FALSE;
}

int CGridBtnCellBase::HotKeyBtnMatch( char acHotKey) // hot key character
// returns:  index of button or -1 if no hot key matches
{
    LPCTSTR pszWork;

    const int iCtlNbr = GetDrawCtlNbr();
    for( int i1=0; i1 < iCtlNbr; i1++)
    {
        pszWork = GetDrawCtlBtnText( i1);
        while( *pszWork != NULL)
        {
            if( *pszWork == '&')
            {
                pszWork++;
                if( *pszWork == NULL)
                    break;

                if( tolower(*pszWork) == tolower( acHotKey) )
                    return i1;
            }

            pszWork++;
        }
    }
    return -1;
}
