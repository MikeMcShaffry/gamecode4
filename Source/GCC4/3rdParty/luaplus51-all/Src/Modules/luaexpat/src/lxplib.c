/*
** $Id: lxplib.c,v 1.17 2008-12-11 19:05:13 carregal Exp $
** LuaExpat: Lua bind for Expat library
** See Copyright Notice in license.html
*/


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "expat.h"

#include "lua.h"
#include "lauxlib.h"
#if ! defined (LUA_VERSION_NUM) || LUA_VERSION_NUM < 501
#include "compat-5.1.h"
#endif


#include "lxplib.h"



enum XPState {
  XPSpre,  /* parser just initialized */
  XPSok,   /* state while parsing */
  XPSfinished,  /* state after finished parsing */
  XPSerror,
  XPSstring  /* state while reading a string */
};

struct lxp_userdata {
  lua_State *L;
  XML_Parser parser;  /* associated expat parser */
  int busy; /* true while parsing, used to avoid calling XML_Parse() recursively */
  int tableref;  /* table with callbacks for this parser */
  enum XPState state;
  luaL_Buffer *b;  /* to concatenate sequences of cdata pieces */
};

typedef struct lxp_userdata lxp_userdata;


static int reporterror (lxp_userdata *xpu) {
  lua_State *L = xpu->L;
  XML_Parser p = xpu->parser;
  lua_pushnil(L);
  lua_pushstring(L, XML_ErrorString(XML_GetErrorCode(p)));
  lua_pushnumber(L, XML_GetCurrentLineNumber(p));
  lua_pushnumber(L, XML_GetCurrentColumnNumber(p) + 1);
  lua_pushnumber(L, XML_GetCurrentByteIndex(p) + 1);
  return 5;
}


static lxp_userdata *createlxp (lua_State *L) {
  lxp_userdata *xpu = (lxp_userdata *)lua_newuserdata(L, sizeof(lxp_userdata));
  memset(xpu, 0, sizeof(*xpu));
  xpu->tableref = LUA_REFNIL;  /* in case of errors... */
  xpu->state = XPSpre;
  luaL_getmetatable(L, ParserType);
  lua_setmetatable(L, -2);
  return xpu;
}


static void lxpclose (lua_State *L, lxp_userdata *xpu) {
  lua_unref(L, xpu->tableref);
  xpu->tableref = LUA_REFNIL;
  if (xpu->parser)
    XML_ParserFree(xpu->parser);
  xpu->parser = NULL;
}




/*
** Auxiliary function to call a Lua handle
*/
static void docall (lxp_userdata *xpu, int nargs, int nres) {
  lua_State *L = xpu->L;
  assert(xpu->state == XPSok);
  if (lua_pcall(L, nargs + 1, nres, 0) != 0) {
    xpu->state = XPSerror;
    luaL_unref(L, LUA_REGISTRYINDEX, xpu->tableref);
    xpu->tableref = luaL_ref(L, LUA_REGISTRYINDEX);  /* error message */
  }
}


/*
** Check whether there is pending Cdata, and call its handle if necessary
*/
static void dischargestring (lxp_userdata *xpu) {
  assert(xpu->state == XPSstring);
  xpu->state = XPSok;
  luaL_pushresult(xpu->b);
  docall(xpu, 1, 0);
}


/*
** Check whether there is a Lua handle for a given event: If so,
** put it on the stack (to be called later), and also push `self'
*/
static int getHandle (lxp_userdata *xpu, const char *handle) {
  lua_State *L = xpu->L;
  if (xpu->state == XPSstring) dischargestring(xpu);
  if (xpu->state == XPSerror)
    return 0;  /* some error happened before; skip all handles */
  lua_pushstring(L, handle);
  lua_gettable(L, 3);
  if (lua_toboolean(L, -1) == 0) {
    lua_pop(L, 1);
    return 0;
  }
  if (!lua_isfunction(L, -1)) {
    luaL_error(L, "lxp `%s' callback is not a function", handle);
  }
  lua_pushvalue(L, 1);  /* first argument in every call (self) */
  return 1;
}



/*
** {======================================================
** Handles
** =======================================================
*/


static void f_StartCdata (void *ud) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (getHandle(xpu, StartCdataKey) == 0) return;  /* no handle */
  docall(xpu, 0, 0);
}


static void f_EndCdataKey (void *ud) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (getHandle(xpu, EndCdataKey) == 0) return;  /* no handle */
  docall(xpu, 0, 0);
}


static void f_CharData (void *ud, const char *s, int len) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (xpu->state == XPSok) {
    if (getHandle(xpu, CharDataKey) == 0) return;  /* no handle */
    xpu->state = XPSstring;
    luaL_buffinit(xpu->L, xpu->b);
  }
  if (xpu->state == XPSstring)
    luaL_addlstring(xpu->b, s, len);
}


static void f_Comment (void *ud, const char *data) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (getHandle(xpu, CommentKey) == 0) return;  /* no handle */
  lua_pushstring(xpu->L, data);
  docall(xpu, 1, 0);
}


static void f_Default (void *ud, const char *data, int len) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (getHandle(xpu, DefaultKey) == 0) return;  /* no handle */
  lua_pushlstring(xpu->L, data, len);
  docall(xpu, 1, 0);
}


static void f_DefaultExpand (void *ud, const char *data, int len) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (getHandle(xpu, DefaultExpandKey) == 0) return;  /* no handle */
  lua_pushlstring(xpu->L, data, len);
  docall(xpu, 1, 0);
}


static void f_StartElement (void *ud, const char *name, const char **attrs) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;
  int lastspec = XML_GetSpecifiedAttributeCount(xpu->parser) / 2;
  int i = 1;
  if (getHandle(xpu, StartElementKey) == 0) return;  /* no handle */
  lua_pushstring(L, name);
  lua_newtable(L);
  while (*attrs) {
    if (i <= lastspec) {
      lua_pushnumber(L, i++);
      lua_pushstring(L, *attrs);
      lua_settable(L, -3);
    }
    lua_pushstring(L, *attrs++);
    lua_pushstring(L, *attrs++);
    lua_settable(L, -3);
  }
  docall(xpu, 2, 0);  /* call function with self, name, and attributes */
}


static void f_EndElement (void *ud, const char *name) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (getHandle(xpu, EndElementKey) == 0) return;  /* no handle */
  lua_pushstring(xpu->L, name);
  docall(xpu, 1, 0);
}


static int f_ExternaEntity (XML_Parser p, const char *context,
                                          const char *base,
                                          const char *systemId,
                                          const char *publicId) {
  lxp_userdata *xpu = (lxp_userdata *)XML_GetUserData(p);
  lua_State *L = xpu->L;
  lxp_userdata *child;
  int status;
  if (getHandle(xpu, ExternalEntityKey) == 0) return 1;  /* no handle */
  child = createlxp(L);
  child->parser = XML_ExternalEntityParserCreate(p, context, NULL);
  if (!child->parser)
    luaL_error(L, "XML_ParserCreate failed");
  lua_getref(L, xpu->tableref);  /* child uses the same table of its father */
  child->tableref = luaL_ref(L, LUA_REGISTRYINDEX);
  lua_pushstring(L, base);
  lua_pushstring(L, systemId);
  lua_pushstring(L, publicId);
  docall(xpu, 4, 1);
  status = lua_toboolean(L, -1);
  lua_pop(L, 1);
  lxpclose(L, child);
  return status;
}


static void f_StartNamespaceDecl (void *ud, const char *prefix,
                                            const char *uri) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;
  if (getHandle(xpu, StartNamespaceDeclKey) == 0) return;  /* no handle */
  lua_pushstring(L, prefix);
  lua_pushstring(L, uri);
  docall(xpu, 2, 0);
}


static void f_EndNamespaceDecl (void *ud, const char *prefix) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  if (getHandle(xpu, EndNamespaceDeclKey) == 0) return;  /* no handle */
  lua_pushstring(xpu->L, prefix);
  docall(xpu, 1, 0);
}


static void f_NotationDecl (void *ud, const char *notationName,
                                      const char *base,
                                      const char *systemId,
                                      const char *publicId) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;
  if (getHandle(xpu, NotationDeclKey) == 0) return;  /* no handle */
  lua_pushstring(L, notationName);
  lua_pushstring(L, base);
  lua_pushstring(L, systemId);
  lua_pushstring(L, publicId);
  docall(xpu, 4, 0);
}


static int f_NotStandalone (void *ud) {
  int status;
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;
  if (getHandle(xpu, NotStandaloneKey) == 0) return 1;  /* no handle */
  docall(xpu, 0, 1);
  status = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return status;
}


static void f_ProcessingInstruction (void *ud, const char *target,
                                               const char *data) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;
  if (getHandle(xpu, ProcessingInstructionKey) == 0) return;  /* no handle */
  lua_pushstring(L, target);
  lua_pushstring(L, data);
  docall(xpu, 2, 0);
}


static void f_UnparsedEntityDecl (void *ud, const char *entityName,
                                            const char *base,
                                            const char *systemId,
                                            const char *publicId,
                                            const char *notationName) {
  lxp_userdata *xpu = (lxp_userdata *)ud;
  lua_State *L = xpu->L;
  if (getHandle(xpu, UnparsedEntityDeclKey) == 0) return;  /* no handle */
  lua_pushstring(L, entityName);
  lua_pushstring(L, base);
  lua_pushstring(L, systemId);
  lua_pushstring(L, publicId);
  lua_pushstring(L, notationName);
  docall(xpu, 5, 0);
}

/* }====================================================== */



static int hasfield (lua_State *L, const char *fname) {
  int res;
  lua_pushstring(L, fname);
  lua_gettable(L, 1);
  res = !lua_isnil(L, -1);
  lua_pop(L, 1);
  return res;
}


static void checkcallbacks (lua_State *L) {
  static const char *const validkeys[] = {
    "StartCdataSection", "EndCdataSection", "CharacterData", "Comment",
    "Default", "DefaultExpand", "StartElement", "EndElement",
    "ExternalEntityRef", "StartNamespaceDecl", "EndNamespaceDecl",
    "NotationDecl", "NotStandalone", "ProcessingInstruction",
    "UnparsedEntityDecl", NULL};
  if (hasfield(L, "_nonstrict")) return;
  lua_pushnil(L);
  while (lua_next(L, 1)) {
    lua_pop(L, 1);  /* remove value */
#if ! defined (LUA_VERSION_NUM) || LUA_VERSION_NUM < 501
    if (lua_type(L, -1) != LUA_TSTRING ||
        luaL_findstring(lua_tostring(L, -1), validkeys) < 0)
      luaL_error(L, "invalid key `%s' in callback table", lua_tostring(L, -1));
#else
    luaL_checkoption(L, -1, NULL, validkeys);
#endif
  }
}


static int lxp_make_parser (lua_State *L) {
  XML_Parser p;
  char sep = *luaL_optstring(L, 2, "");
  lxp_userdata *xpu = createlxp(L);
  p = xpu->parser = (sep == '\0') ? XML_ParserCreate(NULL) :
                                    XML_ParserCreateNS(NULL, sep);
  if (!p)
    luaL_error(L, "XML_ParserCreate failed");
  luaL_checktype(L, 1, LUA_TTABLE);
  checkcallbacks(L);
  lua_pushvalue(L, 1);
  xpu->tableref = luaL_ref(L, LUA_REGISTRYINDEX);
  XML_SetUserData(p, xpu);
  if (hasfield(L, StartCdataKey) || hasfield(L, EndCdataKey))
    XML_SetCdataSectionHandler(p, f_StartCdata, f_EndCdataKey);
  if (hasfield(L, CharDataKey))
    XML_SetCharacterDataHandler(p, f_CharData);
  if (hasfield(L, CommentKey))
    XML_SetCommentHandler(p, f_Comment);
  if (hasfield(L, DefaultKey))
    XML_SetDefaultHandler(p, f_Default);
  if (hasfield(L, DefaultExpandKey))
    XML_SetDefaultHandlerExpand(p, f_DefaultExpand);
  if (hasfield(L, StartElementKey) || hasfield(L, EndElementKey))
    XML_SetElementHandler(p, f_StartElement, f_EndElement);
  if (hasfield(L, ExternalEntityKey))
    XML_SetExternalEntityRefHandler(p, f_ExternaEntity);
  if (hasfield(L, StartNamespaceDeclKey) || hasfield(L, EndNamespaceDeclKey))
    XML_SetNamespaceDeclHandler(p, f_StartNamespaceDecl, f_EndNamespaceDecl);
  if (hasfield(L, NotationDeclKey))
    XML_SetNotationDeclHandler(p, f_NotationDecl);
  if (hasfield(L, NotStandaloneKey))
    XML_SetNotStandaloneHandler(p, f_NotStandalone);
  if (hasfield(L, ProcessingInstructionKey))
    XML_SetProcessingInstructionHandler(p, f_ProcessingInstruction);
  if (hasfield(L, UnparsedEntityDeclKey))
    XML_SetUnparsedEntityDeclHandler(p, f_UnparsedEntityDecl);
  return 1;
}


static lxp_userdata *checkparser (lua_State *L, int idx) {
  lxp_userdata *xpu = (lxp_userdata *)luaL_checkudata(L, idx, ParserType);
  luaL_argcheck(L, xpu, idx, "expat parser expected");
  luaL_argcheck(L, xpu->parser, idx, "parser is closed");
  return xpu;
}


static int parser_gc (lua_State *L) {
  lxp_userdata *xpu = (lxp_userdata *)luaL_checkudata(L, 1, ParserType);
  luaL_argcheck(L, xpu, 1, "expat parser expected");
  lxpclose(L, xpu);
  return 0;
}


static int setbase (lua_State *L) {
  lxp_userdata *xpu = checkparser(L, 1);
  if (XML_SetBase(xpu->parser, luaL_checkstring(L, 2)) == 0)
    luaL_error(L, "no memory to store base");
  return 0;
}


static int getbase (lua_State *L) {
  lxp_userdata *xpu = checkparser(L, 1);
  lua_pushstring(L, XML_GetBase(xpu->parser));
  return 1;
}


static int getcallbacks (lua_State *L) {
  lxp_userdata *xpu = checkparser(L, 1);
  lua_rawgeti(L, LUA_REGISTRYINDEX, xpu->tableref);
  return 1;
}


static int parse_aux (lua_State *L, lxp_userdata *xpu, const char *s,
                      size_t len) {
  luaL_Buffer b;
  int status;
  xpu->L = L;
  xpu->state = XPSok;
  xpu->b = &b;
  lua_settop(L, 2);
  lua_getref(L, xpu->tableref);  /* to be used by handlers */
  xpu->busy = 1;
  status = XML_Parse(xpu->parser, s, (int)len, s == NULL);
  xpu->busy = 0;
  if (xpu->state == XPSstring) dischargestring(xpu);
  if (xpu->state == XPSerror) {  /* callback error? */
    lua_rawgeti(L, LUA_REGISTRYINDEX, xpu->tableref);  /* get original msg. */
    lua_error(L);
  }
  if (s == NULL) xpu->state = XPSfinished;
  if (status) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else { /* error */
    return reporterror(xpu);
  }
}


static int lxp_parse (lua_State *L) {
  lxp_userdata *xpu = checkparser(L, 1);
  size_t len;
  const char *s = luaL_optlstring(L, 2, NULL, &len);
  luaL_argcheck(L, !xpu->busy, 1, "expat parser is busy");
  if (xpu->state == XPSfinished && s != NULL) {
    lua_pushnil(L);
    lua_pushliteral(L, "cannot parse - document is finished");
    return 2;
  }
  return parse_aux(L, xpu, s, len);
}


static int lxp_close (lua_State *L) {
  int status = 1;
  lxp_userdata *xpu = (lxp_userdata *)luaL_checkudata(L, 1, ParserType);
  luaL_argcheck(L, xpu, 1, "expat parser expected");
  luaL_argcheck(L, !xpu->busy, 1, "expat parser is busy");
  if (xpu->state != XPSfinished)
    status = parse_aux(L, xpu, NULL, 0);
  lxpclose(L, xpu);
  if (status > 1) luaL_error(L, "error closing parser: %s",
                                lua_tostring(L, -status+1));
  return 0;
}


static int lxp_pos (lua_State *L) {
  lxp_userdata *xpu = checkparser(L, 1);
  XML_Parser p = xpu->parser;
  lua_pushnumber(L, XML_GetCurrentLineNumber(p));
  lua_pushnumber(L, XML_GetCurrentColumnNumber(p) + 1);
  lua_pushnumber(L, XML_GetCurrentByteIndex(p) + 1);
  return 3;
}


static int lxp_setencoding (lua_State *L) {
  lxp_userdata *xpu = checkparser(L, 1);
  const char *encoding = luaL_checkstring(L, 2);
  luaL_argcheck(L, xpu->state == XPSpre, 1, "invalid parser state");
  XML_SetEncoding(xpu->parser, encoding);
  return 0;
}


static const struct luaL_reg lxp_meths[] = {
  {"parse", lxp_parse},
  {"close", lxp_close},
  {"__gc", parser_gc},
  {"pos", lxp_pos},
  {"setencoding", lxp_setencoding},
  {"getcallbacks", getcallbacks},
  {"getbase", getbase},
  {"setbase", setbase},
  {NULL, NULL}
};

static const struct luaL_reg lxp_funcs[] = {
  {"new", lxp_make_parser},
  {NULL, NULL}
};


/*
** Assumes the table is on top of the stack.
*/
static void set_info (lua_State *L) {
	lua_pushliteral (L, "_COPYRIGHT");
	lua_pushliteral (L, "Copyright (C) 2003-2007 Kepler Project");
	lua_settable (L, -3);
	lua_pushliteral (L, "_DESCRIPTION");
	lua_pushliteral (L, "LuaExpat is a SAX XML parser based on the Expat library");
	lua_settable (L, -3);
	lua_pushliteral (L, "_VERSION");
	lua_pushliteral (L, "LuaExpat 1.1.0");
	lua_settable (L, -3);
}


int luaopen_lxp (lua_State *L) {
  luaL_newmetatable(L, ParserType);
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);
  lua_rawset(L, -3);
  luaL_openlib (L, NULL, lxp_meths, 0);
  luaL_openlib (L, "lxp", lxp_funcs, 0);
  set_info (L);

  return 1;
}
