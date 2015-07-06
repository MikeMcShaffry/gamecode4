/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_GridTreeCellBase_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_)
#define AFX_GridTreeCellBase_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\GRIDCTRL_SRC\GridCell.h"
#include "TreeColumn.h"

// Place these Grid Cells in a column in a grid to
//  create a tree control
class CGridTreeCellBase : public CGridCellBase
{
    DECLARE_DYNCREATE(CGridTreeCellBase)

// Construction/Destruction
public:
    CGridTreeCellBase();
    virtual ~CGridTreeCellBase();

    // initial setup for each cell
    void SetTreeColumn( CTreeColumn* pTreeColumn)
        { ASSERT( pTreeColumn != NULL); m_pTreeColumn = pTreeColumn; }
    CTreeColumn* GetTreeColumn() { return m_pTreeColumn; }

    void SetLevel( UCHAR ucLevel)
        { ASSERT( ucLevel < 0x7F); m_ucLevelAndHide &= 0x80; m_ucLevelAndHide |= ucLevel; }
    UCHAR GetLevel() { return UCHAR( m_ucLevelAndHide & 0x7F ); }

    void SetViewable( BOOL bViewable)
        { bViewable ? m_ucLevelAndHide |= 0x80 : m_ucLevelAndHide &= 0x7F; }
    BOOL IsViewable() { return (m_ucLevelAndHide & 0x80) == 0x80; }

    // used only when communicating with CGridTreeCtrl
    void SetLevelAndHide( UCHAR ucLevelAndHide) { m_ucLevelAndHide = ucLevelAndHide; }
    UCHAR GetLevelAndHide() { return m_ucLevelAndHide; }

	bool GetForcePlus() const			{  return m_forcePlus;  }
	void SetForcePlus( bool forcePlus )	{  m_forcePlus = forcePlus;  }

    typedef enum
    {
        TREE_IMAGE_FOLDER_OPEN,
        TREE_IMAGE_FOLDER_CLOSED,
        TREE_IMAGE_DOCUMENT,

        TREE_IMAGE_FENCE      // ** don't delete or move from end
    }   TREE_IMAGE;  // defines how Tree will use image list

// base Attributes
public:
    // use base class for SetState(DWORD nState);
    // use base class for GetState();
    virtual void SetGrid(CGridCtrl* /* pGrid */) {};
    virtual CGridCtrl* GetGrid() const
        {   ASSERT( m_pTreeColumn != NULL);
            return m_pTreeColumn->GetGrid(); }

    virtual void SetText(LPCTSTR /* szText */) { ASSERT( FALSE); }   // must override
    virtual LPCTSTR  GetText() const      { ASSERT( FALSE); return NULL; } // must override

    virtual void SetImage(int nImage) { ASSERT( nImage <= USHRT_MAX); m_sImage = (short)nImage; }
    virtual int      GetImage() const     { return m_sImage;  }

    // let the CGridTreeCtrl maintain a single instance of the following:
    virtual void SetData(LPARAM lParam)
        { if( m_pTreeColumn != NULL) m_pTreeColumn->SetParam( lParam); }
    virtual void SetFormat(DWORD nFormat)
        { if( m_pTreeColumn != NULL) m_pTreeColumn->SetFormat( nFormat); }
    virtual void SetTextClr(COLORREF clr)
        { if( m_pTreeColumn != NULL) m_pTreeColumn->SetFgClr( clr); }
    virtual void SetBackClr(COLORREF clr)
        { if( m_pTreeColumn != NULL) m_pTreeColumn->SetBkClr( clr); }
    virtual void SetFont(const LOGFONT* plf)
    {
        if( m_pTreeColumn != NULL)
        {
            m_pTreeColumn->SetFont( plf);

            // Calculate the default tree indentation
            CWindowDC dc(NULL);
            CFont font;
            font.CreateFontIndirect(plf);
            CFont* pOldFont = dc.SelectObject(&font);
            TEXTMETRIC tm;
            dc.GetTextMetrics(&tm);
            dc.SelectObject(pOldFont);

            m_pTreeColumn->SetDefTreeIndent( tm.tmHeight+tm.tmExternalLeading);    // so can draw square box

        }
    }

    virtual void SetMargin( UINT nMargin)
        { if( m_pTreeColumn != NULL) m_pTreeColumn->GetGrid()->SetDefCellMargin( nMargin); }
    virtual void SetCoords( int nRow, int nCol) { m_iRow = nRow; m_iCol = nCol; }
    virtual CPoint GetCoords() const { return CPoint( m_iCol, m_iRow); }

    virtual LPARAM   GetData() const
        {
            if( m_pTreeColumn != NULL)
                return m_pTreeColumn->GetParam();
            return 0;
        }
    virtual DWORD    GetFormat() const
        {
            if( m_pTreeColumn != NULL)
                return m_pTreeColumn->GetFormat();
            return 0;
        }
    virtual COLORREF GetTextClr() const
        {
            if( m_pTreeColumn != NULL)
                return m_pTreeColumn->GetFgClr();
            return CLR_DEFAULT;
        }
    virtual COLORREF GetBackClr() const
        {
            if( m_pTreeColumn != NULL)
                return m_pTreeColumn->GetBkClr();
            return CLR_DEFAULT;
        }
    virtual LOGFONT* GetFont() const
        {
            if( m_pTreeColumn != NULL)
                return m_pTreeColumn->GetFont();
            return NULL;
        }

    virtual CFont*  GetFontObject() const
        {
            // If the default font is specified, use the default cell implementation
            if (GetFont() == NULL)
            {
                CGridDefaultCell *pDefaultCell = (CGridDefaultCell*) GetDefaultCell();
                if (!pDefaultCell)
                    return NULL;

                return pDefaultCell->GetFontObject();
            }
            else
            {
                static CFont Font;
                Font.DeleteObject();
                Font.CreateFontIndirect(GetFont());
                return &Font;
            }
        }

    virtual UINT     GetMargin() const
        {
            if( m_pTreeColumn != NULL)
                return m_pTreeColumn->GetGrid()->GetDefCellMargin();
            return 0;
        }

    virtual BOOL IsDefaultFont() const
        {
            if( m_pTreeColumn != NULL)
                return m_pTreeColumn->IsDefaultFont();
            return TRUE;
        }

    virtual void Reset();
    virtual BOOL IsEditing() const { return m_ucEditing; }

    virtual void operator=(CGridCellBase& cell)
    {
        CGridCellBase::operator=( cell);
		m_forcePlus = ((CGridTreeCellBase&)cell).m_forcePlus;
    }

// editing cells
public:
    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);
    BOOL DrawTreeCell(CDC* pDC, int nRow, int nCol, CRect* prect, BOOL bEraseBkgnd);
    virtual BOOL GetTextRect( LPRECT pRect);    // i/o:  i=dims of cell rect; o=dims of text rect
    virtual CSize GetCellExtent(CDC* pDC);

    // override the following
    virtual BOOL PrintCell(CDC* pDC, int nRow, int nCol, CRect rect);

// tree drawing
protected:

    void TreeGetBoxRelCoords(CRect* apRect);      // returns: relative coordinates

    int GetTreeIndent();
    // returns:  device units to indent within a cell for a tree at this level

    BOOL TreeHitPlusMinus(  CPoint aPointMouse); // relative coordinates of mouse click
    // returns:  T=hit a plus or minus;  F=Missed it or tree cell has no plus or minus

    void TreeDrawGraphic(CDC* apDC,      // current CDC
                         CRect aRect);   // coordinates of bounding cell rectangle

    // helpers to assure that I don't draw outside of my rectangle
    BOOL TryMoveTo( const CRect& aRect, CDC* apDC, int aX, int aY)
    {
        CRect rectMargin( aRect);
        rectMargin.InflateRect( GetMargin(), GetMargin() );
        if( rectMargin.PtInRect( CPoint( aX, aY ))  )
        {
            apDC->MoveTo( aX, aY);
            return TRUE;
        }
        return FALSE;
    }

    BOOL TryLineTo( const CRect& aRect, CDC* apDC, int aX, int aY)
    {
        CRect rectMargin( aRect);
        rectMargin.InflateRect( GetMargin(), GetMargin() );
        if( rectMargin.PtInRect( CPoint( aX, aY ))  )
        {
            apDC->LineTo( aX, aY);
            return TRUE;
        }
        return FALSE;
    }

// editing cells
public:
    virtual BOOL Edit(  int /* nRow */, int /* nCol */, CRect /* rect */, CPoint /* point */,
                        UINT /* nID */, UINT /* nChar */) { ASSERT( FALSE); return FALSE;}
    virtual void EndEdit() {};
protected:
    virtual void OnEndEdit() {};

protected:
    virtual void OnClick( CPoint PointCellRelative);
    virtual void OnClickDown( CPoint PointCellRelative);
    virtual void OnRClick( CPoint PointCellRelative);
    virtual void OnDblClick( CPoint PointCellRelative);

protected:
    UCHAR    m_ucEditing;   // Cell being edited?
    UCHAR    m_ucLevelAndHide;  // high-order bit tells if cell is viewable or hidden;
                                //  rest tell the tree level of this cell
	bool	 m_forcePlus;
    short    m_sImage;      // Index of the list view item’s icon
    int      m_iRow;        // Each cell needs to know position relative to neighbor cells
    int      m_iCol;

    CTreeColumn* m_pTreeColumn; // contains collection of cells plus access to parent grid control
};


#endif // !defined(AFX_GridTreeCellBase_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_)
