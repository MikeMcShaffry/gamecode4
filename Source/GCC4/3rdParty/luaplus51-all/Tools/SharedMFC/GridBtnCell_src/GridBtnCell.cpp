/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#include "stdafx.h"
#include "GridBtnCell.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CGridBtnCell, CGridBtnCellBase)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridBtnCell::CGridBtnCell()
    : CGridBtnCellBase()
{
    Reset();
}

CGridBtnCell::~CGridBtnCell()
{
}

void CGridBtnCell::Reset()
{
    CGridBtnCellBase::Reset();

    m_strText.Empty();
    m_nFormat = DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS;

    int i1;
    for( i1=0; i1 < NBR_CTLS; i1++)
    {
        memset( &DrawCtl[i1] ,0, sizeof( STRUCT_DRAWCTL) );
        strBtnAry[ i1].Empty();
    }
}
