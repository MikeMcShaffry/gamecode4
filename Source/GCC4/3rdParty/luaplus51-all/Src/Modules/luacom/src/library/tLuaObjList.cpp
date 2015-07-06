// tLuaObjList.cpp: implementation of the tLuaObjList class.
//
//////////////////////////////////////////////////////////////////////

#include "tLuaObjList.h"
#include "tLuaCOMException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


tLuaObjList::tLuaObjList(stkIndex first_param_pos, long num_params_p)
{
  CHECKPARAM(first_param_pos >= 0 && num_params_p >= 0);

  first_param = first_param_pos;
  num_params = num_params_p;

}

tLuaObjList::tLuaObjList(void)
{
  first_param = 0;
  num_params = 0;
}

stkIndex tLuaObjList::getparam(long i)
{
  CHECKPARAM(i >= 0);

  if(i < num_params)
    return first_param + i;
  else
    return 0;
}

long tLuaObjList::getNumParams()
{
  return num_params;
}
