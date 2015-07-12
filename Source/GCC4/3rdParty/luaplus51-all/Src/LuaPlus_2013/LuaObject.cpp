///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define BUILDING_LUAPLUS
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#define LUA_CORE
#include "src/lfunc.h"
#include "src/lobject.h"
#include "src/lstate.h"
#include "src/lstring.h"
#include "src/ltable.h"
#include "src/lvm.h"
LUA_EXTERN_C_END

#include "LuaPlus.h"
#include <string.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

NAMESPACE_LUA_BEGIN
LUA_EXTERN_C TValue *index2adr (lua_State *L, int idx);
LUA_EXTERN_C Table *getcurrenv (lua_State *L);
NAMESPACE_LUA_END

USING_NAMESPACE_LUA

// From the Loki reference implementation
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

#define lua_addreftobject(obj)

#define setnvalue2n(obj,x) \
  { TValue *i_o=(obj); i_o->tt=LUA_TNUMBER; i_o->value.n=(x); }

#define setpvalue2n(L,obj,x) \
  { TValue *i_o=(obj); i_o->tt=LUA_TLIGHTUSERDATA; i_o->value.p=(x); }

#define setbvalue2n(obj,x) \
  { TValue *i_o=(obj); i_o->tt=LUA_TBOOLEAN; i_o->value.b=(x); }

#define setuvalue2n(L,obj,x) \
  { TValue *i_o=(obj); i_o->tt=LUA_TUSERDATA; \
    i_o->value.gc=cast(GCObject *, (x)); lua_addreftobject(i_o); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TUSERDATA); }

#define setthvalue2n(L,obj,x) \
  { TValue *i_o=(obj); i_o->tt=LUA_TTHREAD; \
    i_o->value.gc=cast(GCObject *, (x)); lua_addreftobject(i_o); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TTHREAD); }

#define setclvalue2n(L,obj,x) \
  { TValue *i_o=(obj); i_o->tt=LUA_TFUNCTION; \
    i_o->value.gc=cast(GCObject *, (x)); lua_addreftobject(i_o); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TFUNCTION); }

#define sethvalue2n(L,obj,x) \
  { TValue *i_o=(obj); i_o->tt=LUA_TTABLE; \
    i_o->value.gc=cast(GCObject *, (x)); lua_addreftobject(i_o); \
    lua_assert(i_o->value.gc->gch.tt == LUA_TTABLE); }

////////////////////////////////////////////////////////////////////////////////
// macro STATIC_CHECK
// Invocation: STATIC_CHECK(expr, id)
// where:
// expr is a compile-time integral or pointer expression
// id is a C++ identifier that does not need to be defined
// If expr is zero, id will appear in a compile-time error message.
////////////////////////////////////////////////////////////////////////////////
#define STATIC_CHECK(expr, msg) \
    { CompileTimeError<((expr) != 0)> ERROR_##msg; (void)ERROR_##msg; }

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

static size_t lua_WChar_len(const lua_WChar* str)
{
	const lua_WChar* ptr = str;
	while (*ptr != 0)
	{
		ptr++;
	}

	return (size_t)(ptr - str);
}


void lua_pushtobject(lua_State *L, void* tobject);

#undef api_check
#define api_check(L, o)		luaplus_assert(o)

LuaObject::LuaObject() :
	m_next(NULL),
	m_prev(NULL),
	m_state(NULL)
{
	setnilvalue2n(L, &m_object);
}


LuaObject::LuaObject(LuaState* state) throw()
{
	lua_State* L = state->m_state;  (void)L;
	AddToUsedList(state);
	setnilvalue2n(L, &m_object);
}


LuaObject::LuaObject(LuaState* state, int stackIndex) throw()
{
	setnilvalue2n(L, &m_object);
	AddToUsedList(state, *index2adr(state->m_state, stackIndex));
}


LuaObject::LuaObject(LuaState* state, const TValue* obj)
{
	luaplus_assert(obj);
	setnilvalue2n(L, &m_object);
	AddToUsedList(state, *obj);
}


LuaObject::LuaObject(const LuaObject& src) throw()
{
	setnilvalue2n(L, &m_object);
	AddToUsedList(src.m_state, src.m_object);
}


LuaObject::LuaObject(const LuaStackObject& src) throw()
{
	setnilvalue2n(L, &m_object);
	AddToUsedList(src.m_state, *index2adr(src.m_state->m_state, src.m_stackIndex));
}


LuaObject& LuaObject::operator=(const LuaObject& src) throw()
{
	RemoveFromUsedList();
	AddToUsedList(src.m_state, src.m_object);
	return *this;
}


LuaObject& LuaObject::operator=(const LuaStackObject& src) throw()
{
	RemoveFromUsedList();
	AddToUsedList(src.m_state, *index2adr(src.m_state->m_state, src.m_stackIndex));
	return *this;
}


LuaObject::~LuaObject()
{
	RemoveFromUsedList();
}


/**
	Resets the LuaObject by removing itself from the used GC list and setting the state to NULL.
**/
void LuaObject::Reset()
{
	RemoveFromUsedList();
	m_state = NULL;
}


// Mirrors lua_typename().
const char* LuaObject::TypeName() const
{
	int t = Type();
	return (t == LUA_TNONE) ? "no value" : luaT_typenames[t];
}


// Mirrors lua_type().
int LuaObject::Type() const
{
	return ttype(&m_object);
}


// Mirrors lua_isnil().
bool LuaObject::IsNil() const
{
	return ttype(&m_object) == LUA_TNIL;
}


// Mirrors lua_istable().
bool LuaObject::IsTable() const
{
	return ttype(&m_object) == LUA_TTABLE;
}


// Mirrors lua_isuserdata().
bool LuaObject::IsUserData() const
{
	return ttisuserdata(&m_object)  ||  ttislightuserdata(&m_object);
}


// Mirrors lua_iscfunction().
bool LuaObject::IsCFunction() const
{
	return iscfunction(&m_object);
}


// Behaves differently than lua_isinteger().  This function only tests for a value that is
// a real integer, not something that can be converted to a integer.
bool LuaObject::IsInteger() const
{
	return ttype(&m_object) == LUA_TNUMBER;
}


// Behaves differently than lua_isnumber().  This function only tests for a value that is
// a real number, not something that can be converted to a number.
bool LuaObject::IsNumber() const
{
	return ttype(&m_object) == LUA_TNUMBER;
}


// Behaves differently than lua_isstring().  This function only tests for a value that is
// a real string, not something that can be converted to a string.
bool LuaObject::IsString() const
{
	return ttype(&m_object) == LUA_TSTRING;
}


bool LuaObject::IsWString() const
{
	return ttype(&m_object) == LUA_TWSTRING;
}


// Mirrors lua_isinteger().
bool LuaObject::IsConvertibleToInteger() const
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();  (void)L;
	const TValue* o = &m_object;
	TValue n;
	setnilvalue2n(L, &n);
    lua_lock(L);
	bool ret = tonumber(o, &n);
    lua_unlock(L);
    return ret;
}


// Mirrors lua_isnumber().
bool LuaObject::IsConvertibleToNumber() const
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();  (void)L;
	const TValue* o = &m_object;
	TValue n;
	setnilvalue2n(L, &n);
    lua_lock(L);
	bool ret = tonumber(o, &n);
    lua_unlock(L);
    return ret;
}


// Mirrors lua_isstring().
bool LuaObject::IsConvertibleToString() const
{
	int t = Type();
	return (t == LUA_TSTRING || t == LUA_TNUMBER);
}


// Mirrors lua_iswstring().
bool LuaObject::IsConvertibleToWString() const
{
	int t = Type();
	return (t == LUA_TNUMBER || t == LUA_TWSTRING);
}


// Mirrors lua_isfunction().
bool LuaObject::IsFunction() const
{
	return ttype(&m_object) == LUA_TFUNCTION;
}


// Mirrors lua_isnone().
bool LuaObject::IsNone() const
{
	return ttype(&m_object) == LUA_TNONE;
}


// Mirrors lua_islightuserdata().
bool LuaObject::IsLightUserData() const
{
	return ttype(&m_object) == LUA_TLIGHTUSERDATA;
}


// Mirrors lua_isboolean().
bool LuaObject::IsBoolean() const
{
	return ttype(&m_object) == LUA_TBOOLEAN;
}


// Mirrors lua_tointeger()
int LuaObject::ToInteger()
{
	lua_State* L = GetCState();  (void)L;
	const TValue* o = &m_object;
	TValue n;
    lua_lock(L);
	bool ret = tonumber(o, &n);
    lua_unlock(L);
    if (ret)
		return (int)nvalue(&m_object);
	else
		return 0;
}


// Mirrors lua_tonumber()
lua_Number LuaObject::ToNumber()
{
	lua_State* L = GetCState();  (void)L;
	const TValue* o = &m_object;
	TValue n;
    lua_lock(L);
	bool ret = tonumber(o, &n);
    lua_unlock(L);
	if (ret)
		return nvalue(&m_object);
	else
		return 0;
}


// Mirrors lua_tostring().
const char* LuaObject::ToString()
{
	if (ttisstring(&m_object))
		return svalue(&m_object);
	else
	{
		const char *s;
		lua_lock(m_state->m_state);  /* `luaV_tostring' may create a new string */
		s = (luaV_tostring(m_state->m_state, &m_object) ? svalue(&m_object) : NULL);
		lua_unlock(m_state->m_state);
		return s;
	}
}


// Mirrors lua_towstring().
const lua_WChar* LuaObject::ToWString()
{
	if (ttiswstring(&m_object))
		return wsvalue(&m_object);
	else
	{
		const lua_WChar *s;
		lua_lock(m_state->m_state);  /* `luaV_tostring' may create a new string */
		s = (luaV_towstring(m_state->m_state, &m_object) ? wsvalue(&m_object) : NULL);
		lua_unlock(m_state->m_state);
		return s;
	}
}


size_t LuaObject::ToStrLen()
{
	if (ttisstring(&m_object)  ||  ttiswstring(&m_object))
		return tsvalue(&m_object)->len;
	else
	{
		size_t l;
		lua_lock(m_state->m_state);  /* `luaV_tostring' may create a new string */
		l = (luaV_tostring(m_state->m_state, &m_object) ? tsvalue(&m_object)->len : 0);
		lua_unlock(m_state->m_state);
		return l;
	}
}


int LuaObject::GetInteger() const
{
	luaplus_assert(m_state  &&  IsInteger());
	return (int)(unsigned int)nvalue(&m_object);
}


float LuaObject::GetFloat() const
{
	luaplus_assert(m_state  &&  IsNumber());
	return (float)nvalue(&m_object);
}


double LuaObject::GetDouble() const
{
	luaplus_assert(m_state  &&  IsNumber());
	return (double)nvalue(&m_object);
}


lua_Number LuaObject::GetNumber() const
{
	luaplus_assert(m_state  &&  IsNumber());
	return (lua_Number)nvalue(&m_object);
}


const char* LuaObject::GetString() const
{
	luaplus_assert(m_state  &&  IsString());
	return svalue(&m_object);
}


const lua_WChar* LuaObject::GetWString()const
{
	luaplus_assert(m_state  &&  IsWString());
	return wsvalue(&m_object);
}


int LuaObject::StrLen()
{
	luaplus_assert(m_state);
	if (IsString()  ||  IsWString())
	{
		return tsvalue(&m_object)->len;
	}
	else if (IsUserData())
	{
		return uvalue(&m_object)->len;
	}
	else
	{
		luaplus_assert(0);
		return 0;
	}
}


NAMESPACE_LUA_PREFIX lua_CFunction LuaObject::GetCFunction() const
{
	luaplus_assert(m_state  &&  IsCFunction());
	return (!iscfunction(&m_object)) ? NULL : clvalue(&m_object)->c.f;
}


// Mirrors lua_touserdata().
void* LuaObject::GetUserData()
{
	luaplus_assert(m_state  &&  IsUserData());

	StkId o = &m_object;
	switch (ttype(o))
	{
		case LUA_TUSERDATA:
		{
			if (uvalue(o)->len & 1)
				return *(void**)(rawuvalue(o) + 1);
			else
				return (rawuvalue(o) + 1);
		}
		case LUA_TLIGHTUSERDATA: return pvalue(o);
		default: return NULL;
	}
}


// Mirrors lua_topointer.
const void* LuaObject::GetLuaPointer()
{
	luaplus_assert(m_state);
	StkId o = &m_object;
	switch (ttype(o))
	{
		case LUA_TTABLE: return hvalue(o);
		case LUA_TFUNCTION: return clvalue(o);
		case LUA_TUSERDATA:
		case LUA_TLIGHTUSERDATA:
			return GetUserData();
		default: return NULL;
	}
}


// No equivalent.
void* LuaObject::GetLightUserData() const
{
	luaplus_assert(m_state  &&  IsLightUserData());
	return pvalue(&m_object);
}


// Mirrors lua_toboolean().
bool LuaObject::GetBoolean() const
{
	luaplus_assert(m_state  &&  IsBoolean()  ||  IsNil());
	return !l_isfalse(&m_object);
}


/**
**/
LuaObject LuaObject::Clone()
{
	lua_State* L = GetCState();  (void)L;
    lua_lock(L);
	if (IsTable())
	{
		LuaObject tableObj(m_state);
		sethvalue(L, &tableObj.m_object, luaH_new(m_state->m_state, hvalue(&m_object)->sizearray, hvalue(&m_object)->lsizenode));
		tableObj.SetMetaTable(GetMetaTable());

		for (LuaTableIterator it(*this); it; ++it)
		{
			if (it.GetValue().IsTable())
			{
				LuaObject clonedChildTableObj = it.GetValue().Clone();
				tableObj.SetObject(it.GetKey(), clonedChildTableObj);
			}
			else
				tableObj.SetObject(it.GetKey(), it.GetValue());
		}

		return tableObj;
	}

    lua_unlock(L);

	return LuaObject(m_state, &m_object);
}


void LuaObject::DeepClone(LuaObject& outObj)
{
	if (IsTable())
	{
		outObj.AssignNewTable(outObj.GetState());

		for (LuaTableIterator it(*this); it; ++it)
		{
			LuaObject keyObj;
			switch (it.GetKey().Type())
			{
				case LUA_TBOOLEAN:	keyObj.AssignBoolean(outObj.GetState(), it.GetKey().GetBoolean());		break;
				case LUA_TNUMBER:		keyObj.AssignNumber(outObj.GetState(), it.GetKey().GetNumber());		break;
				case LUA_TSTRING:		keyObj.AssignString(outObj.GetState(), it.GetKey().GetString());		break;
				case LUA_TWSTRING:	keyObj.AssignWString(outObj.GetState(), it.GetKey().GetWString());		break;
			}

			switch (it.GetValue().Type())
			{
				case LUA_TBOOLEAN:	outObj.SetBoolean(keyObj, it.GetValue().GetBoolean());		break;
				case LUA_TNUMBER:		outObj.SetNumber(keyObj, it.GetValue().GetNumber());		break;
				case LUA_TSTRING:		outObj.SetString(keyObj, it.GetValue().GetString());		break;
				case LUA_TWSTRING:	outObj.SetWString(keyObj, it.GetValue().GetWString());		break;
				case LUA_TTABLE:
				{
					LuaObject newTableObj(outObj.GetState());
					it.GetValue().DeepClone(newTableObj);
					outObj.SetObject(keyObj, newTableObj);
					break;
				}
			}
		}
	}
	else
	{
		switch (Type())
		{
			case LUA_TBOOLEAN:	outObj.AssignBoolean(outObj.GetState(), this->GetBoolean());		break;
			case LUA_TNUMBER:	outObj.AssignNumber(outObj.GetState(), this->GetNumber());			break;
			case LUA_TSTRING:	outObj.AssignString(outObj.GetState(), this->GetString());			break;
			case LUA_TWSTRING:	outObj.AssignWString(outObj.GetState(), this->GetWString());		break;
			case LUA_TTABLE:	DeepClone(outObj);													break;
		}
	}
}

	
static inline int InternalGetTop(LuaState* state)
{
  return (state->GetCState()->top - state->GetCState()->base);
}


#define api_incr_top(L) \
	{if (L->top >= L->ci->top) lua_checkstack(L, 1); L->top++;}

static inline void InternalPushTObject(lua_State *L, const void* tobject)
{
	TValue* tobj = (TValue*)tobject;
	lua_lock(L);
	setobj2s(L, L->top, tobj);
	api_incr_top(L);
	lua_unlock(L);
}


LuaStackObject LuaObject::Push() const
{
	luaplus_assert(m_state);
	InternalPushTObject(m_state->m_state, &m_object);
	return LuaStackObject(m_state, InternalGetTop(m_state));
}


LuaObject LuaObject::GetMetaTable()
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();  (void)L;
    lua_lock(L);

	Table *mt = NULL;
//	int res;
	switch (ttype(&m_object))
	{
		case LUA_TTABLE:
			mt = hvalue(&m_object)->metatable;
			break;
		case LUA_TUSERDATA:
			mt = uvalue(&m_object)->metatable;
			break;
		default:
			mt = G(L)->mt[ttype(&m_object)];
			break;
	}

	LuaObject ret(m_state);
	if (mt)
	{
		sethvalue(L, &ret.m_object, mt);
	}

    lua_unlock(L);
	return ret;
}


void LuaObject::SetMetaTable(const LuaObject& valueObj)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();  (void)L;
    lua_lock(L);

	TValue* obj = &m_object;
	Table* mt = valueObj.IsTable() ? hvalue(&valueObj.m_object) : NULL;
	switch (ttype(obj))
	{
		case LUA_TTABLE:
		{
			hvalue(obj)->metatable = mt;
			if (mt)
				luaC_objbarriert(L, hvalue(obj), mt);
			break;
		}
		case LUA_TUSERDATA:
		{
			uvalue(obj)->metatable = mt;
			if (mt)
				luaC_objbarrier(L, rawuvalue(obj), mt);
			break;
		}
		default:
		{
			G(L)->mt[ttype(obj)] = mt;
			break;
		}
	}	
	
	//jj	luaT_setmetatable(m_state->m_state, &m_object, hvalue(&valueObj.m_object));
    lua_unlock(L);
}


int LuaObject::GetN()
{
	luaplus_assert(m_state);
	LuaAutoBlock autoBlock(GetCState());
	Push();
	return luaL_getn(m_state->m_state, -1);
}


void LuaObject::SetN(int n)
{
	luaplus_assert(m_state);
	LuaAutoBlock autoBlock(GetCState());
	Push();
	luaL_setn(m_state->m_state, -1, n);
}


void LuaObject::Insert(LuaObject& obj)
{
	luaplus_assert(m_state);
	luaplus_assert(m_state == obj.m_state);
	LuaAutoBlock autoBlock(GetCState());
	LuaObject tableObj = m_state->GetGlobal("table");
	LuaObject funcObj = tableObj["insert"];
	luaplus_assert(funcObj.IsFunction());
    LuaCall callObj(funcObj);
	callObj << *this << obj << LuaRun();
}


void LuaObject::Insert(int index, LuaObject& obj)
{
	luaplus_assert(m_state);
	luaplus_assert(m_state == obj.m_state);
	LuaAutoBlock autoBlock(GetCState());
	LuaObject tableObj = m_state->GetGlobal("table");
	LuaObject funcObj = tableObj["insert"];
	luaplus_assert(funcObj.IsFunction());
    LuaCall callObj(funcObj);
	callObj << *this << index << obj << LuaRun();
}


void LuaObject::Remove(int index)
{
	luaplus_assert(m_state);
	LuaAutoBlock autoBlock(GetCState());
	LuaObject tableObj = m_state->GetGlobal("table");
	LuaObject funcObj = tableObj["remove"];
	luaplus_assert(funcObj.IsFunction());
    LuaCall callObj(funcObj);
	callObj << *this << index << LuaRun();
}


void LuaObject::Sort()
{
	luaplus_assert(m_state);
	LuaAutoBlock autoBlock(GetCState());
	LuaObject tableObj = m_state->GetGlobal("table");
	LuaObject funcObj = tableObj["sort"];
	luaplus_assert(funcObj.IsFunction());
    LuaCall callObj(funcObj);
	callObj << *this << LuaRun();
}


int LuaObject::GetCount()
{
	luaplus_assert(m_state);
	Push();
	int count = lua_objlen(m_state->m_state, -1);
	m_state->Pop();
	return count;
}


int LuaObject::GetTableCount()
{
	int count = 0;
	for (LuaTableIterator it(*this); it; ++it)
	{
		count++;
	}
	return count;
}


/**
	Creates a table called [name] within the current LuaObject.

	@param key The name of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
LuaObject LuaObject::CreateTable(const char* key, int narray, int lnhash)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();  (void)L;
	LuaObject ret(m_state);
	sethvalue2n(L, &ret.m_object, luaH_new(m_state->m_state, narray, lnhash));
	SetTableHelper(key, &ret.m_object);
	return ret;
}


/**
	Creates a table called [key] within the current LuaStackObject.

	@param index The index of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
LuaObject LuaObject::CreateTable(int key, int narray, int lnhash)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();  (void)L;
	LuaObject ret(m_state);
	sethvalue2n(L, &ret.m_object, luaH_new(m_state->m_state, narray, lnhash));
	SetTableHelper(key, &ret.m_object);
	return ret;
}


/**
	Creates a table called [key] within the current LuaStackObject.

	@param index The index of the table to create.
	@param size The size of the table.
	@return Returns the object representing the newly created table.
**/
LuaObject LuaObject::CreateTable(LuaObject& key, int narray, int lnhash)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();  (void)L;
	LuaObject ret(m_state);
	sethvalue2n(L, &ret.m_object, luaH_new(m_state->m_state, narray, lnhash));
	SetTableHelper(key, &ret.m_object);
	return ret;
}


LuaObject LuaObject::GetByName(const char* key)
{
	luaplus_assert(m_state);

	lua_State* L = GetCState();

	api_check(L, ttistable(&m_object));

	TValue str;
	setnilvalue2n(L, &str);

	// It's safe to assume that if name is not in the hash table, this function can return nil.
	size_t l = strlen(key);
	GCObject *o;
	unsigned int h = (unsigned int)l;  /* seed */
	size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
	size_t l1;
	for (l1=l; l1>=step; l1-=step)  /* compute hash */
		h = h ^ ((h<<5)+(h>>2)+(unsigned char)(key[l1-1]));
	for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
		o != NULL;
		o = o->gch.next)
	{
		TString *ts = rawgco2ts(o);
		if (ts->tsv.tt == LUA_TSTRING && ts->tsv.len == l && (memcmp(key, getstr(ts), l) == 0))
		{
			setsvalue2n(L, &str, ts);
			break;
		}
	}

	if (ttype(&str) == LUA_TNIL)
		return LuaObject(m_state);

	TValue v;
	luaV_gettable(GetCState(), &m_object, &str, &v);
	setnilvalue(&str);
	return LuaObject(m_state, &v);
}

LuaObject LuaObject::GetByIndex(int key)
{
	luaplus_assert(m_state);
	api_check(GetCState(), ttistable(&m_object));

	TValue obj;
	setnvalue2n(&obj, key);
	TValue v;
	luaV_gettable(GetCState(), &m_object, &obj, &v);
	return LuaObject(m_state, &v);
}

LuaObject LuaObject::GetByObject(const LuaObject& key)
{
	luaplus_assert(m_state);
	api_check(GetCState(), ttistable(&m_object));

	TValue v;
	luaV_gettable(GetCState(), &m_object, (TValue*)&key.m_object, &v);
	return LuaObject(m_state, &v);
}


LuaObject LuaObject::GetByObject(const LuaStackObject& key)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();
	api_check(L, ttistable(&m_object));

	TValue v;
	luaV_gettable(L, &m_object, index2adr(L, key.m_stackIndex), &v);
	return LuaObject(m_state, &v);
}


LuaObject LuaObject::RawGetByName(const char* key)
{
	return (*this)[key];
}

LuaObject LuaObject::RawGetByIndex(int key)
{
	return (*this)[key];
}

LuaObject LuaObject::RawGetByObject(const LuaObject& key)
{
	return (*this)[key];
}


LuaObject LuaObject::RawGetByObject(const LuaStackObject& key)
{
	return (*this)[key];
}


LuaObject LuaObject::operator[](const char* name)
{
	luaplus_assert(m_state);
	api_check(GetCState(), ttistable(&m_object));

	lua_State* L = GetCState();

	TValue str;
	setnilvalue2n(L, &str);

	// It's safe to assume that if name is not in the hash table, this function can return nil.
	size_t l = strlen(name);
	GCObject *o;
	unsigned int h = (unsigned int)l;  /* seed */
	size_t step = (l>>5)+1;  /* if string is too long, don't hash all its chars */
	size_t l1;
	for (l1=l; l1>=step; l1-=step)  /* compute hash */
		h = h ^ ((h<<5)+(h>>2)+(unsigned char)(name[l1-1]));
	for (o = G(L)->strt.hash[lmod(h, G(L)->strt.size)];
		o != NULL;
		o = o->gch.next)
	{
		TString *ts = rawgco2ts(o);
		if (ts->tsv.tt == LUA_TSTRING && ts->tsv.len == l && (memcmp(name, getstr(ts), l) == 0))
		{
			setsvalue2n(L, &str, ts);
			break;
		}
	}

	if (ttype(&str) == LUA_TNIL)
		return LuaObject(m_state);

//	setsvalue(&str, luaS_newlstr(L, name, strlen(name)));
	const TValue* v = luaH_get(hvalue(&m_object), &str);
	setnilvalue(&str);
	return LuaObject(m_state, v);
}

LuaObject LuaObject::operator[](int index)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();		(void)L;
	api_check(L, ttistable(&m_object));

	StkId o = &m_object;
	api_check(L, ttistable(o));
	const TValue* v = luaH_getnum(hvalue(o), index);
	return LuaObject(m_state, v);
}

LuaObject LuaObject::operator[](const LuaObject& obj)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();		(void)L;
	api_check(L, ttistable(&m_object));

	const TValue* v = luaH_get(hvalue(&m_object), &obj.m_object);
	return LuaObject(m_state, v);
}

LuaObject LuaObject::operator[](const LuaStackObject& obj)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();		(void)L;
	api_check(L, ttistable(&m_object));

	const TValue* v = luaH_get(hvalue(&m_object), index2adr(GetCState(), obj.m_stackIndex));
	return LuaObject(m_state, v);
}


LuaObject LuaObject::Lookup(const char* key)
{
	LuaObject table = *this;

	size_t keyLen = strlen(key);
#if defined(_MSC_VER)
 	char* buf = (char*)_alloca(keyLen + 1);
#else // !_MSC_VER
	char* buf = new char[keyLen + 1];
#endif // _MSC_VER
	strncpy(buf, key, keyLen);
	buf[keyLen] = 0;

	char* lastPos = (char*)buf;

	while (true)
	{
		lua_Number num;

		char* curPos = strchr(lastPos, '.');
		if (!curPos)
		{
			if (luaO_str2d(lastPos, &num))
			{
#if !defined(_MSC_VER)
                delete [] buf;
#endif
				return table[(int)num];
			}

#if !defined(_MSC_VER)
            delete [] buf;
#endif
			return table[lastPos];
		}

		*curPos = 0;
		if (luaO_str2d(lastPos, &num))
		{
			table = table[(int)num];
		}
		else
		{
			table = table[lastPos];
		}

		if (table.IsNil())
        {
#if !defined(_MSC_VER)
            delete [] buf;
#endif
			return table;
        }

		lastPos = curPos + 1;
	}

#if !defined(_MSC_VER)
    delete [] buf;
#endif
	return LuaObject(m_state);
}

namespace detail
{
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, const LuaArgNil&)		{  setnilvalue2n(L, obj);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, bool value)			{  setbvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, char value)			{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, unsigned char value)	{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, short value)			{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, unsigned short value)	{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, int value)			{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, unsigned int value)	{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, float value)			{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* /*L*/, lua_TValue* obj, double value)			{  setnvalue2n(obj, value);  }
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const char* value)		{  setsvalue2n(L, obj, luaS_newlstr(L, value, strlen(value)));  }
//jj	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const lua_WChar* value)	{  setwsvalue2n(L, obj, luaS_newlwstr(L, value, lua_WChar_len(value)));  }
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const LuaObject& value)	{  (void)L;  setobj2n(L, obj, value.GetTObject());  }
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, void* value)				{  (void)L;  setpvalue2n(L, obj, value);  }

	LUAPLUS_API void SetNilValue(lua_State* L, lua_TValue* obj)								{  (void)L;  setnilvalue(obj);  }
}

LuaObject& LuaObject::SetNil(const char* key)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnilvalue2n(L, &valueObj);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetNil(int key)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnilvalue2n(L, &valueObj);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetNil(LuaObject& key)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnilvalue2n(L, &valueObj);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetBoolean(const char* key, bool value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setbvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetBoolean(int key, bool value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setbvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetBoolean(LuaObject& key, bool value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setbvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetInteger(const char* key, int value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetInteger(int key, int value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetInteger(LuaObject& key, int value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetNumber(const char* key, lua_Number value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetNumber(int key, lua_Number value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetNumber(LuaObject& key, lua_Number value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetString(const char* key, const char* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = strlen(value);
		setsvalue2n(L, &valueObj, luaS_newlstr(m_state->m_state, value, len));
	}
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetString(int key, const char* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = strlen(value);
		setsvalue2n(L, &valueObj, luaS_newlstr(m_state->m_state, value, len));
	}
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetString(LuaObject& key, const char* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = strlen(value);
		setsvalue2n(L, &valueObj, luaS_newlstr(m_state->m_state, value, len));
	}
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetWString(const char* key, const lua_WChar* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = lua_WChar_len(value);
		setwsvalue2n(L, &valueObj, luaS_newlwstr(m_state->m_state, value, len));
	}
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetWString(int key, const lua_WChar* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = lua_WChar_len(value);
		setwsvalue2n(L, &valueObj, luaS_newlwstr(m_state->m_state, value, len));
	}
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetWString(LuaObject& key, const lua_WChar* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = lua_WChar_len(value);
		setwsvalue2n(L, &valueObj, luaS_newlwstr(m_state->m_state, value, len));
	}
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetUserData(const char* key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	Udata* u = luaS_newudata(m_state->m_state, 4, getcurrenv(L));
	u->uv.len = 1;  // user data box bit is set.
	*(void**)(u + 1) = value;

	TValue valueObj;
	setuvalue2n(L, &valueObj, u);
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetUserData(int key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	Udata* u = luaS_newudata(m_state->m_state, 4, getcurrenv(L));
	u->uv.len = 1;  // user data box bit is set.
	*(void**)(u + 1) = value;

	TValue valueObj;
	setuvalue2n(L, &valueObj, u);
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetUserData(LuaObject& key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	Udata* u = luaS_newudata(m_state->m_state, 4, getcurrenv(L));
	u->uv.len = 1;  // user data box bit is set.
	*(void**)(u + 1) = value;

	TValue valueObj;
	setuvalue2n(L, &valueObj, u);
	SetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::SetLightUserData(const char* key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setpvalue2n(L, &valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetLightUserData(int key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setpvalue2n(L, &valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetLightUserData(LuaObject& key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setpvalue2n(L, &valueObj, value);
	return SetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::SetObject(const char* key, LuaObject& value)
{
	luaplus_assert(m_state  &&  IsTable());
	luaplus_assert(m_state == value.m_state);
	return SetTableHelper(key, &value.m_object);
}


LuaObject& LuaObject::SetObject(int key, LuaObject& value)
{
	luaplus_assert(m_state  &&  IsTable());
	luaplus_assert(m_state == value.m_state);
	return SetTableHelper(key, &value.m_object);
}


LuaObject& LuaObject::SetObject(LuaObject& key, LuaObject& value)
{
	luaplus_assert(m_state  &&  IsTable());
	luaplus_assert(m_state == value.m_state);
	return SetTableHelper(key, &value.m_object);
}



LuaObject& LuaObject::RawSetNil(const char* key)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnilvalue2n(L, &valueObj);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetNil(int key)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnilvalue2n(L, &valueObj);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetNil(LuaObject& key)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnilvalue2n(L, &valueObj);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetBoolean(const char* key, bool value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setbvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetBoolean(int key, bool value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setbvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetBoolean(LuaObject& key, bool value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setbvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetInteger(const char* key, int value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetInteger(int key, int value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetInteger(LuaObject& key, int value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetNumber(const char* key, lua_Number value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetNumber(int key, lua_Number value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetNumber(LuaObject& key, lua_Number value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setnvalue2n(&valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetString(const char* key, const char* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = strlen(value);
		setsvalue2n(L, &valueObj, luaS_newlstr(m_state->m_state, value, len));
	}
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetString(int key, const char* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = strlen(value);
		setsvalue2n(L, &valueObj, luaS_newlstr(m_state->m_state, value, len));
	}
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetString(LuaObject& key, const char* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = strlen(value);
		setsvalue2n(L, &valueObj, luaS_newlstr(m_state->m_state, value, len));
	}
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetWString(const char* key, const lua_WChar* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = lua_WChar_len(value);
//jj		setwsvalue2n(L, &valueObj, luaS_newlwstr(m_state->m_state, value, len));
	}
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetWString(int key, const lua_WChar* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = lua_WChar_len(value);
//jj		setwsvalue2n(L, &valueObj, luaS_newlwstr(m_state->m_state, value, len));
	}
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetWString(LuaObject& key, const lua_WChar* value, int len)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	TValue valueObj;
	if (value == NULL)
	{
		setnilvalue2n(L, &valueObj);
	}
	else
	{
		if (len == -1)
			len = lua_WChar_len(value);
//jj		setwsvalue2n(L, &valueObj, luaS_newlwstr(m_state->m_state, value, len));
	}
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetUserData(const char* key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	Udata* u = luaS_newudata(m_state->m_state, 4, getcurrenv(L));
	u->uv.len = 1;  // user data box bit is set.
	*(void**)(u + 1) = value;

	TValue valueObj;
	setuvalue2n(L, &valueObj, u);
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetUserData(int key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	Udata* u = luaS_newudata(m_state->m_state, 4, getcurrenv(L));
	u->uv.len = 1;  // user data box bit is set.
	*(void**)(u + 1) = value;

	TValue valueObj;
	setuvalue2n(L, &valueObj, u);
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetUserData(LuaObject& key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	lua_State* L = GetCState(); (void)L;
	Udata* u = luaS_newudata(m_state->m_state, 4, getcurrenv(L));
	u->uv.len = 1;  // user data box bit is set.
	*(void**)(u + 1) = value;

	TValue valueObj;
	setuvalue2n(L, &valueObj, u);
	RawSetTableHelper(key, &valueObj);
	setnilvalue(&valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetLightUserData(const char* key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setpvalue2n(L, &valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetLightUserData(int key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setpvalue2n(L, &valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetLightUserData(LuaObject& key, void* value)
{
	luaplus_assert(m_state  &&  IsTable());
	TValue valueObj;
	setpvalue2n(L, &valueObj, value);
	return RawSetTableHelper(key, &valueObj);
}


LuaObject& LuaObject::RawSetObject(const char* key, LuaObject& value)
{
	luaplus_assert(m_state  &&  IsTable());
	return RawSetTableHelper(key, &value.m_object);
}


LuaObject& LuaObject::RawSetObject(int key, LuaObject& value)
{
	luaplus_assert(m_state  &&  IsTable());
	return RawSetTableHelper(key, &value.m_object);
}


LuaObject& LuaObject::RawSetObject(LuaObject& key, LuaObject& value)
{
	luaplus_assert(m_state  &&  IsTable());
	return RawSetTableHelper(key, &value.m_object);
}






void LuaObject::AssignNil(LuaState* state)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	setnilvalue(&m_object);
}


void LuaObject::AssignBoolean(LuaState* state, bool value)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	setbvalue(&m_object, value);
}


void LuaObject::AssignInteger(LuaState* state, int value)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	setnvalue(&m_object, value);
}


void LuaObject::AssignNumber(LuaState* state, lua_Number value)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	setnvalue(&m_object, value);
}


void LuaObject::AssignString(LuaState* state, const char* value, int len)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	if (value == NULL)
	{
		setnilvalue(&m_object);
	}
	else
	{
		if (len == -1)
			len = strlen(value);
		setsvalue(L, &m_object, luaS_newlstr(L, value, len));
	}
}


void LuaObject::AssignWString(LuaState* state, const lua_WChar* value, int len)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	if (value == NULL)
	{
		setnilvalue(&m_object);
	}
	else
	{
		if (len == -1)
			len = lua_WChar_len(value);
		setwsvalue(L, &m_object, luaS_newlwstr(L, value, len));
	}
}


void LuaObject::AssignUserData(LuaState* state, void* value)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	Udata* u = luaS_newudata(L, 4, getcurrenv(L));
	u->uv.len = 1;  // user data box bit is set.
	*(void**)(u + 1) = value;
	setuvalue(L, &m_object, u);
}


void LuaObject::AssignLightUserData(LuaState* state, void* value)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	setpvalue(&m_object, value);
}


void LuaObject::AssignObject(LuaObject& value)
{
	if (value.m_state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(value.m_state);
	}
	lua_State *L = GetCState();  (void)L;
	setobj(L, &m_object, &value.m_object);
}


void LuaObject::AssignNewTable(LuaState* state, int narray, int numhash)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	int lnhash = luaO_log2(numhash) + 1;
	lua_State *L = GetCState();  (void)L;
	sethvalue(L, &m_object, luaH_new(L, narray, lnhash));
}


void LuaObject::AssignTObject(LuaState* state, const TValue* value)
{
	if (state != m_state)
	{
		RemoveFromUsedList();
		AddToUsedList(state);
	}
	lua_State *L = GetCState();  (void)L;
	setobj(L, &m_object, value);
}


/**
	Assuming the current object is a table, registers a C function called
	[funcName] within the table.

	@param funcName The name of the function to register.
	@param function A pointer to the C function to register.
**/
void LuaObject::Register(const char* funcName, NAMESPACE_LUA_PREFIX lua_CFunction function, int nupvalues)
{
	RegisterHelper(funcName, function, nupvalues, NULL, 0, NULL, 0);
}


void LuaObject::Register(const char* funcName, int (*func)(LuaState*), int nupvalues)
{
	RegisterHelper(funcName, LPCD::LuaStateFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
}


void LuaObject::Register(const char* funcName, int (*func)(LuaState*, LuaStackObject*), int nupvalues)
{
	RegisterHelper(funcName, LPCD::LuaStateOldFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
}


void LuaObject::RegisterHelper(const char* funcName, NAMESPACE_LUA_PREFIX lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc)
{
	luaplus_assert(m_state);
	lua_State* L = GetCState();
	lua_lock(L);
	luaC_checkGC(L);

	if (sizeofFunc != 0)
	{
		unsigned char* buffer = (unsigned char*)lua_newuserdata(GetCState(), sizeofCallee + sizeofFunc);
		unsigned int pos = 0;
		if (sizeofCallee > 0)
		{
			memcpy(buffer, callee, sizeofCallee);
			pos += sizeofCallee;
		}

		memcpy(buffer + pos, func, sizeofFunc);

		nupvalues++;
	}

	Closure* cl = luaF_newCclosure(L, nupvalues, getcurrenv(L));
	cl->c.f = function;

	L->top -= nupvalues;
	while (nupvalues--)
	{
		setobj2n(L, &cl->c.upvalue[nupvalues], L->top+nupvalues);
//		setnilvalue(L->top+nupvalues);
	}

	TValue valueObj;
	setclvalue2n(L, &valueObj, cl);
	lua_assert(iswhite(obj2gco(cl)));
	SetTableHelper(funcName, &valueObj);
	setnilvalue(&valueObj);

	lua_unlock(L);
}


/**
	Assuming the current object is a table, unregisters the function called
	[funcName].

	@param funcName The name of the function to unregister.
**/
void LuaObject::Unregister(const char* funcName)
{
	luaplus_assert(m_state);
	SetNil(funcName);
}

LuaObject& LuaObject::SetTableHelper(const char* key, TValue* valueObj)
{
	lua_State *L = m_state->m_state;	(void)L;
	TValue keyObj;
	setsvalue2n(L, &keyObj, luaS_newlstr(L, key, strlen(key)));
	luaV_settable(L, &m_object, &keyObj, valueObj);
	setnilvalue(&keyObj);
	return *this;
}


LuaObject& LuaObject::SetTableHelper(int key, TValue* valueObj)
{
	TValue keyObj;
	setnvalue2n(&keyObj, key);
	luaV_settable(GetCState(), &m_object, &keyObj, valueObj);
	return *this;
}


LuaObject& LuaObject::SetTableHelper(const TValue* keyObj, const TValue* valueObj)
{
	luaV_settable(GetCState(), &m_object, (TValue*)keyObj, (TValue*)valueObj);
	return *this;
}


LuaObject& LuaObject::SetTableHelper(const LuaObject& key, TValue* valueObj)
{
	luaV_settable(GetCState(), &m_object, (TValue*)&key.m_object, valueObj);
	return *this;
}


LuaObject& LuaObject::RawSetTableHelper(const char* key, TValue* valueObj)
{
	lua_State *L = GetCState();	(void)L;
	TValue keyObj;
	setsvalue2n(L, &keyObj, luaS_newlstr(L, key, strlen(key)));

	RawSetTableHelper(&keyObj, valueObj);

	setnilvalue(&keyObj);
	return *this;
}


LuaObject& LuaObject::RawSetTableHelper(int key, TValue* valueObj)
{
	TValue keyObj;
	setnvalue2n(&keyObj, key);
	return RawSetTableHelper(&keyObj, valueObj);
}


LuaObject& LuaObject::RawSetTableHelper(const TValue* keyObj, const TValue* valueObj)
{
	lua_State *L = GetCState();
	Table *h = hvalue(&m_object);
	TValue *oldval = luaH_set(L, h, keyObj);
	if (!ttisnil(oldval))
	{
		setobj2t(L, oldval, valueObj);
        luaC_barriert(L, h, valueObj);
#if LUA_REFCOUNT
		if (ttisnil(oldval))
		{
			TValue* foundKeyObj = (TValue*)(luaH_getkey(h, keyObj));
			if (foundKeyObj)
				setnilvalue(foundKeyObj);
		}
#endif /* LUA_REFCOUNT */
	}

	return *this;
}


LuaObject& LuaObject::RawSetTableHelper(const LuaObject& key, TValue* valueObj)
{
	return RawSetTableHelper(&key.m_object, valueObj);
}


inline void LuaObject::AddToUsedList(LuaState* state)
{
	luaplus_assert(state);
	m_state = state;
	LuaObject& headObject = *(LuaObject*)&G(m_state->m_state)->m_headObject;
	m_next = headObject.m_next;
	headObject.m_next = this;
	m_next->m_prev = this;
	m_prev = &headObject;
}


inline void LuaObject::AddToUsedList(LuaState* state, const lua_TValue& obj)
{
	luaplus_assert(state);
    lua_lock(state->m_state);
	m_state = state;
	LuaObject& headObject = *(LuaObject*)&G(m_state->m_state)->m_headObject;
	m_next = headObject.m_next;
	headObject.m_next = this;
	m_next->m_prev = this;
	m_prev = &headObject;
	lua_State *L = GetCState();  (void)L;
	setobj(L, &m_object, &obj);
    lua_unlock(state->m_state);
}


inline void LuaObject::RemoveFromUsedList()
{
	if (m_state)
	{
		lua_State* L = m_state->m_state;    (void)L;
        lua_lock(L);

        // remove pOldNode from list
		m_prev->m_next = m_next;
		m_next->m_prev = m_prev;

		setnilvalue(&m_object);

        lua_unlock(L);
    }
}




namespace LuaHelper {

bool GetBoolean( LuaObject& obj, int key, bool require, bool defaultValue )
{
	LuaObject boolObj = obj[ key ];
	if ( !boolObj.IsBoolean() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return boolObj.GetBoolean();
}


bool GetBoolean( LuaObject& obj, const char* key, bool require, bool defaultValue )
{
	LuaObject boolObj = obj[ key ];
	if ( !boolObj.IsBoolean() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return boolObj.GetBoolean();
}


int GetInteger( LuaObject& obj, int key, bool require, int defaultValue )
{
	LuaObject intObj = obj[ key ];
	if ( !intObj.IsInteger() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return intObj.GetInteger();
}


int GetInteger( LuaObject& obj, const char* key, bool require, int defaultValue )
{
	LuaObject intObj = obj[ key ];
	if ( !intObj.IsInteger() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return intObj.GetInteger();
}


float GetFloat( LuaObject& obj, int key, bool require, float defaultValue )
{
	LuaObject floatObj = obj[ key ];
	if ( !floatObj.IsNumber() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return (float)floatObj.GetNumber();
}


float GetFloat( LuaObject& obj, const char* key, bool require, float defaultValue )
{
	LuaObject floatObj = obj[ key ];
	if ( !floatObj.IsNumber() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return (float)floatObj.GetNumber();
}


void* GetLightUserData( LuaObject& obj, int key, bool require, void* defaultValue )
{
	LuaObject outObj = obj[ key ];
	if ( !outObj.IsLightUserData() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return outObj.GetLightUserData();
}


void* GetLightUserData( LuaObject& obj, const char* key, bool require, void* defaultValue )
{
	LuaObject outObj = obj[ key ];
	if ( !outObj.IsLightUserData() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return outObj.GetLightUserData();
}


const char* GetString( LuaObject& obj, int key, bool require, const char* defaultValue )
{
	LuaObject stringObj = obj[ key ];
	if ( !stringObj.IsString() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return stringObj.GetString();
}


const char* GetString( LuaObject& obj, const char* key, bool require, const char* defaultValue )
{
	LuaObject stringObj = obj[ key ];
	if ( !stringObj.IsString() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return stringObj.GetString();
}


LuaObject GetTable( LuaObject& obj, int key, bool require )
{
	LuaObject tableObj = obj[ key ];
	if ( !tableObj.IsTable() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
	}
	return tableObj;
}


LuaObject GetTable( LuaObject& obj, const char* key, bool require )
{
	LuaObject tableObj = obj[ key ];
	if ( !tableObj.IsTable() )
	{
		if ( require )
		{
			luaplus_assert( 0 );
		}
	}
	return tableObj;
}

} // namespace LuaHelper

} // namespace LuaPlus

namespace LPCD
{
	void Push(lua_State* L, LuaObject& value)
	{
		(void)L; value.Push();
	}

	LuaObject Get(TypeWrapper<LuaObject>, lua_State* L, int idx)
	{
		return LuaObject(LuaState::CastState(L), idx);
	}
}
