/*
* luuid.c
* uuid interface for Lua 5.1
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 03 Mar 2010 00:01:13
* This code is hereby placed in the public domain.
*/

#ifdef _WIN32
#include "wuuid.h"
#else
#include <uuid/uuid.h>
#endif

#if 0	/* use these if you get link errors (unlikely) */
#define	uuid_generate_random	uuid_generate
#define	uuid_generate_time	uuid_generate
#define	uuid_time(c,p)		(-1)
#endif

#include "lua.h"
#include "lauxlib.h"

#define MYNAME		"uuid"
#define MYVERSION	MYNAME " library for " LUA_VERSION " / Mar 2010"

static int Lnew(lua_State *L)			/** new([s]) */
{
 uuid_t c;
 char s[2*sizeof(c)+4+1];
 const char *t=luaL_optstring(L,1,NULL);
 if (t==NULL) uuid_generate(c); else
 if (*t=='r') uuid_generate_random(c); else
 if (*t=='t') uuid_generate_time(c); else uuid_generate(c);
 uuid_unparse(c,s);
 lua_pushlstring(L,s,sizeof(s)-1);
 return 1;
}

static int Lisvalid(lua_State *L)		/** isvalid(s) */
{
 uuid_t c;
 const char *s=luaL_checkstring(L,1);
 lua_pushboolean(L,uuid_parse(s,c)==0);
 return 1;
}

static int Ltime(lua_State *L)			/** time(s) */
{
 uuid_t c;
 const char *s=luaL_checkstring(L,1);
 if (uuid_parse(s,c)!=0) return 0;
 lua_pushnumber(L,uuid_time(c,NULL));
 return 1;
}

static int Lparse(lua_State *L)		   /** parse(uuidstring) */
{
 uuid_t c;
 const char *s=luaL_checkstring(L,1);
 if (uuid_parse(s,c))
   return 0;
 lua_pushlstring(L,(const char*)c,sizeof(c));
 return 1;
}

static int Lunparse(lua_State *L)		/** unparse(uuidbuffer) */
{
 char s[2*sizeof(uuid_t)+4+1];
 size_t len;
 uuid_t *c=luaL_checklstring(L,1,&len);
 if (len != 16)
   return 0;
 uuid_unparse(*c,s);
 lua_pushlstring(L,s,sizeof(s)-1);
 return 1;
}

static const luaL_reg R[] =
{
	{ "isvalid",	Lisvalid	},
	{ "new",	Lnew		},
	{ "time",	Ltime		},
	{ "parse",	Lparse		},
	{ "unparse",Lunparse	},
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_uuid(lua_State *L)
{
 luaL_register(L,MYNAME,R);
 lua_pushliteral(L,"version");			/** version */
 lua_pushliteral(L,MYVERSION);
 lua_settable(L,-3);
 return 1;
}
