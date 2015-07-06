/* algo.h */
/* See Copyright Notice in the file LICENSE */

#include "common.h"

/* FIXME: Get version from defaults.mak */
#define REX_VERSION "Lrexlib 2.5.3"

/* Forward declarations */
static void gmatch_pushsubject (lua_State *L, TArgExec *argE);
static int findmatch_exec  (TUserdata *ud, TArgExec *argE);
static int split_exec      (TUserdata *ud, TArgExec *argE, int offset);
static int compile_regex   (lua_State *L, const TArgComp *argC, TUserdata **pud);
static int generate_error  (lua_State *L, const TUserdata *ud, int errcode);

#ifndef ALG_GETCARGS
#  define ALG_GETCARGS(a,b,c)
#endif

#ifndef DO_NAMED_SUBPATTERNS
#define DO_NAMED_SUBPATTERNS(a,b,c)
#endif

/*  When doing an iterative search, there can occur a situation of a zero-length
 *  match at the current position, that prevents further advance on the subject
 *  string.
 *  There are two ways to handle that (AFAIK):
 *    a) Advance by one character (continue the search from the next position),
 *       or
 *    b) Search for a non-zero-length match that begins from the current
 *       position ("retry" the search). If the match is not found then advance
 *       by one character.
 *  The "b)" seems more correct, but most regex libraries expose no API for that.
 *  The known exception is PCRE that has flags PCRE_NOTEMPTY and PCRE_ANCHORED.
 */
#ifdef ALG_USERETRY
  #define SET_RETRY(a,b) (a=b)
  static int gsub_exec (TUserdata *ud, TArgExec *argE, int offset, int retry);
  static int gmatch_exec (TUserdata *ud, TArgExec *argE, int retry);
  #define GSUB_EXEC gsub_exec
  #define GMATCH_EXEC gmatch_exec
#else
  #define SET_RETRY(a,b) ((void)a)
  static int gsub_exec (TUserdata *ud, TArgExec *argE, int offset);
  static int gmatch_exec (TUserdata *ud, TArgExec *argE);
  #define GSUB_EXEC(a,b,c,d) gsub_exec(a,b,c)
  #define GMATCH_EXEC(a,b,c) gmatch_exec(a,b)
#endif


#define METHOD_FIND  0
#define METHOD_MATCH 1
#define METHOD_EXEC  2
#define METHOD_TFIND 3


static int OptLimit (lua_State *L, int pos) {
  if (lua_isnoneornil (L, pos))
    return GSUB_UNLIMITED;
  if (lua_isfunction (L, pos))
    return GSUB_CONDITIONAL;
  if (lua_isnumber (L, pos)) {
    int a = lua_tointeger (L, pos);
    return a < 0 ? 0 : a;
  }
  return luaL_typeerror (L, pos, "number or function");
}


static int get_startoffset(lua_State *L, int stackpos, size_t len) {
  int startoffset = luaL_optint(L, stackpos, 1);
  if(startoffset > 0)
    startoffset--;
  else if(startoffset < 0) {
    startoffset += len;
    if(startoffset < 0)
      startoffset = 0;
  }
  return startoffset;
}


static TUserdata* test_ud (lua_State *L, int pos)
{
  TUserdata *ud;
  if (lua_getmetatable(L, pos) &&
      lua_rawequal(L, -1, LUA_ENVIRONINDEX) &&
      (ud = (TUserdata *)lua_touserdata(L, pos)) != NULL) {
    lua_pop(L, 1);
    return ud;
  }
  return NULL;
}


static TUserdata* check_ud (lua_State *L)
{
  TUserdata *ud = test_ud(L, 1);
  if (ud == NULL) luaL_typeerror(L, 1, REX_TYPENAME);
  return ud;
}


static void check_pattern (lua_State *L, int pos, TArgComp *argC)
{
  if (lua_isstring (L, pos)) {
    argC->pattern = lua_tolstring (L, pos, &argC->patlen);
    argC->ud = NULL;
  }
  else if ((argC->ud = test_ud (L, pos)) == NULL)
    luaL_typeerror(L, pos, "string or "REX_TYPENAME);
}

static void checkarg_new (lua_State *L, TArgComp *argC) {
  argC->pattern = luaL_checklstring (L, 1, &argC->patlen);
  argC->cflags = ALG_GETCFLAGS (L, 2);
  ALG_GETCARGS (L, 3, argC);
}


/* function gsub (s, patt, f, [n], [cf], [ef], [larg...]) */
static void checkarg_gsub (lua_State *L, TArgComp *argC, TArgExec *argE) {
  argE->text = luaL_checklstring (L, 1, &argE->textlen);
  check_pattern (L, 2, argC);
  lua_tostring (L, 3);    /* converts number (if any) to string */
  argE->reptype = lua_type (L, 3);
  if (argE->reptype != LUA_TSTRING && argE->reptype != LUA_TTABLE &&
      argE->reptype != LUA_TFUNCTION) {
    luaL_typeerror (L, 3, "string, table or function");
  }
  argE->funcpos = 3;
  argE->funcpos2 = 4;
  argE->maxmatch = OptLimit (L, 4);
  argC->cflags = ALG_GETCFLAGS (L, 5);
  argE->eflags = luaL_optint (L, 6, ALG_EFLAGS_DFLT);
  ALG_GETCARGS (L, 7, argC);
}


/* function find  (s, patt, [st], [cf], [ef], [larg...]) */
/* function match (s, patt, [st], [cf], [ef], [larg...]) */
static void checkarg_find_func (lua_State *L, TArgComp *argC, TArgExec *argE) {
  argE->text = luaL_checklstring (L, 1, &argE->textlen);
  check_pattern (L, 2, argC);
  argE->startoffset = get_startoffset (L, 3, argE->textlen);
  argC->cflags = ALG_GETCFLAGS (L, 4);
  argE->eflags = luaL_optint (L, 5, ALG_EFLAGS_DFLT);
  ALG_GETCARGS (L, 6, argC);
}


/* function gmatch (s, patt, [cf], [ef], [larg...]) */
/* function split  (s, patt, [cf], [ef], [larg...]) */
static void checkarg_gmatch_split (lua_State *L, TArgComp *argC, TArgExec *argE) {
  argE->text = luaL_checklstring (L, 1, &argE->textlen);
  check_pattern (L, 2, argC);
  argC->cflags = ALG_GETCFLAGS (L, 3);
  argE->eflags = luaL_optint (L, 4, ALG_EFLAGS_DFLT);
  ALG_GETCARGS (L, 5, argC);
}


/* method r:tfind (s, [st], [ef]) */
/* method r:exec  (s, [st], [ef]) */
/* method r:find  (s, [st], [ef]) */
/* method r:match (s, [st], [ef]) */
static void checkarg_find_method (lua_State *L, TArgExec *argE, TUserdata **ud) {
  *ud = check_ud (L);
  argE->text = luaL_checklstring (L, 2, &argE->textlen);
  argE->startoffset = get_startoffset (L, 3, argE->textlen);
  argE->eflags = luaL_optint (L, 4, ALG_EFLAGS_DFLT);
}


static int ud_new (lua_State *L) {
  TArgComp argC;
  checkarg_new (L, &argC);
  return compile_regex (L, &argC, NULL);
}

static void push_substrings (lua_State *L, TUserdata *ud, const char *text,
                             TFreeList *freelist) {
  int i;
  if (lua_checkstack (L, ALG_NSUB(ud)) == 0) {
    if (freelist)
      freelist_free (freelist);
    luaL_error (L, "cannot add %d stack slots", ALG_NSUB(ud));
  }
  for (i = 1; i <= ALG_NSUB(ud); i++) {
    ALG_PUSHSUB_OR_FALSE (L, ud, text, i);
  }
}

static int gsub (lua_State *L) {
  TUserdata *ud;
  TArgComp argC;
  TArgExec argE;
  int n_match = 0, n_subst = 0, st = 0, retry;
  TBuffer BufOut, BufRep, BufTemp, *pBuf = &BufOut;
  TFreeList freelist;
  /*------------------------------------------------------------------*/
  checkarg_gsub (L, &argC, &argE);
  if (argC.ud) {
    ud = (TUserdata*) argC.ud;
    lua_pushvalue (L, 2);
  }
  else compile_regex (L, &argC, &ud);
  freelist_init (&freelist);
  /*------------------------------------------------------------------*/
  if (argE.reptype == LUA_TSTRING) {
    buffer_init (&BufRep, 256, L, &freelist);
    bufferZ_putrepstring (&BufRep, argE.funcpos, ALG_NSUB(ud));
  }
  /*------------------------------------------------------------------*/
  if (argE.maxmatch == GSUB_CONDITIONAL) {
    buffer_init (&BufTemp, 1024, L, &freelist);
    pBuf = &BufTemp;
  }
  /*------------------------------------------------------------------*/
  buffer_init (&BufOut, 1024, L, &freelist);
  SET_RETRY (retry, 0);
  while ((argE.maxmatch < 0 || n_match < argE.maxmatch) && st <= (int)argE.textlen) {
    int from, to, res;
    int curr_subst = 0;
    res = GSUB_EXEC (ud, &argE, st, retry);
    if (ALG_NOMATCH (res)) {
#ifdef ALG_USERETRY
      if (retry) {
        if (st < (int)argE.textlen) {  /* advance by 1 char (not replaced) */
          buffer_addlstring (&BufOut, argE.text + st, 1);
          ++st;
          retry = 0;
          continue;
        }
      }
#endif
      break;
    }
    else if (!ALG_ISMATCH (res)) {
      freelist_free (&freelist);
      return generate_error (L, ud, res);
    }
    ++n_match;
    from = ALG_BASE(st) + ALG_SUBBEG(ud,0);
    to = ALG_BASE(st) + ALG_SUBEND(ud,0);
    if (st < from) {
      buffer_addlstring (&BufOut, argE.text + st, from - st);
#ifdef ALG_PULL
      st = from;
#endif
    }
    /*----------------------------------------------------------------*/
    if (argE.reptype == LUA_TSTRING) {
      size_t iter = 0, num;
      const char *str;
      while (bufferZ_next (&BufRep, &iter, &num, &str)) {
        if (str)
          buffer_addlstring (pBuf, str, num);
        else if (num == 0 || ALG_SUBVALID (ud,num))
          buffer_addlstring (pBuf, argE.text + ALG_BASE(st) + ALG_SUBBEG(ud,num), ALG_SUBLEN(ud,num));
      }
      curr_subst = 1;
    }
    /*----------------------------------------------------------------*/
    else if (argE.reptype == LUA_TTABLE) {
      if (ALG_NSUB(ud) > 0)
        ALG_PUSHSUB_OR_FALSE (L, ud, argE.text + ALG_BASE(st), 1);
      else
        lua_pushlstring (L, argE.text + from, to - from);
      lua_gettable (L, argE.funcpos);
    }
    /*----------------------------------------------------------------*/
    else if (argE.reptype == LUA_TFUNCTION) {
      int narg;
      lua_pushvalue (L, argE.funcpos);
      if (ALG_NSUB(ud) > 0) {
        push_substrings (L, ud, argE.text + ALG_BASE(st), &freelist);
        narg = ALG_NSUB(ud);
      }
      else {
        lua_pushlstring (L, argE.text + from, to - from);
        narg = 1;
      }
      if (0 != lua_pcall (L, narg, 1, 0)) {
        freelist_free (&freelist);
        return lua_error (L);  /* re-raise the error */
      }
    }
    /*----------------------------------------------------------------*/
    if (argE.reptype != LUA_TSTRING) {
      if (lua_tostring (L, -1)) {
        buffer_addvalue (pBuf, -1);
        curr_subst = 1;
      }
      else if (!lua_toboolean (L, -1))
        buffer_addlstring (pBuf, argE.text + from, to - from);
      else {
        freelist_free (&freelist);
        luaL_error (L, "invalid replacement value (a %s)", luaL_typename (L, -1));
      }
      if (argE.maxmatch != GSUB_CONDITIONAL)
        lua_pop (L, 1);
    }
    /*----------------------------------------------------------------*/
    if (argE.maxmatch == GSUB_CONDITIONAL) {
      /* Call the function */
      lua_pushvalue (L, argE.funcpos2);
      lua_pushinteger (L, from + 1);
      lua_pushinteger (L, to);
      if (argE.reptype == LUA_TSTRING)
        buffer_pushresult (&BufTemp);
      else {
        lua_pushvalue (L, -4);
        lua_remove (L, -5);
      }
      if (0 != lua_pcall (L, 3, 2, 0)) {
        freelist_free (&freelist);
        lua_error (L);  /* re-raise the error */
      }
      /* Handle the 1-st return value */
      if (lua_isstring (L, -2)) {               /* coercion is allowed here */
        buffer_addvalue (&BufOut, -2);          /* rep2 */
        curr_subst = 1;
      }
      else if (lua_toboolean (L, -2))
        buffer_addbuffer (&BufOut, &BufTemp);   /* rep1 */
      else {
        buffer_addlstring (&BufOut, argE.text + from, to - from); /* "no" */
        curr_subst = 0;
      }
      /* Handle the 2-nd return value */
      if (lua_type (L, -1) == LUA_TNUMBER) {    /* no coercion is allowed here */
        int n = lua_tointeger (L, -1);
        if (n < 0)                              /* n */
          n = 0;
        argE.maxmatch = n_match + n;
      }
      else if (lua_toboolean (L, -1))           /* "yes to all" */
        argE.maxmatch = GSUB_UNLIMITED;
      else
        buffer_clear (&BufTemp);

      lua_pop (L, 2);
      if (argE.maxmatch != GSUB_CONDITIONAL)
        pBuf = &BufOut;
    }
    /*----------------------------------------------------------------*/
    n_subst += curr_subst;
    if (st < to) {
      st = to;
      SET_RETRY (retry, 0);
    }
    else if (st < (int)argE.textlen) {
#ifdef ALG_USERETRY
      retry = 1;
#else
      /* advance by 1 char (not replaced) */
      buffer_addlstring (&BufOut, argE.text + st, 1);
      ++st;
#endif
    }
    else break;
  }
  /*------------------------------------------------------------------*/
  buffer_addlstring (&BufOut, argE.text + st, argE.textlen - st);
  buffer_pushresult (&BufOut);
  lua_pushinteger (L, n_match);
  lua_pushinteger (L, n_subst);
  freelist_free (&freelist);
  return 3;
}


static int finish_generic_find (lua_State *L, TUserdata *ud, TArgExec *argE,
  int method, int res)
{
  if (ALG_ISMATCH (res)) {
    if (method == METHOD_FIND)
      ALG_PUSHOFFSETS (L, ud, ALG_BASE(argE->startoffset), 0);
    if (ALG_NSUB(ud))    /* push captures */
      push_substrings (L, ud, argE->text, NULL);
    else if (method != METHOD_FIND) {
      ALG_PUSHSUB (L, ud, argE->text, 0);
      return 1;
    }
    return (method == METHOD_FIND) ? ALG_NSUB(ud) + 2 : ALG_NSUB(ud);
  }
  else if (ALG_NOMATCH (res))
    return lua_pushnil (L), 1;
  else
    return generate_error (L, ud, res);
}


static int generic_find_func (lua_State *L, int method) {
  TUserdata *ud;
  TArgComp argC;
  TArgExec argE;
  int res;

  checkarg_find_func (L, &argC, &argE);
  if (argE.startoffset > (int)argE.textlen)
    return lua_pushnil (L), 1;

  if (argC.ud) {
    ud = (TUserdata*) argC.ud;
    lua_pushvalue (L, 2);
  }
  else compile_regex (L, &argC, &ud);
  res = findmatch_exec (ud, &argE);
  return finish_generic_find (L, ud, &argE, method, res);
}


static int find (lua_State *L) {
  return generic_find_func (L, METHOD_FIND);
}


static int match (lua_State *L) {
  return generic_find_func (L, METHOD_MATCH);
}


static int gmatch_iter (lua_State *L) {
  int retry;
  TArgExec argE;
  TUserdata *ud    = (TUserdata*) lua_touserdata (L, lua_upvalueindex (1));
  argE.text        = lua_tolstring (L, lua_upvalueindex (2), &argE.textlen);
  argE.eflags      = lua_tointeger (L, lua_upvalueindex (3));
  argE.startoffset = lua_tointeger (L, lua_upvalueindex (4));
#ifdef ALG_USERETRY
  retry            = lua_tointeger (L, lua_upvalueindex (5));
#endif

  if (argE.startoffset > (int)argE.textlen)
    return 0;

  while (1) {
    int res = GMATCH_EXEC (ud, &argE, retry);
    if (ALG_ISMATCH (res)) {
      int incr = 0;
      if (ALG_SUBLEN(ud,0)) {
        SET_RETRY (retry, 0);
      }
      else { /* no progress: prevent endless loop */
#ifdef ALG_USERETRY
        SET_RETRY (retry, 1);
#else
        incr = 1;
#endif
      }
      ALG_PUSHEND (L, ud, ALG_BASE(argE.startoffset)+incr, 0); /* update start offset */
      lua_replace (L, lua_upvalueindex (4));
#ifdef ALG_USERETRY
      lua_pushinteger (L, retry);
      lua_replace (L, lua_upvalueindex (5));         /* update retry */
#endif
      /* push either captures or entire match */
      if (ALG_NSUB(ud)) {
        push_substrings (L, ud, argE.text, NULL);
        return ALG_NSUB(ud);
      }
      else {
        ALG_PUSHSUB (L, ud, argE.text, 0);
        return 1;
      }
    }
    else if (ALG_NOMATCH (res)) {
#ifdef ALG_USERETRY
      if (retry) {
        if (argE.startoffset < (int)argE.textlen) {
          ++argE.startoffset;   /* advance by 1 char */
          SET_RETRY (retry, 0);
          continue;
        }
      }
#endif
      return 0;
    }
    else
      return generate_error (L, ud, res);
  }
}


static int split_iter (lua_State *L) {
  int incr, newoffset, res;
  TArgExec argE;
  TUserdata *ud    = (TUserdata*) lua_touserdata (L, lua_upvalueindex (1));
  argE.text        = lua_tolstring (L, lua_upvalueindex (2), &argE.textlen);
  argE.eflags      = lua_tointeger (L, lua_upvalueindex (3));
  argE.startoffset = lua_tointeger (L, lua_upvalueindex (4));
  incr             = lua_tointeger (L, lua_upvalueindex (5));

  if (argE.startoffset > (int)argE.textlen)
    return 0;

  if ((newoffset = argE.startoffset + incr) > (int)argE.textlen)
    goto nomatch;

  res = split_exec (ud, &argE, newoffset);
  if (ALG_ISMATCH (res)) {
    ALG_PUSHEND (L, ud, ALG_BASE(newoffset), 0);          /* update start offset */
    lua_replace (L, lua_upvalueindex (4));
    lua_pushinteger (L, ALG_SUBLEN(ud,0) ? 0 : 1);    /* update incr */
    lua_replace (L, lua_upvalueindex (5));
    /* push text preceding the match */
    lua_pushlstring (L, argE.text + argE.startoffset,
                     ALG_SUBBEG(ud,0) + ALG_BASE(newoffset) - argE.startoffset);
    /* push either captures or entire match */
    if (ALG_NSUB(ud)) {
      push_substrings (L, ud, argE.text + ALG_BASE(newoffset), NULL);
      return 1 + ALG_NSUB(ud);
    }
    else {
      ALG_PUSHSUB (L, ud, argE.text + ALG_BASE(newoffset), 0);
      return 2;
    }
  }
  else if (ALG_NOMATCH (res))
    goto nomatch;
  else
    return generate_error (L, ud, res);

nomatch:
  lua_pushinteger (L, argE.textlen + 1);    /* mark as last iteration */
  lua_replace (L, lua_upvalueindex (4));    /* update start offset */
  lua_pushlstring (L, argE.text+argE.startoffset, argE.textlen-argE.startoffset);
  return 1;
}


static int gmatch (lua_State *L)
{
  TArgComp argC;
  TArgExec argE;
  TUserdata *ud;
  checkarg_gmatch_split (L, &argC, &argE);
  if (argC.ud) {
    ud = (TUserdata*) argC.ud;
    lua_pushvalue (L, 2);
  }
  else compile_regex (L, &argC, &ud);         /* 1-st upvalue: ud */
  gmatch_pushsubject (L, &argE);              /* 2-nd upvalue: s  */
  lua_pushinteger (L, argE.eflags);           /* 3-rd upvalue: ef */
  lua_pushinteger (L, 0);                     /* 4-th upvalue: startoffset */
#ifdef ALG_USERETRY
  lua_pushinteger (L, 0);                     /* 5-th upvalue: retry */
  lua_pushcclosure (L, gmatch_iter, 5);
#else
  lua_pushcclosure (L, gmatch_iter, 4);
#endif
  return 1;
}

static int split (lua_State *L)
{
  TArgComp argC;
  TArgExec argE;
  TUserdata *ud;
  checkarg_gmatch_split (L, &argC, &argE);
  if (argC.ud) {
    ud = (TUserdata*) argC.ud;
    lua_pushvalue (L, 2);
  }
  else compile_regex (L, &argC, &ud);         /* 1-st upvalue: ud */
  gmatch_pushsubject (L, &argE);              /* 2-nd upvalue: s  */
  lua_pushinteger (L, argE.eflags);           /* 3-rd upvalue: ef */
  lua_pushinteger (L, 0);                     /* 4-th upvalue: startoffset */
  lua_pushinteger (L, 0);                     /* 5-th upvalue: incr */
  lua_pushcclosure (L, split_iter, 5);
  return 1;
}


static void push_substring_table (lua_State *L, TUserdata *ud, const char *text) {
  int i;
  lua_newtable (L);
  for (i = 1; i <= ALG_NSUB(ud); i++) {
    ALG_PUSHSUB_OR_FALSE (L, ud, text, i);
    lua_rawseti (L, -2, i);
  }
}


static void push_offset_table (lua_State *L, TUserdata *ud, int startoffset) {
  int i, j;
  lua_newtable (L);
  for (i=1, j=1; i <= ALG_NSUB(ud); i++) {
    if (ALG_SUBVALID (ud,i)) {
      ALG_PUSHSTART (L, ud, startoffset, i);
      lua_rawseti (L, -2, j++);
      ALG_PUSHEND (L, ud, startoffset, i);
      lua_rawseti (L, -2, j++);
    }
    else {
      lua_pushboolean (L, 0);
      lua_rawseti (L, -2, j++);
      lua_pushboolean (L, 0);
      lua_rawseti (L, -2, j++);
    }
  }
}


static int generic_find_method (lua_State *L, int method) {
  TUserdata *ud;
  TArgExec argE;
  int res;

  checkarg_find_method (L, &argE, &ud);
  if (argE.startoffset > (int)argE.textlen)
    return lua_pushnil(L), 1;

  res = findmatch_exec (ud, &argE);
  if (ALG_ISMATCH (res)) {
    switch (method) {
      case METHOD_EXEC:
        ALG_PUSHOFFSETS (L, ud, ALG_BASE(argE.startoffset), 0);
        push_offset_table (L, ud, ALG_BASE(argE.startoffset));
        DO_NAMED_SUBPATTERNS (L, ud, argE.text);
        return 3;
      case METHOD_TFIND:
        ALG_PUSHOFFSETS (L, ud, ALG_BASE(argE.startoffset), 0);
        push_substring_table (L, ud, argE.text);
        DO_NAMED_SUBPATTERNS (L, ud, argE.text);
        return 3;
      case METHOD_MATCH:
      case METHOD_FIND:
        return finish_generic_find (L, ud, &argE, method, res);
    }
    return 0;
  }
  else if (ALG_NOMATCH (res))
    return lua_pushnil (L), 1;
  else
    return generate_error(L, ud, res);
}


static int ud_find (lua_State *L) {
  return generic_find_method (L, METHOD_FIND);
}
static int ud_match (lua_State *L) {
  return generic_find_method (L, METHOD_MATCH);
}
static int ud_tfind (lua_State *L) {
  return generic_find_method (L, METHOD_TFIND);
}
static int ud_exec (lua_State *L) {
  return generic_find_method (L, METHOD_EXEC);
}


/* function plainfind (s, p, [st], [ci]) */
/* (optimized for performance at the expense of code size) */
static int plainfind_func (lua_State *L) {
  size_t textlen, patlen;
  const char *text = luaL_checklstring (L, 1, &textlen);
  const char *pattern = luaL_checklstring (L, 2, &patlen);
  const char *from = text + get_startoffset (L, 3, textlen);
  int ci = lua_toboolean (L, 4);
  const char *end = text + textlen - patlen;

  if (patlen == 0 && from <= end)
    goto found;
  if (ci ) {
    for (; from <= end; ++from) {
      if (toupper(*from) == toupper(*pattern)) {
        const char *f = from, *p = pattern;
        size_t len = patlen;
        while (--len) {
          if (toupper (*++f) != toupper (*++p))
            break;
        }
        if (len == 0)
          goto found;
      }
    }
  }
  else {
    for (; from <= end; ++from) {
      if (*from == *pattern) {
        const char *f = from, *p = pattern;
        size_t len = patlen;
        while (--len) {
          if (*++f != *++p)
            break;
        }
        if (len == 0)
          goto found;
      }
    }
  }
  lua_pushnil (L);
  return 1;
found:
  lua_pushinteger (L, from - text + 1);
  lua_pushinteger (L, from - text + patlen);
  return 2;
}
