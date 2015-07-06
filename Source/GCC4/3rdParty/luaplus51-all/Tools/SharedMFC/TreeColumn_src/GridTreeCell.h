/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_GRIDTREECELL_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_)
#define AFX_GRIDTREECELL_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GridTreeCellBase.h"

// Place these Grid Cells in a column in a grid to
//  create a tree control
class CGridTreeCell : public CGridTreeCellBase
{
    DECLARE_DYNCREATE(CGridTreeCell)

// Construction/Destruction
public:
    CGridTreeCell();
    virtual ~CGridTreeCell();


// base Attributes
public:
    virtual void SetText(LPCTSTR szText) { m_strText = szText; }
    virtual LPCTSTR  GetText() const      { return (m_strText.IsEmpty())? _T("") : m_strText; }

    virtual void Reset();

    virtual void operator=(CGridCellBase& cell)
    {
        CGridTreeCellBase::operator=( cell);
    }


// editing cells
public:
    BOOL Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar);
    void EndEdit();
protected:
    void OnEndEdit();

protected:
    CString  m_strText;     // Cell text (or binary data if you wish...)
};


#endif // !defined(AFX_GRIDTREECELL_H__97838AF2_EBD9_11D3_B75D_00C04F6A7AE6__INCLUDED_)
