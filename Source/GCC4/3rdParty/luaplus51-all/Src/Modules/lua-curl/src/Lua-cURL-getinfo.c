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

#include "Lua-cURL.h"
#include "Lua-utility.h"

/* prefix of all registered functions */
#define P "getinfo_"

static int l_easy_getinfo_string(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 1);
  char *value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", privatep->error);
  
  lua_pushstring(L, value);
  return 1;
}

static int l_easy_getinfo_long(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 1);
  long value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", privatep->error);
  
  lua_pushinteger(L, value);
  return 1;
} 

static int l_easy_getinfo_double(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 1);
  double value;
  
  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", privatep->error);
  
  lua_pushnumber(L, value);
  return 1;
} 

static int l_easy_getinfo_curl_slist(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 1);  
  struct curl_slist *list;
  struct curl_slist *next;
  int i;

  if (curl_easy_getinfo(curl, *infop, &list) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  
  i = 1;
  next = list;
  lua_newtable(L);		

  while (next) {
    lua_pushstring(L, (char*) next->data);
    lua_rawseti(L, -2, i++);
    next = next->next;
  }

  curl_slist_free_all(list);

  return 1;
}

static struct {
  const char *name;
  CURLINFO info; 
  lua_CFunction func;
} luacurl_getinfo_c[] = {
  {P"effective_url", CURLINFO_EFFECTIVE_URL, l_easy_getinfo_string},
  {P"response_code", CURLINFO_RESPONSE_CODE, l_easy_getinfo_long},
  {P"http_connectcode", CURLINFO_HTTP_CONNECTCODE, l_easy_getinfo_long},
  {P"filetime", CURLINFO_FILETIME, l_easy_getinfo_long},
  {P"total_time", CURLINFO_TOTAL_TIME, l_easy_getinfo_double},
  {P"namelookup_time", CURLINFO_NAMELOOKUP_TIME, l_easy_getinfo_double},
  {P"connect_time", CURLINFO_CONNECT_TIME, l_easy_getinfo_double},
  {P"pretransfer", CURLINFO_PRETRANSFER_TIME, l_easy_getinfo_double},
  {P"starttransfer_time", CURLINFO_STARTTRANSFER_TIME, l_easy_getinfo_double},  
  {P"redirect_time", CURLINFO_REDIRECT_TIME, l_easy_getinfo_double},  
  {P"redirect_count", CURLINFO_REDIRECT_COUNT, l_easy_getinfo_long},  
  {P"size_upload", CURLINFO_SIZE_UPLOAD, l_easy_getinfo_double},
  {P"size_download", CURLINFO_SIZE_DOWNLOAD, l_easy_getinfo_double},
  {P"speed_download", CURLINFO_SPEED_DOWNLOAD, l_easy_getinfo_double},
  {P"speed_upload", CURLINFO_SPEED_UPLOAD, l_easy_getinfo_double},
  {P"header_size", CURLINFO_HEADER_SIZE, l_easy_getinfo_long},
  {P"request_size", CURLINFO_REQUEST_SIZE, l_easy_getinfo_long},
  {P"ssl_verifyresult", CURLINFO_SSL_VERIFYRESULT, l_easy_getinfo_long},
  {P"ssl_engines", CURLINFO_SSL_ENGINES, l_easy_getinfo_curl_slist},
  {P"content_length_download", CURLINFO_CONTENT_LENGTH_DOWNLOAD, l_easy_getinfo_double},
  {P"content_length_upload", CURLINFO_CONTENT_LENGTH_UPLOAD, l_easy_getinfo_double},
  {P"content_type", CURLINFO_CONTENT_TYPE, l_easy_getinfo_string},
  {P"private", CURLINFO_PRIVATE, l_easy_getinfo_string},
  {P"httpauth_avail", CURLINFO_HTTPAUTH_AVAIL, l_easy_getinfo_long},
  {P"proxyauth_avail", CURLINFO_PROXYAUTH_AVAIL, l_easy_getinfo_long},
  {P"os_errno", CURLINFO_OS_ERRNO, l_easy_getinfo_long},
  {P"num_connects", CURLINFO_NUM_CONNECTS, l_easy_getinfo_long},
  {P"cookielist", CURLINFO_COOKIELIST, l_easy_getinfo_curl_slist},
  {P"lastsocket", CURLINFO_LASTSOCKET, l_easy_getinfo_long},
  {P"ftp_entry_path" , CURLINFO_FTP_ENTRY_PATH , l_easy_getinfo_string},  
  {NULL, CURLINFO_EFFECTIVE_URL, NULL}};	


int l_easy_getinfo_register(lua_State *L) {
  int i;

  /* register getinfo closures */
  for (i=0; luacurl_getinfo_c[i].name != NULL; i++) {
    CURLINFO *infop = &(luacurl_getinfo_c[i].info);
    lua_pushlightuserdata(L, infop);
    lua_pushcclosure(L, luacurl_getinfo_c[i].func, 1);
    lua_setfield(L, -2, luacurl_getinfo_c[i].name);
  }  

  return 0;
}
