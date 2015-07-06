/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_GridBtnCellBase_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_)
#define AFX_GridBtnCellBase_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\GRIDCTRL_SRC\GridCell.h"
#include "BtnDataBase.h"

class CGridBtnCellBase : public CGridCellBase
{
    friend class CGridTreeBtnCell;
    DECLARE_DYNCREATE(CGridBtnCellBase)

// Construction/Destruction
public:
    CGridBtnCellBase();
    virtual ~CGridBtnCellBase();

    void SetBtnDataBase( CBtnDataBase* pBtnDataBase)
        {
            ASSERT( pBtnDataBase);
            m_pBtnDataBase = pBtnDataBase;
        }
    CBtnDataBase* GetBtnDataBase() { return m_pBtnDataBase; }

public:
    // structs used when manipulating buttons
    typedef enum
    {
        CTL_ALIGN_CENTER,
        CTL_ALIGN_LEFT,
        CTL_ALIGN_RIGHT,

        CTL_ALIGN_FENCE     // ** don't delete or remove from last in list
    }   CTL_ALIGN;  // how is DrawFrameControl() aligned?

    typedef struct
    {
        int iWidth;             // Fixed width of button; if 0, size to cell width
        short sState;           // e.g. DFCS_PUSHED | DFCS_BUTTONCHECK
        unsigned char ucIsMbrRadioGrp : 1;  // T=btn is member of a radio group
        unsigned char ucAlign : 7;  // how control image is aligned
        unsigned char ucType;   // type of control to draw  e.g. DFC_BUTTON

    }   STRUCT_DRAWCTL; // used with win32's DrawFrameControl() to place an image
                        //  of a windows control within a cell

    BOOL SetupBtns(  int aiWhich,        // zero-based index of image to draw
                     UINT auiType,       // type of frame control to draw e.g. DFC_BUTTON
                     UINT auiState,      // like DrawFrameControl()'s nState  e.g. DFCS_BUTTONCHECK
                     CGridBtnCellBase::CTL_ALIGN aAlign,   // horizontal alignment of control image
                     int aiWidth,        // fixed width of control or 0 for size-to-fit
                     BOOL abIsMbrRadioGrp,   // T=btn is member of a radio group
                     const char* apszText);   // Text to insert centered in button; if NULL no text
    // returns:  success / failure


// for drawing things like buttons and check marks within a cell
//  you can have more than one control per cell
public:
    virtual short GetDrawCtlNbr() { return m_sNbrDrawCtls; }
    virtual short GetLastCtlClicked() { return m_sLastCtlClicked; }

    virtual unsigned char GetDrawCtlNbrMax() { return 0; }

    virtual UINT GetDrawCtlType( int /* aiWhich */)
        { ASSERT( FALSE); return 0; }

    virtual void SetDrawCtlType( int /* aiWhich */, UINT /* auiType */)
        { ASSERT( FALSE); }

    virtual UINT GetDrawCtlState( int /* aiWhich */)
        { ASSERT( FALSE); return 0; }

    virtual void SetDrawCtlState( int /* aiWhich */, UINT /* auiState */)
        { ASSERT( FALSE); }

    virtual int GetDrawCtlWidth( int /* aiWhich */)
        { ASSERT( FALSE); return -1; }

    virtual void SetDrawCtlWidth( int /* aiWhich */, int /* aiWidth */)
        { ASSERT( FALSE); }

    virtual CTL_ALIGN GetDrawCtlAlign( int /* aiWhich */)
        { ASSERT( FALSE); return CTL_ALIGN_FENCE; }

    virtual void SetDrawCtlAlign( int /* aiWhich */, CTL_ALIGN /* aucAlign */)
        { ASSERT( FALSE); }

    virtual LPCTSTR GetDrawCtlBtnText( int /* aiWhich */)
        { ASSERT( FALSE); return NULL; }

    virtual void SetDrawCtlBtnText( int /* aiWhich */, const char* /* apszText */)
        { ASSERT( FALSE); }

    virtual BOOL GetDrawCtlIsMbrRadioGrp( int /* aiWhich */)
        { ASSERT( FALSE); return FALSE; }

    virtual void SetDrawCtlIsMbrRadioGrp( int /* aiWhich */, BOOL /* abOn */)
        { ASSERT( FALSE); }


// base Attributes
public:
    virtual void SetDrawCtlNbr( short asNbr)
        { ASSERT( (int)asNbr <= GetDrawCtlNbrMax());
          m_sNbrDrawCtls = asNbr; }

    virtual void SetText(LPCTSTR szText)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetText( m_iRow, m_iCol, szText); }

    virtual void SetImage(int nImage)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetImage(m_iRow, m_iCol, nImage); }

    virtual void SetData(LPARAM lParam)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetData(m_iRow, m_iCol, lParam); }

    virtual void SetFormat(DWORD nFormat)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetFormat(m_iRow, m_iCol, nFormat); }

    virtual void SetTextClr(COLORREF clr)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetTextClr(m_iRow, m_iCol, clr); }

    virtual void SetBackClr(COLORREF clr)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetBackClr(m_iRow, m_iCol, clr); }

    virtual void SetFont(const LOGFONT* plf)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetFont(m_iRow, m_iCol, plf); }

    virtual void SetMargin( UINT nMargin)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetMargin(m_iRow, m_iCol, nMargin); }

    virtual void SetCoords( int nRow, int nCol) { m_iRow = nRow; m_iCol = nCol; }
    virtual CPoint GetCoords() { return CPoint( m_iCol, m_iRow); }

    virtual void SetGrid(CGridCtrl* pGrid)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetGrid( pGrid); }

    virtual void SetEditWnd(CWnd* pEditWnd)
        { if( m_pBtnDataBase) m_pBtnDataBase->SetEditWnd(pEditWnd); }


    virtual LPCTSTR  GetText() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetText(m_iRow, m_iCol);
            return NULL;
        }

    virtual LPCTSTR  GetTipText();// may override TitleTip return

    virtual int GetImage() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetImage(m_iRow, m_iCol);
            return -1;
        }

    virtual LPARAM GetData() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetData(m_iRow, m_iCol);
            return 0;
        }

    virtual DWORD GetFormat() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetFormat(m_iRow, m_iCol);
            return 0;
        }

    virtual COLORREF GetTextClr() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetTextClr(m_iRow, m_iCol);
            return CLR_DEFAULT;
        }

    virtual COLORREF GetBackClr() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetBackClr(m_iRow, m_iCol);
            return CLR_DEFAULT;
        }

    virtual LOGFONT* GetFont() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetFont(m_iRow, m_iCol);
            return NULL;
        }

    virtual CFont* GetFontObject() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetFontObject(m_iRow, m_iCol);
            return NULL;
        }

    virtual UINT GetMargin() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetMargin(m_iRow, m_iCol);
            return 0;
        }

    virtual CGridCtrl* GetGrid() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetGrid();
            return NULL;
        }

    virtual CWnd* GetEditWnd() const
        {
            if( m_pBtnDataBase)
                return m_pBtnDataBase->GetEditWnd();
            return NULL;
        }


    virtual void Reset();
    virtual BOOL IsEditing() const { return m_ucEditing; }

    virtual void operator=(CGridCellBase& cell)
    {
        CGridCellBase::operator=( cell);
    }

// editing cells
public:
    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);
    BOOL DrawBtnCell(CDC* pDC, int nRow, int nCol, CRect* prect, BOOL bEraseBkgnd);
    virtual BOOL GetTextRect( LPRECT pRect);    // i/o:  i=dims of cell rect; o=dims of text rect
    virtual BOOL GetTipTextRect( LPRECT pRect); // may override for btns, etc.
    virtual CSize GetCellExtent(CDC* pDC);

    // override the following
    virtual BOOL PrintCell(CDC* pDC, int nRow, int nCol, CRect rect);

// editing cells
public:
    virtual BOOL Edit(  int nRow, int nCol, CRect rect, CPoint point,
                        UINT nID, UINT nChar);
    virtual void EndEdit();
protected:
    virtual void OnEndEdit();

protected:
    virtual void OnClick( CPoint PointCellRelative);
    virtual void OnClickDown( CPoint PointCellRelative);
    virtual void OnRClick( CPoint PointCellRelative);
    virtual void OnDblClick( CPoint PointCellRelative);

protected:
    BOOL ClickedCellCtl(  UINT uMsg,      // Command that invoked.  e.g. WM_LBUTTONDOWN
                          int aiWhich);   // zero-based index of image to draw
    // returns:  T=redraw occurred / F=no redraw

    BOOL ProcessCtlClick(   UINT uMsg,              // Command that invoked.  e.g. WM_LBUTTONDOWN
                            const CPoint& arPoint); // point to check for hit
    // returns:  T=hit a control / F=no control hit

    int RelPointInCtl(    const CPoint& arPoint);  // Relative point coords
    // returns:  Index of control that this point is within bounds of or -1 if no control matches

    BOOL CalcDrawCtlRects(CRect* apRect,    // returns:  CRects with coordinates
                                            //  last entry has optional leftover rect
                                            //  available for text, etc.
                  int aiNbrRectEntries,     // nbr of Rects in above array
                  const CRect& arRectCell); // cell rectangle to work with

    BOOL HasCellText();
    // returns:  F=auto-size buttons, only

    int HotKeyBtnMatch( char acHotKey); // hot key character
    // returns:  index of button or -1 if no hot key matches

protected:
    short    m_sLastCtlClicked; // what control was last clicked?
    short    m_sNbrDrawCtls;// how many controls?

    int      m_iRow;        // Each cell needs to know position
    int      m_iCol;

    UCHAR    m_ucEditing;   // Cell being edited?

    CBtnDataBase* m_pBtnDataBase;   // contains data common to this instance of the grid as well
                                    //  as a reference to the grid, too

    static CString m_strTipText;    // may have special title tip - note only 1 tip at a time
};


#endif // !defined(AFX_GridBtnCellBase_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_)
