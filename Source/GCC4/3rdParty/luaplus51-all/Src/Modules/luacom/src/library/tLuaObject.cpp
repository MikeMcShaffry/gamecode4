// tLuaObject.cpp: implementation of the tLuaObject class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#include "tLuaObject.h"
#include "LuaAux.h"
#include "tLuaCOMException.h"
#include "luacom_internal.h"
#include "tUtil.h" 

extern "C"
{
#include "LuaCompat.h"
}

#ifndef MODULENAME
#define MODULENAME "__tLuaObject_object_types"
#endif

const char TLUAOBJECT_POINTER_FIELD[] = "__TLUAOBJECT_pointer__";


int tLuaObject::garbagecollect(lua_State *L)
{
  tLuaObject* lua_obj = (tLuaObject*) luaCompat_getTypedObject(L, -1);

  delete lua_obj;

  return 0;
}

tLuaObject* tLuaObject::getObject(lua_State *L, int pos)
{
  LUASTACK_SET(L);

  if(pos < 0)
    pos += lua_gettop(L) + 1;

  // gets lua object
  lua_pushstring(L, TLUAOBJECT_POINTER_FIELD);
  lua_rawget(L, pos);
  tLuaObject* lua_obj = (tLuaObject*) luaCompat_getTypedObject(L, -1);
  lua_pop(L, 1);

  LUASTACK_CLEAN(L, 0);

  return lua_obj;
}

int tLuaObject::generic_index(lua_State *L)
{
  tLuaObject* lua_obj = tLuaObject::getObject(L, index_table_param);

  // gets the field name
  const char* field_name = lua_tostring(L, index_index_param);

  // finds in the method table
  tMethodType method_type;
  tLuaObjectMethod method;

  bool found = lua_obj->method_table.FindMethod(field_name, &method, &method_type);
  if(!found)
    return lua_obj->index(L);

  // calls method
  switch(method_type)
  {
  case FUNC:
    luaCompat_pushPointer(L, (void *) method);
    lua_pushcclosure(L, closure, 1);
    return 1;
    break;

  default:
    return method(lua_obj, L);
    break;
  }
}

int tLuaObject::generic_newindex(lua_State *L)
{
  lua_rawset(L, -3);

  // must avoid redefinition of pointer field
  return 0;
}



int tLuaObject::generic_PushNew(lua_State* L,
                                tLuaObject* lua_obj,
                                const char* type_name,
                                const char* pointer_type_name
                                )
{
  LUASTACK_SET(L);

  // creates table
  lua_newtable(L);
  luaCompat_pushTypeByName(L, MODULENAME, type_name);

  luaCompat_setType(L, -2);

  lua_pushstring(L, TLUAOBJECT_POINTER_FIELD);

  // pushes typed pointer
  luaCompat_pushTypeByName(L, MODULENAME, pointer_type_name);

  luaCompat_newTypedObject(L, lua_obj);

  // stores in the table
  lua_settable(L, -3);

  LUASTACK_CLEAN(L, 1);

  return 1;
}

void tLuaObject::RegisterType(lua_State* L,
                              const char* type_name,
                              const char* pointer_type_name)
{
  LUASTACK_SET(L);

  luaCompat_moduleCreate(L, MODULENAME);
  lua_pop(L, 1);

  // Registers the table type and the pointer type
  luaCompat_newLuaType(L, MODULENAME, type_name);

  luaCompat_newLuaType(L, MODULENAME, pointer_type_name);

#ifdef LUA5
  // Registers the weak table to store the pairs
  // (pointer, LuaObject) to avoid duplication
  // of Lua objects. This step must be done
  // only once
  luaCompat_moduleGet(L, MODULENAME, INSTANCES_CACHE);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);
    // pushes tables
    lua_newtable(L);

    // pushes metatable and initializes it
    lua_newtable(L);
    lua_pushstring(L, "__mode");
    lua_pushstring(L, "v");
    lua_settable(L, -3);
    lua_setmetatable(L, -2);

    // stores in the registry
    luaCompat_moduleSet(L, MODULENAME, INSTANCES_CACHE);
  }
  else
    lua_pop(L, 1);
#endif  

  luaCompat_pushTypeByName(L, MODULENAME, type_name);

  lua_pushcfunction(L, tLuaObject::generic_index);
  luaCompat_handleNoIndexEvent(L);

  lua_pushcfunction(L, tLuaObject::generic_newindex);
  luaCompat_handleSettableEvent(L);

  lua_pop(L, 1);

  luaCompat_pushTypeByName(L, MODULENAME, pointer_type_name);

  lua_pushcfunction(L, tLuaObject::garbagecollect);
  luaCompat_handleGCEvent(L);

  lua_pop(L, 1);
  
  LUASTACK_CLEAN(L, 0);
}

int tLuaObject::index(lua_State* L)
{
  lua_rawget(L, -2);
  return 1;
}

int tLuaObject::newindex(lua_State* L)
{
  lua_rawset(L, -3);
  return 0;
}

int tLuaObject::closure(lua_State *L)
{
  tLuaObjectMethod method = (tLuaObjectMethod) 
    luaCompat_getPointer(L, luaCompat_upvalueIndex(L, 1, 1));

  tLuaObject* lua_obj = getObject(L, 1);

  if(lua_obj == NULL)
  {
    luacom_error(L, "Error: self parameter is not a tLuaObject.");
    return 0;
  }

  int retval = 0;

  try
  {
    retval = method(lua_obj, L);
  }
  catch(class tLuaCOMException& e)
  {
    luacom_error(L, e.getMessage());

    return 0;
  }

  return retval;
}


///////////////////////////////////////////
// tLuaObject::MethodTable
///////////////////////////////////////////

bool tLuaObject::MethodTable::Add(const char* name,
                                tLuaObjectMethod m,
                                tMethodType type)
{
  CHECKPRECOND(name && m);

  if(num_methods >= MAX_METHODS)
    return false;

  method_list[num_methods].name = name;
  method_list[num_methods].method = m;
  method_list[num_methods].type = type;

  num_methods++;

  return true;
}

bool tLuaObject::MethodTable::FindMethod(const char* name,
                                       tLuaObjectMethod* p_m,
                                       tMethodType* ptype)
{
  for(int i = 0; i < num_methods; i++)
  {
    if(strcmp(name, method_list[i].name) == 0)
    {
      *p_m = method_list[i].method;
      *ptype = method_list[i].type;

      return true;
    }
  }

  return false;
}

bool tLuaObject::MethodTable::GetNthMethod(long i,
                                           const char **name,
                                           tLuaObjectMethod* p_m,
                                           tMethodType* ptype)
{
  if(i < 0 || i >= num_methods)
    return false;
  else
  {
    *name = method_list[i].name;
    *p_m = method_list[i].method;
    *ptype = method_list[i].type;
  }

  return true;
}


tLuaObject::MethodTable::MethodTable()
{
  num_methods = 0;
}

bool tLuaObject::pushCachedObject(lua_State *L, void *pointer)
{
  LUASTACK_SET(L);

#ifdef LUA5
  luaCompat_moduleGet(L, MODULENAME, INSTANCES_CACHE);

  lua_pushlightuserdata(L, pointer);
  lua_gettable(L, -2);

  lua_remove(L, -2);

  if(lua_isnil(L, -1))
  {
    lua_pop(L, 1);

    LUASTACK_CLEAN(L, 0);
    return false;
  }

  LUASTACK_CLEAN(L, 1);

  return true;
#else
  return false;
#endif
}

void tLuaObject::cacheObject(lua_State *L, void* pointer)
{
  LUASTACK_SET(L);

#ifdef LUA5
  luaCompat_moduleGet(L, MODULENAME, INSTANCES_CACHE);

  lua_pushlightuserdata(L, pointer);
  lua_pushvalue(L, -3);
  lua_settable(L, -3);

  lua_remove(L, -1);
#endif

  LUASTACK_CLEAN(L, 0);
}
