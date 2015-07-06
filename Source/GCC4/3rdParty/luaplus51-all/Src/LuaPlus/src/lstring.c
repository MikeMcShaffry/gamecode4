/*
** $Id: lstring.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/


#include <string.h>

#define lstring_c
#define LUA_CORE

#include "lua.h"

#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"

NAMESPACE_LUA_BEGIN

void luaS_resize (lua_State *L, int newsize) {
  GCObject **newhash;
  stringtable *tb;
  int i;
  if (G(L)->gcstate == GCSsweepstring)
    return;  /* cannot resize during GC traverse */
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua.stringtable");
#endif /* LUA_MEMORY_STATS */
  newhash = luaM_newvector(L, newsize, GCObject *);
  tb = &G(L)->strt;
  for (i=0; i<newsize; i++) newhash[i] = NULL;
  /* rehash */
  for (i=0; i<tb->size; i++) {
    GCObject *p = tb->hash[i];
    while (p) {  /* for each node in the list */
      GCObject *next = p->gch.next;  /* save next */
      unsigned int h = gco2ts(p)->hash;
      int h1 = lmod(h, newsize);  /* new position */
      lua_assert(cast_int(h%newsize) == lmod(h, newsize));
      p->gch.next = newhash[h1];  /* chain it */
#if LUA_REFCOUNT
      if (p->gch.next)
        p->gch.next->gch.prev = p;
      p->gch.prev = NULL;
#endif /* LUA_REFCOUNT */
      newhash[h1] = p;
      p = next;
    }
  }
  luaM_freearray(L, tb->hash, tb->size, TString *);
  tb->size = newsize;
  tb->hash = newhash;
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
}


static TString *newlstr (lua_State *L, const char *str, size_t l,
                                       unsigned int h) {
  TString *ts;
  stringtable *tb;
#if LUA_MEMORY_STATS
  luaM_setnameif(L, "lua.string");
#endif /* LUA_MEMORY_STATS */
  if (l+1 > (MAX_SIZET - sizeof(TString))/sizeof(char))
    luaM_toobig(L);
  ts = cast(TString *, luaM_malloc(L, (l+1)*sizeof(char)+sizeof(TString)));
  ts->tsv.len = l;
  ts->tsv.hash = h;
  ts->tsv.marked = luaC_white(G(L));
  ts->tsv.tt = LUA_TSTRING;
  ts->tsv.reserved = 0;
  memcpy(ts+1, str, l*sizeof(char));
  ((char *)(ts+1))[l] = '\0';  /* ending 0 */
  tb = &G(L)->strt;
  h = lmod(h, tb->size);
  ts->tsv.next = tb->hash[h];  /* chain new entry */
#if LUA_REFCOUNT
  ts->tsv.ref = 0;
  if (ts->tsv.next)
    ts->tsv.next->gch.prev = (GCObject*)ts;
  ts->tsv.prev = NULL;
#endif /* LUA_REFCOUNT */
  tb->hash[h] = obj2gco(ts);
  tb->nuse++;
  if (tb->nuse > cast(lu_int32, tb->size) && tb->size <= MAX_INT/2)
    luaS_resize(L, tb->size*2);  /* too crowded */
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return ts;
}

#if LUA_WIDESTRING

static TString *newlwstr (lua_State *L, const lua_WChar *str, size_t l,
						  unsigned int h) {
  TString *tws;
  stringtable *tb;
#if LUA_MEMORY_STATS
  luaM_setnameif(L, "lua.wstring");
#endif /* LUA_MEMORY_STATS */
  if (l+1 > (MAX_SIZET - sizeof(TString))/sizeof(lua_WChar))
    luaM_toobig(L);
  tws = cast(TString *, luaM_malloc(L, (l+1)*sizeof(lua_WChar)+sizeof(TString)));
  tws->tsv.len = l;
  tws->tsv.hash = h;
  tws->tsv.marked = luaC_white(G(L));
  tws->tsv.tt = LUA_TWSTRING;
  tws->tsv.reserved = 0;
  memcpy(tws+1, str, l*sizeof(lua_WChar));
  ((char *)(tws+1))[l * 2] = '\0';  /* ending 0 */
  ((char *)(tws+1))[l * 2 + 1] = '\0';  /* ending 0 */
  tb = &G(L)->strt;
  h = lmod(h, tb->size);
  tws->tsv.next = tb->hash[h];  /* chain new entry */
#if LUA_REFCOUNT
  tws->tsv.ref = 0;
  if (tws->tsv.next)
    tws->tsv.next->gch.prev = (GCObject*)tws;
  tws->tsv.prev = NULL;
#endif /* LUA_REFCOUNT */
  tb->hash[h] = obj2gco(tws);
  tb->nuse++;
  if (tb->nuse > cast(lu_int32, tb->size) && tb->size <= MAX_INT/2)
    luaS_resize(L, tb->size*2);  /* too crowded */
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return tws;
}

#endif /* LUA_WIDESTRING */

TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  GCObject *o;
  unsigned int h = cast(unsigned int, l);  /* seed */
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
  size_t l1;
  for (l1=l; l1>=step; l1-=step)  /* compute hash */
    h = h ^ ((h<<5)+(h>>2)+cast(unsigned char, str[l1-1]));
  for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
       o != NULL;
       o = o->gch.next) {
    TString *ts = rawgco2ts(o);
#if LUA_WIDESTRING
    if (ts->tsv.tt == LUA_TSTRING && ts->tsv.len == l && (memcmp(str, getstr(ts), l) == 0)) {
#else
    if (ts->tsv.len == l && (memcmp(str, getstr(ts), l) == 0)) {
#endif /* LUA_WIDESTRING */
      /* string may be dead */
      if (isdead(G(L), o)) changewhite(o);
      return ts;
    }
  }
  return newlstr(L, str, l, h);  /* not found */
}

#if LUA_WIDESTRING

TString *luaS_newlwstr (lua_State *L, const lua_WChar *str, size_t l) {
  GCObject *o;
  unsigned int h = cast(unsigned int, l);  /* seed */
  size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
  size_t l1;
  for (l1=l; l1>=step; l1-=step)  /* compute hash */
    h = h ^ ((h<<5)+(h>>2)+cast(lua_WChar, str[l1-1]));
  for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
       o != NULL;
       o = o->gch.next) {
    TString *ts = rawgco2ts(o);
    if (ts->tsv.tt == LUA_TWSTRING && ts->tsv.len == l && (memcmp(str, getstr(ts), l * 2) == 0)) {
      /* string may be dead */
      if (isdead(G(L), o)) changewhite(o);
      return ts;
    }
  }
  return newlwstr(L, str, l, h);  /* not found */
}

#endif /* LUA_WIDESTRING */

Udata *luaS_newudata (lua_State *L, size_t s, Table *e) {
  Udata *u;
  if (s > MAX_SIZET - sizeof(Udata))
    luaM_toobig(L);
#if LUA_MEMORY_STATS
  luaM_setname(L, "lua.udata");
#endif /* LUA_MEMORY_STATS */
  u = cast(Udata *, luaM_malloc(L, s + sizeof(Udata)));
  u->uv.marked = luaC_white(G(L));  /* is not finalized */
  u->uv.tt = LUA_TUSERDATA;
#if LUA_REFCOUNT
  u->uv.ref = 0;
  luarc_addreftable(e);
#endif /* LUA_REFCOUNT */
  u->uv.len = s;
  u->uv.metatable = NULL;
  u->uv.env = e;
  /* chain it on udata list (after main thread) */
#if LUA_REFCOUNT
  u->uv.prev = (GCObject*)&G(L)->mainthread->next;
#endif /* LUA_REFCOUNT */
  u->uv.next = G(L)->mainthread->next;
#if LUA_REFCOUNT
  if (u->uv.next)
    u->uv.next->uv.prev = obj2gco(u);
#endif /* LUA_REFCOUNT */
  G(L)->mainthread->next = obj2gco(u);
#if LUA_MEMORY_STATS
  luaM_setname(L, 0);
#endif /* LUA_MEMORY_STATS */
  return u;
}

NAMESPACE_LUA_END
