/* windows.hkey module
require "windows.hkey"
local k1 = windows.hkey.HKEY_LOCAL_MACHINE:open("Software")
local k2 = k1:create("Sample")
k2:setvalue("Cheese", "Cheddar")
k2:setvalue("Fruit", 42)
local valuedata, valuetype = k2:queryvalue("Fruit")
print(valuedata, valuetype)
for keyname in k1:enumkeys() do print(keyname) end
for valuename in k2:enumvalues(false) do print(valuename) end
k2:deletevalue("Cheese")
for valuename, valuedata, valuetype in k2:enumvalues(true) do print(valuename, valuedata, valuetype) end
k2:close()
k1:delete("Sample")
*/

#include <assert.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#if LUA_VERSION_NUM < 501
#include "compat-5.1.h"
#define luaL_register(L,name,libs) luaL_module(L,name,libs,0)
#endif

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>

#include "pusherror.h"

#define nelemof(a) (sizeof(a)/sizeof*(a))

static void *
smalloc(size_t n)
{
	void *p = malloc(n);
	assert(p != 0);
	return p;
}

#define HKEYHANDLE "HKEY*"

/* create a new HKEYHANDLE userdatum */
static HKEY *
newkey(lua_State *L)
{
	HKEY *pk;
	pk = lua_newuserdata(L, sizeof *pk);
	*pk = 0;
	luaL_getmetatable(L, HKEYHANDLE);
	lua_setmetatable(L, -2);
	return pk;
}

/* verify that the given arg is an HKEYHANDLE userdatum */
static HKEY *
checkkey(lua_State *L, int idx)
{
	HKEY *pk;
	pk = luaL_checkudata(L, idx, HKEYHANDLE);
#if LUA_VERSION_NUM < 501
	if (pk == 0) luaL_typerror(L, idx, HKEYHANDLE);
#endif
	return pk;
}

/* k:open(subkeyname)
 * k:open(subkeyname, SAM) */
static int
hkey_open(lua_State *L)
{
	HKEY k, *pk;
	const char *subkey;
	REGSAM sam = KEY_ALL_ACCESS;
	LONG ret;
	k = *checkkey(L, 1);
	subkey = luaL_checkstring(L, 2);
	if (!lua_isnoneornil(L, 3))
		sam = (REGSAM)luaL_checknumber(L, 3);
	pk = newkey(L);
	ret = RegOpenKeyEx(k, subkey, 0, sam, pk);
	return windows_pusherror(L, ret, 1);
}

/* k:create(subkeyname)
 * k:create(subkeyname, SAM) */
static int
hkey_create(lua_State *L)
{
	HKEY k, *pk;
	const char *subkey;
	REGSAM sam = KEY_ALL_ACCESS;
	LONG ret;
	k = *checkkey(L, 1);
	subkey = luaL_checkstring(L, 2);
	if (!lua_isnoneornil(L, 3))
		sam = (REGSAM)luaL_checknumber(L, 3);
	pk = newkey(L);
	ret = RegCreateKeyEx(k, subkey, 0, 0, 0, sam, 0, pk, 0);
	return windows_pusherror(L, ret, 1);
}

/* k:close() */
static int
hkey_close(lua_State *L)
{
	HKEY *pk;
	LONG ret;
	pk = checkkey(L, 1);
	if (*pk) {
		ret = RegCloseKey(*pk);
		*pk = 0;
	}
	lua_pushboolean(L, 1);
	return windows_pusherror(L, ret, 1);
}

/* k:delete(subkeyname) */
static int
hkey_delete(lua_State *L)
{
	HKEY k;
	const char *subkey;
	LONG ret;
	k = *checkkey(L, 1);
	subkey = luaL_checkstring(L, 2);
	ret = RegDeleteKey(k, subkey);
	lua_pushboolean(L, 1);
	return windows_pusherror(L, ret, 1);
}

/* k:queryvalue(valuename) */
static int
hkey_queryvalue(lua_State *L)
{
	HKEY k;
	const char *vnam;
	DWORD type, datalen = 1024;
	char autobuf[1024];
	void *data;
	LONG ret;
	k = *checkkey(L, 1);
	vnam = luaL_checkstring(L, 2);
	data = lua_isnoneornil(L, 3) ? autobuf : 0;
	do {
		ret = RegQueryValueEx(k, vnam, 0, &type, data, &datalen);
	} while (ret == ERROR_MORE_DATA && data && (data = smalloc(datalen)));
	if (data) {
		if (ret == ERROR_SUCCESS) {
			switch (type) {
			case REG_DWORD: {
				const DWORD *num = data;
				lua_pushnumber(L, *num);
				break; }
			case REG_MULTI_SZ: /* return each string? */
			case REG_SZ:
			case REG_EXPAND_SZ: {
				const char *str = data;
				if (str[datalen - 1] == '\0')
					datalen--;
				/*FALLTHRU*/ }
			default:
				lua_pushlstring(L, data, datalen);
				break;
			}
		}
		if (data != autobuf)
			free(data);
	}
	lua_pushnumber(L, type);
	return windows_pusherror(L, ret, 2);
}

/* k:setvalue(valuename,valuedata)
 * k:setvalue(valuename,valuedata,valuetype) */
static int
hkey_setvalue(lua_State *L)
{
	HKEY k;
	const char *vnam;
	DWORD type;
	size_t datalen;
	const void *data;
	DWORD num;
	LONG ret;
	k = *checkkey(L, 1);
	vnam = luaL_checkstring(L, 2);
	switch (lua_type(L, 3)) {
		case LUA_TNONE:
		case LUA_TNIL:
			luaL_argerror(L, 3, "value expected");
			break;
		case LUA_TBOOLEAN:
			num = lua_toboolean(L, 3);
			data = &num;
			datalen = sizeof num;
			type = REG_DWORD;
			break;
		case LUA_TNUMBER:
			num = (REGSAM)lua_tonumber(L, 3);
			data = &num;
			datalen = sizeof num;
			type = REG_DWORD;
			break;
		case LUA_TSTRING:
			data = luaL_checklstring(L, 3, &datalen);
			type = REG_SZ;
			break;
		default:
			luaL_typerror(L, 3, "string, boolean, or number");
			break;
	}
	if (!lua_isnoneornil(L, 4))
		type = (REGSAM)luaL_checknumber(L, 4);
	ret = RegSetValueEx(k, vnam, 0, type, data, datalen);
	lua_pushboolean(L, 1);
	return windows_pusherror(L, ret, 1);
}

/* k:deletevalue(valuename) */
static int
hkey_deletevalue(lua_State *L)
{
	HKEY k;
	const char *vnam;
	LONG ret;
	k = *checkkey(L, 1);
	vnam = luaL_checkstring(L, 2);
	ret = RegDeleteValue(k, vnam);
	lua_pushboolean(L, 1);
	return windows_pusherror(L, ret, 1);
}

/* for keyname in k:enumkeys() do ; end */
struct enumkeys_iter_s {
	HKEY k;
	DWORD index;
};
static int
enumkeys_iter(lua_State *L)
{
	struct enumkeys_iter_s *state;
	char name[256];
	DWORD namelen;
	LONG ret;
	state = lua_touserdata(L, 1);
	namelen = sizeof name / sizeof *name;
	ret = RegEnumKeyEx(state->k, state->index++, name, &namelen, 0, 0, 0, 0);
	if (ret == ERROR_SUCCESS)
		lua_pushlstring(L, name, namelen);
	return windows_pusherror(L, ret, 1);
}
static int
hkey_enumkeys(lua_State *L)
{
	HKEY k;
	struct enumkeys_iter_s *state;
	k = *checkkey(L, 1);
	lua_pushcfunction(L, enumkeys_iter);
	state = lua_newuserdata(L, sizeof *state);
	state->k = k;
	state->index = 0;
	return 2;
}

/* for valuename, valuedata, valuetype in k:enumvalues() do ; end */
struct enumvalues_iter_s {
	HKEY k;
	DWORD index;
	size_t vnamlen;
	char *vnam;
	size_t datalen;
	void *data;
};
static int
enumvalues_iter(lua_State *L)
{
	struct enumvalues_iter_s *state;
	DWORD vnamlen, type, datalen;
	LONG ret;
	state = lua_touserdata(L, 1);
	if (state == 0) {
		char buf[40];
		sprintf(buf, "expected enumvalues iteration state not %s\n", lua_typename(L,lua_type(L, 1)));
		luaL_argerror(L,1,buf);
	}
	vnamlen = state->vnamlen;
	datalen = state->datalen;
	ret = RegEnumValue(state->k, state->index++,
		state->vnam, &vnamlen, 0,
		&type, state->data, &datalen);
	if (ret == ERROR_SUCCESS) {
		lua_pushlstring(L, state->vnam, vnamlen);
		lua_pushlstring(L, state->data, datalen);
		lua_pushnumber(L, type);
	}
	return windows_pusherror(L, ret, state->data ? 3 : 1);
}
static int
hkey_enumvalues(lua_State *L)
{
	HKEY k;
	struct enumvalues_iter_s *state;
	DWORD vnamlen, datalen;
	LONG ret;
	k = *checkkey(L, 1);
	lua_pushcfunction(L, enumvalues_iter);
	ret = RegQueryInfoKey(k, 0, 0, 0, 0, 0, 0, 0, &vnamlen, &datalen, 0, 0);
	if (ret == ERROR_SUCCESS) {
		vnamlen++; /* One more for the Gipper. */
		state = lua_newuserdata(L, sizeof *state + vnamlen + datalen);
		state->k = k;
		state->index = 0;
		state->vnamlen = vnamlen;
		state->vnam = (char *)&state[1];
		state->datalen = datalen;
		state->data = state->vnam + vnamlen;
	}
	return windows_pusherror(L, ret, 2);
}

/* tostring(k) */
static int
hkey_tostring(lua_State *L)
{
	HKEY k;
	char buf[20];
	k = *checkkey(L, 1);
	lua_pushlstring(L, buf, sprintf(buf, "HKEY:%p", (void *)k));
	return 1;
}


int luaopen_windows_hkey(lua_State *L)
{
	const luaL_reg hkey_lib[] = { {0,0} };
	const luaL_reg hkey_methods[] = {
		{ "open",           hkey_open },
		{ "create",         hkey_create },
		{ "close",          hkey_close },
		{ "delete",         hkey_delete },
		{ "queryvalue",     hkey_queryvalue },
		{ "setvalue",       hkey_setvalue },
		{ "deletevalue",    hkey_deletevalue },
		{ "enumkeys",       hkey_enumkeys },
		{ "enumvalues",     hkey_enumvalues },
		{ "__tostring",     hkey_tostring },
		{ "__gc",           hkey_close },
		{ 0, 0 }
	};
	const struct constant {
		const char *name;
		DWORD value;
	} *pconst, hkey_consts[] = {

	/* Registry Value Types */
		{ "REG_SZ",                  REG_SZ },
		{ "REG_EXPAND_SZ",           REG_EXPAND_SZ },
		{ "REG_DWORD",               REG_DWORD },
		{ "REG_QWORD",               REG_QWORD },
		{ "REG_BINARY",              REG_BINARY },
		{ "REG_NONE",                REG_NONE },
		{ "REG_DWORD_LITTLE_ENDIAN", REG_DWORD_LITTLE_ENDIAN },
		{ "REG_DWORD_BIG_ENDIAN",    REG_DWORD_BIG_ENDIAN },
		{ "REG_LINK",                REG_LINK },
		{ "REG_MULTI_SZ",            REG_MULTI_SZ },
		{ "REG_QWORD_LITTLE_ENDIAN", REG_QWORD_LITTLE_ENDIAN },

	/* Registry access rights  for open() and create() methods */
		{ "KEY_ALL_ACCESS",          KEY_ALL_ACCESS },
		{ "KEY_CREATE_LINK",         KEY_CREATE_LINK },
		{ "KEY_CREATE_SUB_KEY",      KEY_CREATE_SUB_KEY },
		{ "KEY_ENUMERATE_SUB_KEYS",  KEY_ENUMERATE_SUB_KEYS },
		{ "KEY_EXECUTE",             KEY_EXECUTE },
		{ "KEY_NOTIFY",              KEY_NOTIFY },
		{ "KEY_QUERY_VALUE",         KEY_QUERY_VALUE },
		{ "KEY_READ",                KEY_READ },
		{ "KEY_SET_VALUE",           KEY_SET_VALUE },
		{ "KEY_WRITE",               KEY_WRITE },
	/*
		{ "KEY_WOW64_64KEY",         KEY_WOW64_64KEY },
		{ "KEY_WOW64_32KEY",         KEY_WOW64_32KEY },
	*/
		{ 0, 0 }
	};
	const struct toplevel {
		const char *name;
		HKEY key;
	} *ptop, hkey_top[] = {
		{ "HKEY_CLASSES_ROOT",   HKEY_CLASSES_ROOT },
		{ "HKEY_CURRENT_CONFIG", HKEY_CURRENT_CONFIG },
		{ "HKEY_CURRENT_USER",   HKEY_CURRENT_USER },
		{ "HKEY_LOCAL_MACHINE",  HKEY_LOCAL_MACHINE },
		{ "HKEY_USERS",          HKEY_USERS },
		{ 0, 0 }
	};

	luaL_newmetatable(L, HKEYHANDLE);           /* mt */
	lua_pushliteral(L, "__index");              /* mt, "__index" */
	lua_pushvalue(L, -2);                       /* mt, "__index", mt */
	lua_settable(L, -3);                        /* mt */
	luaL_openlib(L, 0, hkey_methods, 0);        /* mt */

	luaL_register(L, "windows.hkey", hkey_lib);

	for (pconst = hkey_consts; pconst->name; pconst++) {
		lua_pushstring(L, pconst->name);        /* hkey, name */
		lua_pushnumber(L, pconst->value);       /* hkey, name, value */
		lua_settable(L, -3);                    /* hkey */
	}

	for (ptop = hkey_top; ptop->name; ptop++) {
		lua_pushstring(L, ptop->name);          /* hkey, name */
		*newkey(L) = ptop->key;                 /* hkey, name, key */
		lua_settable(L, -3);                    /* hkey */
	}
	return 1;
}

