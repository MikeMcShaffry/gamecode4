/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_GRIDBTNCELLCOMBO_H__00C20283_F376_11D3_B761_00C04F6A7AE6__INCLUDED_)
#define AFX_GRIDBTNCELLCOMBO_H__00C20283_F376_11D3_B761_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GridBtnCell.h"

class CGridBtnCellCombo : public CGridBtnCell
{
    DECLARE_DYNCREATE(CGridBtnCellCombo)
public:
	CGridBtnCellCombo();
	virtual ~CGridBtnCellCombo();

    virtual void Reset();

    virtual void operator=(CGridCellBase& cell)
    {
        CGridBtnCellBase::operator=( cell);
    }

// accessors
public:
    virtual void SetComboStyle( DWORD dwComboStyle) { m_dwComboStyle = dwComboStyle; }
    virtual DWORD GetComboStyle() { return m_dwComboStyle; }

    virtual void SetComboString( const CStringArray& astrAry) { m_StringArrayCombo.Copy( astrAry); }
    virtual void GetComboString( CStringArray* apstrAry) { apstrAry->Copy( m_StringArrayCombo); }

// editing cells
public:
    virtual BOOL Edit(  int nRow, int nCol, CRect rect, CPoint point,
                        UINT nID, UINT nChar);
    virtual void EndEdit();
protected:
    virtual void OnEndEdit();

protected:
    virtual void OnClick( CPoint PointCellRelative);

protected:
    DWORD m_dwComboStyle;
    CStringArray m_StringArrayCombo;    // combo Items

};

#endif // !defined(AFX_GRIDBTNCELLCOMBO_H__00C20283_F376_11D3_B761_00C04F6A7AE6__INCLUDED_)
