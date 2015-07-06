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

#include <string.h>		/* memcpy */

#include "Lua-cURL.h"
#include "Lua-utility.h"

static int l_easy_readfunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  lua_State* L = (lua_State*)stream;
  size_t n;
  int old_top = lua_gettop(L);
  const char *str;
  lua_getfield(L, -1, "readfunction");
  lua_pushinteger(L, nmemb * size);
  lua_call(L, 1, 1);
  str = lua_tolstring(L, -1, &n);
  if (n > nmemb*size)
    luaL_error(L, "String returned from readfunction is too long (%d)", n);
  memcpy(ptr, str, n);
  lua_settop(L, old_top);
  return n;
}

static int l_easy_writefunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  lua_State* L = (lua_State*)stream;

  lua_getfield(L, -1, "writefunction");
  lua_pushlstring(L, (char*) ptr, nmemb * size);
  lua_call(L, 1, 0);
  return nmemb*size;
}

static int l_easy_headerfunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  lua_State* L = (lua_State*)stream;
  lua_getfield(L, -1, "headerfunction");
  lua_pushlstring(L, (char*) ptr, nmemb * size);
  lua_call(L, 1, 0);
  return nmemb*size;
}


int l_easy_setup_writefunction(lua_State *L, CURL* curl) {
    /* Lua State as userdata argument */
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  if (curl_easy_setopt(curl, CURLOPT_WRITEDATA ,L) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, l_easy_writefunction) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

int l_easy_setup_readfunction(lua_State *L, CURL* curl) {
    /* Lua State as userdata argument */
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  if (curl_easy_setopt(curl, CURLOPT_READDATA ,L) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  if (curl_easy_setopt(curl, CURLOPT_READFUNCTION, l_easy_readfunction) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

int l_easy_setup_headerfunction(lua_State *L, CURL* curl) {
  /* Lua State as userdata argument */
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  if (curl_easy_setopt(curl, CURLOPT_WRITEHEADER ,L) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);

  if (curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, l_easy_headerfunction) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

int l_easy_clear_headerfunction(lua_State *L, CURL* curl) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  if (curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NULL) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

int l_easy_clear_writefunction(lua_State *L, CURL* curl) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

int l_easy_clear_readfunction(lua_State *L, CURL* curl) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  if (curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}
