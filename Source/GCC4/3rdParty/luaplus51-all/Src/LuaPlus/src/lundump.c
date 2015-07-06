/*
** $Id: lundump.c,v 2.7.1.4 2008/04/04 19:51:41 roberto Exp $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#include <string.h>

#define lundump_c
#define LUA_CORE

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstring.h"
#include "lundump.h"
#include "lzio.h"

NAMESPACE_LUA_BEGIN

typedef struct {
 lua_State* L;
 ZIO* Z;
 Mbuffer* b;
 const char* name;
#if LUA_ENDIAN_SUPPORT
 int swap;
#endif /* LUA_ENDIAN_SUPPORT */
} LoadState;

#ifdef LUAC_TRUST_BINARIES
#define IF(c,s)
#define error(S,s)
#else
#define IF(c,s)		if (c) error(S,s)

static void error(LoadState* S, const char* why)
{
 luaO_pushfstring(S->L,"%s: %s in precompiled chunk",S->name,why);
 luaD_throw(S->L,LUA_ERRSYNTAX);
}
#endif

#if !LUA_ENDIAN_SUPPORT
#define LoadMem(S,b,n,size)	LoadBlock(S,b,(n)*(size))
#endif /* !LUA_ENDIAN_SUPPORT */
#define	LoadByte(S)		(lu_byte)LoadChar(S)
#define LoadVar(S,x)		LoadMem(S,&x,1,sizeof(x))
#define LoadVector(S,b,n,size)	LoadMem(S,b,n,size)

static void LoadBlock(LoadState* S, void* b, size_t size)
{
 size_t r=luaZ_read(S->Z,b,size);
 IF (r!=0, "unexpected end");
}

#if LUA_ENDIAN_SUPPORT
static void LoadMem (LoadState* S, void* b, size_t n, size_t size)
{
 LoadBlock(S,b,n*size);
 if (S->swap)
 {
  char* p=(char*) b;
  char c;
  switch (size)
  {
   case 1:
    break;
   case 2:
    while (n--) {
     c=p[0]; p[0]=p[1]; p[1]=c;
     p+=2;
    }
    break;
   case 4:
    while (n--) {
     c=p[0]; p[0]=p[3]; p[3]=c;
     c=p[1]; p[1]=p[2]; p[2]=c;
     p+=4;
    }
    break;
   case 8:
    while (n--) {
     c=p[0]; p[0]=p[7]; p[7]=c;
     c=p[1]; p[1]=p[6]; p[6]=c;
     c=p[2]; p[2]=p[5]; p[5]=c;
     c=p[3]; p[3]=p[4]; p[4]=c;
     p+=8;
    }
    break;
   default:
    IF(1, "bad size");
    break;
  }
 }
}
#endif /* LUA_ENDIAN_SUPPORT */

static int LoadChar(LoadState* S)
{
 char x;
 LoadVar(S,x);
 return x;
}

static int LoadInt(LoadState* S)
{
 int x;
 LoadVar(S,x);
 IF (x<0, "bad integer");
 return x;
}

static lua_Number LoadNumber(LoadState* S)
{
 lua_Number x;
 LoadVar(S,x);
 return x;
}

static TString* LoadString(LoadState* S)
{
 size_t size;
 LoadVar(S,size);
 if (size==0)
  return NULL;
 else
 {
  char* s=luaZ_openspace(S->L,S->b,size);
  LoadBlock(S,s,size);
  return luaS_newlstr(S->L,s,size-1);		/* remove trailing '\0' */
 }
}

#if LUA_WIDESTRING
static TString* LoadWString(LoadState* S)
{
 size_t size;
 LoadVar(S,size);
 if (size==0)
  return NULL;
 else
 {
  lua_WChar* s=(lua_WChar*)luaZ_openspace(S->L,S->b,size*sizeof(lua_WChar));
  LoadVector(S,s,size,sizeof(lua_WChar));
  return luaS_newlwstr(S->L,s,size-1);		/* remove trailing '\0' */
 }
}
#endif /* LUA_WIDESTRING */

static void LoadCode(LoadState* S, Proto* f)
{
 int n=LoadInt(S);
 f->code=luaM_newvector(S->L,n,Instruction);
 f->sizecode=n;
 LoadVector(S,f->code,n,sizeof(Instruction));
}

static Proto* LoadFunction(LoadState* S, TString* p);

static void LoadConstants(LoadState* S, Proto* f)
{
 int i,n;
#if LUA_REFCOUNT    
 lua_State *L = S->L;
#endif /* LUA_REFCOUNT */
 n=LoadInt(S);
 f->k=luaM_newvector(S->L,n,TValue);
 f->sizek=n;
#if LUA_REFCOUNT
 for (i=0; i<n; i++) setnilvalue2n(L, &f->k[i]);
#else
 for (i=0; i<n; i++) setnilvalue(&f->k[i]);
#endif /* LUA_REFCOUNT */
 for (i=0; i<n; i++)
 {
  TValue* o=&f->k[i];
  int t=LoadChar(S);
  switch (t)
  {
   case LUA_TNIL:
   	setnilvalue(o);
	break;
   case LUA_TBOOLEAN:
   	setbvalue(o,LoadChar(S)!=0);
	break;
   case LUA_TNUMBER:
	setnvalue(o,LoadNumber(S));
	break;
   case LUA_TSTRING:
	setsvalue2n(S->L,o,LoadString(S));
	break;
#if LUA_WIDESTRING
   case LUA_TWSTRING:
	setwsvalue2n(S->L,o,LoadWString(S));
	break;
#endif /* LUA_WIDESTRING */
   default:
	error(S,"bad constant");
	break;
  }
 }
 n=LoadInt(S);
 f->p=luaM_newvector(S->L,n,Proto*);
 f->sizep=n;
 for (i=0; i<n; i++) f->p[i]=NULL;
#if LUA_REFCOUNT
 for (i=0; i<n; i++) {
  f->p[i]=LoadFunction(S,f->source);
  luarc_addrefproto(f->p[i]);
 }
#else
 for (i=0; i<n; i++) f->p[i]=LoadFunction(S,f->source);
#endif /* LUA_REFCOUNT */
}

static void LoadDebug(LoadState* S, Proto* f)
{
 int i,n;
 n=LoadInt(S);
 f->lineinfo=luaM_newvector(S->L,n,int);
 f->sizelineinfo=n;
 LoadVector(S,f->lineinfo,n,sizeof(int));
 n=LoadInt(S);
 f->locvars=luaM_newvector(S->L,n,LocVar);
 f->sizelocvars=n;
 for (i=0; i<n; i++) f->locvars[i].varname=NULL;
 for (i=0; i<n; i++)
 {
  f->locvars[i].varname=LoadString(S);
#if LUA_REFCOUNT
  luarc_addrefstring(f->locvars[i].varname);
#endif /* LUA_REFCOUNT */
  f->locvars[i].startpc=LoadInt(S);
  f->locvars[i].endpc=LoadInt(S);
 }
 n=LoadInt(S);
 f->upvalues=luaM_newvector(S->L,n,TString*);
 f->sizeupvalues=n;
 for (i=0; i<n; i++) f->upvalues[i]=NULL;
#if LUA_REFCOUNT
 for (i=0; i<n; i++) {
  f->upvalues[i]=LoadString(S);
  luarc_addrefstring(f->upvalues[i]);
 }
#else
 for (i=0; i<n; i++) f->upvalues[i]=LoadString(S);
#endif /* LUA_REFCOUNT */
}

static Proto* LoadFunction(LoadState* S, TString* p)
{
 Proto* f;
 if (++S->L->nCcalls > LUAI_MAXCCALLS) error(S,"code too deep");
 f=luaF_newproto(S->L);
 setptvalue2s(S->L,S->L->top,f); incr_top(S->L);
 f->source=LoadString(S); if (f->source==NULL) f->source=p;
#if LUA_REFCOUNT
 luarc_addrefstring(f->source);
#endif /* LUA_REFCOUNT */
 f->linedefined=LoadInt(S);
 f->lastlinedefined=LoadInt(S);
 f->nups=LoadByte(S);
 f->numparams=LoadByte(S);
 f->is_vararg=LoadByte(S);
 f->maxstacksize=LoadByte(S);
 LoadCode(S,f);
 LoadConstants(S,f);
 LoadDebug(S,f);
 IF (!luaG_checkcode(f), "bad code");
 S->L->top--;
 S->L->nCcalls--;
 return f;
}

static void LoadHeader(LoadState* S)
{
 char h[LUAC_HEADERSIZE];
 char s[LUAC_HEADERSIZE];
#if LUA_ENDIAN_SUPPORT
 luaU_header(h, '=');
 LoadBlock(S,s,LUAC_HEADERSIZE);
 S->swap=(s[6]!=h[6]); s[6]=h[6];
#else
 luaU_header(h);
 LoadBlock(S,s,LUAC_HEADERSIZE);
#endif /* LUA_ENDIAN_SUPPORT */
 IF (memcmp(h,s,LUAC_HEADERSIZE)!=0, "bad header");
}

/*
** load precompiled chunk
*/
Proto* luaU_undump (lua_State* L, ZIO* Z, Mbuffer* buff, const char* name)
{
 LoadState S;
 if (*name=='@' || *name=='=')
  S.name=name+1;
 else if (*name==LUA_SIGNATURE[0])
  S.name="binary string";
 else
  S.name=name;
 S.L=L;
 S.Z=Z;
 S.b=buff;
 LoadHeader(&S);
#if LUA_REFCOUNT
 {
  Proto *proto = LoadFunction(&S,luaS_newliteral(L,"=?"));
  luarc_addrefproto(proto);
  return proto;
 }
#else
 return LoadFunction(&S,luaS_newliteral(L,"=?"));
#endif /* LUA_REFCOUNT */
}

/*
* make header
*/
#if LUA_ENDIAN_SUPPORT
void luaU_header (char* h, char endian)
#else
void luaU_header (char* h)
#endif /* LUA_ENDIAN_SUPPORT */
{
 int x=1;
#if LUA_ENDIAN_SUPPORT
 char endianness = (char)((endian == '=') ? *(char*)&x : ((endian == '<') ? 1 : 0));
#endif /* LUA_ENDIAN_SUPPORT */
 memcpy(h,LUA_SIGNATURE,sizeof(LUA_SIGNATURE)-1);
 h+=sizeof(LUA_SIGNATURE)-1;
 *h++=(char)LUAC_VERSION;
 *h++=(char)LUAC_FORMAT;
#if LUA_ENDIAN_SUPPORT
 *h++=(char)endianness;				/* endianness */
#else
 *h++=(char)*(char*)&x;				/* endianness */
#endif /* LUA_ENDIAN_SUPPORT */
 *h++=(char)sizeof(int);
 *h++=(char)sizeof(size_t);
 *h++=(char)sizeof(Instruction);
 *h++=(char)sizeof(lua_Number);
 *h++=(char)(((lua_Number)0.5)==0);		/* is lua_Number integral? */
}

NAMESPACE_LUA_END
