#pragma once

#include <atlstr.h>

void ReadMem(void* pHelper, const void* fromPtr, void* toPtr, size_t size);

bool g_swapEndian = true;

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


//////////////////////////////////////////////////////////////////////////
// lua.h
//////////////////////////////////////////////////////////////////////////
/*
** pseudo-indices
*/
#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)
#define lua_upvalueindex(i)	(LUA_GLOBALSINDEX-(i))


typedef struct lua_State lua_State;

typedef int (*lua_CFunction) (lua_State *L);


/*
** prototype for memory-allocation functions
*/
typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize, const char* name, unsigned int alloc_flags);


/*
** basic types
*/
#define LUA_TNONE		(-1)

#define LUA_TNIL		0
#define LUA_TBOOLEAN		1
#define LUA_TLIGHTUSERDATA	2
#define LUA_TNUMBER		3
#define LUA_TSTRING		4
#define LUA_TTABLE		5
#define LUA_TFUNCTION		6
#define LUA_TUSERDATA		7
#define LUA_TTHREAD		8
#define LUA_TWSTRING	9


/* type of lex characters in Lua */
typedef unsigned short lua_WChar;

typedef void* (*lua_ReallocFunction)(void* ptr, int oldsize, int size, void* data, const char* allocName, unsigned int flags);
typedef void (*lua_FreeFunction)(void* ptr, int oldsize, void* data);

typedef struct lua_Debug lua_Debug;  /* activation record */

typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);


#define LUA_NUMBER	double

/* type of numbers in Lua */
typedef LUA_NUMBER lua_Number;

//////////////////////////////////////////////////////////////////////////
// luaconf.h
//////////////////////////////////////////////////////////////////////////
#define LUAI_UINT32	unsigned int
#define LUAI_INT32	int
#define LUAI_MAXINT32	INT_MAX
#define LUAI_UMEM	size_t
#define LUAI_MEM	ptrdiff_t


#define LUAI_USER_ALIGNMENT_T	union { double u; void *s; long l; }


#define luai_numeq(a,b)		((a)==(b))

#define lua_number2int(i,d)   i = (int)d;

//////////////////////////////////////////////////////////////////////////
// llimits.h
//////////////////////////////////////////////////////////////////////////
typedef LUAI_UINT32 lu_int32;

typedef LUAI_UMEM lu_mem;

typedef LUAI_MEM l_mem;



/*
** conversion of pointer to integer
** this is for hashing only; there is no problem if the integer
** cannot hold the whole pointer value
*/
#define IntPoint(p)  ((unsigned int)(lu_mem)(p))



/* chars used as small naturals (so that `char' is reserved for characters) */
typedef unsigned char lu_byte;


/* type to ensure maximum alignment */
typedef LUAI_USER_ALIGNMENT_T L_Umaxalign;


/*
** type for virtual-machine instructions
** must be an unsigned with (at least) 4 bytes (see details in lopcodes.h)
*/
typedef lu_int32 Instruction;


#ifndef cast
#define cast(t, exp)	((t)(exp))
#endif

#define cast_byte(i)	cast(lu_byte, (i))
#define cast_num(i)	cast(lua_Number, (i))
#define cast_int(i)	cast(int, (i))


#define check_exp(c,e)		(e)

//////////////////////////////////////////////////////////////////////////
// lobject.h
//////////////////////////////////////////////////////////////////////////
/* tags for values visible from Lua */
#define LAST_TAG	LUA_TWSTRING

#define NUM_TAGS	(LAST_TAG+1)


/* tags for values visible from Lua */
#define LAST_TAG	LUA_TWSTRING

#define NUM_TAGS	(LAST_TAG+1)


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	(LAST_TAG+1)
#define LUA_TUPVAL	(LAST_TAG+2)
#define LUA_TDEADKEY	(LAST_TAG+3)


/*
** Union of all collectable objects
*/
typedef union GCObject GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


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


/*
** Tagged Values
*/

#define TValuefields	Value value; int tt

typedef struct lua_TValue {
  TValuefields;
} TValue;


/* Macros to test type */
#define ttisnil(o)	(ttype(o) == LUA_TNIL)
#define ttisnumber(o)	(ttype(o) == LUA_TNUMBER)
#define ttisstring(o)	(ttype(o) == LUA_TSTRING)
#define ttistable(o)	(ttype(o) == LUA_TTABLE)
#define ttisfunction(o)	(ttype(o) == LUA_TFUNCTION)
#define ttisboolean(o)	(ttype(o) == LUA_TBOOLEAN)
#define ttisuserdata(o)	(ttype(o) == LUA_TUSERDATA)
#define ttisthread(o)	(ttype(o) == LUA_TTHREAD)
#define ttislightuserdata(o)	(ttype(o) == LUA_TLIGHTUSERDATA)
#define ttiswstring(o)	(ttype(o) == LUA_TWSTRING)

/* Macros to access values */
#define ttype(o)	((o)->tt)
#define gcvalue(o)	check_exp(iscollectable(o), (o)->value.gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), (o)->value.p)
#define nvalue(o)	check_exp(ttisnumber(o), (o)->value.n)
#define rawtsvalue(o)	check_exp(ttisstring(o), &(o)->value.gc->ts)
#define tsvalue(o)	(&rawtsvalue(o)->tsv)
#define rawuvalue(o)	check_exp(ttisuserdata(o), &(o)->value.gc->u)
#define uvalue(o)	(&rawuvalue(o)->uv)
#define clvalue(o)	check_exp(ttisfunction(o), &(o)->value.gc->cl)
#define hvalue(o)	check_exp(ttistable(o), &(o)->value.gc->h)
#define bvalue(o)	check_exp(ttisboolean(o), (o)->value.b)
#define thvalue(o)	check_exp(ttisthread(o), &(o)->value.gc->th)
#define rawtwsvalue(o)	check_exp(ttiswstring(o), &(o)->value.gc->ts)
#define twsvalue(o)	(&rawtwsvalue(o)->tsv)

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

#define iscollectable(o)	(ttype(o) >= LUA_TSTRING)



typedef TValue *StkId;  /* index to stack elements */


/*
** for internal debug only
*/
#define checkconsistency(obj)

#define checkliveness(g,obj)


/* Macros to set values */
#define setnilvalue(obj) ((obj)->tt=LUA_TNIL)

#define setnvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.n=(x); i_o->tt=LUA_TNUMBER; }

#define setpvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.p=(x); i_o->tt=LUA_TLIGHTUSERDATA; }

#define setbvalue(obj,x) \
  { TValue *i_o=(obj); i_o->value.b=(x); i_o->tt=LUA_TBOOLEAN; }

#define setsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TSTRING; \
    checkliveness(G(L),i_o); }

#define setuvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TUSERDATA; \
    checkliveness(G(L),i_o); }

#define setthvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTHREAD; \
    checkliveness(G(L),i_o); }

#define setclvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TFUNCTION; \
    checkliveness(G(L),i_o); }

#define sethvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TTABLE; \
    checkliveness(G(L),i_o); }

#define setptvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TPROTO; \
    checkliveness(G(L),i_o); }




#define setobj(L,obj1,obj2) \
  { const TValue *o2=(obj2); TValue *o1=(obj1); \
    o1->value = o2->value; o1->tt=o2->tt; \
    checkliveness(G(L),o1); }

#define setwsvalue(L,obj,x) \
  { TValue *i_o=(obj); \
    i_o->value.gc=cast(GCObject *, (x)); i_o->tt=LUA_TWSTRING; \
    checkliveness(G(L),i_o); }


/*
** different types of sets, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define setwsvalue2s	setwsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to table */
#define setobj2t	setobj
/* to new object */
#define setnilvalue2n(L,obj) ((obj)->tt=LUA_TNIL)
#define setobj2n	setobj
#define setsvalue2n	setsvalue
#define setwsvalue2n	setwsvalue

#define setttype(obj, tt) (ttype(obj) = (tt))


/*
** String headers for string table
*/
typedef union TString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  struct {
    CommonHeader;
    lu_byte reserved;
    unsigned int hash;
    size_t len;
  } tsv;
} TString;


typedef union Udata {
  L_Umaxalign dummy;  /* ensures maximum alignment for `local' udata */
  struct {
    CommonHeader;
    struct Table *metatable;
    struct Table *env;
    size_t len;
  } uv;
} Udata;




/*
** Function Prototypes
*/
typedef struct Proto {
  CommonHeader;
  TValue *k;  /* constants used by the function */
  Instruction *code;
  struct Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines */
  struct LocVar *locvars;  /* information about local variables */
  TString **upvalues;  /* upvalue names */
  TString  *source;
  int sizeupvalues;
  int sizek;  /* size of `k' */
  int sizecode;
  int sizelineinfo;
  int sizep;  /* size of `p' */
  int sizelocvars;
  int linedefined;
  int lastlinedefined;
  GCObject *gclist;
  lu_byte nups;  /* number of upvalues */
  lu_byte numparams;
  lu_byte is_vararg;
  lu_byte maxstacksize;
} Proto;


typedef struct LocVar {
  TString *varname;
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} LocVar;



/*
** Upvalues
*/

typedef struct UpVal {
  CommonHeader;
  TValue *v;  /* points to stack or to its own value */
  union {
    TValue value;  /* the value (when closed) */
    struct {  /* double linked list (when open) */
      struct UpVal *prev;
      struct UpVal *next;
    } l;
  } u;
} UpVal;


/*
** Closures
*/

#define ClosureHeader \
	CommonHeader; lu_byte isC; lu_byte nupvalues; GCObject *gclist; \
	struct Table *env

typedef struct CClosure {
  ClosureHeader;
  lua_CFunction f;
  TValue upvalue[1];
} CClosure;


typedef struct LClosure {
  ClosureHeader;
  struct Proto *p;
  UpVal *upvals[1];
} LClosure;


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;


/*
** Tables
*/

typedef union TKey {
  struct {
    TValuefields;
    struct Node *next;  /* for chaining */
  } nk;
  TValue tvk;
} TKey;


typedef struct Node {
  TValue i_val;
  TKey i_key;
} Node;


typedef struct Table {
  CommonHeader;
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
  lu_byte lsizenode;  /* log2 of size of `node' array */
  struct Table *metatable;
  TValue *array;  /* array part */
  Node *node;
  Node *lastfree;  /* any free position is before this position */
  GCObject *gclist;
  int sizearray;  /* size of `array' array */
} Table;



/*
** `module' operation for hashing (size is always a power of 2)
*/
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


#define luaO_nilobject		(&luaO_nilobject_)

//////////////////////////////////////////////////////////////////////////
// lzio.h
//////////////////////////////////////////////////////////////////////////
typedef struct Mbuffer {
  char *buffer;
  size_t n;
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
  TM_MOD,
  TM_POW,
  TM_UNM,
  TM_LEN,
  TM_LT,
  TM_LE,
  TM_CONCAT,
  TM_CALL,
  TM_N		/* number of elements in the enum */
} TMS;


//////////////////////////////////////////////////////////////////////////
// lstate.h
//////////////////////////////////////////////////////////////////////////
typedef struct stringtable {
  GCObject **hash;
  lu_int32 nuse;  /* number of elements */
  int size;
} stringtable;


/*
** informations about a call
*/
typedef struct CallInfo {
  StkId base;  /* base for this function */
  StkId func;  /* function index in the stack */
  StkId	top;  /* top for this function */
  const Instruction *savedpc;
  int nresults;  /* expected number of results from this function */
  int tailcalls;  /* number of tail calls lost under this entry */
} CallInfo;


#define curr_func(L)	(clvalue(L->ci->func))
#define ci_func(ci)	(clvalue((ci)->func))
#define f_isLua(ci)	(!ci_func(ci)->c.isC)
#define isLua(ci)	(ttisfunction((ci)->func) && f_isLua(ci))


typedef struct MiniLuaObject
{
	void* m_next;		   // only valid when in free list
	void* m_prev;		   // only valid when in used list
} MiniLuaObject;


/*
** `global state', shared by all threads of this state
*/
typedef struct global_State {
  stringtable strt;  /* hash table for strings */
  lua_Alloc frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to `frealloc' */
  lu_byte currentwhite;
  lu_byte gcstate;  /* state of garbage collector */
  int sweepstrgc;  /* position of sweep in `strt' */
  GCObject *rootgc;  /* list of all collectable objects */
  GCObject **sweepgc;  /* position of sweep in `rootgc' */
  GCObject *gray;  /* list of gray objects */
  GCObject *grayagain;  /* list of objects to be traversed atomically */
  GCObject *weak;  /* list of weak tables (to be cleared) */
  GCObject *tmudata;  /* last element of list of userdata to be GC */
  Mbuffer buff;  /* temporary buffer for string concatentation */
  lu_mem GCthreshold;
  lu_mem totalbytes;  /* number of bytes currently allocated */
  lu_mem estimate;  /* an estimate of number of bytes actually in use */
  lu_mem gcdept;  /* how much GC is `behind schedule' */
  int gcpause;  /* size of pause between successive GCs */
  int gcstepmul;  /* GC `granularity' */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TValue l_registry;
  struct lua_State *mainthread;
  UpVal uvhead;  /* head of double-linked list of all open upvalues */
  struct Table *mt[NUM_TAGS];  /* metatables for basic types */
  TString *tmname[TM_N];  /* array with tag-method names */
  // Custom data starts here.
  void (*fatalErrorFunc)(void);
#ifdef LUA_MTSUPPORT
  void* lockData;
  void (*lockFunc)(void*);
  void (*unlockFunc)(void*);
#endif /* LUA_MTSUPPORT */
  int minimumstrings;
  void* globalUserData;
  void (*userGCFunction)(void*);
  MiniLuaObject m_headObject;
  MiniLuaObject m_tailObject;
} global_State;


/*
** `per thread' state
*/
struct lua_State {
  CommonHeader;
  lu_byte status;
  StkId top;  /* first free slot in the stack */
  StkId base;  /* base of current function */
  global_State *l_G;
  CallInfo *ci;  /* call info for current function */
  const Instruction *savedpc;  /* `savedpc' of current function */
  StkId stack_last;  /* last free slot in the stack */
  StkId stack;  /* stack base */
  CallInfo *end_ci;  /* points after end of ci array*/
  CallInfo *base_ci;  /* array of CallInfo's */
  int stacksize;
  int size_ci;  /* size of array `base_ci' */
  unsigned short nCcalls;  /* number of nested C calls */
  lu_byte hookmask;
  lu_byte allowhook;
  int basehookcount;
  int hookcount;
  lua_Hook hook;
  TValue l_gt;  /* table of globals */
  TValue env;  /* temporary place for environments */
  GCObject *openupval;  /* list of open upvalues in this stack */
  GCObject *gclist;
  struct lua_longjmp *errorJmp;  /* current error recover point */
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
#ifdef _DEBUG
  const char* allocName;
#endif /* _DEBUG */
  unsigned int allocFlags;
  void* stateUserData;
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
  struct Proto p;
  struct UpVal uv;
  struct lua_State th;  /* thread */
};


/* macros to convert a GCObject into a specific value */
#define rawgco2ts(o)	check_exp((o)->gch.tt == LUA_TSTRING, &((o)->ts))
#define gco2ts(o)	(&rawgco2ts(o)->tsv)
#define rawgco2u(o)	check_exp((o)->gch.tt == LUA_TUSERDATA, &((o)->u))
#define gco2u(o)	(&rawgco2u(o)->uv)
#define gco2cl(o)	check_exp((o)->gch.tt == LUA_TFUNCTION, &((o)->cl))
#define gco2h(o)	check_exp((o)->gch.tt == LUA_TTABLE, &((o)->h))
#define gco2p(o)	check_exp((o)->gch.tt == LUA_TPROTO, &((o)->p))
#define gco2uv(o)	check_exp((o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define ngcotouv(o) \
	check_exp((o) == NULL || (o)->gch.tt == LUA_TUPVAL, &((o)->uv))
#define gco2th(o)	check_exp((o)->gch.tt == LUA_TTHREAD, &((o)->th))

/* macro to convert any Lua object into a GCObject */
#define obj2gco(v)	(cast(GCObject *, (v)))


//////////////////////////////////////////////////////////////////////////
// ltable.h
//////////////////////////////////////////////////////////////////////////
#define gnode(t,i)	(&(t)->node[i])
#define gkey(n)		(&(n)->i_key.nk)
#define gval(n)		(&(n)->i_val)
#define gnext(n)	((n)->i_key.nk.next)

#define key2tval(n)	(&(n)->i_key.tvk)


//////////////////////////////////////////////////////////////////////////
// LuaState.h
//////////////////////////////////////////////////////////////////////////
struct LuaState
{
	lua_State* m_state;
	bool m_ownState;
};


//////////////////////////////////////////////////////////////////////////
// LuaObject.h
//////////////////////////////////////////////////////////////////////////
struct LuaObject
{
	LuaObject* m_next;		   // only valid when in free list
	LuaObject* m_prev;		   // only valid when in used list
	lua_TValue m_object;
	LuaState* m_state;
};


//////////////////////////////////////////////////////////////////////////
// LuaStackObject.h
//////////////////////////////////////////////////////////////////////////
struct LuaStackObject
{
	LuaState* m_state;		//!< The parent state of this object.
	int m_stackIndex;		//!< The stack index representing this object.
};


lua_State theState;

//////////////////////////////////////////////////////////////////////////
// lobject.c
//////////////////////////////////////////////////////////////////////////
const TValue luaO_nilobject_ = {{NULL}, LUA_TNIL};


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
//      lua_assert(iscollectable(t1));
      return gcvalue(t1) == gcvalue(t2);
  }
}


//////////////////////////////////////////////////////////////////////////
// ltable.c
//////////////////////////////////////////////////////////////////////////
#define _offsetof(s,m)  (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))

void ReadMem_Table(void* pHelper, void* fromIn, Table& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem(pHelper, from, &to, sizeof(Table));
	SwapEndian(to.next);
//	SwapEndian(to.prev);
//	SwapEndian(to.ref);
	SwapEndian(to.metatable);
	SwapEndian(to.array);
	SwapEndian(to.node);
	SwapEndian(to.lastfree);
	SwapEndian(to.gclist);
	SwapEndian(to.sizearray);
}


void ReadMem_TValue(void* pHelper, void* fromIn, TValue& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem(pHelper, from, &to, sizeof(to));
	SwapEndian(to.tt);
	switch (to.tt)
	{
		case LUA_TBOOLEAN:			SwapEndian(to.value.b);		break;
		case LUA_TLIGHTUSERDATA:	SwapEndian(to.value.p);		break;
		case LUA_TNUMBER:			SwapEndian(to.value.n);		break;
		case LUA_TSTRING:			SwapEndian(to.value.gc);	break;
		case LUA_TTABLE:			SwapEndian(to.value.gc);	break;
		case LUA_TFUNCTION:			SwapEndian(to.value.gc);	break;
		case LUA_TUSERDATA:			SwapEndian(to.value.gc);	break;
		case LUA_TTHREAD:			SwapEndian(to.value.gc);	break;
		case LUA_TWSTRING:			SwapEndian(to.value.gc);	break;
	}
}


void ReadMem_LuaObject(void* pHelper, void* fromIn, LuaObject& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem(pHelper, from + _offsetof(LuaObject, m_next), &to.m_next, sizeof(to.m_next));
	ReadMem(pHelper, from + _offsetof(LuaObject, m_prev), &to.m_prev, sizeof(to.m_prev));
	ReadMem(pHelper, from + _offsetof(LuaObject, m_state), &to.m_state, sizeof(to.m_state));
	ReadMem_TValue(pHelper, from + _offsetof(LuaObject, m_object), to.m_object);
	SwapEndian(to.m_next);
	SwapEndian(to.m_prev);
	SwapEndian(to.m_state);
}


void ReadMem_LuaStackObject(void* pHelper, void* fromIn, LuaStackObject& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem(pHelper, from, &to, sizeof(to));
	SwapEndian(to.m_state);
	SwapEndian(to.m_stackIndex);
}


void ReadMem_Node(void* pHelper, void* fromIn, Node& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem_TValue(pHelper, from + _offsetof(Node, i_val), to.i_val);
	ReadMem_TValue(pHelper, from + _offsetof(Node, i_key.nk.value), (TValue&)to.i_key.nk.value);
	ReadMem(pHelper, from + _offsetof(Node, i_key.nk.tt), &to.i_key.nk.next, sizeof(to.i_key.nk.tt));
	ReadMem(pHelper, from + _offsetof(Node, i_key.nk.next), &to.i_key.nk.next, sizeof(to.i_key.nk.next));
	ReadMem_TValue(pHelper, from + _offsetof(Node, i_key.tvk), (TValue&)to.i_key.tvk);
	SwapEndian(to.i_key.nk.tt);
	SwapEndian(to.i_key.nk.next);
}


void ReadMem_TString(void* pHelper, void* fromIn, TString& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem(pHelper, from, &to, sizeof(to));
	SwapEndian(to.tsv.next);
	SwapEndian(to.tsv.tt);
	SwapEndian(to.tsv.marked);
	SwapEndian(to.tsv.reserved);
	SwapEndian(to.tsv.hash);
	SwapEndian(to.tsv.len);
}


void ReadMem_GCObject(void* pHelper, void* fromIn, GCObject& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem(pHelper, from, &to, sizeof(to));
	SwapEndian(to.gch.next);
	SwapEndian(to.gch.tt);
	SwapEndian(to.gch.marked);
}


/*
** for some types, it is better to avoid modulus by power of 2, as
** they tend to have many 2 factors.
*/
#define hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1)|1))))


#define hashpointer(t,p)	hashmod(t, IntPoint(p))


#define hashpow2(t,n)      (gnode(t, lmod((n), sizenode(t))))

#define hashstr(t,str)  hashpow2(t, (str)->tsv.hash)
#define hashboolean(t,p)        hashpow2(t, p)


/*
** number of ints inside a lua_Number
*/
#define numints		cast_int(sizeof(lua_Number)/sizeof(int))



/*
** hash for lua_Numbers
*/
static Node *hashnum (const Table *t, lua_Number n) {
  unsigned int a[numints];
  int i;
  n += 1;  /* normalize number (avoid -0) */
//  lua_assert(sizeof(a) <= sizeof(n));
  memcpy(a, &n, sizeof(a));
  for (i = 1; i < numints; i++) a[0] += a[i];
  return hashmod(t, a[0]);
}


/*
** returns the `main' position of an element in a table (that is, the index
** of its hash value)
*/
static Node *mainposition (void* pHelper, const Table *t, const TValue *key) {
  switch (ttype(key)) {
    case LUA_TNUMBER:
      return hashnum(t, nvalue(key));
	case LUA_TSTRING: {
      TString data;
      ReadMem_TString(pHelper, rawtsvalue(key), data);
      return hashpow2(t, data.tsv.hash); //hashstr(t, &data);
	}
	case LUA_TWSTRING: {
      TString data;
      ReadMem_TString(pHelper, rawtwsvalue(key), data);
      return hashpow2(t, data.tsv.hash); //hashstr(t, &data);
	}
    case LUA_TBOOLEAN:
      return hashboolean(t, bvalue(key));
    case LUA_TLIGHTUSERDATA:
      return hashpointer(t, pvalue(key));
    default:
      return hashpointer(t, gcvalue(key));
  }
}




/*
** search function for integers
*/
const TValue *luaH_getnum (void* pHelper, Table *t, int key)
{
	Table tBuf;
	ReadMem_Table(pHelper, t, tBuf);

    if (cast(unsigned int, key-1) < cast(unsigned int, tBuf.sizearray))
	if (1 <= key && key <= tBuf.sizearray)
	{
	    return &tBuf.array[key-1];
	}
	else
	{
		lua_Number nk = cast_num(key);
		Node *n = hashnum(&tBuf, nk);
		Node nBuf;
		ReadMem_Node(pHelper, n, nBuf);

		do {  /* check whether `key' is somewhere in the chain */
			if (ttisnumber(gkey(&nBuf)) && luai_numeq(nvalue(gkey(&nBuf)), nk))
				return gval(&nBuf);  /* that's it */
			else
			{
				n = gnext(&nBuf);
				if (n)
					ReadMem_Node(pHelper, n, nBuf);
			}
		} while (n);

		return luaO_nilobject;
	}

	return luaO_nilobject;
}


CStringA ReadTStringToCString(void* pHelper, TString* str)
{
	TString data;
	ReadMem_TString(pHelper, str, data);

	CStringA out;
	LPSTR buf = out.GetBufferSetLength((int)data.tsv.len);
	ReadMem(pHelper, str + 1, buf, data.tsv.len);
	out.ReleaseBuffer();

	return out;
}


TString* ReadTString(void* pHelper, TString* str)
{
	TString data;
	ReadMem_TString(pHelper, str, data);

	TString* outStr = (TString*)new BYTE[sizeof(TString) + data.tsv.len + 1];
	memcpy(outStr, &data, sizeof(TString));
	ReadMem(pHelper, str + 1, outStr + 1, data.tsv.len);
	char* buf = (char*)(outStr + 1);
	buf[data.tsv.len] = 0;

	return outStr;
}


/*
** search function for strings
*/
const TValue *luaH_getstr (void* pHelper, Table *t, TString *key, bool localString)
{
	Table tBuf;
	ReadMem_Table(pHelper, t, tBuf);
	t = &tBuf;

	bool mustDeleteKey = false;
	if (!localString)
	{
		key = ReadTString(pHelper, key);
		mustDeleteKey = true;
	}

	Node *n = hashstr(t, key);
	Node nBuf;
	ReadMem_Node(pHelper, n, nBuf);

	do
	{  /* check whether `key' is somewhere in the chain */
		if (ttisstring(gkey(&nBuf)))
		{
			CStringA destKeyStr = ReadTStringToCString(pHelper, &key2tval(&nBuf)->value.gc->ts);
			if (destKeyStr == (char*)(key + 1))
				return gval(n); /* that's it */
			goto DoNext;
		}
		else
		{
DoNext:
			n = gnext(&nBuf);
			if (n)
				ReadMem_Node(pHelper, n, nBuf);
		}
	} while (n);
	return luaO_nilobject;
}

/*
** main search function
*/
const TValue* luaH_get (void* pHelper, Table *t, const TValue *key, bool localString)
{
	switch (key->tt)
	{
	    case LUA_TNIL: return luaO_nilobject;
		case LUA_TSTRING: return luaH_getstr(pHelper, t, rawtsvalue(key), localString);
		case LUA_TWSTRING: return luaH_getstr(pHelper, t, rawtsvalue(key), localString);
		case LUA_TNUMBER:
		{
			int k;
			lua_Number n = nvalue(key);
			lua_number2int(k, n);
			if (luai_numeq(cast_num(k), nvalue(key))) /* index is int? */
				return luaH_getnum(pHelper, t, k);  /* use specialized version */
			/* else go through */
		}
		default:
		{
			Node *n = mainposition(pHelper, t, key);
			do
			{  /* check whether `key' is somewhere in the chain */
				Node nBuf;
				ReadMem_Node(pHelper, n, nBuf);
				if (luaO_rawequalObj(key2tval(&nBuf), key))
					return gval(n);  /* that's it */
				else n = gnext(&nBuf);
			} while (n);
			return luaO_nilobject;
		}
	}
}

Table* getmetatable(void* pHelper, const TValue *o)
{
	global_State gs;
	GCObject gcobj;
	ReadMem_GCObject(pHelper, o->value.gc, gcobj);
	ReadMem(pHelper, theState.l_G, &gs, sizeof(global_State));
	switch (ttype(o))
	{
		case LUA_TTABLE:
		{
			SwapEndian(gcobj.h.metatable);
//			SwapEndian(gs._defaultmetatypes[LUA_TTABLE].value.gc->h);
			if (gs.mt[LUA_TTABLE] == gcobj.h.metatable)
				return NULL;
			return gcobj.h.metatable;
		}
		case LUA_TUSERDATA:
		{
			SwapEndian(gcobj.u.uv.metatable);
//			SwapEndian(gs._defaultmetatypes[LUA_TUSERDATA].value.gc->h);
			if (gs.mt[LUA_TUSERDATA] == gcobj.u.uv.metatable)
				return NULL;
			return gcobj.u.uv.metatable;
		}
		default:
			return NULL;
	}
}

void acquiremetatable(Table* in, TValue* out)
{
	out->tt = LUA_TTABLE;
	out->value.p = in;
}

#define LUA_IDSIZE	60

struct lua_Debug {
  const char *event;     /* `call', `return' */
  const char *name;      /* (n) */
  const char *namewhat;  /* (n) `global', `tag method', `local', `field' */
  const char *what;      /* (S) `Lua' function, `C' function, Lua `main' */
  const char *source;    /* (S) */
  int currentline;       /* (l) */
  int nups;              /* (u) number of upvalues */
  int linedefined;       /* (S) */
  char short_src[LUA_IDSIZE]; /* (S) */
  /* private part */
  int _ci;  /* active function */
};


int lua_getstack (lua_State *L, int level, lua_Debug *ar) {
  int status;
  if (L->ci - L->base_ci <= level) status = 0;  /* there is no such level */
  else {
    ar->_ci = (L->ci - L->base_ci) - level;
    status = 1;
  }
  return status;
}



static void infoLproto (lua_Debug *ar, Proto *f) {
//jj  ar->source = getstr(f->source);
  ar->source = "Unsupported";
  ar->linedefined = f->linedefined;
  ar->what = "Lua";
}


void ReadMem_Closure(void* pHelper, void* fromIn, Closure& to)
{
	BYTE* from = (BYTE*)fromIn;
	ReadMem(pHelper, from, &to, sizeof(to));
}



static void funcinfo (void* pHelper, lua_State *L, lua_Debug *ar, StkId func) {
  Closure clBuf;
  Closure *cl;
  TValue funcBuf;
  ReadMem_TValue(pHelper, func, funcBuf);

  if (ttype(&funcBuf) == LUA_TFUNCTION)
    cl = clvalue(&funcBuf);
  else {
//jj    luaD_error(L, "value for `lua_getinfo' is not a function");
    cl = NULL;  /* to avoid warnings */
  }

  ReadMem_Closure(pHelper, cl, clBuf);
  cl = &clBuf;				// Reassign.

  if (cl->c.isC) {
    ar->source = "=C";
    ar->linedefined = -1;
    ar->what = "C";
  }
//jj  else
//jj    infoLproto(ar, cl->l.p);
//jj  luaO_chunkid(ar->short_src, ar->source, LUA_IDSIZE);
  if (ar->linedefined == 0)
    ar->what = "main";
}


static int isLmark (void* pHelper, CallInfo *ci)
{
//	return (ttype(ci->base - 1) == LUA_TFUNCTION && !ci_func(ci)->c.isC);
	CallInfo ciBuf;
	ReadMem(pHelper, ci, &ciBuf, sizeof(CallInfo));

	TValue objBuf;
	ReadMem(pHelper, ciBuf.base - 1, &objBuf, sizeof(TValue));

	if (ttype(&objBuf) != LUA_TFUNCTION)
		return 0;

	Closure clBuf;
	ReadMem(pHelper, clvalue(&objBuf), &clBuf, sizeof(Closure));

	return (!clBuf.c.isC);
}


#define pcRel(pc, p)	(cast(int, (pc) - (p)->code) - 1)

static int currentpc (void* pHelper, lua_State *L, CallInfo *ci) {
//  if (!isLua(ci)) return -1;  /* function is not a Lua function? */
  CallInfo ciBuf;
  ReadMem(pHelper, ci, &ciBuf, sizeof(CallInfo));
  ci = &ciBuf;
  if (!isLua(&ciBuf)) return -1;  /* function is not a Lua function? */
//  if (ciBuf.pc == NULL) return 0;  /* function is not active */
  if (ci == L->ci)
    ci->savedpc = L->savedpc;
  return pcRel(ci->savedpc, ci_func(ci)->l.p);

//  if (ci == L->ci || ci->pc != (ci+1)->pc)  /* no other function using `pc'? */
//    return (*ci->pc - ci_func(ci)->l.p->code) - 1;
//  else  /* function's pc is saved */
//    return (ci->savedpc - ci_func(ci)->l.p->code) - 1;
#if 0
  CallInfo ci1Buf;
  ReadMem(pHelper, ci + 1, &ci1Buf, sizeof(CallInfo));

  // ci_func(ci)->l.p
  TObject toCiBase_1Buf;
  ReadMem(pHelper, ciBuf.base - 1, &toCiBase_1Buf, sizeof(TObject));

  Closure clBuf;
  ReadMem(pHelper, clvalue(&toCiBase_1Buf), &clBuf, sizeof(Closure));

  Proto protoBuf;
  ReadMem(pHelper, clBuf.l.p, &protoBuf, sizeof(Proto));

  if (ci == L->ci || ciBuf.pc != ci1Buf.pc)  /* no other function using `pc'? */
  {
	  Instruction* ciBufpc;
	  ReadMem(pHelper, ciBuf.pc, &ciBufpc, sizeof(Instruction*));

	  return (ciBufpc - protoBuf.code) - 1;
  }
  else  /* function's pc is saved */
    return (ciBuf.savedpc - protoBuf.code) - 1;
#endif 0
}


static int currentline (CallInfo *ci) {
#if 0
  int pc = currentpc(ci);
  if (pc < 0)
    return -1;  /* only active lua functions have current-line information */
  else
    return getline(ci_func(ci)->l.p, pc);
#endif
  return 0;
}


#if 0
static int currentline (void* pHelper, lua_State *L, CallInfo *ci)
{
  if (!isLmark(ci))
    return -1;  /* only active lua functions have current-line information */
  else {
	CallInfo ciBuf;
	ReadMem(pHelper, ci, &ciBuf, sizeof(CallInfo));

	TObject obj;
	ReadMem(pHelper, ciBuf.base - 1, &obj, sizeof(TObject));

//    int *lineinfo = ci_func(&ciBuf)->l.p->lineinfo;
	return 0;
//jj    return luaG_getline(lineinfo, currentpc(L, ci), 1, NULL);
  }
}
#endif 0

#if 0
static void funcinfo (lua_Debug *ar, StkId func) {
  Closure *cl = clvalue(func);
  Closure clBuf;
  ReadMem(pHelper, cl, &clBuf, sizeof(Closure));
  cl = &clBuf;

  if (cl->c.isC) {
    ar->source = "=[C]";
    ar->linedefined = -1;
    ar->what = "C";
  }
  else {
    ar->source = getstr(cl->l.p->source);
    ar->linedefined = cl->l.p->lineDefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, LUA_IDSIZE);
}


static int auxgetinfo (lua_State *L, const char *what, lua_Debug *ar,
                    StkId f, CallInfo *ci) {
  int status = 1;
  for (; *what; what++) {
    switch (*what) {
      case 'S': {
        funcinfo(ar, f);
        break;
      }
      case 'l': {
        ar->currentline = (ci) ? currentline(ci) : -1;
        break;
      }
      case 'u': {
        ar->nups = clvalue(f)->c.nupvalues;
        break;
      }
      case 'n': {
        ar->namewhat = (ci) ? getfuncname(ci, &ar->name) : NULL;
        if (ar->namewhat == NULL) {
          /* try to find a global name */
          if ((ar->name = travglobals(L, f)) != NULL)
            ar->namewhat = "global";
          else ar->namewhat = "";  /* not found */
        }
        break;
      }
      case 'f': {
        setobj2s(L->top, f);
        break;
      }
      default: status = 0;  /* invalid option */
    }
  }
  return status;
}


int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar) {
  int status = 1;

  lua_Debug arBuf;
  ReadMem(pHelper, ar, &arBuf, sizeof(lua_Debug));
  ar = &arBuf;

  if (*what == '>') {
    StkId f = L->top - 1;

	TObject obj;
	ReadMem(pHelper, f, &obj, sizeof(TObject));
	f = &obj;

    if (!ttisfunction(f))
	  throw -1;

    status = auxgetinfo(L, what + 1, ar, f, NULL);
    L->top--;  /* pop function */
  }
  else if (ar->i_ci != 0) {  /* no tail call? */
    CallInfo *ci = L->base_ci + ar->i_ci;
    lua_assert(ttisfunction(ci->base - 1));
    status = auxgetinfo(L, what, ar, ci->base - 1, ci);
  }
  else
    info_tailcall(L, ar);
  if (strchr(what, 'f')) incr_top(L);
  return status;
}

#endif 0


static Proto *getluaproto (void* pHelper, CallInfo *ci) {
	CallInfo ciBuf;
	ReadMem(pHelper, ci, &ciBuf, sizeof(CallInfo));

	TValue objBuf;
	ReadMem(pHelper, ciBuf.base - 1, &objBuf, sizeof(TValue));

	Closure clBuf;
	ReadMem(pHelper, clvalue(&objBuf), &clBuf, sizeof(Closure));

  return (isLmark(pHelper, ci) ? clBuf.l.p : NULL);
}


#define getstr(ts)	cast(const char *, (ts) + 1)


/*
** Look for n-th local variable at line `line' in function `func'.
** Returns NULL if not found.
*/
bool luaF_getlocalname(void* pHelper, const Proto *f, int local_number, int pc, CStringA& name)
{
	int i;
	Proto fBuf;
	ReadMem(pHelper, f, &fBuf, sizeof(Proto));
	f = &fBuf;			// Reassign

	LocVar locVarBuf;

	for (i = 0; i < f->sizelocvars; i++)
	{
		ReadMem(pHelper, &f->locvars[i], &locVarBuf, sizeof(LocVar));
		if (locVarBuf.startpc > pc)
			break;
		if (pc < locVarBuf.endpc) {  /* is variable active? */
			local_number--;
			if (local_number == 0)
			{
				TString tStr;
				ReadMem(pHelper, locVarBuf.varname, &tStr, sizeof(TString));

				LPSTR buf = name.GetBufferSetLength(tStr.tsv.len);
				ReadMem(pHelper, locVarBuf.varname + 1, buf, tStr.tsv.len);
				name.ReleaseBuffer();
				return true;
			}
		}
	}
	return false;  /* not found */
}

#if 0
static Proto *getluaproto (CallInfo *ci) {
  return (isLua(ci) ? ci_func(ci)->l.p : NULL);
}


const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n, TObject& obj) {
  const char *name;
  CallInfo *ci;
  Proto *fp;

  lua_Debug arBuf;
  ReadMem(pHelper, ar, &arBuf, sizeof(lua_Debug));
  ar = &arBuf;

  name = NULL;
  ci = L->base_ci + ar->i_ci;
  fp = getluaproto(ci);
  if (fp) {  /* is a Lua function? */
    name = luaF_getlocalname(fp, n, currentpc(ci));
    if (name)
      ReadMem(pHelper, ci->base+(n-1), &obj, sizeof(TObject));
  }
  return name;
}
#endif 0

