/* lgnu.c - Lua binding of GNU regular expressions library */
/* See Copyright Notice in the file LICENSE */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lua.h"
#include "lauxlib.h"
#include "../common.h"

#define _GNU_SOURCE
#ifndef __USE_GNU
#  define __USE_GNU
#endif
#ifndef REX_GNU_INCLUDE
#  include <regex.h>
#else
#  include REX_GNU_INCLUDE
#endif

/* These 2 settings may be redefined from the command-line or the makefile.
 * They should be kept in sync between themselves and with the target name.
 */
#ifndef REX_LIBNAME
#  define REX_LIBNAME "rex_gnu"
#endif
#ifndef REX_OPENLIB
#  define REX_OPENLIB luaopen_rex_gnu
#endif

#define REX_TYPENAME REX_LIBNAME"_regex"

#define ALG_CFLAGS_DFLT RE_SYNTAX_POSIX_EXTENDED
#define ALG_EFLAGS_DFLT 0

#define ALG_GETCFLAGS(L,pos)  luaL_optint(L, pos, ALG_CFLAGS_DFLT)

static const unsigned char *gettranslate (lua_State *L, int pos);
#define ALG_GETCARGS(L,pos,argC)  argC->translate = gettranslate (L, pos)

#define ALG_NOMATCH(res)   ((res) == -1 || (res) == -2)
#define ALG_ISMATCH(res)   ((res) >= 0)
#define ALG_SUBBEG(ud,n)   ud->match.start[n]
#define ALG_SUBEND(ud,n)   ud->match.end[n]
#define ALG_SUBLEN(ud,n)   (ALG_SUBEND(ud,n) - ALG_SUBBEG(ud,n))
#define ALG_SUBVALID(ud,n) (ALG_SUBBEG(ud,n) >= 0)
#define ALG_NSUB(ud)     ((int)ud->r.re_nsub)

#define ALG_PUSHSUB(L,ud,text,n) \
  lua_pushlstring (L, (text) + ALG_SUBBEG(ud,n), ALG_SUBLEN(ud,n))

#define ALG_PUSHSUB_OR_FALSE(L,ud,text,n) \
  (ALG_SUBVALID(ud,n) ? ALG_PUSHSUB (L,ud,text,n) : lua_pushboolean (L,0))

#define ALG_PUSHSTART(L,ud,offs,n)   lua_pushinteger(L, (offs) + ALG_SUBBEG(ud,n) + 1)
#define ALG_PUSHEND(L,ud,offs,n)     lua_pushinteger(L, (offs) + ALG_SUBEND(ud,n))
#define ALG_PUSHOFFSETS(L,ud,offs,n) \
  (ALG_PUSHSTART(L,ud,offs,n), ALG_PUSHEND(L,ud,offs,n))

#define ALG_BASE(st)                  (st)

typedef struct {
  struct re_pattern_buffer r;
  struct re_registers      match;
  int                      freed;
  const char *             errmsg;
} TGnu;

#define TUserdata TGnu

#include "../algo.h"

/*  Functions
 ******************************************************************************
 */

/* Execution flags, which we need to simulate as GNU does not use flags for this. */
#define GNU_NOTBOL   1
#define GNU_NOTEOL   2
#define GNU_BACKWARD 4

static int generate_error (lua_State *L, const TUserdata *ud, int errcode) {
  const char *errmsg;
  switch (errcode) {
  case 0:
    errmsg = ud->errmsg;
    break;
  case -1:
    errmsg = "no match";
    break;
  case -2:
    errmsg = "internal error in GNU regex";
    break;
  default:
    errmsg = "unknown error";
  }
  return luaL_error (L, "%s", errmsg);
}

#define ALG_TRANSLATE_SIZE (UCHAR_MAX + 1)
static const unsigned char *gettranslate (lua_State *L, int pos) {
  unsigned i;
  const unsigned char *translate;

  if (lua_isnoneornil (L, pos))
    return NULL;

  translate = (const unsigned char *) Lmalloc (L, ALG_TRANSLATE_SIZE);
  memset ((unsigned char *) translate, 0, ALG_TRANSLATE_SIZE); /* initialize all members to 0 */
  for (i = 0; i <= UCHAR_MAX; i++) {
    lua_pushinteger (L, i);
    lua_gettable (L, pos);
    if (lua_tostring (L, -1))
      ((unsigned char *) translate)[i] = *lua_tostring (L, -1);
    lua_pop (L, 1);
  }
  return translate;
}

static void seteflags (TGnu *ud, TArgExec *argE) {
  ud->r.not_bol = (argE->eflags & GNU_NOTBOL) != 0;
  ud->r.not_eol = (argE->eflags & GNU_NOTEOL) != 0;
}

static int compile_regex (lua_State *L, const TArgComp *argC, TGnu **pud) {
  const char *res;
  TGnu *ud;
  int ret;

  ud = (TGnu *)lua_newuserdata (L, sizeof (TGnu));
  memset (ud, 0, sizeof (TGnu));          /* initialize all members to 0 */

  re_set_syntax (argC->cflags);

  /* translate table is never written to, so this cast is safe */
  ud->r.translate = (unsigned char *) argC->translate;

  res = re_compile_pattern (argC->pattern, argC->patlen, &ud->r);
  if (res != NULL) {
      ud->errmsg = res;
      ret = generate_error (L, ud, 0);
  } else {
    lua_pushvalue (L, LUA_ENVIRONINDEX);
    lua_setmetatable (L, -2);

    if (pud) *pud = ud;
    ret = 1;
  }

  return ret;
}

static int gmatch_exec (TUserdata *ud, TArgExec *argE) {
  seteflags (ud, argE);
  if (argE->startoffset > 0)
    ud->r.not_bol = 1;
  argE->text += argE->startoffset;
  argE->textlen -= argE->startoffset;
  if (argE->eflags & GNU_BACKWARD)
    return re_search (&ud->r, argE->text, argE->textlen, argE->textlen, -argE->textlen, &ud->match);
  else
    return re_search (&ud->r, argE->text, argE->textlen, 0, argE->textlen, &ud->match);
}

static void gmatch_pushsubject (lua_State *L, TArgExec *argE) {
  lua_pushlstring (L, argE->text, argE->textlen);
}

static int findmatch_exec (TGnu *ud, TArgExec *argE) {
  argE->text += argE->startoffset;
  argE->textlen -= argE->startoffset;
  seteflags (ud, argE);
  if (argE->eflags & GNU_BACKWARD)
    return re_search (&ud->r, argE->text, argE->textlen, argE->textlen, -argE->textlen, &ud->match);
  else
    return re_search (&ud->r, argE->text, argE->textlen, 0, argE->textlen, &ud->match);
}

static int gsub_exec (TGnu *ud, TArgExec *argE, int st) {
  seteflags (ud, argE);
  if (st > 0)
    ud->r.not_bol = 1;
  if (argE->eflags & GNU_BACKWARD)
    return re_search (&ud->r, argE->text + st, argE->textlen - st, argE->textlen - st, -(argE->textlen - st), &ud->match);
  else
    return re_search (&ud->r, argE->text + st, argE->textlen - st, 0, argE->textlen - st, &ud->match);
}

static int split_exec (TGnu *ud, TArgExec *argE, int offset) {
  seteflags (ud, argE);
  if (offset > 0)
    ud->r.not_bol = 1;
  if (argE->eflags & GNU_BACKWARD)
    return re_search (&ud->r, argE->text + offset, argE->textlen - offset, argE->textlen - offset, -(argE->textlen - offset), &ud->match);
  else
    return re_search (&ud->r, argE->text + offset, argE->textlen - offset, 0, argE->textlen - offset, &ud->match);
}

static int Gnu_gc (lua_State *L) {
  TGnu *ud = check_ud (L);
  if (ud->freed == 0) {           /* precaution against "manual" __gc calling */
    ud->freed = 1;
    regfree (&ud->r);
    free (ud->match.start);
    free (ud->match.end);
  }
  return 0;
}

static int Gnu_tostring (lua_State *L) {
  TGnu *ud = check_ud (L);
  if (ud->freed == 0)
    lua_pushfstring (L, "%s (%p)", REX_TYPENAME, (void*)ud);
  else
    lua_pushfstring (L, "%s (deleted)", REX_TYPENAME);
  return 1;
}

static flag_pair gnu_flags[] =
{
  { "not_bol",  GNU_NOTBOL },
  { "not_eol",  GNU_NOTEOL },
  { "backward", GNU_BACKWARD },
/*---------------------------------------------------------------------------*/
  { NULL, 0 }
};

static flag_pair gnu_syntax_flags[] = {
  /* Syntax flag sets. */
  { "SYNTAX_EMACS",                     RE_SYNTAX_EMACS },
  { "SYNTAX_AWK",                       RE_SYNTAX_AWK },
  { "SYNTAX_GNU_AWK",                   RE_SYNTAX_GNU_AWK },
  { "SYNTAX_POSIX_AWK",                 RE_SYNTAX_POSIX_AWK },
  { "SYNTAX_POSIX_AWK",                 RE_SYNTAX_POSIX_AWK },
  { "SYNTAX_EGREP",                     RE_SYNTAX_EGREP },
  { "SYNTAX_POSIX_EGREP",               RE_SYNTAX_POSIX_EGREP },
  { "SYNTAX_ED",                        RE_SYNTAX_ED },
  { "SYNTAX_SED",                       RE_SYNTAX_SED },
  { "SYNTAX_POSIX_AWK",                 RE_SYNTAX_POSIX_AWK },
  { "SYNTAX_GREP",                      RE_SYNTAX_GREP },
  { "SYNTAX_POSIX_BASIC",               RE_SYNTAX_POSIX_BASIC },
  { "SYNTAX_POSIX_MINIMAL_BASIC",       RE_SYNTAX_POSIX_MINIMAL_BASIC },
  { "SYNTAX_POSIX_EXTENDED",            RE_SYNTAX_POSIX_EXTENDED },
  { "SYNTAX_POSIX_MINIMAL_EXTENDED",    RE_SYNTAX_POSIX_MINIMAL_EXTENDED },

  /* Individual syntax flags. */
  { "BACKSLASH_ESCAPE_IN_LISTS",        RE_BACKSLASH_ESCAPE_IN_LISTS },
  { "BK_PLUS_QM",                       RE_BK_PLUS_QM },
  { "CHAR_CLASSES",                     RE_CHAR_CLASSES },
  { "CONTEXT_INDEP_ANCHORS",            RE_CONTEXT_INDEP_ANCHORS },
  { "CONTEXT_INDEP_OPS",                RE_CONTEXT_INDEP_OPS },
  { "CONTEXT_INVALID_OPS",              RE_CONTEXT_INVALID_OPS },
  { "DOT_NEWLINE",                      RE_DOT_NEWLINE },
  { "DOT_NOT_NULL",                     RE_DOT_NOT_NULL },
  { "HAT_LISTS_NOT_NEWLINE",            RE_HAT_LISTS_NOT_NEWLINE },
  { "INTERVALS",                        RE_INTERVALS },
  { "LIMITED_OPS",                      RE_LIMITED_OPS },
  { "NEWLINE_ALT",                      RE_NEWLINE_ALT },
  { "NO_BK_BRACES",                     RE_NO_BK_BRACES },
  { "NO_BK_PARENS",                     RE_NO_BK_PARENS },
  { "NO_BK_REFS",                       RE_NO_BK_REFS },
  { "NO_BK_VBAR",                       RE_NO_BK_VBAR },
  { "NO_EMPTY_RANGES",                  RE_NO_EMPTY_RANGES },
  { "UNMATCHED_RIGHT_PAREN_ORD",        RE_UNMATCHED_RIGHT_PAREN_ORD },
  { "NO_POSIX_BACKTRACKING",            RE_NO_POSIX_BACKTRACKING },
  { "NO_GNU_OPS",                       RE_NO_GNU_OPS },
  { "DEBUG",                            RE_DEBUG },
  { "INVALID_INTERVAL_ORD",             RE_INVALID_INTERVAL_ORD },
  { "ICASE",                            RE_ICASE },
  { "CARET_ANCHORS_HERE",               RE_CARET_ANCHORS_HERE },
  { "CONTEXT_INVALID_DUP",              RE_CONTEXT_INVALID_DUP },
  { "NO_SUB",                           RE_NO_SUB },
#ifdef RE_PLAIN
  { "PLAIN",                            RE_PLAIN },
#endif
/*---------------------------------------------------------------------------*/
  { NULL, 0 }
};

static int Gnu_get_flags (lua_State *L) {
  const flag_pair* fps[] = { gnu_flags, gnu_syntax_flags, NULL };
  return get_flags (L, fps);
}

static const luaL_reg gnumeta[] = {
  { "exec",       ud_exec },
  { "tfind",      ud_tfind },    /* old match */
  { "find",       ud_find },
  { "match",      ud_match },
  { "__gc",       Gnu_gc },
  { "__tostring", Gnu_tostring },
  { NULL, NULL}
};

static const luaL_reg rexlib[] = {
  { "match",      match },
  { "find",       find },
  { "gmatch",     gmatch },
  { "gsub",       gsub },
  { "split",      split },
  { "new",        ud_new },
  { "flags",      Gnu_get_flags },
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
  luaL_register (L, NULL, gnumeta);

  /* register functions */
  luaL_register (L, REX_LIBNAME, rexlib);
  lua_pushliteral (L, REX_VERSION" (for GNU regexes)");
  lua_setfield (L, -2, "_VERSION");
  return 1;
}
