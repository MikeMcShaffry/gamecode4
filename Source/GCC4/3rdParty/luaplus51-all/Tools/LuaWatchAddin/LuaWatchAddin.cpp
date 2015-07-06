
#include "custview.h"

#include "LuaWatchAddIn.h"

bool g_swapEndian = true;

#pragma warning(disable: 4996)


inline char& SwapEndian(char& val)
{
	return val;
}

inline unsigned char& SwapEndian(unsigned char& val)
{
	return val;
}

inline wchar_t& SwapEndian(wchar_t& val)
{
	if (g_swapEndian)
		val = (WORD)((val >> 8) | (val << 8));

	return val;
}

inline unsigned short& SwapEndian(unsigned short& val)
{
	if (g_swapEndian)
		val = (WORD)((val >> 8) | (val << 8));

	return val;
}

inline short& SwapEndian(short& val)
{
	SwapEndian((WORD&)val);

	return val;
}

inline unsigned int& SwapEndian(unsigned int& val)
{
	if (g_swapEndian)
	{
		BYTE dest[8];
		BYTE* src = (BYTE*)&val;

		dest[0] = src[3];
		dest[1] = src[2];
		dest[2] = src[1];
		dest[3] = src[0];

		val = *(unsigned long*)dest;
	//	val = (val >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | (val << 24);
	}

	return val;
}

inline unsigned long& SwapEndian(unsigned long& val)
{
	SwapEndian((unsigned int&)val);
	return val;
}

template <typename T>
inline T*& SwapEndian(T*& val)
{
	SwapEndian((unsigned int&)val);
	return val;
}

inline int& SwapEndian(int& val)
{
	SwapEndian((unsigned int&)val);

	return val;
}

inline float& SwapEndian(float& val)
{
	SwapEndian((unsigned int&)val);

	return val;
}

inline double& SwapEndian(double& val)
{
	if (g_swapEndian)
	{
		BYTE dest[8];
		BYTE* src = (BYTE*)&val;

		dest[0] = src[7];
		dest[1] = src[6];
		dest[2] = src[5];
		dest[3] = src[4];

		dest[4] = src[3];
		dest[5] = src[2];
		dest[6] = src[1];
		dest[7] = src[0];

		val = *(double*)dest;
	}

	return val;
}

inline unsigned __int64& SwapEndian(unsigned __int64& val)
{
	if (g_swapEndian)
	{
		BYTE dest[8];
		BYTE* src = (BYTE*)&val;

		dest[0] = src[3];
		dest[1] = src[2];
		dest[2] = src[1];
		dest[3] = src[0];

		dest[4] = src[7];
		dest[5] = src[6];
		dest[6] = src[5];
		dest[7] = src[4];

		val = *(unsigned __int64*)dest;
	}

	return val;
}



#if 0
//////////////////////////////////////////////////////////////////////////
// lua.h
//////////////////////////////////////////////////////////////////////////
/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_GLOBALSINDEX	(-10001)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


/*
** basic types
*/
#define LUA_TNONE	(-1)

#define LUA_TNIL	0
#define LUA_TBOOLEAN	1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER	3
#define LUA_TSTRING	4
#define LUA_TTABLE	5
#define LUA_TFUNCTION	6
#define LUA_TUSERDATA	7
#define LUA_TTHREAD 8
#define LUA_TWSTRING	9
#define LUA_NTYPES 11


struct lua_State;

typedef int (*lua_CFunction) (lua_State *L);

typedef void* (*lua_ReallocFunction)(void* ptr, int oldsize, int size, void* data, const char* allocName, unsigned int flags);
typedef void (*lua_FreeFunction)(void* ptr, int oldsize, void* data);

typedef struct lua_Debug lua_Debug;  /* activation record */

typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


//////////////////////////////////////////////////////////////////////////
// llimits.h
//////////////////////////////////////////////////////////////////////////
/* type of numbers in Lua */
#ifndef LUA_NUMBER
typedef double lua_Number;
#else
typedef LUA_NUMBER lua_Number;
#endif


/* an unsigned integer to hold hash values */
typedef unsigned int lu_hash;
/* its signed equivalent */
typedef int ls_hash;

/* an unsigned integer big enough to count the total memory used by Lua; */
/* it should be at least as large as size_t */
typedef unsigned long lu_mem;

/* an integer big enough to count the number of strings in use */
typedef long ls_nstr;

/* chars used as small naturals (so that `char' is reserved for characters) */
typedef unsigned char lu_byte;

/* type to ensure maximum alignment */
#ifndef LUSER_ALIGNMENT_T
typedef union { double u; void *s; long l; } L_Umaxalign;
#else
typedef LUSER_ALIGNMENT_T L_Umaxalign;
#endif


#ifndef check_exp
#define check_exp(c,e)	(e)
#endif


#ifndef cast
#define cast(t, exp)	((t)(exp))
#endif


///////////////////////////////////////////////////////////////////////////////
// lobject.h
///////////////////////////////////////////////////////////////////////////////

/*
** Union of all collectable objects
*/
typedef union GCObject GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	GCObject *next; GCObject *prev; lu_byte tt; lu_byte marked; unsigned short ref;


/*
** Common header in struct form
*/
typedef struct GCheader {
  CommonHeader;
} GCheader;



/*
** Union of all Lua values
*/
typedef union {
  GCObject *gc;
  void *p;
  lua_Number n;
  int b;
} Value;


typedef struct lua_TValue {
  int tt;
  Value value;
} TValue;

/* Macros to test type */
#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttiswstring(o)	(ttype(o) == LUA_TWSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)

/* Macros to access values */
#define ttype(o)	((o)->tt)
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
#define tsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)
#define uvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)
#define twsvalue(o)	check_exp(ttiswstring(o), &(o)->value.gc->ts)

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)

typedef TValue *StkId;  /* index to stack elements */


/*
** String headers for string table
*/
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  struct {
    CommonHeader;
    lu_byte reserved;
    lu_hash hash;
    size_t len;
  } tsv;
} TString;


#define getstr(ts)	cast(const char *, (ts) + 1)
#define svalue(o)       getstr(tsvalue(o))
#define getwstr(ts)	cast(const wchar_t *, (ts) + 1)
#define wsvalue(o)       getwstr(tsvalue(o))



typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
  struct {
    CommonHeader;
    struct Table *metatable;
    size_t len;
  } uv;
} Udata;



/*
** Closures
*/

#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist

typedef struct CClosure {
  ClosureHeader;
  int functiontype;
  lua_CFunction f;
/*  TValue upvalue[1];  Due to resizable closures, this can't be used anymore */
} CClosure;


// Differs from Lua's!
#define getcclosureupvalue(G, cl, n) \
	((TValue*)((unsigned char*)&cl+((G).sizeCClosure)+((n)*sizeof(TValue))))


typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  TValue g;  /* global table for this closure */
#if 0
  UpVal *upvals[1];
#endif 0
} LClosure;


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;



/*
** Tables
*/

typedef struct Node {
  TValue i_key;
  TValue i_val;
  struct Node *next;  /* for chaining */
} Node;


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of size of `node' array */
  struct Table *metatable;
  TValue *array;  /* array part */
  Node *node;
  Node *firstfree;  /* this position is free; all positions after it are full */
  GCObject *gclist;
  int sizearray;  /* size of `array' array */
} Table;


/*
** `module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))
#define sizearray(t)	((t)->sizearray)


//////////////////////////////////////////////////////////////////////////
// lzio.h
//////////////////////////////////////////////////////////////////////////
typedef struct Mbuffer {
  char *buffer;
  size_t buffsize;
} Mbuffer;


//////////////////////////////////////////////////////////////////////////
// ltm.h
//////////////////////////////////////////////////////////////////////////
/*
* WARNING: if you change the order of this enumeration,
* grep "ORDER TM"
*/
typedef enum {
  TM_INDEX,
  TM_NEWINDEX,
  TM_GC,
  TM_MODE,
  TM_EQ,  /* last tag method with `fast' access */
  TM_ADD,
  TM_SUB,
  TM_MUL,
  TM_DIV,
  TM_POW,
  TM_UNM,
  TM_LT,
  TM_LE,
  TM_CONCAT,
  TM_CALL,
  TM_N		/* number of elements in the enum */
} TMS;


//////////////////////////////////////////////////////////////////////////
// lstate.h
//////////////////////////////////////////////////////////////////////////
/* table of globals */
#define gt(L)	(&L->_gt)

/* registry */
#define registry(L)	(&G(L)->_registry)

typedef struct stringtable {
  GCObject **hash;
  ls_nstr nuse;  /* number of elements */
  int size;
} stringtable;


/*
** informations about a call
*/
typedef struct CallInfo {
  StkId base;  /* base for called function */
  StkId	top;  /* top for this function */
  int state;  /* bit fields; see below */
#if 0
  union {
    struct {  /* for Lua functions */
      const Instruction *savedpc;
      const Instruction **pc;  /* points to `pc' variable in `luaV_execute' */
    } l;
    struct {  /* for C functions */
      int dummy;  /* just to avoid an empty struct */
    } c;
  } u;
#endif 0
} CallInfo;


/*
** `global state', shared by all threads of this state
*/
typedef struct global_State {
  stringtable strt;  /* hash table for strings */
  GCObject *rootgc;  /* list of (almost) all collectable objects */
  GCObject *rootudata;   /* (separated) list of all userdata */
  GCObject *tmudata;  /* list of userdata to be GC */
  Mbuffer buff;  /* temporary buffer for string concatentation */
  lu_mem GCthreshold;
  lu_mem nblocks;  /* number of `bytes' currently allocated */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TValue _registry;
  TValue _defaultmeta;
  struct lua_State *mainthread;
  Node dummynode[1];  /* common node array for all empty tables */
  TString *tmname[TM_N];  /* array with tag-method names */
  // Custom data starts here.
  TValue _defaultmetatypes[LUA_NTYPES];
  void (*fatalErrorFunc)();
  void* lockData;
  void (*lockFunc)(void*);
  void (*unlockFunc)(void*);
  void* memData;
  lua_ReallocFunction reallocFunc;
  lua_FreeFunction freeFunc;
  int minimumstrings;
  unsigned int sizeCClosure;
#if 0
  lua_CClosureManager cclosureManager;
  void* globalUserData;
  void (*userGCFunction)(struct GCState*);
#endif
} global_State;


/*
** `per thread' state
*/
struct lua_State {
  CommonHeader;
  StkId top;  /* first free slot in the stack */
  StkId base;  /* base of current function */
  global_State *l_G;
  CallInfo *ci;  /* call info for current function */
  StkId stack_last;  /* last free slot in the stack */
  StkId stack;  /* stack base */
  int stacksize;
  CallInfo *end_ci;  /* points after end of ci array*/
  CallInfo *base_ci;  /* array of CallInfo's */
  unsigned short size_ci;  /* size of array `base_ci' */
  unsigned short nCcalls;  /* number of nested C calls */
  lu_byte hookmask;
  lu_byte allowhook;
  lu_byte hookinit;
  int basehookcount;
  int hookcount;
  lua_Hook hook;
  TValue _gt;  /* table of globals */
  GCObject *openupval;  /* list of open upvalues in this stack */
  GCObject *gclist;
  struct lua_longjmp *errorJmp;  /* current error recover point */
#if 0
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
  const char* allocName;
  unsigned int allocFlags;
  void* stateUserData;
#endif 0
};


#define G(L)	(L->l_G)


/*
** Union of all collectable objects
*/
union GCObject {
  GCheader gch;
  union TString ts;
  union Udata u;
  union Closure cl;
  struct Table h;
#if 0
  struct Proto p;
  struct UpVal uv;
  struct lua_State th;  /* thread */
#endif 0
};


struct LuaState
{
//	void* vtable;
	lua_State* m_state;
};


struct LuaObject
{
	LuaObject* m_next;		   // only valid when in free list
	LuaObject* m_prev;		   // only valid when in used list
	TValue m_object;
	LuaState* m_state;
	char filler[4];
};


bool g_swapEndian = false;


inline unsigned short& SwapEndian(unsigned short& val)
{
	if (g_swapEndian)
		val = (WORD)((val >> 8) | (val << 8));

	return val;
}

inline short& SwapEndian(short& val)
{
	SwapEndian((WORD&)val);

	return val;
}

inline unsigned int& SwapEndian(unsigned int& val)
{
	if (g_swapEndian)
	{
		BYTE dest[8];
		BYTE* src = (BYTE*)&val;

		dest[0] = src[3];
		dest[1] = src[2];
		dest[2] = src[1];
		dest[3] = src[0];

		val = *(unsigned long*)dest;
	//	val = (val >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | (val << 24);
	}

	return val;
}

inline unsigned long& SwapEndian(unsigned long& val)
{
	SwapEndian((unsigned int&)val);
	return val;
}

template <typename T>
inline T*& SwapEndian(T*& val)
{
	SwapEndian((unsigned int&)val);
	return val;
}

inline int& SwapEndian(int& val)
{
	SwapEndian((unsigned int&)val);

	return val;
}

inline float& SwapEndian(float& val)
{
	SwapEndian((unsigned int&)val);

	return val;
}

inline double& SwapEndian(double& val)
{
	if (g_swapEndian)
	{
		BYTE dest[8];
		BYTE* src = (BYTE*)&val;

		dest[0] = src[7];
		dest[1] = src[6];
		dest[2] = src[5];
		dest[3] = src[4];

		dest[4] = src[3];
		dest[5] = src[2];
		dest[6] = src[1];
		dest[7] = src[0];

		val = *(double*)dest;
	}

	return val;
}

inline unsigned __int64& SwapEndian(unsigned __int64& val)
{
	if (g_swapEndian)
	{
		BYTE dest[8];
		BYTE* src = (BYTE*)&val;

		dest[0] = src[3];
		dest[1] = src[2];
		dest[2] = src[1];
		dest[3] = src[0];

		dest[4] = src[7];
		dest[5] = src[6];
		dest[6] = src[5];
		dest[7] = src[4];

		val = *(unsigned __int64*)dest;
	}

	return val;
}



/* macros to convert a GCObject into a specific value */
#define gcotots(o)	check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#define gcotou(o)	check_exp((o)->gch.tt == LUA_TUSERDATA, &((o)->u))
#define gcotocl(o)	check_exp((o)->gch.tt == LUA_TFUNCTION, &((o)->cl))
#define gcotoh(o)	check_exp((o)->gch.tt == LUA_TTABLE, &((o)->h))
#define gcotop(o)	check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
#define gcotouv(o)	check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define ngcotouv(o) \
	check_exp((o) == NULL || (o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define gcototh(o)	check_exp((o)->gch.tt == LUA_TTHREAD, &((o)->th))

/* macro to convert any value into a GCObject */
#define valtogco(v)	(cast(GCObject *, (v)))


typedef struct LuaStackObject
{
	LuaState* m_state;
	int m_stackIndex;
} LuaStackObject;
#endif

void ReadMem( void* pHelperIn, const void* fromPtr, void* toPtr,
			 size_t size )
{
	DEBUGHELPER* pHelper = (DEBUGHELPER*)pHelperIn;
	SIZE_T numRead;
	if ( pHelper->ReadDebuggeeMemory( pHelper, (DWORD)fromPtr, size, toPtr,
			&numRead ) != S_OK )
	{
		throw -1;
	}

	if (size != numRead)
	{
		throw -1;
	}
}


#include "../LuaPlusDebuggerAddin/LuaPlusDebuggerControls/LuaPlusTypes.h"

int ReadTString( DEBUGHELPER *pHelper, TString* str, char* pResult, size_t max )
{
	TString data;
	size_t size;

	ReadMem( pHelper, str, &data, sizeof( TString ) );
	SwapEndian(data.tsv.len);

	if (data.tsv.tt == LUA_TSTRING)
	{
		size = min( data.tsv.len, max );
		ReadMem( pHelper, (void*)( str + 1 ), pResult, size );
		pResult[ size ] = 0;
		return size;
	}
	else if (data.tsv.tt == LUA_TWSTRING)
	{
		size = min( data.tsv.len * 2, max / 2 );
		ReadMem( pHelper, (void*)( str + 1 ), pResult, size );
		if (g_swapEndian)
		{
			WORD* chars = (WORD*)pResult;
			for (size_t i = 0; i < size / 2; ++i)
			{
				SwapEndian(chars[i]);
			}
		}
		pResult[ size ] = 0;
		pResult[ size + 1 ] = 0;
		return size;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// lapi.c
//////////////////////////////////////////////////////////////////////////
// Differs from Lua's!
//#define getcclosureupvalue(G, cl, n) \
//	((TValue*)((unsigned char*)&cl+((G).sizeCClosure)+((n)*sizeof(TValue))))

/* table of globals */
#define gt(L)	(&L->l_gt)

/* registry */
#define registry(L)	(&G(L)->l_registry)
#if 0
TValue *negindex (DEBUGHELPER* pHelper, lua_State *L, int index)
{
	if (index > LUA_REGISTRYINDEX)
	{
		return L->top+index;
	}
	else if (index < -40000)
	{
		return (TValue*)luaH_getnum(pHelper, hvalue(registry(L)), -index - 40000);
	}
	else switch (index) {  /* pseudo-indices */
		case LUA_REGISTRYINDEX: return registry(L);
		case LUA_GLOBALSINDEX: return gt(L);
		default:
		{
//      TValue *func = (L->base - 1);
//      index = LUA_GLOBALSINDEX - index;
//      return getcclosureupvalue(L, clvalue(func)->c, index-1);
			TValue *func = (L->base - 1);
			index = LUA_GLOBALSINDEX - index;

			global_State globalState;
			ReadMem(pHelper, L->l_G, &globalState, sizeof(global_State) );

			CClosure cClosure;
			ReadMem(pHelper, &clvalue(func)->c, &cClosure, sizeof(CClosure));

			return getcclosureupvalue(globalState, cClosure, index-1);
		 }
	}
}


TValue *luaA_indexAcceptable (DEBUGHELPER* pHelper, lua_State *L, int index)
{
	if (index > 0)
	{
		TValue *o = L->base+(index-1);
		if (o >= L->top) return NULL;
		else return o;
	}
	else
		return negindex(pHelper, L, index);
}
#endif

TValue *index2adr (DEBUGHELPER* pHelper, lua_State *L, int idx) {
	if (idx > 0) {
		TValue *o = L->base + (idx - 1);
		if (o >= L->top) return cast(TValue *, luaO_nilobject);
		else return o;
	}
	else if (idx > LUA_REGISTRYINDEX) {
		return L->top + idx;
	}
	else switch (idx) {  /* pseudo-indices */
		case LUA_REGISTRYINDEX: return registry(L);
		case LUA_ENVIRONINDEX: {
			Closure *func = curr_func(L);
			sethvalue(L, &L->env, func->c.env);
			return &L->env;
		}
		case LUA_GLOBALSINDEX: return gt(L);
		default: {
			Closure *func = curr_func(L);
			idx = LUA_GLOBALSINDEX - idx;
			return (idx <= func->c.nupvalues)
				? &func->c.upvalue[idx-1]
			: cast(TValue *, luaO_nilobject);
		}
	}
}


void Readlua_State( DEBUGHELPER* pHelper, lua_State* from, lua_State& luaState)
{
//#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked
	ReadMem(pHelper, from , &luaState, sizeof(lua_State));
}


void ReadTValue( DEBUGHELPER *pHelper, lua_State* state, TValue& obj, char* pResult,
				  size_t max )
{
	char outBuf[1024];
	size_t maxOutBuf;

	maxOutBuf = min(1022, max - 2);
	switch (ttype(&obj))
	{
		case LUA_TNONE:
			strcpy(pResult, "[NONE]");
			break;

		case LUA_TNIL:
			strcpy(pResult, "[NIL]");
			break;

		case LUA_TBOOLEAN:
			sprintf( pResult, "[BOOLEAN] %s", obj.value.b != 0 ? "true" : "false" );
			break;

		case LUA_TLIGHTUSERDATA:
			sprintf( pResult, "[LIGHTUSERDATA] %08x", obj.value.p );
			break;

		case LUA_TNUMBER:
			sprintf(pResult, "[NUMBER] %.14g", nvalue(&obj));
			break;

		case LUA_TSTRING:
		{
			TString data;
			size_t size;

			ReadMem(pHelper, obj.value.gc, &data, sizeof(TString));
			SwapEndian(data.tsv.len);

			size = min( data.tsv.len, maxOutBuf );
			ReadMem( pHelper, (void*)( ((TString*)obj.value.gc) + 1 ), &outBuf, size );
			outBuf[ size ] = 0;

			sprintf( pResult, "[STRING] \"%s\"", outBuf );
			break;
		}

		case LUA_TTABLE:
			strcpy( pResult, "[TABLE]" );
			break;

		case LUA_TFUNCTION:
		{
			Closure cl;
			ReadMem(pHelper, obj.value.gc, &cl, sizeof(Closure));

			if (cl.c.isC)
				strcpy( pResult, "[C-FUNCTION]" );
			else
				strcpy( pResult, "[FUNCTION]" );
			break;
		}

		case LUA_TUSERDATA:
		{
			Udata data;
			void* ptr;

			ReadMem( pHelper, obj.value.gc, &data, sizeof(Udata) );

			ptr = ((Udata*)obj.value.gc) + 1;
			if ( data.uv.len & 4 )
			{
				// The user data pointer is embedded.
				ReadMem( pHelper, ptr, &ptr, sizeof(void*) );
				SwapEndian(ptr);
			}

			sprintf( pResult, "[USERDATA] %08x", ptr );
			break;
		}

		case LUA_TTHREAD:
			strcpy(pResult, "[THREAD]");
			break;

		case LUA_TWSTRING:
		{
			TString data;
			size_t size;

			ReadMem(pHelper, obj.value.gc, &data, sizeof(TString));
			SwapEndian(data.tsv.len);

			size = min(data.tsv.len, maxOutBuf / 2);
			ReadMem( pHelper, (void*)( ((TString*)obj.value.gc) + 1 ), &outBuf, size * 2 );
			if (g_swapEndian)
			{
				WORD* chars = (WORD*)&outBuf;
				for (size_t i = 0; i < size; ++i)
				{
					SwapEndian(chars[i]);
				}
			}
			outBuf[ size * 2 ] = 0;
			outBuf[ size * 2 + 1 ] = 0;

			sprintf( pResult, "[WSTRING] \"%S\"", (wchar_t*)outBuf );
			break;
		}
	}
}


void ReadLuaStack( DEBUGHELPER *pHelper, lua_State* state, int stackIndex, char* pResult,
				  size_t max )
{
	lua_State luaState;
	CallInfo callInfo;
	TValue obj;
	TValue* srcO;

	ReadMem( pHelper, state, &luaState, sizeof(lua_State) );

	ReadMem( pHelper, luaState.ci, &callInfo, sizeof(CallInfo) );

	srcO = index2adr(pHelper, &luaState, stackIndex);
	if (!srcO)
	{
		strcpy(pResult, "**Out of bounds**");
		return;
	}

	ReadMem( pHelper, srcO, &obj, sizeof(TValue) );

	ReadTValue(pHelper, state, obj, pResult, max);
}


void ReadLuaObject( DEBUGHELPER* pHelper, LuaObject& luaObject, char *pResult, size_t max )
{
	if (!luaObject.m_state)
	{
		strcpy(pResult, "** Unassigned **");
		return;
	}

	LuaState luaState;
	ReadMem( pHelper, (void*)luaObject.m_state, &luaState, sizeof( LuaState ) );

	SwapEndian(luaState.m_state);

	ReadTValue( pHelper, luaState.m_state, luaObject.m_object, pResult, max );
}


void ReadLuaObject( DEBUGHELPER* pHelper, DWORD dwAddress, char *pResult, size_t max )
{
	LuaObject luaObject;
	ReadMem( pHelper, (void*)dwAddress, &luaObject, sizeof( LuaObject ) );

	if (luaObject.m_object.tt < LUA_TNONE  ||  luaObject.m_object.tt >= LUA_TWSTRING)
	{
		g_swapEndian = true;
	}
	
	SwapEndian(luaObject.m_prev);
	SwapEndian(luaObject.m_next);
	SwapEndian(luaObject.m_object.tt);
	switch (luaObject.m_object.tt)
	{
		case LUA_TBOOLEAN:			SwapEndian(luaObject.m_object.value.b);		break;
		case LUA_TLIGHTUSERDATA:	SwapEndian(luaObject.m_object.value.p);		break;
		case LUA_TNUMBER:			SwapEndian(luaObject.m_object.value.n);		break;
		case LUA_TSTRING:			SwapEndian(luaObject.m_object.value.gc);	break;
		case LUA_TTABLE:			SwapEndian(luaObject.m_object.value.gc);	break;
		case LUA_TFUNCTION:			SwapEndian(luaObject.m_object.value.gc);	break;
		case LUA_TUSERDATA:			SwapEndian(luaObject.m_object.value.gc);	break;
		case LUA_TTHREAD:			SwapEndian(luaObject.m_object.value.gc);	break;
		case LUA_TWSTRING:			SwapEndian(luaObject.m_object.value.gc);	break;
	}

	SwapEndian(luaObject.m_state);

	ReadLuaObject( pHelper, luaObject, pResult, max );
}


void SetEndian(DEBUGHELPER* pHelper)
{
	int processorType = pHelper->GetProcessorType(pHelper);
	if (processorType == 14)
	{
		g_swapEndian = true;
	}
	else
	{
		g_swapEndian = false;
	}
}


extern "C" __declspec(dllexport) HRESULT WINAPI AddIn_LuaObject( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved )
{
	reserved, bUniStrings, nBase;

	SetEndian(pHelper);

	try
	{
		ReadLuaObject( pHelper, dwAddress, pResult, max );
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	return S_OK;
}


class LuaTableIterator
{
public:
	LuaObject m_keyObj;
	LuaObject m_valueObj;
	LuaObject m_tableObj;				///< The table object being iterated.
	bool m_isDone;
};


extern "C" HRESULT WINAPI AddIn_LuaTableIterator( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t maxSize, DWORD reserved )
{
	reserved, bUniStrings, nBase;

	SetEndian(pHelper);

	try
	{
		const int RESULT_LEN = 1024;
		char keyResult[ RESULT_LEN ];
		char valueResult[ RESULT_LEN ];

		LuaTableIterator luaTableIterator;
		ReadLuaObject( pHelper, dwAddress, keyResult, RESULT_LEN );
		ReadLuaObject( pHelper, dwAddress + sizeof(LuaObject), valueResult, RESULT_LEN );
		ReadMem( pHelper, (void*)(dwAddress + sizeof(LuaObject) * 3), &luaTableIterator.m_isDone, sizeof( bool ) );

		if (luaTableIterator.m_isDone)
		{
			strcpy(pResult, "** DONE **");
			return S_OK;
		}

		maxSize -= strlen( "Key:  Value:  " );
		size_t keyResultLen = strlen( keyResult );
		size_t valueResultLen = strlen( valueResult );
		keyResultLen = min( keyResultLen, maxSize );
		valueResultLen = min( valueResultLen, maxSize - keyResultLen );

		strcpy( pResult, "Key: " );
		int len = strlen( pResult );
		strncpy( pResult + len, keyResult, keyResultLen );
		pResult[ len + keyResultLen ] = '\0';
		strcat( pResult, " Value: " );
		len = strlen( pResult );
		strncpy( pResult + len, valueResult, valueResultLen );
		pResult[ len + valueResultLen ] = ' ';
		pResult[ len + valueResultLen + 1 ] = '\0';
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	return S_OK;
}


extern "C" HRESULT WINAPI AddIn_LuaStackObject( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t max, DWORD reserved )
{
	reserved, bUniStrings, nBase;

	SetEndian(pHelper);

	try
	{
		LuaStackObject luaObject;
		ReadMem( pHelper, (void*)dwAddress, &luaObject, sizeof( LuaStackObject ) );

		LuaState luaState;
		ReadMem( pHelper, (void*)luaObject.m_state, &luaState, sizeof( LuaState ) );

		ReadLuaStack( pHelper, luaState.m_state, luaObject.m_stackIndex, pResult, max );
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	return S_OK;
}


class LuaStackTableIterator
{
public:
	LuaStackObject m_tableObj;				///< The table object being iterated.
	int m_startStackIndex;				///< The current starting stack index or -1 if the iterator is invalid.
	bool m_autoStackManagement;			///< Auto stack management enabled or not?
};


extern "C" HRESULT WINAPI AddIn_LuaStackTableIterator( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t maxSize, DWORD reserved )
{
	reserved, bUniStrings, nBase;

	SetEndian(pHelper);

	try
	{
		LuaStackTableIterator luaTableIterator;
		ReadMem( pHelper, (void*)dwAddress, &luaTableIterator, sizeof( LuaStackTableIterator ) );

		const int RESULT_LEN = 1024;
		char keyResult[ RESULT_LEN ];
		char valueResult[ RESULT_LEN ];

		int keyIndex = luaTableIterator.m_startStackIndex + 1;
		int valueIndex = luaTableIterator.m_startStackIndex + 2;

		LuaState luaState;
		ReadMem( pHelper, (void*)luaTableIterator.m_tableObj.m_state, &luaState, sizeof( LuaState ) );

		lua_State* state = luaState.m_state;

		ReadLuaStack( pHelper, state, keyIndex, keyResult, RESULT_LEN );
		ReadLuaStack( pHelper, state, valueIndex, valueResult, RESULT_LEN );

		maxSize -= strlen( "Key:  Value:  " );
		size_t keyResultLen = strlen( keyResult );
		size_t valueResultLen = strlen( valueResult );
		keyResultLen = min( keyResultLen, maxSize );
		valueResultLen = min( valueResultLen, maxSize - keyResultLen );

		strcpy( pResult, "Key: " );
		int len = strlen( pResult );
		strncpy( pResult + len, keyResult, keyResultLen );
		pResult[ len + keyResultLen ] = '\0';
		strcat( pResult, " Value: " );
		len = strlen( pResult );
		strncpy( pResult + len, valueResult, valueResultLen );
		pResult[ len + valueResultLen ] = ' ';
		pResult[ len + valueResultLen + 1 ] = '\0';
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	return S_OK;
}


extern "C" HRESULT WINAPI AddIn_TString( DWORD dwAddress, DEBUGHELPER *pHelper, int nBase, BOOL bUniStrings, char *pResult, size_t maxSize, DWORD reserved )
{
	reserved, bUniStrings, nBase;

	SetEndian(pHelper);

	try
	{
		pResult[ 0 ] = '"';
		int len = ReadTString( pHelper, (TString*)dwAddress, pResult + 1, maxSize - 2 ) + 1;
		pResult[ len ] = '"';
		pResult[ len + 1 ] = '\0';
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	return S_OK;
}
