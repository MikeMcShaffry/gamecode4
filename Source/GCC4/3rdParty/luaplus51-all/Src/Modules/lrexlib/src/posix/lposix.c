/* lposix.c - Lua binding of POSIX regular expressions library */
/* See Copyright Notice in the file LICENSE */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lua.h"
#include "lauxlib.h"
#include "../common.h"

#ifndef REX_POSIX_INCLUDE
#  include <regex.h>
#else
#  include REX_POSIX_INCLUDE
#endif

/* These 2 settings may be redefined from the command-line or the makefile.
 * They should be kept in sync between themselves and with the target name.
 */
#ifndef REX_LIBNAME
#  define REX_LIBNAME "rex_posix"
#endif
#ifndef REX_OPENLIB
#  define REX_OPENLIB luaopen_rex_posix
#endif

#define REX_TYPENAME REX_LIBNAME"_regex"

/* Test if regex.h corresponds to the extended POSIX library, i.e. H.Spencer's.
   This test may not work as intended if regex.h introduced REG_BASIC, etc.
   via enum rather than #define.
   If that's the case, add -DREX_POSIX_EXT in the makefile/command line.
*/
#ifndef REX_POSIX_EXT
#  if defined(REG_BASIC) && defined(REG_STARTEND)
#    define REX_POSIX_EXT
#  endif
#endif

#define ALG_CFLAGS_DFLT REG_EXTENDED
#ifdef REX_POSIX_EXT
#  define ALG_EFLAGS_DFLT REG_STARTEND
#else
#  define ALG_EFLAGS_DFLT 0
#endif

#define ALG_NOMATCH(res)   ((res) == REG_NOMATCH)
#define ALG_ISMATCH(res)   ((res) == 0)
#define ALG_SUBBEG(ud,n)   ud->match[n].rm_so
#define ALG_SUBEND(ud,n)   ud->match[n].rm_eo
#define ALG_SUBLEN(ud,n)   (ALG_SUBEND(ud,n) - ALG_SUBBEG(ud,n))
#define ALG_SUBVALID(ud,n) (ALG_SUBBEG(ud,n) >= 0)
#ifdef REX_NSUB_BASE1
#  define ALG_NSUB(ud)     ((int)ud->r.re_nsub - 1)
#else
#  define ALG_NSUB(ud)     ((int)ud->r.re_nsub)
#endif

#define ALG_PUSHSUB(L,ud,text,n) \
  lua_pushlstring (L, (text) + ALG_SUBBEG(ud,n), ALG_SUBLEN(ud,n))

#define ALG_PUSHSUB_OR_FALSE(L,ud,text,n) \
  (ALG_SUBVALID(ud,n) ? ALG_PUSHSUB (L,ud,text,n) : lua_pushboolean (L,0))

#define ALG_PUSHSTART(L,ud,offs,n)   lua_pushinteger(L, (offs) + ALG_SUBBEG(ud,n) + 1)
#define ALG_PUSHEND(L,ud,offs,n)     lua_pushinteger(L, (offs) + ALG_SUBEND(ud,n))
#define ALG_PUSHOFFSETS(L,ud,offs,n) \
  (ALG_PUSHSTART(L,ud,offs,n), ALG_PUSHEND(L,ud,offs,n))

#define ALG_BASE(st)                  (st)
#define ALG_GETCFLAGS(L,pos)          luaL_optint(L, pos, ALG_CFLAGS_DFLT)

typedef struct {
  regex_t      r;
  regmatch_t * match;
  int          freed;
} TPosix;

#define TUserdata TPosix

#include "../algo.h"

/*  Functions
 ******************************************************************************
 */

static int generate_error (lua_State *L, const TPosix *ud, int errcode) {
  char errbuf[80];
  regerror (errcode, &ud->r, errbuf, sizeof (errbuf));
  return luaL_error (L, "%s", errbuf);
}

static int compile_regex (lua_State *L, const TArgComp *argC, TPosix **pud) {
  int res;
  TPosix *ud;

  ud = (TPosix *)lua_newuserdata (L, sizeof (TPosix));
  memset (ud, 0, sizeof (TPosix));          /* initialize all members to 0 */

#ifdef REX_POSIX_EXT
  if (argC->cflags & REG_PEND)
    ud->r.re_endp = argC->pattern + argC->patlen;
#endif

  res = regcomp (&ud->r, argC->pattern, argC->cflags);
  if (res != 0)
    return generate_error (L, ud, res);

  if (argC->cflags & REG_NOSUB)
    ud->r.re_nsub = 0;
  ud->match = (regmatch_t *) Lmalloc (L, (ALG_NSUB(ud) + 1) * sizeof (regmatch_t));
  lua_pushvalue (L, LUA_ENVIRONINDEX);
  lua_setmetatable (L, -2);

  if (pud) *pud = ud;
  return 1;
}

#ifdef REX_POSIX_EXT
static void CheckStartEnd (TArgExec *argE, TPosix *ud) {
  if (argE->eflags & REG_STARTEND) {
    ud->match[0].rm_so = argE->startoffset;
    ud->match[0].rm_eo = argE->textlen;
    argE->startoffset = 0;
  }
  else
    argE->text += argE->startoffset;
}
#endif

static int gmatch_exec (TUserdata *ud, TArgExec *argE) {
  if (argE->startoffset > 0)
    argE->eflags |= REG_NOTBOL;

#ifdef REX_POSIX_EXT
  if (argE->eflags & REG_STARTEND) {
    ALG_SUBBEG(ud,0) = 0;
    ALG_SUBEND(ud,0) = argE->textlen - argE->startoffset;
  }
#endif

  argE->text += argE->startoffset;
  return regexec (&ud->r, argE->text, ALG_NSUB(ud) + 1, ud->match, argE->eflags);
}

static void gmatch_pushsubject (lua_State *L, TArgExec *argE) {
#ifdef REX_POSIX_EXT
  if (argE->eflags & REG_STARTEND)
    lua_pushlstring (L, argE->text, argE->textlen);
  else
    lua_pushlstring (L, argE->text, strlen (argE->text));
#else
    lua_pushlstring (L, argE->text, strlen (argE->text));
#endif
}

static int findmatch_exec (TPosix *ud, TArgExec *argE) {
#ifdef REX_POSIX_EXT
  CheckStartEnd (argE, ud);
#else
  argE->text += argE->startoffset;
#endif
  return regexec (&ud->r, argE->text, ALG_NSUB(ud) + 1, ud->match, argE->eflags);
}

static int gsub_exec (TPosix *ud, TArgExec *argE, int st) {
#ifdef REX_POSIX_EXT
  if(argE->eflags & REG_STARTEND) {
    ALG_SUBBEG(ud,0) = 0;
    ALG_SUBEND(ud,0) = argE->textlen - st;
  }
#endif
  if (st > 0)
    argE->eflags |= REG_NOTBOL;
  return regexec (&ud->r, argE->text+st, ALG_NSUB(ud)+1, ud->match, argE->eflags);
}

static int split_exec (TPosix *ud, TArgExec *argE, int offset) {
#ifdef REX_POSIX_EXT
  if (argE->eflags & REG_STARTEND) {
    ALG_SUBBEG(ud,0) = 0;
    ALG_SUBEND(ud,0) = argE->textlen - offset;
  }
#endif
  if (offset > 0)
    argE->eflags |= REG_NOTBOL;

  return regexec (&ud->r, argE->text + offset, ALG_NSUB(ud) + 1, ud->match, argE->eflags);
}

static int Posix_gc (lua_State *L) {
  TPosix *ud = check_ud (L);
  if (ud->freed == 0) {           /* precaution against "manual" __gc calling */
    ud->freed = 1;
    regfree (&ud->r);
    free (ud->match);
  }
  return 0;
}

static int Posix_tostring (lua_State *L) {
  TPosix *ud = check_ud (L);
  if (ud->freed == 0)
    lua_pushfstring (L, "%s (%p)", REX_TYPENAME, (void*)ud);
  else
    lua_pushfstring (L, "%s (deleted)", REX_TYPENAME);
  return 1;
}

static flag_pair posix_flags[] =
{
#ifdef REX_POSIX_EXT
  { "BASIC",    REG_BASIC },
  { "NOSPEC",   REG_NOSPEC },
  { "PEND",     REG_PEND },
  { "STARTEND", REG_STARTEND },
#endif
  { "EXTENDED", REG_EXTENDED },
  { "ICASE",    REG_ICASE },
  { "NOSUB",    REG_NOSUB },
  { "NEWLINE",  REG_NEWLINE },
  { "NOTBOL",   REG_NOTBOL },
  { "NOTEOL",   REG_NOTEOL },
/*---------------------------------------------------------------------------*/
  { NULL, 0 }
};

static flag_pair posix_error_flags[] = {
  { "NOMATCH",  REG_NOMATCH },
  { "BADPAT",   REG_BADPAT },
  { "ECOLLATE", REG_ECOLLATE },
  { "ECTYPE",   REG_ECTYPE },
  { "EESCAPE",  REG_EESCAPE },
  { "ESUBREG",  REG_ESUBREG },
  { "EBRACK",   REG_EBRACK },
  { "EPAREN",   REG_EPAREN },
  { "EBRACE",   REG_EBRACE },
  { "BADBR",    REG_BADBR },
  { "ERANGE",   REG_ERANGE },
  { "ESPACE",   REG_ESPACE },
  { "BADRPT",   REG_BADRPT },
#ifdef REX_POSIX_EXT
  { "EMPTY",    REG_EMPTY },
  { "ASSERT",   REG_ASSERT },
  { "INVARG",   REG_INVARG },
#endif
/*---------------------------------------------------------------------------*/
  { NULL, 0 }
};

static int Posix_get_flags (lua_State *L) {
  const flag_pair* fps[] = { posix_flags, posix_error_flags, NULL };
  return get_flags (L, fps);
}

static const luaL_reg posixmeta[] = {
  { "exec",       ud_exec },
  { "tfind",      ud_tfind },    /* old match */
  { "find",       ud_find },
  { "match",      ud_match },
  { "__gc",       Posix_gc },
  { "__tostring", Posix_tostring },
  { NULL, NULL}
};

static const luaL_reg rexlib[] = {
  { "match",      match },
  { "find",       find },
  { "gmatch",     gmatch },
  { "gsub",       gsub },
  { "split",      split },
  { "new",        ud_new },
  { "flags",      Posix_get_flags },
  { "plainfind",  plainfind_func },
  { NULL, NULL }
};

/* Open the library */
REX_API int REX_OPENLIB (lua_State *L)
{
  /* create a new function environment to serve as a metatable for methods */
  lua_newtable (L);
  lua_pushvalue (L, -1);
  lua_replace (L, LUA_ENVIRONINDEX);
  lua_pushvalue(L, -1); /* mt.__index = mt */
  lua_setfield(L, -2, "__index");
  luaL_register (L, NULL, posixmeta);

  /* register functions */
  luaL_register (L, REX_LIBNAME, rexlib);
  lua_pushliteral (L, REX_VERSION" (for POSIX regexes)");
  lua_setfield (L, -2, "_VERSION");
  return 1;
}
