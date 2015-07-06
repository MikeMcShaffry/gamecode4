/* lonig.c - Lua binding of Oniguruma library */
/* See Copyright Notice in the file LICENSE */

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <oniguruma.h>

#include "lua.h"
#include "lauxlib.h"
#include "../common.h"

extern int LOnig_get_flags (lua_State *L);

/* These 2 settings may be redefined from the command-line or the makefile.
 * They should be kept in sync between themselves and with the target name.
 */
#ifndef REX_LIBNAME
#  define REX_LIBNAME "rex_onig"
#endif
#ifndef REX_OPENLIB
#  define REX_OPENLIB luaopen_rex_onig
#endif

#define REX_TYPENAME REX_LIBNAME"_regex"

#define ALG_CFLAGS_DFLT ONIG_OPTION_NONE
#define ALG_EFLAGS_DFLT 0

static int getcflags (lua_State *L, int pos);
#define ALG_GETCFLAGS(L,pos)  getcflags(L, pos)

static void checkarg_compile (lua_State *L, int pos, TArgComp *argC);
#define ALG_GETCARGS(a,b,c)  checkarg_compile(a,b,c)

#define ALG_NOMATCH(res)   ((res) == ONIG_MISMATCH)
#define ALG_ISMATCH(res)   ((res) >= 0)
#define ALG_SUBBEG(ud,n)   ud->region->beg[n]
#define ALG_SUBEND(ud,n)   ud->region->end[n]
#define ALG_SUBLEN(ud,n)   (ALG_SUBEND(ud,n) - ALG_SUBBEG(ud,n))
#define ALG_SUBVALID(ud,n) (ALG_SUBBEG(ud,n) >= 0)
#define ALG_NSUB(ud)       onig_number_of_captures(ud->reg)

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
/* #define ALG_USERETRY */

typedef struct {
  regex_t *reg;
  OnigRegion *region;
  OnigErrorInfo einfo;
} TOnig;

#define TUserdata TOnig

static void do_named_subpatterns (lua_State *L, TOnig *ud, const char *text);
#  define DO_NAMED_SUBPATTERNS do_named_subpatterns

#include "../algo.h"

#define CUC const unsigned char*

/*  Functions
 ******************************************************************************
 */

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
        if (ch == 'i') res |= ONIG_OPTION_IGNORECASE;
        else if (ch == 'm') res |= ONIG_OPTION_NEGATE_SINGLELINE;
        else if (ch == 's') res |= ONIG_OPTION_MULTILINE;
        else if (ch == 'x') res |= ONIG_OPTION_EXTEND;
        /* else if (ch == 'U') res |= PCRE_UNGREEDY; */
        /* else if (ch == 'X') res |= PCRE_EXTRA; */
      }
      return res;
    }
    default:
      return luaL_typeerror (L, pos, "number or string");
  }
}

static int generate_error (lua_State *L, const TOnig *ud, int errcode) {
  char buf [ONIG_MAX_ERROR_MESSAGE_LEN];
  onig_error_code_to_str(buf, errcode, ud->einfo);
  return luaL_error(L, buf);
}

typedef struct {
  const char * name;
  void * value;
} EncPair;

/* ATTENTION:
   This array must always be kept alphabetically sorted, as it's used in the
   binary search, so take care when manually inserting new elements.
 */
static EncPair Encodings[] = {
  { "ASCII",          ONIG_ENCODING_ASCII },
  { "BIG5",           ONIG_ENCODING_BIG5 },
  { "CP1251",         ONIG_ENCODING_CP1251 },
  { "EUC_CN",         ONIG_ENCODING_EUC_CN },
  { "EUC_JP",         ONIG_ENCODING_EUC_JP },
  { "EUC_KR",         ONIG_ENCODING_EUC_KR },
  { "EUC_TW",         ONIG_ENCODING_EUC_TW },
  { "GB18030",        ONIG_ENCODING_GB18030 },
  { "ISO_8859_1",     ONIG_ENCODING_ISO_8859_1 },
  { "ISO_8859_10",    ONIG_ENCODING_ISO_8859_10 },
  { "ISO_8859_11",    ONIG_ENCODING_ISO_8859_11 },
  { "ISO_8859_13",    ONIG_ENCODING_ISO_8859_13 },
  { "ISO_8859_14",    ONIG_ENCODING_ISO_8859_14 },
  { "ISO_8859_15",    ONIG_ENCODING_ISO_8859_15 },
  { "ISO_8859_16",    ONIG_ENCODING_ISO_8859_16 },
  { "ISO_8859_2",     ONIG_ENCODING_ISO_8859_2 },
  { "ISO_8859_3",     ONIG_ENCODING_ISO_8859_3 },
  { "ISO_8859_4",     ONIG_ENCODING_ISO_8859_4 },
  { "ISO_8859_5",     ONIG_ENCODING_ISO_8859_5 },
  { "ISO_8859_6",     ONIG_ENCODING_ISO_8859_6 },
  { "ISO_8859_7",     ONIG_ENCODING_ISO_8859_7 },
  { "ISO_8859_8",     ONIG_ENCODING_ISO_8859_8 },
  { "ISO_8859_9",     ONIG_ENCODING_ISO_8859_9 },
/*{ "KOI8",           ONIG_ENCODING_KOI8 }, */
  { "KOI8_R",         ONIG_ENCODING_KOI8_R },
  { "SJIS",           ONIG_ENCODING_SJIS },
  { "UNDEF",          ONIG_ENCODING_UNDEF },
  { "UTF16_BE",       ONIG_ENCODING_UTF16_BE },
  { "UTF16_LE",       ONIG_ENCODING_UTF16_LE },
  { "UTF32_BE",       ONIG_ENCODING_UTF32_BE },
  { "UTF32_LE",       ONIG_ENCODING_UTF32_LE },
  { "UTF8",           ONIG_ENCODING_UTF8 },
};

/* ATTENTION:
   This array must always be kept alphabetically sorted, as it's used in the
   binary search, so take care when manually inserting new elements.
 */
static EncPair Syntaxes[] = {
  { "ASIS",           ONIG_SYNTAX_ASIS },
/*{ "DEFAULT",        ONIG_SYNTAX_DEFAULT }, */
  { "EMACS",          ONIG_SYNTAX_EMACS },
  { "GNU_REGEX",      ONIG_SYNTAX_GNU_REGEX },
  { "GREP",           ONIG_SYNTAX_GREP },
  { "JAVA",           ONIG_SYNTAX_JAVA },
  { "PERL",           ONIG_SYNTAX_PERL },
  { "PERL_NG",        ONIG_SYNTAX_PERL_NG },
  { "POSIX_BASIC",    ONIG_SYNTAX_POSIX_BASIC },
  { "POSIX_EXTENDED", ONIG_SYNTAX_POSIX_EXTENDED },
  { "RUBY",           ONIG_SYNTAX_RUBY },
};

static int fcmp(const void *p1, const void *p2) {
  return strcmp(((EncPair*)p1)->name, ((EncPair*)p2)->name);
}

static const char *getlocale (lua_State *L, int pos) {
  EncPair key;
  if ((key.name = luaL_optstring(L, pos, NULL)) == NULL)
    return (const char*)ONIG_ENCODING_ASCII;
  else {
    EncPair *pair = (EncPair*) bsearch(&key, Encodings,
      sizeof(Encodings)/sizeof(EncPair), sizeof(EncPair), fcmp);
    if (pair != NULL)
      return (const char*)pair->value;
    else {
      luaL_argerror(L, pos, "invalid or unsupported encoding string");
      return NULL;
    }
  }
}

static void *getsyntax (lua_State *L, int pos) {
  EncPair key, *found;
  if ((key.name = luaL_optstring(L, pos, NULL)) == NULL)
    return ONIG_SYNTAX_DEFAULT;
  found = (EncPair*) bsearch(&key, Syntaxes, sizeof(Syntaxes)/sizeof(EncPair),
          sizeof(EncPair), fcmp);
  if (found == NULL)
    luaL_argerror(L, pos, "invalid or unsupported syntax string");
  return found->value;
}

static void checkarg_compile (lua_State *L, int pos, TArgComp *argC) {
  argC->locale = getlocale (L, pos);
  argC->syntax = getsyntax (L, pos + 1);
}

/*
   rex.setdefaultsyntax (syntax)
   @param syntax: one of the predefined strings listed in array 'Syntaxes'
   @return: nothing
*/
static int LOnig_setdefaultsyntax (lua_State *L) {
  (void)luaL_checkstring(L, 1);
  onig_set_default_syntax(getsyntax(L, 1));
  return 0;
}

static int compile_regex (lua_State *L, const TArgComp *argC, TOnig **pud) {
  TOnig *ud;
  int r;

  ud = (TOnig*)lua_newuserdata (L, sizeof (TOnig));
  memset (ud, 0, sizeof (TOnig));           /* initialize all members to 0 */
  lua_pushvalue (L, LUA_ENVIRONINDEX);
  lua_setmetatable (L, -2);

  r = onig_new(&ud->reg, (CUC)argC->pattern, (CUC)argC->pattern + argC->patlen,
    argC->cflags, (OnigEncoding)argC->locale, (OnigSyntaxType*)argC->syntax,
    &ud->einfo);
  if (r != ONIG_NORMAL)
    return generate_error(L, ud, r);

  if ((ud->region = onig_region_new()) == NULL)
    return luaL_error(L, "`onig_region_new' failed");

  if (pud) *pud = ud;
  return 1;
}

typedef struct {
  lua_State *L;
  TOnig *ud;
  const char *text;
} TNameArg;

static int name_callback (const UChar *name, const UChar *name_end,
      int ngroups, int *groupnumlist, regex_t *reg, void *arg)
{
  (void) ngroups;
  (void) groupnumlist;
  TNameArg *A = (TNameArg*)arg;
  int num = onig_name_to_backref_number(reg, name, name_end, A->ud->region);
  lua_pushlstring (A->L, (const char*)name, name_end - name);
  ALG_PUSHSUB_OR_FALSE (A->L, A->ud, A->text, num);
  lua_rawset (A->L, -3);
  return 0;
}

/* the target table must be on lua stack top */
static void do_named_subpatterns (lua_State *L, TOnig *ud, const char *text) {
  if (onig_number_of_names (ud->reg) > 0) {
    TNameArg A = { L, ud, text };
    onig_foreach_name(ud->reg, name_callback, &A);
  }
}

static int findmatch_exec (TUserdata *ud, TArgExec *argE) {
  const char *end = argE->text + argE->textlen;
  onig_region_clear(ud->region);
  return onig_search (ud->reg, (CUC)argE->text, (CUC)end,
                      (CUC)argE->text + argE->startoffset, (CUC)end,
                      ud->region, argE->eflags);
}

static void gmatch_pushsubject (lua_State *L, TArgExec *argE) {
  lua_pushlstring (L, argE->text, argE->textlen);
}

static int gmatch_exec (TOnig *ud, TArgExec *argE) {
  return findmatch_exec(ud, argE);
}

static int gsub_exec (TOnig *ud, TArgExec *argE, int st) {
  const char *end = argE->text + argE->textlen;
  onig_region_clear(ud->region);
  return onig_search (ud->reg, (CUC)argE->text, (CUC)end, (CUC)argE->text + st,
    (CUC)end, ud->region, argE->eflags);
}

static int split_exec (TOnig *ud, TArgExec *argE, int st) {
  return gsub_exec(ud, argE, st);
}

static int LOnig_gc (lua_State *L) {
  TOnig *ud = check_ud (L);
  if (ud->reg) {           /* precaution against "manual" __gc calling */
    onig_free (ud->reg);
    ud->reg = NULL;
  }
  if (ud->region) {
    onig_region_free (ud->region, 1);
    ud->region = NULL;
  }
  return 0;
}

static int LOnig_tostring (lua_State *L) {
  TOnig *ud = check_ud (L);
  if (ud->reg)
    lua_pushfstring (L, "%s (%p)", REX_TYPENAME, (void*)ud);
  else
    lua_pushfstring (L, "%s (deleted)", REX_TYPENAME);
  return 1;
}

static int LOnig_version (lua_State *L) {
  lua_pushstring (L, onig_version ());
  return 1;
}

static const luaL_reg regex_meta[] = {
  { "exec",        ud_exec },
  { "tfind",       ud_tfind },    /* old name: match */
  { "find",        ud_find },
  { "match",       ud_match },
  { "__gc",        LOnig_gc },
  { "__tostring",  LOnig_tostring },
  { NULL, NULL }
};

static const luaL_reg rexlib[] = {
  { "match",            match },
  { "find",             find },
  { "gmatch",           gmatch },
  { "gsub",             gsub },
  { "split",            split },
  { "new",              ud_new },
  { "plainfind",        plainfind_func },
  { "flags",            LOnig_get_flags },
  { "version",          LOnig_version },
  { "setdefaultsyntax", LOnig_setdefaultsyntax },
  { NULL, NULL }
};

/* Open the library */
REX_API int REX_OPENLIB (lua_State *L) {
  if (ONIGURUMA_VERSION_MAJOR > atoi (onig_version ())) {
    return luaL_error (L, "%s requires at least version %d of Oniguruma library",
      REX_LIBNAME, (int)ONIGURUMA_VERSION_MAJOR);
  }

  onig_init();
  onig_set_default_syntax(ONIG_SYNTAX_RUBY);

  /* create a new function environment to serve as a metatable for methods */
  lua_newtable (L);
  lua_pushvalue (L, -1);
  lua_replace (L, LUA_ENVIRONINDEX);
  lua_pushvalue(L, -1); /* mt.__index = mt */
  lua_setfield(L, -2, "__index");
  luaL_register (L, NULL, regex_meta);

  /* register functions */
  luaL_register (L, REX_LIBNAME, rexlib);
  lua_pushliteral (L, REX_VERSION" (for Oniguruma)");
  lua_setfield (L, -2, "_VERSION");

  return 1;
}
