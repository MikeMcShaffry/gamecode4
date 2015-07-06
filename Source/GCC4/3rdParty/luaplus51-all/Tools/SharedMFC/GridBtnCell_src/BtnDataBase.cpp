/*****************************************************************************
    COPYRIGHT (C) 2000-2001, Ken Bertelson <kbertelson@yahoo.com>


*****************************************************************************/
#include "stdafx.h"
#include "BtnDataBase.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBtnDataBase::CBtnDataBase()
{
    m_strText.Empty();
    m_nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS;
    m_nImage  = -1;
    m_crBkClr = CLR_DEFAULT;
    m_crFgClr = CLR_DEFAULT;
    m_lParam  = 0;
    m_nMargin = 3;
    m_pGrid   = NULL;
    m_pEditWnd = NULL;
    m_plfFont = NULL;
}

CBtnDataBase::~CBtnDataBase()
{
    delete m_plfFont;
}
