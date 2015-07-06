/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_BtnDataBase_H__937E4923_EF80_11D3_B75F_00C04F6A7AE6__INCLUDED_)
#define AFX_BtnDataBase_H__937E4923_EF80_11D3_B75F_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\GRIDCTRL_SRC\GridCtrl.h"

class CBtnDataBase
{
public:
	CBtnDataBase();
	virtual ~CBtnDataBase();

// Attributes
public:
    virtual void SetText(int /* nRow */, int /* nCol */, LPCTSTR szText)    { m_strText = szText; }
    virtual void SetImage(int /* nRow */, int /* nCol */, int nImage)       { m_nImage = nImage; }
    virtual void SetData(int /* nRow */, int /* nCol */, LPARAM lParam)     { m_lParam = lParam; }
    virtual void SetFormat(int /* nRow */, int /* nCol */, DWORD nFormat)   { m_nFormat = nFormat; }
    virtual void SetTextClr(int /* nRow */, int /* nCol */, COLORREF clr)   { m_crFgClr = clr; }
    virtual void SetBackClr(int /* nRow */, int /* nCol */, COLORREF clr)   { m_crBkClr = clr; }
    virtual void SetFont(int /* nRow */, int /* nCol */, const LOGFONT* plf)
        {
            delete m_plfFont;
            if (plf != NULL)
            {
                m_plfFont = new LOGFONT;
                if (m_plfFont) 
                    memcpy(m_plfFont, plf, sizeof(LOGFONT)); 
            }
            else
                m_plfFont = NULL;
        }
    virtual void SetMargin(int /* nRow */, int /* nCol */, UINT nMargin)    { m_nMargin = nMargin; }
    virtual void SetGrid(CGridCtrl* pGrid)                                  { m_pGrid = pGrid; }
    virtual void SetEditWnd(CWnd* pEditWnd)                                 { m_pEditWnd = pEditWnd; }

    virtual LPCTSTR  GetText(int /* nRow */, int /* nCol */) const
        { return (m_strText.IsEmpty())? _T("") : m_strText; }
    virtual int      GetImage(int /* nRow */, int /* nCol */) const      { return m_nImage;  }
    virtual LPARAM   GetData(int /* nRow */, int /* nCol */) const       { return m_lParam;  }
    virtual DWORD    GetFormat(int /* nRow */, int /* nCol */) const     { return m_nFormat; }
    virtual COLORREF GetTextClr(int /* nRow */, int /* nCol */) const    { return m_crFgClr; }
    virtual COLORREF GetBackClr(int /* nRow */, int /* nCol */) const    { return m_crBkClr; }
    virtual LOGFONT* GetFont(int /* nRow */, int /* nCol */) const
        {
            if (m_plfFont)
                return (LOGFONT*) m_plfFont; 
            else
                return GetGrid()->GetDefaultCell(FALSE, FALSE)->GetFont();
        }
    virtual CFont*  GetFontObject(int nRow, int nCol) const
        {
            static CFont Font;
            Font.DeleteObject();
            Font.CreateFontIndirect(GetFont(nRow,nCol));
            return &Font;
        }
    virtual UINT     GetMargin(int /* nRow */, int /* nCol */) const     { return m_nMargin; }
    virtual CGridCtrl* GetGrid() const                                   { return m_pGrid;   }
    virtual CWnd*    GetEditWnd() const                                  { return m_pEditWnd; }

    virtual BOOL IsDefaultFont() const       { return (m_plfFont == NULL); }

protected:
    CString  m_strText;     // Cell text (or binary data if you wish...)

    DWORD    m_nFormat;     // Cell format
    int      m_nImage;      // Index of the list view item’s icon
    COLORREF m_crBkClr;     // Background colour (or CLR_DEFAULT)
    COLORREF m_crFgClr;     // Forground colour (or CLR_DEFAULT)
    LPARAM   m_lParam;      // 32-bit value to associate with item
    LOGFONT *m_plfFont;     // Cell font
    UINT     m_nMargin;     // Internal cell margin

    CGridCtrl* m_pGrid;     // Parent grid control
    CWnd* m_pEditWnd;
};

#endif // !defined(AFX_BtnDataBase_H__937E4923_EF80_11D3_B75F_00C04F6A7AE6__INCLUDED_)
