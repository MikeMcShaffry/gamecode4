/*
 * tLuaDispatch.h
 */

#ifndef __TLUADISPATCH_H
#define __TLUADISPATCH_H

#include <string.h>
//#include <iostream.h>

#include <ole2.h>
#include <ocidl.h>

#include <assert.h>
#include <stdio.h>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

#include "luabeans.h"
#include "tLuaCOMTypeHandler.h"
#include "tLuaCOMConnPoints.h"

class ILuaDispatch : public IUnknown {
public:
	STDMETHOD(PushIfSameState)(lua_State *L)=0;
};


// {FE5AC331-0A3B-4093-A02F-41C39DDD06D8}
static const GUID IID_ILuaDispatch = 
{ 0xfe5ac331, 0xa3b, 0x4093, { 0xa0, 0x2f, 0x41, 0xc3, 0x9d, 0xdd, 0x6, 0xd8 } };

static void *idxDispatch;

class tLuaDispatch : public IDispatch, public ILuaDispatch
{
public:
	tLuaCOMConnPointContainer* GetConnPointContainer(void);
	void BeConnectable(void);
	void SetCoClassinfo(ITypeInfo* coclassinfo);
	void FillExceptionInfo(EXCEPINFO *pexcepinfo, const char* text);
	static tLuaDispatch * CreateLuaDispatch(
    lua_State* L,
    ITypeInfo* typeinfo,
    int ref
    );

  tLuaDispatch(lua_State* L, ITypeInfo *pTypeinfo, int ref);
  ~tLuaDispatch();

  /* IUnknown methods */
  STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
  STDMETHOD_(ULONG, AddRef)(void);
  STDMETHOD_(ULONG, Release)(void);

  /* IDispatch methods */
  STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pcTypeInfo);

  STDMETHOD(GetTypeInfo)(
    unsigned int iTypeInfo,
    LCID lcid,
    ITypeInfo FAR* FAR* ppTypeInfo);

  STDMETHOD(GetIDsOfNames)(
    REFIID riid,
    OLECHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    LCID lcid,
    DISPID FAR* rgdispid);

  STDMETHOD(Invoke)(
    DISPID dispidMember,
    REFIID riid,
    LCID lcid,
    unsigned short wFlags,
    DISPPARAMS FAR* pdispparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);

  STDMETHOD(PushIfSameState)(lua_State *L);

  ITypeInfo *typeinfo;

protected:
	tLuaCOMConnPointContainer* cpc;
	HRESULT propertyget(
    const char* name, 
    FUNCDESC* funcdesc,
    DISPPARAMS* pdispparams,
    VARIANT* pvarResult,
    EXCEPINFO* pexcepinfo,
    unsigned int* puArgErr);

  HRESULT propertyput(
    const char* name,
    DISPPARAMS *pdispparams,
    VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo,
    unsigned int *puArgErr);

  HRESULT method(
    const char* name,
    FUNCDESC* funcdesc,
    DISPPARAMS *pdispparams,
    VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo,
    unsigned int *puArgErr);

	lua_State* L;
	IID interface_iid;
	tLuaCOMTypeHandler * typehandler;
  static int tag;

  unsigned long m_refs;

  struct tFuncInfo
  {
    FUNCDESC *funcdesc;
    char *name;
  } *funcinfo;

  class ProvideClassInfo2 : IProvideClassInfo2
  {
  public:
    ProvideClassInfo2(ITypeInfo* p_coclassinfo, IUnknown* p_pUnk);
	ProvideClassInfo2::~ProvideClassInfo2();

    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD(GetClassInfo)(ITypeInfo** ppTypeInfo);
    STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID * pGUID);

  protected:
    ITypeInfo* coclassinfo;
    IUnknown* pUnk;
  };
  
  ProvideClassInfo2* classinfo2;

  int num_methods;
  int table_ref;

private:
  static long NEXT_ID;
  long ID;

};

#endif // __TLUADISPATCH_H
