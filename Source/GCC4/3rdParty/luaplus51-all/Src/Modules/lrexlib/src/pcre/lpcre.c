/* lpcre.c - Lua binding of PCRE library */
/* See Copyright Notice in the file LICENSE */

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <pcre.h>

#include "lua.h"
#include "lauxlib.h"
#include "../common.h"

extern int Lpcre_get_flags (lua_State *L);
extern int Lpcre_config (lua_State *L);
extern flag_pair pcre_error_flags[];

/* These 2 settings may be redefined from the command-line or the makefile.
 * They should be kept in sync between themselves and with the target name.
 */
#ifndef REX_LIBNAME
#  define REX_LIBNAME "rex_pcre"
#endif
#ifndef REX_OPENLIB
#  define REX_OPENLIB luaopen_rex_pcre
#endif

#define REX_TYPENAME REX_LIBNAME"_regex"

#define ALG_CFLAGS_DFLT 0
#define ALG_EFLAGS_DFLT 0

static int getcflags (lua_State *L, int pos);
#define ALG_GETCFLAGS(L,pos)  getcflags(L, pos)

static void checkarg_compile (lua_State *L, int pos, TArgComp *argC);
#define ALG_GETCARGS(a,b,c)  checkarg_compile(a,b,c)

#define ALG_NOMATCH(res)   ((res) == PCRE_ERROR_NOMATCH)
#define ALG_ISMATCH(res)   ((res) >= 0)
#define ALG_SUBBEG(ud,n)   ud->match[n+n]
#define ALG_SUBEND(ud,n)   ud->match[n+n+1]
#define ALG_SUBLEN(ud,n)   (ALG_SUBEND(ud,n) - ALG_SUBBEG(ud,n))
#define ALG_SUBVALID(ud,n) (ALG_SUBBEG(ud,n) >= 0)
#define ALG_NSUB(ud)       ((int)ud->ncapt)

#define ALG_PUSHSUB(L,ud,text,n) \
  lua_pushlstring (L, (text) + ALG_SUBBEG(ud,n), ALG_SUBLEN(ud,n))

#define ALG_PUSHSUB_OR_FALSE(L,ud,text,n) \
  (ALG_SUBVALID(ud,n) ? ALG_PUSHSUB (L,ud,text,n) : lua_pushboolean (L,0))

#define ALG_PUSHSTART(L,ud,offs,n)   lua_pushinteger(L, (offs) + ALG_SUBBEG(ud,n) + 1)
#define ALG_PUSHEND(L,ud,offs,n)     lua_pushinteger(L, (offs) + ALG_SUBEND(ud,n))
#define ALG_PUSHOFFSETS(L,ud,offs,n) \
  (ALG_PUSHSTART(L,ud,offs,n), ALG_PUSHEND(L,ud,offs,n))

#define ALG_BASE(st)  0
#define ALG_PULL
#define ALG_USERETRY

typedef struct {
  pcre       * pr;
  pcre_extra * extra;
  int        * match;
  int          ncapt;
  const unsigned char * tables;
  int          freed;
} TPcre;

#define TUserdata TPcre

#if PCRE_MAJOR >= 4
static void do_named_subpatterns (lua_State *L, TPcre *ud, const char *text);
#  define DO_NAMED_SUBPATTERNS do_named_subpatterns
#endif

#include "../algo.h"

/* Locations of the 2 permanent tables in the function environment */
#define INDEX_CHARTABLES_META  1      /* chartables type's metatable */
#define INDEX_CHARTABLES_LINK  2      /* link chartables to compiled regex */

const char chartables_typename[] = "chartables";

/*  Functions
 ******************************************************************************
 */

static void push_chartables_meta (lua_State *L) {
  lua_pushinteger (L, INDEX_CHARTABLES_META);
  lua_rawget (L, LUA_ENVIRONINDEX);
}

static int getcflags (lua_State *L, int pos) {
  switch (lua_type (L, pos)) {
    case LUA_TNONE:
    case LUA_TNIL:
      return ALG_CFLAGS_DFLT;
    case LUA_TNUMBER:
      return lua_tointeger (L, pos);
    case LUA_TSTRING: {
      const char *s = lua_tostring (L, pos);
      int res = 0, ch;
      while ((ch = *s++) != '\0') {
        if (ch == 'i') res |= PCRE_CASELESS;
        else if (ch == 'm') res |= PCRE_MULTILINE;
        else if (ch == 's') res |= PCRE_DOTALL;
        else if (ch == 'x') res |= PCRE_EXTENDED;
        else if (ch == 'U') res |= PCRE_UNGREEDY;
        else if (ch == 'X') res |= PCRE_EXTRA;
      }
      return res;
    }
    default:
      return luaL_typeerror (L, pos, "number or string");
  }
}

static int generate_error (lua_State *L, const TPcre *ud, int errcode) {
  const char *key = get_flag_key (pcre_error_flags, errcode);
  (void) ud;
  if (key)
    return luaL_error (L, "error PCRE_%s", key);
  else
    return luaL_error (L, "PCRE error code %d", errcode);
}

#if PCRE_MAJOR >= 6
/* method r:dfa_exec (s, [st], [ef], [ovecsize], [wscount]) */
static void checkarg_dfa_exec (lua_State *L, TArgExec *argE, TPcre **ud) {
  *ud = check_ud (L);
  argE->text = luaL_checklstring (L, 2, &argE->textlen);
  argE->startoffset = get_startoffset (L, 3, argE->textlen);
  argE->eflags = luaL_optint (L, 4, ALG_EFLAGS_DFLT);
  argE->ovecsize = luaL_optint (L, 5, 100);
  argE->wscount = luaL_optint (L, 6, 50);
}
#endif

static int Lpcre_maketables (lua_State *L) {
  *(const void**)lua_newuserdata (L, sizeof(void*)) = pcre_maketables();
  push_chartables_meta (L);
  lua_setmetatable (L, -2);
  return 1;
}

static void **check_chartables (lua_State *L, int pos) {
  void **q;
  /* Compare the metatable against the C function environment. */
  if (lua_getmetatable(L, pos)) {
    push_chartables_meta (L);
    if (lua_rawequal(L, -1, -2) &&
        (q = (void **)lua_touserdata(L, pos)) != NULL) {
      lua_pop(L, 2);
      return q;
    }
  }
  luaL_argerror(L, pos, lua_pushfstring (L, "not a %s", chartables_typename));
  return NULL;
}

static int chartables_gc (lua_State *L) {
  void **ud = check_chartables (L, 1);
  if (*ud) {
    pcre_free (*ud);
    *ud = NULL;
  }
  return 0;
}

static void checkarg_compile (lua_State *L, int pos, TArgComp *argC) {
  argC->locale = NULL;
  argC->tables = NULL;
  if (!lua_isnoneornil (L, pos)) {
    if (lua_isstring (L, pos))
      argC->locale = lua_tostring (L, pos);
    else {
      argC->tablespos = pos;
      argC->tables = *check_chartables (L, pos);
    }
  }
}

static int compile_regex (lua_State *L, const TArgComp *argC, TPcre **pud) {
  const char *error;
  int erroffset;
  TPcre *ud;
  const unsigned char *tables = NULL;

  ud = (TPcre*)lua_newuserdata (L, sizeof (TPcre));
  memset (ud, 0, sizeof (TPcre));           /* initialize all members to 0 */
  lua_pushvalue (L, LUA_ENVIRONINDEX);
  lua_setmetatable (L, -2);

  if (argC->locale) {
    char old_locale[256];
    strcpy (old_locale, setlocale (LC_CTYPE, NULL));  /* store the locale */
    if (NULL == setlocale (LC_CTYPE, argC->locale))   /* set new locale */
      return luaL_error (L, "cannot set locale");
    ud->tables = tables = pcre_maketables ();  /* make tables with new locale */
    setlocale (LC_CTYPE, old_locale);          /* restore the old locale */
  }
  else if (argC->tables) {
    tables = argC->tables;
    lua_pushinteger (L, INDEX_CHARTABLES_LINK);
    lua_rawget (L, LUA_ENVIRONINDEX);
    lua_pushvalue (L, -2);
    lua_pushvalue (L, argC->tablespos);
    lua_rawset (L, -3);
    lua_pop (L, 1);
  }

  ud->pr = pcre_compile (argC->pattern, argC->cflags, &error, &erroffset, tables);
  if (!ud->pr)
    return luaL_error (L, "%s (pattern offset: %d)", error, erroffset + 1);

  ud->extra = pcre_study (ud->pr, 0, &error);
  if (error) return luaL_error (L, "%s", error);

  pcre_fullinfo (ud->pr, ud->extra, PCRE_INFO_CAPTURECOUNT, &ud->ncapt);
  /* need (2 ints per capture, plus one for substring match) * 3/2 */
  ud->match = (int *) Lmalloc (L, (ALG_NSUB(ud) + 1) * 3 * sizeof (int));

  if (pud) *pud = ud;
  return 1;
}

#if PCRE_MAJOR >= 4
/* the target table must be on lua stack top */
static void do_named_subpatterns (lua_State *L, TPcre *ud, const char *text) {
  int i, namecount, name_entry_size;
  unsigned char *name_table, *tabptr;

  /* do named subpatterns - NJG */
  pcre_fullinfo (ud->pr, ud->extra, PCRE_INFO_NAMECOUNT, &namecount);
  if (namecount <= 0)
    return;
  pcre_fullinfo (ud->pr, ud->extra, PCRE_INFO_NAMETABLE, &name_table);
  pcre_fullinfo (ud->pr, ud->extra, PCRE_INFO_NAMEENTRYSIZE, &name_entry_size);
  tabptr = name_table;
  for (i = 0; i < namecount; i++) {
    int n = (tabptr[0] << 8) | tabptr[1]; /* number of the capturing parenthesis */
    if (n > 0 && n <= ALG_NSUB(ud)) {   /* check range */
      lua_pushstring (L, (char *)tabptr + 2); /* name of the capture, zero terminated */
      ALG_PUSHSUB_OR_FALSE (L, ud, text, n);
      lua_rawset (L, -3);
    }
    tabptr += name_entry_size;
  }
}
#endif /* #if PCRE_MAJOR >= 4 */

#if PCRE_MAJOR >= 6
static int Lpcre_dfa_exec (lua_State *L)
{
  TArgExec argE;
  TPcre *ud;
  int res;
  int *buf, *ovector, *wspace;

  checkarg_dfa_exec (L, &argE, &ud);
  buf = (int*) Lmalloc (L, (argE.ovecsize + argE.wscount) * sizeof(int));
  ovector = buf;
  wspace = buf + argE.ovecsize;

  res = pcre_dfa_exec (ud->pr, ud->extra, argE.text, (int)argE.textlen,
    argE.startoffset, argE.eflags, ovector, argE.ovecsize, wspace, argE.wscount);

  if (ALG_ISMATCH (res) || res == PCRE_ERROR_PARTIAL) {
    int i;
    int max = (res>0) ? res : (res==0) ? (int)argE.ovecsize/2 : 1;
    lua_pushinteger (L, ovector[0] + 1);         /* 1-st return value */
    lua_newtable (L);                            /* 2-nd return value */
    for (i=0; i<max; i++) {
      lua_pushinteger (L, ovector[i+i+1]);
      lua_rawseti (L, -2, i+1);
    }
    lua_pushinteger (L, res);                    /* 3-rd return value */
    free (buf);
    return 3;
  }
  else {
    free (buf);
    if (ALG_NOMATCH (res))
      return lua_pushnil (L), 1;
    else
      return generate_error (L, ud, res);
  }
}
#endif /* #if PCRE_MAJOR >= 6 */

#ifdef ALG_USERETRY
  static int gmatch_exec (TUserdata *ud, TArgExec *argE, int retry) {
    int eflags = retry ? (argE->eflags|PCRE_NOTEMPTY|PCRE_ANCHORED) : argE->eflags;
    return pcre_exec (ud->pr, ud->extra, argE->text, argE->textlen,
      argE->startoffset, eflags, ud->match, (ALG_NSUB(ud) + 1) * 3);
  }
#else
  static int gmatch_exec (TUserdata *ud, TArgExec *argE) {
    return pcre_exec (ud->pr, ud->extra, argE->text, argE->textlen,
      argE->startoffset, argE->eflags, ud->match, (ALG_NSUB(ud) + 1) * 3);
  }
#endif

static void gmatch_pushsubject (lua_State *L, TArgExec *argE) {
  lua_pushlstring (L, argE->text, argE->textlen);
}

static int findmatch_exec (TPcre *ud, TArgExec *argE) {
  return pcre_exec (ud->pr, ud->extra, argE->text, argE->textlen,
    argE->startoffset, argE->eflags, ud->match, (ALG_NSUB(ud) + 1) * 3);
}

#ifdef ALG_USERETRY
  static int gsub_exec (TPcre *ud, TArgExec *argE, int st, int retry) {
    int eflags = retry ? (argE->eflags|PCRE_NOTEMPTY|PCRE_ANCHORED) : argE->eflags;
    return pcre_exec (ud->pr, ud->extra, argE->text, argE->textlen,
      st, eflags, ud->match, (ALG_NSUB(ud) + 1) * 3);
  }
#else
  static int gsub_exec (TPcre *ud, TArgExec *argE, int st) {
    return pcre_exec (ud->pr, ud->extra, argE->text, argE->textlen,
      st, argE->eflags, ud->match, (ALG_NSUB(ud) + 1) * 3);
  }
#endif

static int split_exec (TPcre *ud, TArgExec *argE, int offset) {
  return pcre_exec (ud->pr, ud->extra, argE->text, argE->textlen, offset,
                    argE->eflags, ud->match, (ALG_NSUB(ud) + 1) * 3);
}

static int Lpcre_gc (lua_State *L) {
  TPcre *ud = check_ud (L);
  if (ud->freed == 0) {           /* precaution against "manual" __gc calling */
    ud->freed = 1;
    if (ud->pr)      pcre_free (ud->pr);
    if (ud->extra)   pcre_free (ud->extra);
    if (ud->tables)  pcre_free ((void *)ud->tables);
    free (ud->match);
  }
  return 0;
}

static int Lpcre_tostring (lua_State *L) {
  TPcre *ud = check_ud (L);
  if (ud->freed == 0)
    lua_pushfstring (L, "%s (%p)", REX_TYPENAME, (void*)ud);
  else
    lua_pushfstring (L, "%s (deleted)", REX_TYPENAME);
  return 1;
}

static int chartables_tostring (lua_State *L) {
  void **ud = check_chartables (L, 1);
  lua_pushfstring (L, "%s (%p)", chartables_typename, ud);
  return 1;
}

static int Lpcre_version (lua_State *L) {
  lua_pushstring (L, pcre_version ());
  return 1;
}

static const luaL_reg chartables_meta[] = {
  { "__gc",        chartables_gc },
  { "__tostring",  chartables_tostring },
  { NULL, NULL }
};

static const luaL_reg regex_meta[] = {
  { "exec",        ud_exec },
  { "tfind",       ud_tfind },    /* old name: match */
  { "find",        ud_find },
  { "match",       ud_match },
#if PCRE_MAJOR >= 6
  { "dfa_exec",    Lpcre_dfa_exec },
#endif
  { "__gc",        Lpcre_gc },
  { "__tostring",  Lpcre_tostring },
  { NULL, NULL }
};

static const luaL_reg rexlib[] = {
  { "match",       match },
  { "find",        find },
  { "gmatch",      gmatch },
  { "gsub",        gsub },
  { "split",       split },
  { "new",         ud_new },
  { "plainfind",   plainfind_func },
  { "flags",       Lpcre_get_flags },
  { "version",     Lpcre_version },
  { "maketables",  Lpcre_maketables },
#if PCRE_MAJOR >= 4
  { "config",      Lpcre_config },
#endif
  { NULL, NULL }
};

/* Open the library */
REX_API int REX_OPENLIB (lua_State *L) {
  if (PCRE_MAJOR > atoi (pcre_version ())) {
    return luaL_error (L, "%s requires at least version %d of PCRE library",
      REX_LIBNAME, (int)PCRE_MAJOR);
  }
  /* create a new function environment to serve as a metatable for methods */
  lua_newtable (L);
  lua_pushvalue (L, -1);
  lua_replace (L, LUA_ENVIRONINDEX);
  lua_pushvalue(L, -1); /* mt.__index = mt */
  lua_setfield(L, -2, "__index");
  luaL_register (L, NULL, regex_meta);

  /* register functions */
  luaL_register (L, REX_LIBNAME, rexlib);
  lua_pushliteral (L, REX_VERSION" (for PCRE)");
  lua_setfield (L, -2, "_VERSION");

  /* create a table and register it as a metatable for "chartables" userdata */
  lua_pushinteger (L, INDEX_CHARTABLES_META);
  lua_newtable (L);
  lua_pushliteral (L, "access denied");
  lua_setfield (L, -2, "__metatable");
  luaL_register (L, NULL, chartables_meta);
  lua_rawset (L, LUA_ENVIRONINDEX);

  /* create a table for connecting "chartables" userdata to "regex" userdata */
  lua_pushinteger (L, INDEX_CHARTABLES_LINK);
  lua_newtable (L);
  lua_pushliteral (L, "k");         /* weak keys */
  lua_setfield (L, -2, "__mode");
  lua_pushvalue (L, -1);            /* setmetatable (tb, tb) */
  lua_setmetatable (L, -2);
  lua_rawset (L, LUA_ENVIRONINDEX);

  return 1;
}
