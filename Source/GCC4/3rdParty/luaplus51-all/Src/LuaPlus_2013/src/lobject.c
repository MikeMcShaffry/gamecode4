/*
** $Id: lobject.c,v 2.22 2006/02/10 17:43:52 roberto Exp $
** Some generic functions over Lua objects
** See Copyright Notice in lua.h
*/

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lobject_c
#define LUA_CORE

#include "lua.h"

#include "ldo.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "lvm.h"

#include <math.h>
#include <assert.h>

NAMESPACE_LUA_BEGIN

static int triow_to_upper(int source)
{
  /* Does not handle locales or non-contiguous alphabetic characters */
  return ((source >= (int)'a') && (source <= (int)'z'))
    ? source - 'a' + 'A'
    : source;
}


/**
   Convert string to floating-point number.

   @param source String to be converted.
   @param endp Pointer to end of the converted string.
   @return A floating-point number.

   The following Extended Backus-Naur form is used
   @verbatim
   double        ::= [ <sign> ]
                     ( <number> |
                       <number> <decimal_point> <number> |
                       <decimal_point> <number> )
                     [ <exponential> [ <sign> ] <number> ]
   number        ::= 1*( <digit> )
   digit         ::= ( '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' )
   exponential   ::= ( 'e' | 'E' )
   sign          ::= ( '-' | '+' )
   decimal_point ::= '.'
   @endverbatim
*/
/* FIXME: Add EBNF for hex-floats */
static double triow_to_double(const lua_WChar *source, lua_WChar **endp)
{
  int isNegative = 0;
  int isExponentNegative = 0;
  double integer = 0.0;
  double fraction = 0.0;
  unsigned long exponent = 0;
  double base;
  double fracdiv = 1.0;
  double value = 0.0;

  /* First try hex-floats */
  if ((source[0] == '0') && ((source[1] == 'x') || (source[1] == 'X')))
    {
      base = 16.0;
      source += 2;
      while (isxdigit((int)*source))
	{
	  integer *= base;
	  integer += (isdigit((int)*source)
		      ? (*source - '0')
		      : 10 + (triow_to_upper((int)*source) - 'A'));
	  source++;
	}
      if (*source == '.')
	{
	  source++;
	  while (isxdigit((int)*source))
	    {
	      fracdiv /= base;
	      fraction += fracdiv * (isdigit((int)*source)
				     ? (*source - '0')
				     : 10 + (triow_to_upper((int)*source) - 'A'));
	      source++;
	    }
	  if ((*source == 'p') || (*source == 'P'))
	    {
	      source++;
	      if ((*source == '+') || (*source == '-'))
		{
		  isExponentNegative = (*source == '-');
		  source++;
		}
	      while (isdigit((int)*source))
		{
		  exponent *= 10;
		  exponent += (*source - '0');
		  source++;
		}
	    }
	}
      /* For later use with exponent */
      base = 2.0;
    }
  else /* Then try normal decimal floats */
    {
      base = 10.0;
      isNegative = (*source == '-');
      /* Skip sign */
      if ((*source == '+') || (*source == '-'))
	source++;

      /* Integer part */
      while (isdigit((int)*source))
	{
	  integer *= base;
	  integer += (*source - '0');
	  source++;
	}

      if (*source == '.')
	{
	  source++; /* skip decimal point */
	  while (isdigit((int)*source))
	    {
	      fracdiv /= base;
	      fraction += (*source - '0') * fracdiv;
	      source++;
	    }
	}
      if ((*source == 'e')
	  || (*source == 'E')
#if TRIO_MICROSOFT
	  || (*source == 'd')
	  || (*source == 'D')
#endif
	  )
	{
	  source++; /* Skip exponential indicator */
	  isExponentNegative = (*source == '-');
	  if ((*source == '+') || (*source == '-'))
	    source++;
	  while (isdigit((int)*source))
	    {
	      exponent *= (int)base;
	      exponent += (*source - '0');
	      source++;
	    }
	}
    }
  
  value = integer + fraction;
  if (exponent != 0)
    {
      if (isExponentNegative)
	value /= pow(base, (double)exponent);
      else
	value *= pow(base, (double)exponent);
    }
  if (isNegative)
    value = -value;

  if (endp)
    *endp = (lua_WChar *)source;
  return value;
}


const TValue luaO_nilobject_ = {{NULL}, LUA_TNIL};


/*
** converts an integer to a "floating point byte", represented as
** (eeeeexxx), where the real value is (1xxx) * 2^(eeeee - 1) if
** eeeee != 0 and (xxx) otherwise.
*/
int luaO_int2fb (unsigned int x) {
  int e = 0;  /* expoent */
  while (x >= 16) {
    x = (x+1) >> 1;
    e++;
  }
  if (x < 8) return x;
  else return ((e+1) << 3) | (cast_int(x) - 8);
}


/* converts back */
int luaO_fb2int (int x) {
  int e = (x >> 3) & 31;
  if (e == 0) return x;
  else return ((x & 7)+8) << (e - 1);
}


int luaO_log2 (unsigned int x) {
  static const lu_byte log_2[256] = {
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  int l = -1;
  while (x >= 256) { l += 8; x >>= 8; }
  return l + log_2[x];

}


int luaO_rawequalObj (const TValue *t1, const TValue *t2) {
  if (ttype(t1) != ttype(t2)) return 0;
  else switch (ttype(t1)) {
    case LUA_TNIL:
      return 1;
    case LUA_TNUMBER:
      return luai_numeq(nvalue(t1), nvalue(t2));
    case LUA_TBOOLEAN:
      return bvalue(t1) == bvalue(t2);  /* boolean true must be 1 !! */
    case LUA_TLIGHTUSERDATA:
      return pvalue(t1) == pvalue(t2);
    default:
      lua_assert(iscollectable(t1));
      return gcvalue(t1) == gcvalue(t2);
  }
}


int luaO_str2d (const char *s, lua_Number *result) {
  char *endptr;
  *result = lua_str2number(s, &endptr);
  if (endptr == s) return 0;  /* conversion failed */
  if (*endptr == 'x' || *endptr == 'X')  /* maybe an hexadecimal constant? */
    *result = cast_num(strtoul(s, &endptr, 16));
  if (*endptr == '\0') return 1;  /* most common case */
  while (isspace(cast(unsigned char, *endptr))) endptr++;
  if (*endptr != '\0') return 0;  /* invalid trailing characters? */
  return 1;
}


int luaO_wstr2d (const lua_WChar *s, lua_Number *result) {
  lua_WChar *endptr;
  *result = lua_wstr2number(s, &endptr);
  if (endptr == s) return 0;  /* conversion failed */
  if (*endptr == 'x' || *endptr == 'X') { /* maybe an hexadecimal constant? */
    assert(0);
/*    *result = cast_num(strtoul(s, &endptr, 16)); */
  }
  if (*endptr == '\0') return 1;  /* most common case */
  while (*endptr < 256 && isspace(cast(unsigned char, *endptr))) endptr++;
  if (*endptr != '\0') return 0;  /* invalid trailing characters? */
  return 1;
}


static void pushstr (lua_State *L, const char *str) {
  setsvalue2s(L, L->top, luaS_new(L, str));
  incr_top(L);
}


/* this function handles only `%d', `%c', %f, %p, and `%s' formats */
const char *luaO_pushvfstring (lua_State *L, const char *fmt, va_list argp) {
  int n = 1;
  pushstr(L, "");
  for (;;) {
    const char *e = strchr(fmt, '%');
    if (e == NULL) break;
    setsvalue2s(L, L->top, luaS_newlstr(L, fmt, e-fmt));
    incr_top(L);
    switch (*(e+1)) {
      case 's': {
        const char *s = va_arg(argp, char *);
        if (s == NULL) s = "(null)";
        pushstr(L, s);
        break;
      }
      case 'c': {
        char buff[2];
        buff[0] = cast(char, va_arg(argp, int));
        buff[1] = '\0';
        pushstr(L, buff);
        break;
      }
      case 'd': {
        setnvalue(L->top, cast_num(va_arg(argp, int)));
        incr_top(L);
        break;
      }
      case 'f': {
        setnvalue(L->top, cast_num(va_arg(argp, l_uacNumber)));
        incr_top(L);
        break;
      }
      case 'p': {
        char buff[4*sizeof(void *) + 8]; /* should be enough space for a `%p' */
        sprintf(buff, "%p", va_arg(argp, void *));
        pushstr(L, buff);
        break;
      }
      case '%': {
        pushstr(L, "%");
        break;
      }
      default: {
        char buff[3];
        buff[0] = '%';
        buff[1] = *(e+1);
        buff[2] = '\0';
        pushstr(L, buff);
        break;
      }
    }
    n += 2;
    fmt = e+2;
  }
  pushstr(L, fmt);
  luaV_concat(L, n+1, cast_int(L->top - L->base) - 1);
  L->top -= n;
  return svalue(L->top - 1);
}


const char *luaO_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *msg;
  va_list argp;
  va_start(argp, fmt);
  msg = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  return msg;
}


void luaO_chunkid (char *out, const char *source, size_t bufflen) {
  if (*source == '=') {
    strncpy(out, source+1, bufflen);  /* remove first char */
    out[bufflen-1] = '\0';  /* ensures null termination */
  }
  else {  /* out = "source", or "...source" */
    if (*source == '@') {
      size_t l;
      source++;  /* skip the `@' */
      bufflen -= sizeof(" '...' ");
      l = strlen(source);
      strcpy(out, "");
      if (l > bufflen) {
        source += (l-bufflen);  /* get last part of file name */
        strcat(out, "...");
      }
      strcat(out, source);
    }
    else {  /* out = [string "string"] */
      size_t len = strcspn(source, "\n\r");  /* stop at first newline */
      bufflen -= sizeof(" [string \"...\"] ");
      if (len > bufflen) len = bufflen;
      strcpy(out, "[string \"");
      if (source[len] != '\0') {  /* must truncate? */
        strncat(out, source, len);
        strcat(out, "...");
      }
      else
        strcat(out, source);
      strcat(out, "\"]");
    }
  }
}

NAMESPACE_LUA_END