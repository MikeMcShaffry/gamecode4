/*
** $Id: ldump.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#include <stddef.h>

#define ldump_c
#define LUA_CORE

#include "lua.h"

#include "lobject.h"
#include "lstate.h"
#include "lundump.h"

NAMESPACE_LUA_BEGIN

typedef struct {
 lua_State* L;
 lua_Writer writer;
 void* data;
 int strip;
 int status;
#if LUA_ENDIAN_SUPPORT
 int swap;
 char endian;
 Mbuffer b;
#endif /* LUA_ENDIAN_SUPPORT */
} DumpState;

#define DumpMem(b,n,size,D)	DumpBlock(b,(n)*(size),D)

#if LUA_ENDIAN_SUPPORT
#define DumpVar(x,D)	 	DumpEndianBlock(&x,sizeof(x),D)
#else
#define DumpVar(x,D)	 	DumpMem(&x,1,sizeof(x),D)
#endif /* LUA_ENDIAN_SUPPORT */

static void DumpBlock(const void* b, size_t size, DumpState* D)
{
 if (D->status==0)
 {
  lua_unlock(D->L);
  D->status=(*D->writer)(D->L,b,size,D->data);
  lua_lock(D->L);
 }
}

#if LUA_ENDIAN_SUPPORT

static void DumpEndianBlock(const void* b, size_t size, DumpState* D)
{
 if (D->status==0)
 {
  lua_unlock(D->L);
  if (D->swap)
  {
   char* origSwapBuffer=luaZ_openspace(D->L,&D->b,size);
   char* swapBuffer = origSwapBuffer;
   char* p =(char*) b+size-1;
   while (p >= (char*)b)
    *swapBuffer++ = *p--;
   D->status=(*D->writer)(D->L,origSwapBuffer,size,D->data);
  }
  else
  {
   D->status=(*D->writer)(D->L,b,size,D->data);
  }
  lua_lock(D->L);
 }
}

#endif /* LUA_ENDIAN_SUPPORT */

static void DumpChar(int y, DumpState* D)
{
 char x=(char)y;
 DumpVar(x,D);
}

static void DumpInt(int x, DumpState* D)
{
 DumpVar(x,D);
}

static void DumpNumber(lua_Number x, DumpState* D)
{
 DumpVar(x,D);
}

static void DumpVector(const void* b, size_t n, size_t size, DumpState* D)
{
 DumpInt((int)n,D);
#if LUA_ENDIAN_SUPPORT
 if (D->status==0)
 {
  lua_unlock(D->L);
  if (D->swap)
  {
   char* origSwapBuffer=luaZ_openspace(D->L,&D->b,n*size);
   char* swapBuffer = origSwapBuffer;
   char* q=(char*) b;
   size_t orign = n;
   while (n--)
   {
    char* p =(char*) q+size-1;
    while (p >= q)
     *swapBuffer++ = *p--;
    q+=size;
   }
   D->status=(*D->writer)(D->L,origSwapBuffer,orign*size,D->data);
  }
  else
  {
   D->status=(*D->writer)(D->L,b,n*size,D->data);
  }
  lua_lock(D->L);
 }
#else
 DumpMem(b,n,size,D);
#endif /* LUA_ENDIAN_SUPPORT */
}

static void DumpString(const TString* s, DumpState* D)
{
 if (s==NULL || getstr(s)==NULL)
 {
  size_t size=0;
  DumpVar(size,D);
 }
 else
 {
  size_t size=s->tsv.len+1;		/* include trailing '\0' */
  DumpVar(size,D);
  DumpBlock(getstr(s),size,D);
 }
}

#if LUA_WIDESTRING

static void DumpWString(TString* s, DumpState* D)
{
 if (s==NULL || getwstr(s)==NULL)
 {
  size_t size=0;
  DumpVar(size,D);
 }
 else
 {
  size_t size=s->tsv.len+1;		/* include trailing '\0' */
  DumpVector(getwstr(s),size,2,D);
 }
}

#endif /* LUA_WIDESTRING */

#define DumpCode(f,D)	 DumpVector(f->code,f->sizecode,sizeof(Instruction),D)

static void DumpFunction(const Proto* f, const TString* p, DumpState* D);

static void DumpConstants(const Proto* f, DumpState* D)
{
 int i,n=f->sizek;
 DumpInt(n,D);
 for (i=0; i<n; i++)
 {
  const TValue* o=&f->k[i];
  DumpChar(ttype(o),D);
  switch (ttype(o))
  {
   case LUA_TNIL:
	break;
   case LUA_TBOOLEAN:
	DumpChar(bvalue(o),D);
	break;
   case LUA_TNUMBER:
	DumpNumber(nvalue(o),D);
	break;
   case LUA_TSTRING:
	DumpString(rawtsvalue(o),D);
	break;
#if LUA_WIDESTRING
   case LUA_TWSTRING:
	DumpWString(rawtsvalue(o),D);
	break;
#endif /* LUA_WIDESTRING */
   default:
	lua_assert(0);			/* cannot happen */
	break;
  }
 }
 n=f->sizep;
 DumpInt(n,D);
 for (i=0; i<n; i++) DumpFunction(f->p[i],f->source,D);
}

static void DumpDebug(const Proto* f, DumpState* D)
{
 int i,n;
 n= (D->strip) ? 0 : f->sizelineinfo;
 DumpVector(f->lineinfo,n,sizeof(int),D);
 n= (D->strip) ? 0 : f->sizelocvars;
 DumpInt(n,D);
 for (i=0; i<n; i++)
 {
  DumpString(f->locvars[i].varname,D);
  DumpInt(f->locvars[i].startpc,D);
  DumpInt(f->locvars[i].endpc,D);
 }
 n= (D->strip) ? 0 : f->sizeupvalues;
 DumpInt(n,D);
 for (i=0; i<n; i++) DumpString(f->upvalues[i],D);
}

static void DumpFunction(const Proto* f, const TString* p, DumpState* D)
{
 DumpString((f->source==p || D->strip) ? NULL : f->source,D);
 DumpInt(f->linedefined,D);
 DumpInt(f->lastlinedefined,D);
 DumpChar(f->nups,D);
 DumpChar(f->numparams,D);
 DumpChar(f->is_vararg,D);
 DumpChar(f->maxstacksize,D);
 DumpCode(f,D);
 DumpConstants(f,D);
 DumpDebug(f,D);
}

static void DumpHeader(DumpState* D)
{
 char h[LUAC_HEADERSIZE];
#if LUA_ENDIAN_SUPPORT
 luaU_header(h, D->endian);
#else
 luaU_header(h);
#endif /* LUA_ENDIAN_SUPPORT */
 DumpBlock(h,LUAC_HEADERSIZE,D);
}

#if LUA_ENDIAN_SUPPORT

#define	OP_LITTLEENDIAN	'<'		/* little endian */
#define	OP_BIGENDIAN	'>'		/* big endian */
#define	OP_NATIVE	'='		/* native endian */

static int doendian(int c)
{
 int x=1;
 int e=*(char*)&x;
 if (c==OP_LITTLEENDIAN) return !e;
 if (c==OP_BIGENDIAN) return e;
 if (c==OP_NATIVE) return 0;
 return 0;
}

#endif /* LUA_ENDIAN_SUPPORT */

/*
** dump Lua function as precompiled chunk
*/
#if LUA_ENDIAN_SUPPORT
int luaU_dump (lua_State* L, const Proto* f, lua_Writer w, void* data, int strip, char endian)
#else
int luaU_dump (lua_State* L, const Proto* f, lua_Writer w, void* data, int strip)
#endif /* LUA_ENDIAN_SUPPORT */
{
 DumpState D;
 D.L=L;
 D.writer=w;
 D.data=data;
 D.strip=strip;
 D.status=0;
#if LUA_ENDIAN_SUPPORT
 D.swap=doendian(endian);
 D.endian=endian;
 luaZ_initbuffer(L, &D.b);
#endif /* LUA_ENDIAN_SUPPORT */
 DumpHeader(&D);
 DumpFunction(f,NULL,&D);
#if LUA_ENDIAN_SUPPORT
 luaZ_freebuffer(L, &D.b);
#endif /* LUA_ENDIAN_SUPPORT */
 return D.status;
}

NAMESPACE_LUA_END
