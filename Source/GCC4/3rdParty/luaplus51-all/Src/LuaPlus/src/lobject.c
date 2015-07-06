/*
** $Id: lobject.c,v 2.22.1.1 2007/12/27 13:02:25 roberto Exp $
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

#if LUA_REFCOUNT
#include "ltable.h"
#include "lfunc.h"
#endif /* LUA_REFCOUNT */

#if LUA_WIDESTRING
#include <math.h>
#include <assert.h>
#endif /* LUA_WIDESTRING */

NAMESPACE_LUA_BEGIN

#if LUA_WIDESTRING
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
#endif /* LUA_WIDESTRING */

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

#if LUA_WIDESTRING
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
#endif /* LUA_WIDESTRING */


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
#if LUA_REFCOUNT
  luarc_cleanarray(L->top - n, L->top);
#endif /* LUA_REFCOUNT */
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

#if LUA_REFCOUNT

static void traverseclosure (lua_State *L, Closure *cl) {
  luarc_releasetable(L, cl->c.env);
  if (cl->c.isC) {
    int i;
    for (i=0; i<cl->c.nupvalues; i++)  /* mark its upvalues */
      luarc_release(L, &cl->c.upvalue[i]);
  }
  else {
    int i;
    lua_assert(cl->l.nupvalues == cl->l.p->nups);
    luarc_releaseproto(L, cl->l.p);
    for (i=0; i<cl->l.nupvalues; i++)  /* mark its upvalues */
      luarc_releaseupval(L, cl->l.upvals[i]);
  }
}

static void removeentry (lua_State *L, Node *n) {
  lua_assert(ttisnil(gval(n)));
  if (iscollectable(gkey(n)))
    luarc_release(L, gkey(n));  /* dead key; remove it */
}


static int traversetable (lua_State *L, Table *h) {
  int i;
  int weakkey = 0;
  int weakvalue = 0;
//  const TValue *mode;
  if (h->metatable)
    luarc_releasetable(L, h->metatable);
#if 0
  mode = gfasttm(g, h->metatable, TM_MODE);
  if (mode && ttisstring(mode)) {  /* is there a weak mode? */
    weakkey = (strchr(svalue(mode), 'k') != NULL);
    weakvalue = (strchr(svalue(mode), 'v') != NULL);
    if (weakkey || weakvalue) {  /* is really weak? */
      h->marked &= ~(KEYWEAK | VALUEWEAK);  /* clear bits */
      h->marked |= cast_byte((weakkey << KEYWEAKBIT) |
                             (weakvalue << VALUEWEAKBIT));
      h->gclist = g->weak;  /* must be cleared after GC, ... */
      g->weak = obj2gco(h);  /* ... so put in the appropriate list */
    }
  }
  if (weakkey && weakvalue) return 1;
#endif
  if (!weakvalue) {
    i = h->sizearray;
    while (i--)
      luarc_release(L, &h->array[i]);
  }
  i = sizenode(h);
  while (i--) {
    Node *n = gnode(h, i);
    lua_assert(ttype(gkey(n)) != LUA_TDEADKEY || ttisnil(gval(n)));
    if (ttisnil(gval(n)))
      removeentry(L, n);  /* remove empty entries */
    else {
      lua_assert(!ttisnil(gkey(n)));
      if (!weakkey) luarc_release(L, gkey(n));
      if (!weakvalue) luarc_release(L, gval(n));
    }
  }
  return weakkey || weakvalue;
}

/*
** All marks are conditional because a GC may happen while the
** prototype is still being created
*/
static void traverseproto (lua_State *L, Proto *f) {
  int i;
  if (f->source) luarc_releasestring(L, f->source);
  for (i=0; i<f->sizek; i++)  /* mark literals */
    luarc_release(L, &f->k[i]);
  for (i=0; i<f->sizeupvalues; i++) {  /* mark upvalue names */
    if (f->upvalues[i])
      luarc_releasestring(L, f->upvalues[i]);
  }
  for (i=0; i<f->sizep; i++) {  /* mark nested protos */
    if (f->p[i])
      luarc_releaseproto(L, f->p[i]);
  }
  for (i=0; i<f->sizelocvars; i++) {  /* mark local-variable names */
    if (f->locvars[i].varname)
      luarc_releasestring(L, f->locvars[i].varname);
  }
}


static void traversestack (lua_State *origL, lua_State *l) {
  StkId o, lim;
  CallInfo *ci;
  lua_State *L = l;
  luarc_release(origL, gt(l));
  lim = l->top;
  for (ci = l->base_ci; ci <= l->ci; ci++) {
    lua_assert(ci->top <= l->stack_last);
    if (lim < ci->top) lim = ci->top;
  }
  for (o = l->stack; o < l->top; o++)
    luarc_release(l, o);
  for (; o <= lim; o++)
    setnilvalue(o);
/*  checkstacksizes(l, lim);*/
}



static void Unlink(GCObject* o)
{
  o->gch.prev->gch.next = o->gch.next;
  if (o->gch.next)
    o->gch.next->gch.prev = o->gch.prev;
}


static void UnlinkString(lua_State *L, TString *ts)
{
  if (ts->tsv.prev)
  {
    ts->tsv.prev->gch.next = ts->tsv.next;
  }
  else
  {
    unsigned int index = lmod(ts->tsv.hash, G(L)->strt.size);
    lua_assert(G(L)->strt.hash[index] == (GCObject*)ts);
    G(L)->strt.hash[index] = ts->tsv.next;
  }

  if (ts->tsv.next)
  {
    ts->tsv.next->gch.prev = ts->tsv.prev;
  }

  G(L)->strt.nuse--;
}


extern void GCTM (lua_State *L);


void luarc_releaseobject(lua_State *L, GCObject* o)
{
	global_State *g = G(L);
    /* reset sweep marks to sweep all elements (returning them to white) */
    g->sweepstrgc = 0;
    g->sweepgc = &g->rootgc;
    /* reset other collector lists */
    g->gray = NULL;
    g->grayagain = NULL;
    g->weak = NULL;
    g->gcstate = GCSsweepstring;
  switch (o->gch.tt)
  {
    case LUA_TSTRING: {
      const char* str = getstr(&o->ts); (void)str;
      UnlinkString(L, rawgco2ts(o));
      luaM_freemem(L, o, sizestring(gco2ts(o)));
      break;
    }
    case LUA_TWSTRING: {
      UnlinkString(L, rawgco2ts(o));
      luaM_freemem(L, o, sizestring(gco2ts(o)));
      break;
    }
    case LUA_TUSERDATA: {
      Unlink((GCObject*)gco2u(o));

      if (G(L)->tmudata == NULL)  /* list is empty? */
        G(L)->tmudata = o->gch.next = o;  /* creates a circular list */
      else {
        o->gch.next = G(L)->tmudata->gch.next;
        G(L)->tmudata->gch.next = o;
        G(L)->tmudata = o;
      }
      GCTM(L);

      if (gco2u(o)->metatable) {
        luarc_releasetable(L, gco2u(o)->metatable);
      }

      if (gco2u(o)->env) {
        luarc_releasetable(L, gco2u(o)->env);
      }

      G(L)->mainthread->next = G(L)->mainthread->next->gch.next;
      if (G(L)->mainthread->next)
        G(L)->mainthread->next->gch.prev = (GCObject*)&G(L)->mainthread->next;
      luaM_freemem(L, o, sizeudata(gco2u(o)));
      break;
    }
    case LUA_TFUNCTION: {
      Closure *cl = gco2cl(o);
      Unlink((GCObject*)cl);
      traverseclosure(L, cl);
      luaF_freeclosure(L, cl);
      break;
    }
    case LUA_TTABLE: {
      Table *h = gco2h(o);
      Unlink((GCObject*)h);
      traversetable(L, h);
      luaH_free(L, gco2h(o));
      break;
    }
    case LUA_TPROTO: {
      Proto *p = gco2p(o);
      Unlink((GCObject*)p);
      traverseproto(L, p);
      luaF_freeproto(L, p);
      break;
    }
    case LUA_TUPVAL: {
      UpVal *uv = gco2uv(o);
      if (uv->prev)
        uv->prev->gch.next = uv->next;
      else {
        L->openupval = uv->next;
      }
      if (uv->next)
        uv->next->gch.prev = uv->prev;
      luarc_release(L, &uv->u.value);
      luaF_freeupval(L, uv);
      break;
    }
    case LUA_TTHREAD: {
      lua_assert(gco2th(o) != L && gco2th(o) != G(L)->mainthread);
      Unlink((GCObject*)gco2th(o));
      traversestack(L, gco2th(o));
      luaE_freethread(L, gco2th(o));
      break;
    }
    default:
      lua_assert(0);
  }
}

#endif /* LUA_REFCOUNT */

NAMESPACE_LUA_END
