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

#include <string.h>

#include "Lua-cURL.h"
#include "Lua-cURL-share.h"
#include "Lua-utility.h"

#define P "setopt_"

static int l_easy_setopt_long(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 1);
  long value = luaL_checklong(L,2);

  if (curl_easy_setopt(curl, *optionp, value) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

static int l_easy_setopt_string(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 1);
  const char *value = luaL_checkstring(L, 2);

  if (curl_easy_setopt(curl, *optionp, value) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

void l_easy_setopt_free_slist(l_easy_private *privp, CURLoption option) {
  int i = 0;

  while (privp->option_slists[i].option != 0) {
    if (privp->option_slists[i].option == option) {
      /* free existing slist for this option */  
      if (privp->option_slists[i].slist != NULL) {
	curl_slist_free_all(privp->option_slists[i].slist);
	privp->option_slists[i].slist = NULL;
      }
      break;
    }
    i++;
  }
}

struct curl_slist**  l_easy_setopt_get_slist(l_easy_private *privp, CURLoption option) {
 int i = 0;
 
 while (privp->option_slists[i].option != 0) {
   if (privp->option_slists[i].option == option) 
     return &(privp->option_slists[i].slist);
   i++;
 }
 return NULL;
}

static int l_easy_setopt_strings(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 1);
  struct curl_slist *headerlist = NULL;
  int i = 1;

  /* free previous slist for this option */
  l_easy_setopt_free_slist(privatep, *optionp);

  if (lua_isstring(L, 2)) 
    *l_easy_setopt_get_slist(privatep, *optionp) = curl_slist_append(headerlist, lua_tostring(L, 2));
  else {
    if (lua_type(L, 2) != LUA_TTABLE)
      luaL_error(L, "wrong argument (%s): expected string or table", lua_typename(L, 2));
    
    lua_rawgeti(L, 2, i++);
    while (!lua_isnil(L, -1)) {
      struct curl_slist *current_slist = *l_easy_setopt_get_slist(privatep, *optionp);
      struct curl_slist *new_slist = curl_slist_append(current_slist, lua_tostring(L, -1));
      *l_easy_setopt_get_slist(privatep, *optionp) = new_slist;
      lua_pop(L, 1);
      lua_rawgeti(L, 2, i++);
    } 
    lua_pop(L, 1);
  }

  if (curl_easy_setopt(curl, *optionp, *l_easy_setopt_get_slist(privatep, *optionp)) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);  
  /* memory leak: we need to free this in __gc */
  /*   curl_slist_free_all(headerlist);  */
  return 0;
}

static int l_easy_setopt_proxytype(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 1);
  const char *value = luaL_checkstring(L, 2);

  /* check for valid OPTION: */
  curl_proxytype type;

  if (!strcmp("HTTP", value)) 
    type = CURLPROXY_HTTP;
  else if (!strcmp("SOCKS4", value)) 
    type = CURLPROXY_SOCKS4;
  else if (!strcmp("SOCKS5", value)) 
    type = CURLPROXY_SOCKS5;
  else 
    luaL_error(L, "Invalid proxytype: %s", value);

  if (curl_easy_setopt(curl, *optionp, type) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);  
  return 0;
}

static int l_easy_setopt_share(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 1);
  CURLSH *curlsh = ((l_share_userdata*) luaL_checkudata(L, 2, LUACURL_SHAREMETATABLE))->curlsh;
  
  if (curl_easy_setopt(curl, CURLOPT_SHARE, curlsh) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);  
  return 0;
}


/* closures assigned to setopt in setopt table */
static struct {
  const char *name;
  CURLoption option; 
  lua_CFunction func;
} luacurl_setopt_c[] = {
  /* behavior options */
  {P"verbose", CURLOPT_VERBOSE, l_easy_setopt_long},
  {P"header", CURLOPT_HEADER, l_easy_setopt_long},
  {P"noprogrss", CURLOPT_NOPROGRESS, l_easy_setopt_long},
  {P"nosignal", CURLOPT_NOSIGNAL, l_easy_setopt_long},
  /* callback options */
  /* network options */
  /* names and passwords options  */
  /* http options */
  
  {P"autoreferer", CURLOPT_AUTOREFERER, l_easy_setopt_long},
  {P"encoding", CURLOPT_ENCODING, l_easy_setopt_string},
  {P"followlocation", CURLOPT_FOLLOWLOCATION, l_easy_setopt_long},
  {P"unrestricted_AUTH", CURLOPT_UNRESTRICTED_AUTH, l_easy_setopt_long},
  {P"maxredirs", CURLOPT_MAXREDIRS, l_easy_setopt_long},
  /* not implemented */
  /*   {P"put", CURLOPT_PUT, l_easy_setopt_long}, */
  {P"post", CURLOPT_POST, l_easy_setopt_long},
  {P"postfields", CURLOPT_POSTFIELDS, l_easy_setopt_string},
  {P"postfieldsize", CURLOPT_POSTFIELDSIZE, l_easy_setopt_long},
  {P"postfieldsize_LARGE", CURLOPT_POSTFIELDSIZE_LARGE, l_easy_setopt_long}, 
  {P"httppost", CURLOPT_HTTPPOST, l_easy_setopt_long},
  {P"referer", CURLOPT_REFERER, l_easy_setopt_string},
  {P"useragent", CURLOPT_USERAGENT, l_easy_setopt_string},
  {P"httpheader", CURLOPT_HTTPHEADER, l_easy_setopt_strings}, 
/*  Not implemented:  {P"http200aliases", CURLOPT_HTTP200ALIASES, l_easy_setopt_long}, */
  {P"cookie", CURLOPT_COOKIE, l_easy_setopt_string},
  {P"cookiefile", CURLOPT_COOKIEFILE, l_easy_setopt_string},
  {P"cookiejar", CURLOPT_COOKIEJAR, l_easy_setopt_string},
  {P"cookiesession", CURLOPT_COOKIESESSION, l_easy_setopt_long},
#ifdef CURLOPT_COOKIELIST
  {P"cookielist", CURLOPT_COOKIELIST, l_easy_setopt_string},
#endif
  {P"httpget", CURLOPT_HTTPGET, l_easy_setopt_long},
/*  Not implemented: {P"http_version", CURLOPT_HTTP_VERSION, l_easy_setopt_long}, */
  {P"ignore_content_length", CURLOPT_IGNORE_CONTENT_LENGTH, l_easy_setopt_long},
#ifdef CURLOPT_HTTP_CONTENT_DECODING
  {P"http_content_decoding", CURLOPT_HTTP_CONTENT_DECODING, l_easy_setopt_long},
#endif
#ifdef CURLOPT_HTTP_TRANSFER_DECODING
  {P"http_transfer_decoding ", CURLOPT_HTTP_TRANSFER_DECODING , l_easy_setopt_long},
#endif
  /* ftp options */
  /* protocol options */
  {P"transfertext", CURLOPT_TRANSFERTEXT, l_easy_setopt_long},
  {P"crlf", CURLOPT_CRLF, l_easy_setopt_long},
  {P"range", CURLOPT_RANGE, l_easy_setopt_string},
  {P"resume_from", CURLOPT_RESUME_FROM, l_easy_setopt_long},
  {P"resume_from_large", CURLOPT_RESUME_FROM_LARGE, l_easy_setopt_long},
  {P"customrequest", CURLOPT_CUSTOMREQUEST, l_easy_setopt_string},
  {P"filetime", CURLOPT_FILETIME, l_easy_setopt_long},
  {P"nobody", CURLOPT_NOBODY, l_easy_setopt_long},
  {P"infilesize", CURLOPT_INFILESIZE, l_easy_setopt_long},
  {P"infilesize_large", CURLOPT_INFILESIZE_LARGE, l_easy_setopt_long},
  {P"upload", CURLOPT_UPLOAD, l_easy_setopt_long},
  {P"maxfilesize", CURLOPT_MAXFILESIZE, l_easy_setopt_long},
  {P"maxfilesize_large", CURLOPT_MAXFILESIZE_LARGE, l_easy_setopt_long},
  {P"timecondition", CURLOPT_TIMECONDITION, l_easy_setopt_long},
  {P"timevalue ", CURLOPT_TIMEVALUE , l_easy_setopt_long},
  /* network options */
  {P"url", CURLOPT_URL, l_easy_setopt_string},
  {P"proxy", CURLOPT_PROXY, l_easy_setopt_string},
  {P"proxyport", CURLOPT_PROXYPORT, l_easy_setopt_long},
  {P"proxytype", CURLOPT_PROXYTYPE, l_easy_setopt_proxytype},
  {P"httpproxytunnel", CURLOPT_HTTPPROXYTUNNEL, l_easy_setopt_long},
  {P"interface", CURLOPT_INTERFACE, l_easy_setopt_string},
  {P"localport", CURLOPT_LOCALPORT, l_easy_setopt_long},
  {P"localportrange", CURLOPT_LOCALPORTRANGE, l_easy_setopt_long},
  {P"dns_cache_timeout", CURLOPT_DNS_CACHE_TIMEOUT, l_easy_setopt_long},
  {P"dns_use_global_cache", CURLOPT_DNS_USE_GLOBAL_CACHE, l_easy_setopt_long},
  {P"buffersize", CURLOPT_BUFFERSIZE, l_easy_setopt_long},
  {P"port", CURLOPT_PORT, l_easy_setopt_long},
  {P"TCP_nodelay", CURLOPT_TCP_NODELAY, l_easy_setopt_long},
  {P"ssl_verifypeer", CURLOPT_SSL_VERIFYPEER, l_easy_setopt_long},
  /* ssl options */
  {P"sslcert", CURLOPT_SSLCERT, l_easy_setopt_string},
  {P"sslcerttype", CURLOPT_SSLCERTTYPE, l_easy_setopt_string},
  {P"sslcertpasswd", CURLOPT_SSLCERTPASSWD, l_easy_setopt_string},
  {P"sslkey", CURLOPT_SSLKEY, l_easy_setopt_string},
  {P"sslkeytype", CURLOPT_SSLKEYTYPE, l_easy_setopt_string},
  {P"sslkeypasswd", CURLOPT_SSLKEYPASSWD, l_easy_setopt_string},
  {P"sslengine", CURLOPT_SSLENGINE, l_easy_setopt_string},
  {P"sslengine_default", CURLOPT_SSLENGINE_DEFAULT, l_easy_setopt_long},
  /* not implemented  {P"sslversion", CURLOPT_SSLVERSION, l_easy_setopt_string}, */
  {P"ssl_verifypeer", CURLOPT_SSL_VERIFYPEER, l_easy_setopt_long},
  {P"cainfo", CURLOPT_CAINFO, l_easy_setopt_string},
  {P"capath", CURLOPT_CAPATH, l_easy_setopt_string},
  {P"random_file", CURLOPT_RANDOM_FILE, l_easy_setopt_string},
  {P"egdsocket", CURLOPT_EGDSOCKET, l_easy_setopt_string},
  {P"ssl_verifyhost", CURLOPT_SSL_VERIFYHOST, l_easy_setopt_long},
  {P"ssl_cipher_list", CURLOPT_SSL_CIPHER_LIST, l_easy_setopt_string},
#ifdef CURLOPT_SSL_SESSIONID_CACHE
  {P"ssl_sessionid_cache", CURLOPT_SSL_SESSIONID_CACHE, l_easy_setopt_long},
#endif
  /* not implemented:   {P"krblevel", CURLOPT_KRBLEVEL, l_easy_setopt_string}, */
  /* other options */
  {P"share", CURLOPT_SHARE, l_easy_setopt_share},
  /* dummy opt value */
  {NULL, CURLOPT_VERBOSE, NULL}};	

int l_easy_setopt_register(lua_State *L) {
  int i;

  /* register setopt closures */
  for (i=0; luacurl_setopt_c[i].name != NULL; i++) {
    CURLoption *optionp = &(luacurl_setopt_c[i].option);
    lua_pushlightuserdata(L, optionp);
    lua_pushcclosure(L, luacurl_setopt_c[i].func, 1);
    lua_setfield(L, -2, luacurl_setopt_c[i].name);
  }

  return 0;
}

void  l_easy_setopt_init_slists(lua_State *L, l_easy_private *privp) {
  int i, n;

  /* count required slists */
  for (i=0, n=0; luacurl_setopt_c[i].name != NULL; i++) 
    if (luacurl_setopt_c[i].func == l_easy_setopt_strings) n++;
  
  privp->option_slists = (l_option_slist*) malloc(sizeof(l_option_slist) * ++n);
  if (privp->option_slists == NULL)
    luaL_error(L, "can't malloc option slists");

  /* Init slists */
  for (i=0, n=0; luacurl_setopt_c[i].name != NULL; i++) {
    CURLoption option = luacurl_setopt_c[i].option;
    if (luacurl_setopt_c[i].func == l_easy_setopt_strings) {
      privp->option_slists[n].option = option;
      privp->option_slists[n].slist = NULL;
      n++;
    }
  }
  /* term list */
  privp->option_slists[n].option = 0;
  privp->option_slists[n].slist = NULL;
}

void  l_easy_setopt_free_slists(l_easy_private *privp) {
  int i = 0;
  
  while (privp->option_slists[i].option != 0) {
    if (privp->option_slists[i].slist != NULL) 
      curl_slist_free_all(privp->option_slists[i].slist);
    i++;
  }

  free(privp->option_slists);
}
