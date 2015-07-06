/*
 *  luacom_internal.h
 *
 *    Global definitions for the LuaCOM library
 */

// Here we define the module name we'll use in Lua
// to be used in the registry

#ifndef __LUACOM_INTERNAL_H
#define __LUACOM_INTERNAL_H

#include "luabeans.h"

#define MODULENAME "__LuaCOM__"


// This is the name of the configuration table inside
// the library table

#define CONFIGTABLE_NAME "config"


// Here is the libname, that is, the name used to access
// luacom methods

#define LIBNAME "luacom"


// keys in the configuration table used by luacom
// internally

#define LCOM_LUACOM_TYPENAME "LuaCOM"
#define LCOM_IUNKNOWN_TYPENAME "IUnknown"
#define LCOM_CONNPOINT_TYPENAME "IConnectionPoint"


// names of the configuration keys

#define LUACOM_SHOULD_ABORT "abort_on_error"
#define LUACOM_SHOULD_ABORT_API "abort_on_API_error"
#define LUACOM_LAST_ERROR "last_error"


#define LCOM_IUNKNOWN_ATTRIBUTE "IUnknown"


void luacom_error(lua_State* L, const char* message);

LuaBeans *getLuaBeans(lua_State *L);

extern HINSTANCE g_hInstance;
extern HWND     g_hwndParking;

#endif // __LUACOM_INTERNAL_H
