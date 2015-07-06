/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_GRIDTREEBTNCELL_H__00C20285_F376_11D3_B761_00C04F6A7AE6__INCLUDED_)
#define AFX_GRIDTREEBTNCELL_H__00C20285_F376_11D3_B761_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\TREECOLUMN_SRC\GridTreeCell.h"
#include "..\GridBtnCell_src\GridBtnCell.h"

class CGridTreeBtnCell : public CGridTreeCell
{
    DECLARE_DYNCREATE(CGridTreeBtnCell)
public:
    CGridTreeBtnCell();
    virtual ~CGridTreeBtnCell();

// base Attributes
public:
    virtual void Reset();

    virtual void operator=(CGridCellBase& cell)
    {
        CGridTreeCellBase::operator=( cell);
    }

    virtual void operator=(CGridTreeCell& cell)
    {
        SetTreeColumn(cell.GetTreeColumn());    // set before copy operation or
                                                //  else font copy won't happen

        CGridTreeCellBase::operator=( (CGridCellBase&)cell);
        m_GridBtnCell = (CGridCellBase&)cell;

        CPoint pointCoords = cell.GetCoords();
        SetCoords( pointCoords.y, pointCoords.x);
        m_GridBtnCell.SetCoords( pointCoords.y, pointCoords.x);

        SetLevelAndHide( cell.GetLevelAndHide());
    }

    virtual void operator=(CGridTreeBtnCell& cell)
    {
        SetTreeColumn(cell.GetTreeColumn());    // set before copy operation or
                                                //  else font copy won't happen

        CGridTreeCellBase::operator=( (CGridCellBase&)cell);
        m_GridBtnCell = (CGridCellBase&)cell;

        CPoint pointCoords = cell.GetCoords();
        SetCoords( pointCoords.y, pointCoords.x);
        m_GridBtnCell.SetCoords( pointCoords.y, pointCoords.x);

        SetLevelAndHide( cell.GetLevelAndHide());
        SetBtnDataBase( cell.GetBtnDataBase());
    }

// base attributes
    virtual void SetText(LPCTSTR szText);

// editing cells
public:
    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);
    virtual BOOL GetTextRect( LPRECT pRect);    // i/o:  i=dims of cell rect; o=dims of text rect
    virtual CSize GetCellExtent(CDC* pDC);

// editing cells
public:
    BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
    virtual void EndEdit();
protected:
    virtual void OnEndEdit();

protected:
    virtual void OnClick( CPoint PointCellRelative);
    virtual void OnClickDown( CPoint PointCellRelative);
    virtual void OnRClick( CPoint PointCellRelative);
    virtual void OnDblClick( CPoint PointCellRelative);

/*****************************************************************************
"Button" inteface begins here
*****************************************************************************/
public:
    void SetBtnDataBase( CBtnDataBase* pBtnDataBase)
        { m_GridBtnCell.SetBtnDataBase( pBtnDataBase); }
    CBtnDataBase* GetBtnDataBase()
        { return m_GridBtnCell.GetBtnDataBase(); }

    BOOL SetupBtns(  int aiWhich,        // zero-based index of image to draw
                     UINT auiType,       // type of frame control to draw e.g. DFC_BUTTON
                     UINT auiState,      // like DrawFrameControl()'s nState  e.g. DFCS_BUTTONCHECK
                     CGridBtnCellBase::CTL_ALIGN aAlign,   // horizontal alignment of control image
                     int aiWidth,        // fixed width of control or 0 for size-to-fit
                     BOOL abIsMbrRadioGrp,   // T=btn is member of a radio group
                     const char* apszText)   // Text to insert centered in button; if NULL no text
    // returns:  success / failure
    {
        return m_GridBtnCell.SetupBtns(
                                    aiWhich,
                                    auiType,
                                    auiState,
                                    aAlign,
                                    aiWidth,
                                    abIsMbrRadioGrp,
                                    apszText);
    }


// for drawing things like buttons and check marks within a cell
//  you can have more than one control per cell
public:
    virtual short GetDrawCtlNbr()
        { return m_GridBtnCell.GetDrawCtlNbr(); }

    virtual short GetLastCtlClicked()
        { return m_GridBtnCell.GetLastCtlClicked(); }

    virtual unsigned char GetDrawCtlNbrMax()
        { return m_GridBtnCell.GetDrawCtlNbrMax(); }

    virtual UINT GetDrawCtlType( int aiWhich)
        { return m_GridBtnCell.GetDrawCtlType( aiWhich); }

    virtual void SetDrawCtlType( int aiWhich, UINT auiType)
        { m_GridBtnCell.SetDrawCtlType( aiWhich, auiType); }

    virtual UINT GetDrawCtlState( int aiWhich)
        { return m_GridBtnCell.GetDrawCtlState( aiWhich); }

    virtual void SetDrawCtlState( int aiWhich, UINT auiState)
        { m_GridBtnCell.SetDrawCtlState( aiWhich, auiState); }

    virtual int GetDrawCtlWidth( int aiWhich)
        { return m_GridBtnCell.GetDrawCtlWidth( aiWhich); }

    virtual void SetDrawCtlWidth( int aiWhich, int aiWidth)
        { m_GridBtnCell.SetDrawCtlWidth( aiWhich, aiWidth); }

    virtual CGridBtnCellBase::CTL_ALIGN GetDrawCtlAlign( int aiWhich)
        { return m_GridBtnCell.GetDrawCtlAlign( aiWhich); }

    virtual void SetDrawCtlAlign( int aiWhich, CGridBtnCellBase::CTL_ALIGN aucAlign)
        { m_GridBtnCell.SetDrawCtlAlign( aiWhich, aucAlign); }

    virtual LPCTSTR GetDrawCtlBtnText( int aiWhich)
        { return m_GridBtnCell.GetDrawCtlBtnText( aiWhich); }

    virtual void SetDrawCtlBtnText( int aiWhich, const char* apszText)
        { m_GridBtnCell.SetDrawCtlBtnText( aiWhich, apszText); }

    virtual BOOL GetDrawCtlIsMbrRadioGrp( int aiWhich)
        { return m_GridBtnCell.GetDrawCtlIsMbrRadioGrp( aiWhich); }

    virtual void SetDrawCtlIsMbrRadioGrp( int aiWhich, BOOL abOn)
        { m_GridBtnCell.SetDrawCtlIsMbrRadioGrp( aiWhich, abOn); }

/*****************************************************************************
"Button" inteface ends here
*****************************************************************************/


protected:
    CGridBtnCell m_GridBtnCell; // provides button capabilities
};

#endif // !defined(AFX_GRIDTREEBTNCELL_H__00C20285_F376_11D3_B761_00C04F6A7AE6__INCLUDED_)
