/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#if !defined(AFX_GRIDBTNCELL_H__937E4927_EF80_11D3_B75F_00C04F6A7AE6__INCLUDED_)
#define AFX_GRIDBTNCELL_H__937E4927_EF80_11D3_B75F_00C04F6A7AE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GridBtnCellBase.h"

class CGridBtnCell : public CGridBtnCellBase
{
    DECLARE_DYNCREATE(CGridBtnCell)
public:
    CGridBtnCell();
    virtual ~CGridBtnCell();

    virtual void Reset();

    virtual void operator=(CGridCellBase& cell)
    {
        CGridBtnCellBase::operator=( cell);
    }

public:
    virtual void SetText(LPCTSTR szText)    { m_strText = szText; }
    virtual LPCTSTR  GetText() const        { return (m_strText.IsEmpty())? _T("") : m_strText; }

    virtual void SetFormat(DWORD nFormat)   { m_nFormat = nFormat; }
    virtual DWORD GetFormat() const         { return m_nFormat; }



// for drawing things like buttons and check marks within a cell
//  you can have more than one control per cell
public:
    virtual unsigned char GetDrawCtlNbrMax() { return NBR_CTLS; }

    UINT GetDrawCtlType( int aiWhich)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          return (UINT)(DrawCtl[ aiWhich].ucType); }

    void SetDrawCtlType( int aiWhich, UINT auiType)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          DrawCtl[ aiWhich].ucType = (unsigned char)auiType; }

    UINT GetDrawCtlState( int aiWhich)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          return (UINT)(DrawCtl[ aiWhich].sState); }

    void SetDrawCtlState( int aiWhich, UINT auiState)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          ASSERT( auiState < USHRT_MAX);
          DrawCtl[ aiWhich].sState = (short)auiState; }

    int GetDrawCtlWidth( int aiWhich)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          return DrawCtl[ aiWhich].iWidth; }

    void SetDrawCtlWidth( int aiWhich, int aiWidth)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          DrawCtl[ aiWhich].iWidth = aiWidth; }

    CTL_ALIGN GetDrawCtlAlign( int aiWhich)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          return (CTL_ALIGN) DrawCtl[ aiWhich].ucAlign; }

    void SetDrawCtlAlign( int aiWhich, CTL_ALIGN aucAlign)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          ASSERT( aucAlign < CTL_ALIGN_FENCE);
          DrawCtl[ aiWhich].ucAlign = (unsigned char)aucAlign; }

    LPCTSTR GetDrawCtlBtnText( int aiWhich)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          if( strBtnAry[ aiWhich].IsEmpty())
              return NULL;
          return strBtnAry[ aiWhich]; }

    void SetDrawCtlBtnText( int aiWhich, const char* apszText)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          if( apszText == NULL)
              strBtnAry[ aiWhich].Empty();
          strBtnAry[ aiWhich] = apszText; }

    BOOL GetDrawCtlIsMbrRadioGrp( int aiWhich)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          return (BOOL)DrawCtl[ aiWhich].ucIsMbrRadioGrp; }

    void SetDrawCtlIsMbrRadioGrp( int aiWhich, BOOL abOn)
        { ASSERT( aiWhich < GetDrawCtlNbrMax());
          DrawCtl[ aiWhich].ucIsMbrRadioGrp = abOn ? 1 : 0;}

protected:
    CString  m_strText;     // Cell text (or binary data if you wish...)
    DWORD m_nFormat;
    enum { NBR_CTLS = 4};

    STRUCT_DRAWCTL DrawCtl[ NBR_CTLS]; // Stores draw control information
    CString strBtnAry[ NBR_CTLS];         // button text -- for push buttons
};

#endif // !defined(AFX_GRIDBTNCELL_H__937E4927_EF80_11D3_B75F_00C04F6A7AE6__INCLUDED_)
