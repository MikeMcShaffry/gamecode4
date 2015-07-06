#include "lua.h"
#include "lauxlib.h"

#if defined(_MSC_VER)
#include "malloc.h"
#endif

typedef struct Mbuffer {
  char *buffer;
  size_t buffsize;
} Mbuffer;


#define LUA_MINBUFFER	32

char *luaZ_openspace (lua_State *L, Mbuffer *buff, size_t n);

#define luaZ_initbuffer(L, buff) ((buff)->buffer = NULL, (buff)->buffsize = 0)

#define luaZ_sizebuffer(buff)	((buff)->buffsize)
#define luaZ_buffer(buff)	((buff)->buffer)

#define luaZ_resizebuffer(L, buff, size) \
	((buff)->buffer = (char*)realloc((buff)->buffer, size), \
	(buff)->buffsize = size)

#define luaZ_freebuffer(L, buff)	luaZ_resizebuffer(L, buff, 0)

/* ------------------------------------------------------------------------ */
char *luaZ_openspace (lua_State *L, Mbuffer *buff, size_t n) {
  if (n > buff->buffsize) {
    if (n < LUA_MINBUFFER) n = LUA_MINBUFFER;
    buff->buffer = (char*)realloc(buff->buffer, n);
    buff->buffsize = n;
  }
  return buff->buffer;
}

/*
* lpack.c
* a Lua library for packing and unpacking binary data
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 27 Apr 2004 00:08:42
* This code is hereby placed in the public domain.
* with contributions from Ignacio Castaño <castanyo@yahoo.es> and
* Roberto Ierusalimschy <roberto@inf.puc-rio.br>.
*/

#define	OP_ZSTRING	'z'		/* zero-terminated string */
#define	OP_BSTRING	'p'		/* string preceded by length byte */
#if LUA_WIDESTRING
#define	OP_WSTRING	'P'		/* string preceded by length word */
#endif // LUA_WIDESTRING
#define	OP_SSTRING	'a'		/* string preceded by length size_t */
#define	OP_STRING	'A'		/* string */
#define	OP_FLOAT	'f'		/* float */
#define	OP_DOUBLE	'd'		/* double */
#define	OP_NUMBER	'n'		/* Lua number */
#define	OP_CHAR		'c'		/* char */
#define	OP_BYTE		'b'		/* byte = unsigned char */
#define	OP_SHORT	'h'		/* short */
#define	OP_USHORT	'H'		/* unsigned short */
#define	OP_INT		'i'		/* int */
#define	OP_UINT		'I'		/* unsigned int */
#define	OP_LONG		'l'		/* long */
#define	OP_ULONG	'L'		/* unsigned long */
#define	OP_LITTLEENDIAN	'<'		/* little endian */
#define	OP_BIGENDIAN	'>'		/* big endian */
#define	OP_NATIVE	'='		/* native endian */

#define	OP_BOOLEAN	'B'		/* boolean = unsigned char */
#define	OP_WIDESTRING	'w'
#define OP_PAD '@'

#include <ctype.h>
#include <string.h>

//#include "lualib.h"
//#include "lauxlib.h"

static void badcode(lua_State *L, int c)
{
 char s[]="bad code `?'";
 s[sizeof(s)-3]=c;
 luaL_argerror(L,1,s);
}

static int doendian(int c)
{
 int x=1;
 int e=*(char*)&x;
 if (c==OP_LITTLEENDIAN) return !e;
 if (c==OP_BIGENDIAN) return e;
 if (c==OP_NATIVE) return 0;
 return 0;
}

static void doswap(int swap, void *p, size_t n)
{
 if (swap)
 {
  char *a=(char*)p;
  int i,j;
  for (i=0, j=n-1, n=n/2; n--; i++, j--)
  {
   char t=a[i]; a[i]=a[j]; a[j]=t;
  }
 }
}

static void dowideswap(int swap, void *p, size_t n)
{
	if (swap)
	{
		char *a=(char*)p;
		size_t i;
		for (i = 0; i < n; ++i)
		{
			char temp = a[0];
			a[0] = a[1];
			a[1] = temp;
			a += 2;
		}
	}
}

#define UNPACKNUMBER(OP,T)		\
   case OP:				\
   {					\
    T a;				\
    int m=sizeof(a);			\
    if (i+m>(int)len) goto done;		\
    memcpy(&a,s+i,m);			\
    i+=m;				\
    doswap(swap,&a,m);			\
    lua_pushnumber(L,(lua_Number)a);	\
    ++n;				\
    break;				\
   }

#if LUA_WIDESTRING
#define UNPACKSTRING(OP,T)						\
	case OP:									\
	{											\
		T l;									\
		int m=sizeof(l);						\
		if (i+m>(int)len) goto done;			\
		memcpy(&l,s+i,m);						\
		doswap(swap,&l,m);						\
		if (isWide)								\
		{										\
			if (i+m+l*2>(int)len) goto done;	\
		}										\
		else									\
		{										\
			if (i+m+l>(int)len) goto done;		\
		}										\
		i+=m;									\
		if (isWide)								\
		{										\
			wchar_t* out=(wchar_t *)luaZ_openspace(L,&buff,l*2);	\
			memcpy(out, s+i, l * 2);			\
			dowideswap(swap, (void*)out, l);	\
			lua_pushlwstring(L,out,l);			\
			i += l * 2;							\
		}										\
		else									\
		{										\
			lua_pushlstring(L,s+i,l);			\
			i+=l;								\
		}										\
		++n;									\
		break;									\
	}
#else
#define UNPACKSTRING(OP,T)						\
	case OP:									\
	{											\
		T l;									\
		int m=sizeof(l);						\
		if (i+m>(int)len) goto done;			\
		memcpy(&l,s+i,m);						\
		if (i+m+l>(int)len) goto done;			\
		i+=m;									\
		lua_pushlstring(L,s+i,l);				\
		i+=l;									\
		++n;									\
		break;									\
	}
#endif // LUA_WIDESTRING

static int l_unpack(lua_State *L) 		/** unpack(s,f,[init]) */
{
	Mbuffer buff;
	const char *s=luaL_checkstring(L,1);
	const char *f=luaL_checkstring(L,2);
	int i=(int)luaL_optnumber(L,3,1)-1;
	size_t len=lua_strlen(L,1);
	int n=0;
	int swap=0;
	luaZ_initbuffer(L, &buff);
	lua_pushnil(L);
	while (*f)
	{
		int c=*f++;
		int N=0;
		int isWide = 0;
		if (c == OP_WIDESTRING)
		{
			isWide = 1;
			c=*f++;
			if (!c)
				break;
		}
		while (isdigit(*f)) N=10*N+(*f++)-'0';
		if (N==0) N=1;
		while (N--)
		{
			switch (c)
			{
				case OP_LITTLEENDIAN:
				case OP_BIGENDIAN:
				case OP_NATIVE:
				{
					swap=doendian(c);
					N=0;
					break;
				}
				case OP_STRING:
				{
					++N;
#if LUA_WIDESTRING
					if (isWide)
					{
						wchar_t* out;
						if (i+N*2>(int)len) goto done;
						out=(wchar_t *)luaZ_openspace(L,&buff,N*2);
						memcpy(out, s+i, N * 2);
						dowideswap(swap, (void*)out, N);
						lua_pushlwstring(L,out,N);
						i+=N*2;
					}
					else
#endif // LUA_WIDESTRING
					{
						if (i+N>(int)len) goto done;
						lua_pushlstring(L,s+i,N);
						i+=N;
					}
					++n;
					N=0;
					break;
				}
				case OP_ZSTRING:
				{
					size_t l;
					if (i>=(int)len) goto done;
#if LUA_WIDESTRING
					if (isWide)
					{
						wchar_t* out;
						l=wcslen((const wchar_t*)(s+i));
						out=(wchar_t *)luaZ_openspace(L,&buff,l*2);
						memcpy(out, s+i, l * 2);
						dowideswap(swap, (void*)out, l);
						lua_pushlwstring(L,out,l);
						i+=(l+1) * 2;
					}
					else
#endif // LUA_WIDESTRING
					{
						l=strlen(s+i);
						lua_pushlstring(L,s+i,l);
						i+=l+1;
					}
					++n;
					break;
				}
				UNPACKSTRING(OP_BSTRING, unsigned char)
#if LUA_WIDESTRING
				UNPACKSTRING(OP_WSTRING, unsigned short)
#endif // LUA_WIDESTRING
				UNPACKSTRING(OP_SSTRING, size_t)
				UNPACKNUMBER(OP_NUMBER, lua_Number)
				UNPACKNUMBER(OP_DOUBLE, double)
				UNPACKNUMBER(OP_FLOAT, float)
				UNPACKNUMBER(OP_CHAR, char)
				UNPACKNUMBER(OP_BYTE, unsigned char)
				UNPACKNUMBER(OP_SHORT, short)
				UNPACKNUMBER(OP_USHORT, unsigned short)
				UNPACKNUMBER(OP_INT, int)
				UNPACKNUMBER(OP_UINT, unsigned int)
				UNPACKNUMBER(OP_LONG, long)
				UNPACKNUMBER(OP_ULONG, unsigned long)
				case OP_BOOLEAN:
				{
					unsigned char a;
					int m=sizeof(a);
					if (i+m>(int)len) goto done;
					memcpy(&a,s+i,m);
					i+=m;
					doswap(swap,&a,m);
					lua_pushboolean(L,a != 0);
					++n;				\
					break;				\
				}


			case ' ': case ',':
				break;
			default:
				badcode(L,c);
				break;
			}
		}
	}
done:
	lua_pushnumber(L,i+1);
	lua_replace(L,-n-2);
	luaZ_freebuffer(L, &buff);
	return n+1;
}

#define PACKNUMBER(OP,T)			\
   case OP:					\
   {						\
    T a=(T)luaL_checknumber(L,i++);		\
    doswap(swap,&a,sizeof(a));			\
    luaL_addlstring(&b,(const char*)&a,sizeof(a));	\
    break;					\
   }

#if LUA_WIDESTRING
#define PACKSTRING(OP,T)									\
	case OP:												\
	{														\
		size_t l;											\
		const char *a;										\
		const wchar_t *wa;									\
		T ll;												\
		if (isWide)											\
			wa=luaL_checklwstring(L,i++,&l);				\
		else												\
			a=luaL_checklstring(L,i++,&l);					\
		ll=(T)l;											\
		doswap(swap,&ll,sizeof(ll));						\
		luaL_addlstring(&b,(const char*)&ll,sizeof(ll));	\
		if (isWide)											\
		{													\
			size_t curPos = b.p - b.buffer;					\
			luaL_addlwstring(&b,wa,l);						\
			dowideswap(swap,b.buffer+curPos,l);				\
		}													\
		else												\
		{													\
			luaL_addlstring(&b, a, l);						\
		}													\
		break;												\
	}
#else
#define PACKSTRING(OP,T)									\
	case OP:												\
	{														\
		size_t l;											\
		const char *a;										\
		T ll;												\
		a=luaL_checklstring(L,i++,&l);						\
		ll=(T)l;											\
		doswap(swap,&ll,sizeof(ll));						\
		luaL_addlstring(&b,(const char*)&ll,sizeof(ll));	\
		luaL_addlstring(&b, a, l);							\
		break;												\
	}
#endif // LUA_WIDESTRING

static int l_pack(lua_State *L) 		/** pack(f,...) */
{
	int i=2;
	const char *f=luaL_checkstring(L,1);
	int swap=0;
	int padCount = 0;
	char padChar = 0;
	size_t padStartPos = 0;
	luaL_Buffer b;
	luaL_buffinit(L,&b);
	while (*f)
	{
		int c=*f++;
		int N=0;
		int isWide = 0;
		if (c == OP_WIDESTRING)
		{
			isWide = 1;
			c=*f++;
			if (!c)
				break;
		}
		else if (c == OP_PAD)
		{
			if (padCount == 0)
			{
				while (isdigit(*f))
					padCount = 10 * padCount + (*f++) - '0';
				if (*f == ':')
				{
					f++;
					while (isdigit(*f))
						padChar = 10 * (unsigned char)padChar + (*f++) - '0';
				}
				padStartPos = b.p - b.buffer;
			}
			else
			{
				size_t curPos = b.p - b.buffer;
				padCount -= curPos - padStartPos;
				while (padCount-- > 0)
					luaL_addlstring(&b, &padChar, 1);
			}
			continue;
		}
		while (isdigit(*f)) N=10*N+(*f++)-'0';
		if (N==0) N=1;
		while (N--)
		{
			switch (c)
			{
				case OP_LITTLEENDIAN:
				case OP_BIGENDIAN:
				case OP_NATIVE:
				{
					swap=doendian(c);
					N=0;
					break;
				}
				case OP_STRING:
				case OP_ZSTRING:
				{
					size_t l;
#if LUA_WIDESTRING
					if (isWide)
					{
						const wchar_t *a=luaL_checklwstring(L,i++,&l);
						size_t curPos = b.p - b.buffer;
						luaL_addlwstring(&b,a,l+(c==OP_ZSTRING));
						dowideswap(swap,b.buffer+curPos,l+(c==OP_ZSTRING));
					}
					else
#endif // LUA_WIDESTRING
					{
						const char *a=luaL_checklstring(L,i++,&l);
						luaL_addlstring(&b,a,l+(c==OP_ZSTRING));
					}
					break;
				}
				PACKSTRING(OP_BSTRING, unsigned char)
#if LUA_WIDESTRING
				PACKSTRING(OP_WSTRING, unsigned short)
#endif /* LUA_WIDESTRING */
				PACKSTRING(OP_SSTRING, size_t)
				PACKNUMBER(OP_NUMBER, lua_Number)
				PACKNUMBER(OP_DOUBLE, double)
				PACKNUMBER(OP_FLOAT, float)
				PACKNUMBER(OP_CHAR, char)
				PACKNUMBER(OP_BYTE, unsigned char)
				PACKNUMBER(OP_SHORT, short)
				PACKNUMBER(OP_USHORT, unsigned short)
				PACKNUMBER(OP_INT, int)
				PACKNUMBER(OP_UINT, unsigned int)
				PACKNUMBER(OP_LONG, long)
				PACKNUMBER(OP_ULONG, unsigned long)
				case OP_BOOLEAN:
				{
					unsigned char a;
					if (!lua_isboolean(L, i))
						luaL_error(L, "expected a boolean");
					a = (unsigned char)lua_toboolean(L, i++);
					luaL_addlstring(&b,(const char*)&a,sizeof(a));
					break;
				}
				case ' ': case ',':
					break;
				default:
					badcode(L,c);
					break;
			}
		}
	}
	luaL_pushresult(&b);
	return 1;
}

static const luaL_reg R[] =
{
	{"pack",	l_pack},
	{"unpack",	l_unpack},
	{NULL,	NULL}
};


int luaopen_pack(lua_State *L)
{
#ifdef USE_GLOBALS
 lua_register(L,"bpack",l_pack);
 lua_register(L,"bunpack",l_unpack);
#else
 luaL_openlib(L, "string", R, 0);
#endif
 return 0;
}


