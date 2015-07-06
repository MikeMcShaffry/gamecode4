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

#include <curl/curl.h>


#include "Lua-cURL.h"
#include "Lua-cURL-share.h"
#include "Lua-utility.h"

static l_share_userdata* l_share_newuserdata(lua_State *L) { 
  l_share_userdata *share_userdata = (l_share_userdata *) lua_newuserdata(L, sizeof(l_share_userdata));
  luaL_getmetatable(L, LUACURL_SHAREMETATABLE);
  lua_setmetatable(L, -2);
  return share_userdata;
}

int l_share_init(lua_State *L) {
  
  l_share_userdata *share_userdata = l_share_newuserdata(L);
  if ((share_userdata->curlsh = curl_share_init()) == NULL)
    luaL_error(L, "something went wrong and you cannot use the other curl functions");
  /* return userdata */
  return 1;			
}

int l_share_setopt_share(lua_State *L) {
  l_share_userdata *privatep = luaL_checkudata(L, 1, LUACURL_SHAREMETATABLE);
  CURLSH *curlsh = privatep->curlsh;
  const char *value = luaL_checkstring(L, 2);  
  CURLoption type;
  CURLSHcode  errornum;

  if (!strcmp("COOKIE", value)) 
    type = CURL_LOCK_DATA_COOKIE;
  else if (!strcmp("DNS", value)) 
    type = CURL_LOCK_DATA_DNS;
  else luaL_error(L, "Invalid share type: %s", value);

  if ((errornum = curl_share_setopt(curlsh, CURLSHOPT_SHARE, type)) != CURLSHE_OK)
    luaL_error(L, "%s", curl_share_strerror(errornum));
  return 0;
}

int l_share_gc(lua_State *L) {
  /* gc resources */
  l_share_userdata *privp = lua_touserdata(L, 1);
  CURLSH *curlsh = privp->curlsh;
  CURLSHcode  errornum;
  if ((errornum = curl_share_cleanup(curlsh)) != CURLSHE_OK)
    luaL_error(L, "%s", curl_share_strerror(errornum));
  
  return 0;
}
