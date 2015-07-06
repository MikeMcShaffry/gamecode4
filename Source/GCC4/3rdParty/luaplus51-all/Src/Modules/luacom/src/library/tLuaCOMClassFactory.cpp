// tLuaCOMClassFactory.cpp: implementation of the tLuaCOMClassFactory class.
//
//////////////////////////////////////////////////////////////////////

extern "C" {
  #include "lua.h"
  #include "LuaCompat.h"
}
#include "luacom.h"
#include "tLuaCOMClassFactory.h"
#include "tLuaCOMException.h"
#include "tUtil.h"


STDMETHODIMP
tLuaCOMClassFactory::QueryInterface(REFIID iid, void** ppv) 
{   
  *ppv = NULL;

  if (iid == IID_IUnknown || iid == IID_IClassFactory)
    *ppv = this;
  else 
    return E_NOINTERFACE; 

  AddRef();
  return NOERROR;    
}


STDMETHODIMP_(ULONG)
tLuaCOMClassFactory::AddRef(void)
{ 
  return ++m_cRef;
}


STDMETHODIMP_(ULONG)
tLuaCOMClassFactory::Release(void)
{ 
  if(--m_cRef == 0)
  {
    delete this;
    return 0;
  }

  return m_cRef;
}

/*
 * tLuaCOMClassFactory::CreateInstance, LockServer
 *
 * Purpose:
 *  Implements IClassFactory::CreateInstance, LockServer
 *
 */
STDMETHODIMP
tLuaCOMClassFactory::CreateInstance(IUnknown* punkOuter,
                         REFIID riid,
                         void** ppv)
{
  HRESULT hr = E_NOTIMPL;
    
  *ppv = NULL;
    
  // This implementation does'nt allow aggregation
  if (punkOuter)
    return CLASS_E_NOAGGREGATION;
    
  // This is REGCLS_SINGLEUSE class factory, so CreateInstance will be
  // called atmost once. An application objects has a REGCLS_SINGLEUSE class
  // factory. The global application object has already been created when 
  // CreateInstance is called. A REGCLS_MULTIPLEUSE class factory's 
  // CreateInstance would be called multiple times and would create a new 
  // object each time. An MDI application would have a REGCLS_MULTIPLEUSE 
   // class factory for it's document objects.
  hr = object->QueryInterface(riid, ppv); 

  return hr;
}

STDMETHODIMP
tLuaCOMClassFactory::LockServer(BOOL fLock)
{    
  CoLockObjectExternal(object, fLock, TRUE); 
  return NOERROR;       
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tLuaCOMClassFactory::tLuaCOMClassFactory(IDispatch* object_arg)
{
  CHECKPARAM(object_arg);

  m_cRef = 0;

  L_inproc = NULL;
  object = object_arg;
  object->AddRef();
}

tLuaCOMClassFactory::tLuaCOMClassFactory(lua_State* L)
{
  CHECKPARAM(L);

  m_cRef = 0;

  L_inproc = L;
  lua_getregistry(L);
  lua_pushstring(L,"object");
  lua_gettable(L,-2);
  object = (IDispatch*)luaCompat_getPointer(L,-1);
  object->AddRef();
  lua_pop(L,1);
  lua_pushstring(L,"object");
  lua_pushnil(L);
  lua_settable(L,-3);
  lua_pop(L,1);
}

tLuaCOMClassFactory::~tLuaCOMClassFactory()
{
  object->Release();
  if(L_inproc!=NULL) {
    luacom_close(L_inproc);
    lua_close(L_inproc);
  }
}
