// tLuaCOMClassFactory.h: interface for the tLuaCOMClassFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLUACOMCLASSFACTORY_H__B2C2D8B2_FE51_4745_8C21_FFC29FE81F86__INCLUDED_)
#define AFX_TLUACOMCLASSFACTORY_H__B2C2D8B2_FE51_4745_8C21_FFC29FE81F86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "lua.h"
#include <unknwn.h> // IClassFactory

//
// LuaCOM Classfactory
//

class tLuaCOMClassFactory : public IClassFactory
{
public:
  // IUnknown methods 
  STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
  STDMETHOD_(ULONG, AddRef)(void);
  STDMETHOD_(ULONG, Release)(void); 
    
  // IClassFactory methods
  STDMETHOD(CreateInstance)(IUnknown* punkOuter, REFIID riid, 
                              void** ppv);
  STDMETHOD(LockServer)(BOOL fLock);    

	tLuaCOMClassFactory(IDispatch* object);
	tLuaCOMClassFactory(lua_State* L);
	virtual ~tLuaCOMClassFactory();

private:
  ULONG m_cRef;
  lua_State* L_inproc;
protected:
	IDispatch* object;
};

#endif // !defined(AFX_TLUACOMCLASSFACTORY_H__B2C2D8B2_FE51_4745_8C21_FFC29FE81F86__INCLUDED_)
