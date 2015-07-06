/////////////////////////////////////////////////////////////////////////////
// GridCell.h : header file
//
// MFC Grid Control - Grid cell class header file
//
// Written by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2000. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with CGridCtrl v2.10
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRIDCELL_H__519FA702_722C_11D1_ABBA_00A0243D1382__INCLUDED_)
#define AFX_GRIDCELL_H__519FA702_722C_11D1_ABBA_00A0243D1382__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CGridCtrl;
#include "GridCellBase.h"

//#define GRIDCTRL_MINSIZE

#ifdef GRIDCTRL_MINSIZE
class CGridCellAttr
{
public:
    virtual DWORD    GetFormat()                { return m_nFormat; }
    virtual COLORREF GetTextClr()               { return m_crFgClr; }
    virtual COLORREF GetBackClr()               { return m_crBkClr; }
    virtual const LOGFONT* GetFont()            { return &m_lfFont; }
    virtual UINT     GetMargin()                { return m_nMargin; }

    static DWORD    GetDefFormat()              { return m_nDefFormat; }
    static COLORREF GetDefTextClr()             { return m_crDefFgClr; }
    static COLORREF GetDefBackClr()             { return m_crDefBkClr; }
    static const LOGFONT* GetDefFont()          { return &m_lfFont; }
    static UINT     GetDefMargin()              { return m_nMargin; }

protected:
    DWORD    m_nFormat;     // Cell format
    COLORREF m_crBkClr;     // Background colour (or CLR_DEFAULT)
    COLORREF m_crFgClr;     // Forground colour (or CLR_DEFAULT)
    LOGFONT  m_lfFont;      // Cell font
    UINT     m_nMargin;     // Internal cell margin

protected:
    static DWORD    m_nDefFormat;     // Cell format
    static COLORREF m_crDefBkClr;     // Background colour (or CLR_DEFAULT)
    static COLORREF m_crDefFgClr;     // Forground colour (or CLR_DEFAULT)
    static LOGFONT  m_lfDefFont;      // Cell font
    static UINT     m_nDefMargin;     // Internal cell margin
};
#endif

// Each cell contains one of these. Fields "row" and "column" are not stored since we
// will usually have acces to them in other ways, and they are an extra 8 bytes per
// cell that is probably unnecessary.

class CGridCell : public CGridCellBase
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridCell)

// Construction/Destruction
public:
    CGridCell();
    virtual ~CGridCell();

// Attributes
public:
    virtual void SetText(LPCTSTR szText)        { m_strText = szText; }                       
    virtual void SetImage(int nImage)           { m_nImage = nImage; }                        
    virtual void SetData(LPARAM lParam)         { m_lParam = lParam; }      
    virtual void SetGrid(CGridCtrl* pGrid)      { m_pGrid = pGrid; }                          
    
 // virtual void SetState(DWORD nState);  -  use base class version   

    virtual void SetFormat(DWORD nFormat);
    virtual void SetTextClr(COLORREF clr);
    virtual void SetBackClr(COLORREF clr);
    virtual void SetFont(const LOGFONT* plf);
    virtual void SetMargin( UINT nMargin);

    virtual void SetCoords( int /* nRow */, int /* nCol */) {}  // don't need to know the row and
                                                                // column for base implementation

    virtual LPCTSTR  GetText()                  { return (m_strText.IsEmpty())? _T("") : m_strText; }
    virtual int      GetImage()                 { return m_nImage;  }
    virtual LPARAM   GetData()                  { return m_lParam;  }
    virtual CGridCtrl* GetGrid()                { return m_pGrid;   }

    // virtual DWORD    GetState()  - use base class

    virtual DWORD    GetFormat();
    virtual COLORREF GetTextClr();
    virtual COLORREF GetBackClr();
    virtual const LOGFONT* GetFont();
    virtual UINT     GetMargin();

    virtual BOOL     IsEditing()                { return m_bEditing; }
    virtual void     Reset();

// editing cells
public:
    virtual BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
    virtual void EndEdit();
protected:
    virtual void OnEndEdit();

protected:
    CString         m_strText;      // Cell text (or binary data if you wish...)
    LPARAM          m_lParam;       // 32-bit value to associate with item
    int             m_nImage;       // Index of the list view item’s icon

#ifdef GRIDCTRL_MINSIZE
    CGridCellAttr*  m_pAttr;        // Less commonly used attributes. NULL if defaults are used
#else
    DWORD    m_nFormat;
    COLORREF m_crFgClr;
    COLORREF m_crBkClr;
    LOGFONT  m_lfFont;
    UINT     m_nMargin;
#endif

    BOOL            m_bEditing;     // Cell being edited?

    CGridCtrl*      m_pGrid;        // Parent grid control
    CWnd*           m_pEditWnd;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCELL_H__519FA702_722C_11D1_ABBA_00A0243D1382__INCLUDED_)
