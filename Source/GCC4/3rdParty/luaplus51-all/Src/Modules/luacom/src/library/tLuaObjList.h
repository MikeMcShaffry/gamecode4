// tLuaObjList.h: interface for the tLuaObjList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLUAOBJLIST_H__D9585072_945D_11D4_B881_0000B45D7541__INCLUDED_)
#define AFX_TLUAOBJLIST_H__D9585072_945D_11D4_B881_0000B45D7541__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "lua.h"

#include "tUtil.h"
#include "LuaAux.h"

class tLuaObjList
{
protected:
  stkIndex first_param;
  long num_params;

public:
	long getNumParams(void);

  tLuaObjList(stkIndex first_param_pos, long num_params_p);
  tLuaObjList(void);

  stkIndex getparam(long i);
};

#endif // !defined(AFX_TLUAOBJLIST_H__D9585072_945D_11D4_B881_0000B45D7541__INCLUDED_)
