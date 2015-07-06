/*
 * ttLuaCOM.h
 */


#ifndef __LUACOM_H
#define __LUACOM_H

#include <ole2.h>
#include <ocidl.h> 

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
}

#include "luabeans.h"
#include "tLuaCOMTypeHandler.h"

#include "tLuaObjList.h"

enum tWhichInterface {DISP, SOURCE};

// Types of FUNCDESC

#define MAX_FUNCINFOS 150

struct FuncInfo
{
  char*     name;

  FUNCDESC* propget;
  FUNCDESC* propput;
  FUNCDESC* func;
};


class tLuaCOM
{
public:
	bool hasTypeInfo(void);
	int Lock(void);
  int Unlock(void);
	ITypeInfo* GetTypeInfo(void);
	CLSID GetCLSID(void);
	void GetIID(IID *piid);
	IDispatch * GetIDispatch(void);
	void ReleaseFuncDesc(FUNCDESC *pfuncdesc);
	ITypeInfo * GetDefaultEventsInterface(void);
  static tLuaCOM * CreateLuaCOM(
    lua_State* L,
    IDispatch * pdisp,
    const CLSID& coclass = IID_NULL,
    ITypeInfo* typeinfo=NULL,
    bool untyped = false
    );

	void getHelpInfo(char **ppHelpFile, DWORD *pHelpContext);
	bool isMember(const char *name);

   ~tLuaCOM();

   DWORD addConnection(tLuaCOM *client);
   void releaseConnection(tLuaCOM* server, DWORD cookie);
   void releaseConnection();
   void releaseConnections(void);

   int call(
     lua_State* L, 
     DISPID dispid,
     int invkind,
     FUNCDESC *pFuncDesc,
     tLuaObjList params);

   bool getFUNCDESC(const char* name, FuncInfo& funcinfo);
   bool getConstant(lua_State* L, const char* name);
   bool getDISPID(const char* name, DISPID* dispid);

  static long NEXT_ID;
protected:
	ITypeComp* ptcomp;
  ITypeComp* plib_tcomp;
	long lock_count;
  CLSID clsid;
  IConnectionPoint *conn_point;
  DWORD conn_cookie;

  tLuaCOM(
    lua_State* L,
    IDispatch *pdisp_arg,
    ITypeInfo *ptinfo_arg,
    CLSID coclass
    );

	tLuaCOMTypeHandler * typehandler;


  LPDISPATCH pdisp;
  ITypeInfo* ptinfo;

  // struct to hold all type information for a
  // member

  FuncInfo pFuncInfo[MAX_FUNCINFOS];
private:
	long ID;
};

#endif // __LUACOM_H
