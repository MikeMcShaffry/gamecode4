// tLuaTLB.h: interface for the tLuaTLB class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _TLUATLB_H
#define _TLUATLB_H

#include <ocidl.h>

#include "tLuaObject.h"
#include "tLuaCOMTypeHandler.h"


class tLuaTLB : public tLuaObject  
{
public:
	static int pushNew(lua_State* L, ITypeLib* p_typelib);
  static void Init(lua_State* L);

protected:
	int index(lua_State* L);
  ITypeLib* typelib;

  static const char type_name[];
  static const char pointer_type_name[];

  static int ShowHelp(tLuaObject* lua_obj, lua_State* L);
  static int GetDocumentation(tLuaObject* lua_obj, lua_State* L);
  static int GetTypeInfo(tLuaObject* lua_obj, lua_State* L);
  static int GetTypeInfoCount(tLuaObject* lua_obj, lua_State* L);
  static int ExportConstants(tLuaObject* lua_obj, lua_State* L);
  static int ExportEnumerations(tLuaObject* lua_obj, lua_State* L);

	tLuaTLB(lua_State* L, ITypeLib *p_typelib);
	virtual ~tLuaTLB();
public:
  int GetDocumentation(void);
};



//
// tLuaTypeInfo: generic Lua proxy for ITypeInfo objects
//

class tLuaTypeInfo: public tLuaObject
{
public:
  static void pushNew(lua_State* L, ITypeInfo* p_typeinfo);
  static void Init(lua_State* L);

protected:
	static int GetTypeLib(tLuaObject* lua_obj, lua_State* L);
	static int GetFuncDesc(tLuaObject* lua_obj, lua_State* L);
  static int GetVarDesc(tLuaObject* lua_obj, lua_State* L);
	static int GetDocumentation(tLuaObject* lua_obj, lua_State* L);
	static int GetTypeAttr(tLuaObject* lua_obj, lua_State* L);
  static int GetImplType(tLuaObject* lua_obj, lua_State* L);
  static int GetImplTypeFlags(tLuaObject* lua_obj, lua_State* L);

  ITypeInfo* typeinfo;
  tLuaCOMTypeHandler* typehandler;

  static const char type_name[];
  static const char pointer_type_name[];

	tLuaTypeInfo(lua_State* L,
               ITypeInfo *p_typeinfo
               );

	virtual ~tLuaTypeInfo();
};

#endif // _TLUATLB_H
