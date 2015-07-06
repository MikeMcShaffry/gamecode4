/******************************************************************************
* Copyright (C) 2007 Juergen Hoetzel
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#ifndef LUACURL_H
#define LUACURL_H

/* lua specific */
#include <lua.h>
#include <lauxlib.h>

/* curl specific */
#include <curl/curl.h>
#include <curl/easy.h>

/* custom metatables */
#define LUACURL_EASYMETATABLE "CURL.easy"
#define LUACURL_MULTIMETATABLE "CURL.multi"
#define LUACURL_SHAREMETATABLE "CURL.shared"
#define LUACURL_REGISTRYPREFIX "Lua-cURL_private"

#define MULTIREGISTRY_KEY "_LUA_CURL"

/* custom macros */
#define LUACURL_CHECKEASY(L) (CURL *) luaL_checkudata(L, 1, LUACURL_EASYMETATABLE)
#define LUACURL_OPTIONP_UPVALUE(L, INDEX) ((CURLoption *) lua_touserdata(L, lua_upvalueindex(INDEX)))
#define LUACURL_INFOP_UPVALUE(L, INDEX) ((CURLINFO *) lua_touserdata(L, lua_upvalueindex(INDEX)))

typedef struct l_option_slist {
    CURLoption option;
    struct curl_slist *slist;
} l_option_slist;

typedef struct l_easy_private {
  CURL *curl; 
  char error[CURL_ERROR_SIZE];
  
  /* slists, allocated by l_easy_setopt_strings */
  l_option_slist *option_slists;
} l_easy_private;

/* Lua closures (CURL* upvalue) */
int l_tostring (lua_State *L);

/* setopt closures */
int l_easy_opt_long (lua_State *L);
int l_easy_opt_string (lua_State *L);

/* easy interface */
int l_easy_escape (lua_State *L);
int l_easy_init (lua_State *L);
int l_easy_perform (lua_State *L);
int l_easy_unescape (lua_State *L);
int l_easy_post(lua_State *L);
 int l_easy_userdata(lua_State *L);

/* multi interface */
int l_multi_init (lua_State *L);
int l_multi_add_handle (lua_State *L);
int l_multi_perform (lua_State *L);
int l_multi_gc (lua_State *L);

/* shared interface */
int l_share_init (lua_State *L);
int l_share_setopt_share(lua_State *L);
int l_share_gc (lua_State *L);

/* subtable creation */
int l_easy_getinfo_register (lua_State *L);
int l_easy_setopt_register (lua_State *L);
int l_easy_callback_newtable(lua_State *L);

/* init private list of curl_slists */
void  l_easy_setopt_init_slists(lua_State *L, l_easy_private *privp);
void l_easy_setopt_free_slists(l_easy_private *privp);

/* setup callback function */
int l_easy_setup_writefunction(lua_State *L, CURL* curl);
int l_easy_setup_headerfunction(lua_State *L, CURL* curl);
int l_easy_setup_readfunction(lua_State *L, CURL* curl);
int l_easy_clear_headerfunction(lua_State *L, CURL* curl);
int l_easy_clear_writefunction(lua_State *L, CURL* curl);
int l_easy_clear_readfunction(lua_State *L, CURL* curl);

/* Lua module functions */
int l_easy_init (lua_State *L);
int l_getdate (lua_State *L);
int l_unescape (lua_State *L);
int l_version (lua_State *L);
int l_version_info (lua_State *L);

/* Lua metatable functions */
int l_tostring (lua_State *L);
int l_easy_gc (lua_State *L);
int l_setopt(lua_State *L);
int l_getopt(lua_State *L);

#endif
