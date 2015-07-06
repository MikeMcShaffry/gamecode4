// tLuaObject.h: interface for the tLuaObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLUAOBJECT_H__459DE88B_29C0_4245_AEA7_122F2749F202__INCLUDED_)
#define AFX_TLUAOBJECT_H__459DE88B_29C0_4245_AEA7_122F2749F202__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern "C"
{
#include <lua.h>
}

#define index_table_param -2
#define index_index_param -1

#define newindex_table_param -3
#define newindex_index_param -2
#define newindex_value_param -1


#define INSTANCES_CACHE "__luacom_instances_cache"


class tLuaObject;

typedef int (*tLuaObjectMethod) (tLuaObject* lua_object, lua_State *L);

typedef enum
{
  GET,
  SET,
  FUNC
} tMethodType;

#define MAX_METHODS 40

class tLuaObject
{
public:
	static void cacheObject(lua_State* L, void* pointer);
	static bool pushCachedObject(lua_State* L, void* pointer);

  class MethodTable
  {
  public:
    MethodTable();
    bool Add(const char* name, tLuaObjectMethod m, tMethodType type);

    bool FindMethod(const char* name,
                    tLuaObjectMethod* p_m,
                    tMethodType* ptype);

    bool GetNthMethod(long i,
                      const char **name,
                      tLuaObjectMethod* p_m,
                      tMethodType* ptype);


  protected:
    struct tMethodList
    {
      const char* name;
      tLuaObjectMethod method;
      tMethodType type;
    } method_list[MAX_METHODS];

    long num_methods;
  } method_table;

protected:
	static int closure(lua_State* L);
	virtual int newindex(lua_State* L);
	virtual int index(lua_State* L);

  static void RegisterType(lua_State* L,
                           const char* type_name,
                           const char* pointer_type_name
                           );

  static int generic_PushNew(lua_State* L,
                             tLuaObject* lua_obj,
                             const char* type_name,
                             const char* pointer_type_name
                             );

  static tLuaObject* getObject(lua_State* L, int pos);

private:
	static int garbagecollect(lua_State* L);
	static int generic_index(lua_State* L);
	static int generic_newindex(lua_State* L);
};



#endif // !defined(AFX_TLUAOBJECT_H__459DE88B_29C0_4245_AEA7_122F2749F202__INCLUDED_)
