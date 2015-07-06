// tLuaCOMEnumerator.cpp: implementation of the tLuaCOMEnumerator class.
//
//////////////////////////////////////////////////////////////////////

#include "tLuaCOMEnumerator.h"

#include "tUtil.h"
#include "tCOMUtil.h"
#include "tLuaCOMException.h"

#include "LuaAux.h"

#include "luacom_internal.h"

extern "C"
{
#include "LuaCompat.h"
}



// initialization

const char *tLuaCOMEnumerator::type_name = "__ENUMERATOR_LUACOM_TYPE"; 
const char *tLuaCOMEnumerator::pointer_type_name = "__ENUMERATOR_POINTER_LUACOM_TYPE"; 
const char *tLuaCOMEnumerator::module_name = NULL; 

#define ENUMERATOR_FIELD "__TLUACOMENUMERATOR__internal"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tLuaCOMEnumerator::tLuaCOMEnumerator(IEnumVARIANT* pEV_param)
{
  CHECKPRECOND(pEV_param);
  
  // stores enumerator
  pEV = pEV_param;
  pEV->AddRef();

  typehandler = new tLuaCOMTypeHandler(NULL);
}

tLuaCOMEnumerator::~tLuaCOMEnumerator()
{
  COM_RELEASE(pEV);
  delete typehandler;
}

int tLuaCOMEnumerator::index(lua_State *L)
{
  // pushes connection point
  lua_pushstring(L, ENUMERATOR_FIELD);
  lua_gettable(L, -3);

  // pushes method name
  lua_pushvalue(L, 2);

  // pushes closure to call all sinks
  lua_pushcclosure(L, call_method, 2);

  return 1;
}

void tLuaCOMEnumerator::push(lua_State* L)
{
  LUASTACK_SET(L);

  // creates table
  lua_newtable(L);
  luaCompat_pushTypeByName(L, 
    tLuaCOMEnumerator::module_name, 
    tLuaCOMEnumerator::type_name);

  luaCompat_setType(L, -2);

  lua_pushstring(L, ENUMERATOR_FIELD);

  // pushes typed pointer
  luaCompat_pushTypeByName(L, 
    tLuaCOMEnumerator::module_name, 
    tLuaCOMEnumerator::pointer_type_name);

  luaCompat_newTypedObject(L, this);

  // stores in the table
  lua_settable(L, -3);

  LUASTACK_CLEAN(L, 1);
}

int tLuaCOMEnumerator::garbagecollect(lua_State *L)
{
  LUASTACK_SET(L);

  // gets the enumerator
  tLuaCOMEnumerator* enumerator = 
    (tLuaCOMEnumerator*) luaCompat_getTypedObject(L, -1);

  delete enumerator;

  LUASTACK_CLEAN(L, 0);

  return 0;
}

void tLuaCOMEnumerator::registerLuaType(lua_State *L, const char *module)
{
  LUASTACK_SET(L);

  tLuaCOMEnumerator::module_name = module;

  luaCompat_newLuaType(L, 
    tLuaCOMEnumerator::module_name, 
    tLuaCOMEnumerator::type_name);

  luaCompat_newLuaType(L, 
    tLuaCOMEnumerator::module_name, 
    tLuaCOMEnumerator::pointer_type_name);

  luaCompat_pushTypeByName(L, 
    tLuaCOMEnumerator::module_name, 
    tLuaCOMEnumerator::type_name);

  lua_pushcfunction(L, tLuaCOMEnumerator::index);
  luaCompat_handleNoIndexEvent(L);

  lua_pop(L, 1);

  luaCompat_pushTypeByName(L, 
    tLuaCOMEnumerator::module_name, 
    tLuaCOMEnumerator::pointer_type_name);

  lua_pushcfunction(L, tLuaCOMEnumerator::garbagecollect);
  luaCompat_handleGCEvent(L);

  lua_pop(L, 1);
  
  LUASTACK_CLEAN(L, 0);
}


int tLuaCOMEnumerator::call_method(lua_State *L)
{
  /// positions of parameters
  
  // self param (not used, but explicited to ensure
  // consistency)
  const int self_param        = 1;

  // first user param 
  const int user_first_param  = 2;
  
  // last user param, excluding upvalues
  const int user_last_param   = luaCompat_getNumParams(L, 2);

  // upvalues
  const int enumerator_param  = luaCompat_upvalueIndex(L, 1, 2);
  const int method_param      = luaCompat_upvalueIndex(L, 2, 2);

  int num_params = 0;

  if(user_last_param < user_first_param)
    num_params = 0;
  else
    num_params = user_last_param - user_first_param + 1;

  // gets the enumerator
  tLuaCOMEnumerator* enumerator = 
    (tLuaCOMEnumerator*) luaCompat_getTypedObject(L, enumerator_param);

  // gets the method name
  const char* method_name = lua_tostring(L, method_param);

  // call method
  int retval = 0;

  try
  {
    retval = enumerator->callCOMmethod(L, method_name, user_first_param, num_params);
  }
  catch(class tLuaCOMException& e)
  {
    luacom_error(L, e.getMessage());

    return 0;
  }

  return retval;
}

int tLuaCOMEnumerator::callCOMmethod(lua_State* L, const char *name, int first_param, int num_params)
{
  HRESULT hr = S_OK;

  // Next method
  if(strcmp(name, "Next") == 0)
  {
    VARIANT* pVar = NULL;
    unsigned long num_elements = 1, counter = 0;
    ULONG fetched = 0;


    if(num_params > 0)
    {
      num_elements = (unsigned long) lua_tonumber(L, first_param);
    }

    pVar = new VARIANT[num_elements];

    for(counter = 0; counter <  num_elements; counter++)
      VariantInit(&pVar[counter]);


    hr = pEV->Next(num_elements, pVar, &fetched);
    
    for(counter = 0; counter < fetched; counter++)
    {
      typehandler->com2lua(L, pVar[counter]);
      typehandler->releaseVariant(&pVar[counter]);
    }

    for(counter = 0; counter <  num_elements; counter++)
	  VariantClear(&pVar[counter]);

    delete[] pVar;

	pVar = NULL;

    return fetched;
  }

  if(strcmp(name, "Reset") == 0)
  {
    hr = pEV->Reset();
    CHK_LCOM_ERR(hr == S_OK, "Unable to reset enumeration.");
    
    return 0;
  }

  if(strcmp(name, "Skip") == 0)
  {
    CHK_LCOM_ERR(num_params > 0, "Not enough parameters.");

    unsigned long num_elements = (unsigned long) lua_tonumber(L, first_param);

    hr = pEV->Skip(num_elements);

    luaCompat_pushBool(L, hr == S_OK);

    return 1;
  }

  if(strcmp(name, "Clone") == 0)
  {
    IEnumVARIANT* p_newEV = NULL;

    hr = pEV->Clone(&p_newEV);
    CHK_COM_CODE(hr);

    tLuaCOMEnumerator* enumerator = new tLuaCOMEnumerator(p_newEV);
    
    COM_RELEASE(p_newEV);

    enumerator->push(L);

    return 1;
  }



  return 0;
}
