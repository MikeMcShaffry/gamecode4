/* lpcre.c - PCRE regular expression library */
/* See Copyright Notice in the file LICENSE */

#include <pcre.h>
#include "lua.h"
#include "lauxlib.h"
#include "../common.h"

#define VERSION_PCRE (PCRE_MAJOR*100 + PCRE_MINOR)

static flag_pair pcre_flags[] = {
  { "MAJOR",                         PCRE_MAJOR },
  { "MINOR",                         PCRE_MINOR },
/*---------------------------------------------------------------------------*/
  { "CASELESS",                      PCRE_CASELESS },
  { "MULTILINE",                     PCRE_MULTILINE },
  { "DOTALL",                        PCRE_DOTALL },
  { "EXTENDED",                      PCRE_EXTENDED },
  { "ANCHORED",                      PCRE_ANCHORED },
  { "DOLLAR_ENDONLY",                PCRE_DOLLAR_ENDONLY },
  { "EXTRA",                         PCRE_EXTRA },
  { "NOTBOL",                        PCRE_NOTBOL },
  { "NOTEOL",                        PCRE_NOTEOL },
  { "UNGREEDY",                      PCRE_UNGREEDY },
  { "NOTEMPTY",                      PCRE_NOTEMPTY },
  { "UTF8",                          PCRE_UTF8 },
#if VERSION_PCRE >= 400
  { "NO_AUTO_CAPTURE",               PCRE_NO_AUTO_CAPTURE },
  { "NO_UTF8_CHECK",                 PCRE_NO_UTF8_CHECK },
#endif
#if VERSION_PCRE >= 500
  { "AUTO_CALLOUT",                  PCRE_AUTO_CALLOUT },
  { "PARTIAL",                       PCRE_PARTIAL },
#endif
#ifdef PCRE_PARTIAL_SOFT
  { "PARTIAL_SOFT",                  PCRE_PARTIAL_SOFT },
#endif
#if VERSION_PCRE >= 600
  { "DFA_SHORTEST",                  PCRE_DFA_SHORTEST },
  { "DFA_RESTART",                   PCRE_DFA_RESTART },
  { "FIRSTLINE",                     PCRE_FIRSTLINE },
#endif
#if VERSION_PCRE >= 607
  { "DUPNAMES",                      PCRE_DUPNAMES },
  { "NEWLINE_CR",                    PCRE_NEWLINE_CR },
  { "NEWLINE_LF",                    PCRE_NEWLINE_LF },
  { "NEWLINE_CRLF",                  PCRE_NEWLINE_CRLF },
#endif
#if VERSION_PCRE >= 700
  { "NEWLINE_ANY",                   PCRE_NEWLINE_ANY },
#endif
#if VERSION_PCRE >= 701
  { "NEWLINE_ANYCRLF",               PCRE_NEWLINE_ANYCRLF },
#endif
#if VERSION_PCRE >= 704
  { "BSR_ANYCRLF",                   PCRE_BSR_ANYCRLF },
  { "BSR_UNICODE",                   PCRE_BSR_UNICODE },
#endif
#if VERSION_PCRE >= 707
  { "JAVASCRIPT_COMPAT",             PCRE_JAVASCRIPT_COMPAT },
#endif
#ifdef PCRE_NO_START_OPTIMIZE
  { "NO_START_OPTIMIZE",             PCRE_NO_START_OPTIMIZE },
#endif
#ifdef PCRE_NO_START_OPTIMISE
  { "NO_START_OPTIMISE",             PCRE_NO_START_OPTIMISE },
#endif
#ifdef PCRE_PARTIAL_HARD
  { "PARTIAL_HARD",                  PCRE_PARTIAL_HARD },
#endif
#ifdef PCRE_NOTEMPTY_ATSTART
  { "NOTEMPTY_ATSTART",              PCRE_NOTEMPTY_ATSTART },
#endif
#ifdef PCRE_UCP
  { "UCP",                           PCRE_UCP },
#endif
/*---------------------------------------------------------------------------*/
  { "INFO_OPTIONS",                  PCRE_INFO_OPTIONS },
  { "INFO_SIZE",                     PCRE_INFO_SIZE },
  { "INFO_CAPTURECOUNT",             PCRE_INFO_CAPTURECOUNT },
  { "INFO_BACKREFMAX",               PCRE_INFO_BACKREFMAX },
#if VERSION_PCRE >= 400
  { "INFO_FIRSTBYTE",                PCRE_INFO_FIRSTBYTE },
#endif
  { "INFO_FIRSTCHAR",                PCRE_INFO_FIRSTCHAR },
  { "INFO_FIRSTTABLE",               PCRE_INFO_FIRSTTABLE },
  { "INFO_LASTLITERAL",              PCRE_INFO_LASTLITERAL },
#if VERSION_PCRE >= 400
  { "INFO_NAMEENTRYSIZE",            PCRE_INFO_NAMEENTRYSIZE },
  { "INFO_NAMECOUNT",                PCRE_INFO_NAMECOUNT },
  { "INFO_NAMETABLE",                PCRE_INFO_NAMETABLE },
  { "INFO_STUDYSIZE",                PCRE_INFO_STUDYSIZE },
#endif
#if VERSION_PCRE >= 500
  { "INFO_DEFAULT_TABLES",           PCRE_INFO_DEFAULT_TABLES },
#endif
#ifdef PCRE_INFO_OKPARTIAL
  { "INFO_OKPARTIAL",                PCRE_INFO_OKPARTIAL },
#endif
#ifdef PCRE_INFO_JCHANGED
  { "INFO_JCHANGED",                 PCRE_INFO_JCHANGED },
#endif
#ifdef PCRE_INFO_HASCRORLF
  { "INFO_HASCRORLF",                PCRE_INFO_HASCRORLF },
#endif
#ifdef PCRE_INFO_MINLENGTH
  { "INFO_MINLENGTH",                PCRE_INFO_MINLENGTH },
#endif
/*---------------------------------------------------------------------------*/
#if VERSION_PCRE >= 400
  { "EXTRA_STUDY_DATA",              PCRE_EXTRA_STUDY_DATA },
  { "EXTRA_MATCH_LIMIT",             PCRE_EXTRA_MATCH_LIMIT },
  { "EXTRA_CALLOUT_DATA",            PCRE_EXTRA_CALLOUT_DATA },
#endif
#if VERSION_PCRE >= 500
  { "EXTRA_TABLES",                  PCRE_EXTRA_TABLES },
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
  { "EXTRA_MATCH_LIMIT_RECURSION",   PCRE_EXTRA_MATCH_LIMIT_RECURSION },
#endif
#ifdef PCRE_EXTRA_MARK
  { "EXTRA_MARK",                    PCRE_EXTRA_MARK },
#endif
/*---------------------------------------------------------------------------*/
  { NULL, 0 }
};

flag_pair pcre_error_flags[] = {
  { "ERROR_NOMATCH",                 PCRE_ERROR_NOMATCH },
  { "ERROR_NULL",                    PCRE_ERROR_NULL },
  { "ERROR_BADOPTION",               PCRE_ERROR_BADOPTION },
  { "ERROR_BADMAGIC",                PCRE_ERROR_BADMAGIC },
#if VERSION_PCRE >= 700
  { "ERROR_UNKNOWN_OPCODE",          PCRE_ERROR_UNKNOWN_OPCODE },
#endif
  { "ERROR_UNKNOWN_NODE",            PCRE_ERROR_UNKNOWN_NODE },
  { "ERROR_NOMEMORY",                PCRE_ERROR_NOMEMORY },
  { "ERROR_NOSUBSTRING",             PCRE_ERROR_NOSUBSTRING },
#if VERSION_PCRE >= 400
  { "ERROR_MATCHLIMIT",              PCRE_ERROR_MATCHLIMIT },
  { "ERROR_CALLOUT",                 PCRE_ERROR_CALLOUT },
  { "ERROR_BADUTF8",                 PCRE_ERROR_BADUTF8 },
  { "ERROR_BADUTF8_OFFSET",          PCRE_ERROR_BADUTF8_OFFSET },
#endif
#if VERSION_PCRE >= 500
  { "ERROR_PARTIAL",                 PCRE_ERROR_PARTIAL },
  { "ERROR_BADPARTIAL",              PCRE_ERROR_BADPARTIAL },
  { "ERROR_INTERNAL",                PCRE_ERROR_INTERNAL },
  { "ERROR_BADCOUNT",                PCRE_ERROR_BADCOUNT },
#endif
#if VERSION_PCRE >= 600
  { "ERROR_DFA_UITEM",               PCRE_ERROR_DFA_UITEM },
  { "ERROR_DFA_UCOND",               PCRE_ERROR_DFA_UCOND },
  { "ERROR_DFA_UMLIMIT",             PCRE_ERROR_DFA_UMLIMIT },
  { "ERROR_DFA_WSSIZE",              PCRE_ERROR_DFA_WSSIZE },
  { "ERROR_DFA_RECURSE",             PCRE_ERROR_DFA_RECURSE },
#endif
#if VERSION_PCRE >= 607
  { "ERROR_RECURSIONLIMIT",          PCRE_ERROR_RECURSIONLIMIT },
#endif
#if VERSION_PCRE >= 700
  { "ERROR_BADNEWLINE",              PCRE_ERROR_BADNEWLINE },
#endif
#ifdef PCRE_ERROR_NULLWSLIMIT
  { "ERROR_NULLWSLIMIT",             PCRE_ERROR_NULLWSLIMIT },
#endif
#ifdef PCRE_ERROR_BADOFFSET
  { "ERROR_BADOFFSET",               PCRE_ERROR_BADOFFSET },
#endif
#ifdef PCRE_ERROR_SHORTUTF8
  { "ERROR_SHORTUTF8",               PCRE_ERROR_SHORTUTF8 },
#endif
/*---------------------------------------------------------------------------*/
  { NULL, 0 }
};

#if VERSION_PCRE >= 400
static flag_pair pcre_config_flags[] = {
  { "CONFIG_UTF8",                   PCRE_CONFIG_UTF8 },
  { "CONFIG_NEWLINE",                PCRE_CONFIG_NEWLINE },
  { "CONFIG_LINK_SIZE",              PCRE_CONFIG_LINK_SIZE },
  { "CONFIG_POSIX_MALLOC_THRESHOLD", PCRE_CONFIG_POSIX_MALLOC_THRESHOLD },
  { "CONFIG_MATCH_LIMIT",            PCRE_CONFIG_MATCH_LIMIT },
  { "CONFIG_STACKRECURSE",           PCRE_CONFIG_STACKRECURSE },
#if VERSION_PCRE >= 500
  { "CONFIG_UNICODE_PROPERTIES",     PCRE_CONFIG_UNICODE_PROPERTIES },
#endif
#if VERSION_PCRE >= 650
  { "CONFIG_MATCH_LIMIT_RECURSION",  PCRE_CONFIG_MATCH_LIMIT_RECURSION },
#endif
#if VERSION_PCRE >= 704
  { "CONFIG_BSR",                    PCRE_CONFIG_BSR },
#endif
/*---------------------------------------------------------------------------*/
  { NULL, 0 }
};

int Lpcre_config (lua_State *L) {
  int val;
  flag_pair *fp;
  if (lua_istable (L, 1))
    lua_settop (L, 1);
  else
    lua_newtable (L);
  for (fp = pcre_config_flags; fp->key; ++fp) {
    if (0 == pcre_config (fp->val, &val)) {
      lua_pushinteger (L, val);
      lua_setfield (L, -2, fp->key);
    }
  }
  return 1;
}
#endif /* #if VERSION_PCRE >= 400 */

int Lpcre_get_flags (lua_State *L) {
  const flag_pair* fps[] = { pcre_flags, pcre_error_flags, NULL };
  return get_flags (L, fps);
}

