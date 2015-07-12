///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER
#ifndef LUAOBJECT_H
#define LUAOBJECT_H

#include "LuaPlusInternal.h"
#include "LuaPlusCD.h"

NAMESPACE_LUA_BEGIN

/*
** Union of all Lua values
*/
typedef union GCObject GCObject;
typedef union {
  GCObject *gc;
  void *p;
  lua_Number n;
  int b;
} LP_Value;


/*
** Tagged Values
*/

#define LP_TValuefields	LP_Value value; int tt

typedef struct LP_lua_TValue {
  LP_TValuefields;
} LP_TValue;

NAMESPACE_LUA_END

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

USING_NAMESPACE_LUA;

/**
	Representation of a Lua object.
**/
class LUAPLUS_CLASS LuaObject
{
	friend class LuaState;
public:
	LuaObject();
	LuaObject(LuaState* state) throw();
	LuaObject(LuaState* state, int stackIndex) throw();
	LuaObject(LuaState* state, const lua_TValue* obj);
	LuaObject(const LuaObject& src) throw();
	LuaObject(const LuaStackObject& src) throw();
	LuaObject& operator=(const LuaObject& src) throw();
	LuaObject& operator=(const LuaStackObject& src) throw();

/*	template <typename T>
	LuaObject& operator=(const T& value)
	{
		assert(m_state);
		LCD::Assign(*this, value);
		return *this;
	}*/

	~LuaObject();

	void Reset();

	/**
		Retrieves the LuaState object associated with this LuaObject.
	**/
	LuaState* GetState() const;
	lua_State* GetCState() const;
	lua_TValue* GetTObject() const;

	bool operator==(const LuaObject& right) const;
	bool operator<(const LuaObject& right) const;

	const char* TypeName() const;
	int Type() const;

	bool IsNil() const;
	bool IsTable() const;
	bool IsUserData() const;
	bool IsCFunction() const;
	bool IsInteger() const;
	bool IsNumber() const;
	bool IsString() const;
	bool IsWString() const;
	bool IsConvertibleToInteger() const;
	bool IsConvertibleToNumber() const;
	bool IsConvertibleToString() const;
	bool IsConvertibleToWString() const;
	bool IsFunction() const;
	bool IsNone() const;
	bool IsLightUserData() const;
	bool IsBoolean() const;

	int ToInteger();
	lua_Number ToNumber();
	const char* ToString();
	const lua_WChar* ToWString();
	size_t ToStrLen();

	int GetInteger() const;
	float GetFloat() const;
	double GetDouble() const;
	lua_Number GetNumber() const;
	const char* GetString() const;
	const lua_WChar* GetWString() const;
	int StrLen();
	lua_CFunction GetCFunction() const;
	void* GetUserData();
	const void* GetLuaPointer();
	void* GetLightUserData() const;
	bool GetBoolean() const;

	LuaStackObject Push() const;

	LuaObject GetMetaTable();
	void SetMetaTable(const LuaObject& valueObj);

	int GetN();
	void SetN(int n);

	void Insert(LuaObject& obj);
	void Insert(int index, LuaObject& obj);
	void Remove(int index = -1);
	void Sort();

	int GetCount();
	int GetTableCount();

	LuaObject Clone();
	void DeepClone(LuaObject& outObj);

	LuaObject CreateTable(const char* key, int narray = 0, int lnhash = 0);
	LuaObject CreateTable(int key, int narray = 0, int lnhash = 0);
	LuaObject CreateTable(LuaObject& key, int narray = 0, int lnhash = 0);

	template <typename KeyT, typename ValueT> LuaObject& Set(const KeyT& key, const ValueT& value);
	template <typename KeyT> LuaObject& SetNil(const KeyT& key);

	LuaObject& SetNil(const char* key);
	LuaObject& SetNil(int key);
	LuaObject& SetNil(LuaObject& key);
	LuaObject& SetBoolean(const char* key, bool value);
	LuaObject& SetBoolean(int key, bool value);
	LuaObject& SetBoolean(LuaObject& key, bool value);
	LuaObject& SetInteger(const char* key, int value);
	LuaObject& SetInteger(int key, int value);
	LuaObject& SetInteger(LuaObject& key, int value);
	LuaObject& SetNumber(const char* key, lua_Number value);
	LuaObject& SetNumber(int key, lua_Number value);
	LuaObject& SetNumber(LuaObject& key, lua_Number value);
	LuaObject& SetString(const char* key, const char* value, int len = -1);
	LuaObject& SetString(int key, const char* value, int len = -1);
	LuaObject& SetString(LuaObject& key, const char* value, int len = -1);
	LuaObject& SetWString(const char* key, const lua_WChar* value, int len = -1);
	LuaObject& SetWString(int key, const lua_WChar* value, int len = -1);
	LuaObject& SetWString(LuaObject& key, const lua_WChar* value, int len = -1);
	LuaObject& SetUserData(const char* key, void* value);
	LuaObject& SetUserData(int key, void* value);
	LuaObject& SetUserData(LuaObject& key, void* value);
	LuaObject& SetLightUserData(const char* key, void* value);
	LuaObject& SetLightUserData(int key, void* value);
	LuaObject& SetLightUserData(LuaObject& key, void* value);
	LuaObject& SetObject(const char* key, LuaObject& value);
	LuaObject& SetObject(int key, LuaObject& value);
	LuaObject& SetObject(LuaObject& key, LuaObject& value);

	LuaObject& RawSetNil(const char* key);
	LuaObject& RawSetNil(int key);
	LuaObject& RawSetNil(LuaObject& key);
	LuaObject& RawSetBoolean(const char* key, bool value);
	LuaObject& RawSetBoolean(int key, bool value);
	LuaObject& RawSetBoolean(LuaObject& key, bool value);
	LuaObject& RawSetInteger(const char* key, int value);
	LuaObject& RawSetInteger(int key, int value);
	LuaObject& RawSetInteger(LuaObject& key, int value);
	LuaObject& RawSetNumber(const char* key, lua_Number value);
	LuaObject& RawSetNumber(int key, lua_Number value);
	LuaObject& RawSetNumber(LuaObject& key, lua_Number value);
	LuaObject& RawSetString(const char* key, const char* value, int len = -1);
	LuaObject& RawSetString(int key, const char* value, int len = -1);
	LuaObject& RawSetString(LuaObject& key, const char* value, int len = -1);
	LuaObject& RawSetWString(const char* key, const lua_WChar* value, int len = -1);
	LuaObject& RawSetWString(int key, const lua_WChar* value, int len = -1);
	LuaObject& RawSetWString(LuaObject& key, const lua_WChar* value, int len = -1);
	LuaObject& RawSetUserData(const char* key, void* value);
	LuaObject& RawSetUserData(int key, void* value);
	LuaObject& RawSetUserData(LuaObject& key, void* value);
	LuaObject& RawSetLightUserData(const char* key, void* value);
	LuaObject& RawSetLightUserData(int key, void* value);
	LuaObject& RawSetLightUserData(LuaObject& key, void* value);
	LuaObject& RawSetObject(const char* key, LuaObject& value);
	LuaObject& RawSetObject(int key, LuaObject& value);
	LuaObject& RawSetObject(LuaObject& key, LuaObject& value);

	void AssignNil(LuaState* state);
	void AssignBoolean(LuaState* state, bool value);
	void AssignInteger(LuaState* state, int value);
	void AssignNumber(LuaState* state, lua_Number value);
	void AssignString(LuaState* state, const char* value, int len = -1);
	void AssignWString(LuaState* state, const lua_WChar* value, int len = -1);
	void AssignUserData(LuaState* state, void* value);
	void AssignLightUserData(LuaState* state, void* value);
	void AssignObject(LuaObject& value);		// Should this function be removed??
	void AssignNewTable(LuaState* state, int narray = 0, int numhash = 0);
	void AssignTObject(LuaState* state, const lua_TValue* value);

	LuaObject GetByName(const char* name);
	LuaObject GetByIndex(int index);
	LuaObject GetByObject(const LuaStackObject& obj);
	LuaObject GetByObject(const LuaObject& obj);
	LuaObject RawGetByName(const char* name);
	LuaObject RawGetByIndex(int index);
	LuaObject RawGetByObject(const LuaStackObject& obj);
	LuaObject RawGetByObject(const LuaObject& obj);

	// Raw
	LuaObject operator[](const char* name);
	LuaObject operator[](int index);
	LuaObject operator[](const LuaStackObject& obj);
	LuaObject operator[](const LuaObject& obj);

	LuaObject Lookup(const char* key);

	void Register(const char* funcName, lua_CFunction func, int nupvalues = 0);

	void Register(const char* funcName, int (*func)(LuaState*), int nupvalues = 0);
	void Register(const char* funcName, int (*func)(LuaState*, LuaStackObject*), int nupvalues = 0);

	template <class Callee>
	void Register(const char* funcName, const Callee& callee, int (Callee::*func)(LuaState*), int nupvalues = 0)
	{
		const void* pCallee = &callee;
		RegisterHelper(funcName, LPCD::LuaStateMemberDispatcherHelper<Callee>::LuaStateMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <class Callee>
	void Register(const char* funcName, const Callee& callee, int (Callee::*func)(LuaState*, LuaStackObject*), int nupvalues = 0)
	{
		const void* pCallee = &callee;
		RegisterHelper(funcName, &LPCD::LuaStateOldMemberDispatcherHelper<Callee>::LuaStateOldMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <class Callee>
	void RegisterObjectFunctor(const char* funcName, int (Callee::*func)(LuaState*), int nupvalues = 0)
	{
		RegisterHelper(funcName, LPCD::Object_MemberDispatcher_to_LuaStateHelper<Callee>::Object_MemberDispatcher_to_LuaState, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Func>
	inline void RegisterDirect(const char* funcName, Func func, unsigned int nupvalues = 0)
	{
		RegisterHelper(funcName, LPCD::DirectCallFunctionDispatchHelper<Func>::DirectCallFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline void RegisterDirect(const char* funcName, const Callee& callee, Func func, unsigned int nupvalues = 0)
	{
		const void* pCallee = &callee;
		RegisterHelper(funcName, LPCD::DirectCallMemberDispatcherHelper<Callee, Func>::DirectCallMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline void RegisterObjectDirect(const char* funcName, const Callee* callee, Func func, unsigned int nupvalues = 0)
	{
		RegisterHelper(funcName, LPCD::DirectCallObjectMemberDispatcherHelper<Callee, Func, 2>::DirectCallMemberDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	void Unregister(const char* funcName);

protected:
	void SetNilHelper(lua_TValue& obj);
	void RegisterHelper(const char* funcName, lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc);

	LuaObject& SetTableHelper(const char* key, lua_TValue* valueObj);
	LuaObject& SetTableHelper(int key, lua_TValue* valueObj);
	LuaObject& SetTableHelper(const lua_TValue* keyObj, const lua_TValue* valueObj);
	LuaObject& SetTableHelper(const LuaObject& key, lua_TValue* valueObj);
	LuaObject& RawSetTableHelper(const char* key, lua_TValue* valueObj);
	LuaObject& RawSetTableHelper(int key, lua_TValue* valueObj);
	LuaObject& RawSetTableHelper(const lua_TValue* keyObj, const lua_TValue* valueObj);
	LuaObject& RawSetTableHelper(const LuaObject& key, lua_TValue* valueObj);

private:
	// Private functions and data
	void AddToUsedList(LuaState* state);
	void AddToUsedList(LuaState* state, const lua_TValue& obj);
	void RemoveFromUsedList();

	LuaObject* m_next;		   // only valid when in free list
	LuaObject* m_prev;		   // only valid when in used list
#if defined(BUILDING_LUAPLUS)
	lua_TValue m_object;
#else
	LP_lua_TValue m_object;
#endif
	LuaState* m_state;
};


namespace detail
{
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const LuaArgNil&);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, bool value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, char value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, unsigned char value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, short value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, unsigned short value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, int value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, unsigned int value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, float value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, double value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const char* value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const lua_WChar* value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const LuaObject& value);
	LUAPLUS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, void* value);

	LUAPLUS_API void SetNilValue(lua_State* L, lua_TValue* obj);
}

template <typename T>
inline LuaObject& LuaObject::SetNil(const T& key)
{
	luaplus_assert(m_state  &&  IsTable());
	LP_lua_TValue keyObj;
	LuaArgNil argNil;
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&keyObj, argNil);
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&keyObj, key);
	LP_lua_TValue valueObj;
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&valueObj, LuaArgNil());
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&valueObj, LuaArgNil());
	SetTableHelper((lua_TValue*)&keyObj, (lua_TValue*)&valueObj);
	detail::SetNilValue(m_state->m_state, (lua_TValue*)&keyObj);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::Set(const KeyT& key, const ValueT& value)
{
	luaplus_assert(m_state  &&  IsTable());
	LP_lua_TValue keyObj;
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&keyObj, LuaArgNil());
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&keyObj, key);
	LP_lua_TValue valueObj;
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&valueObj, LuaArgNil());
	detail::AssignNewTObject(m_state->m_state, (lua_TValue*)&valueObj, value);
	SetTableHelper((lua_TValue*)&keyObj, (lua_TValue*)&valueObj);
	detail::SetNilValue(m_state->m_state, (lua_TValue*)&valueObj);
	detail::SetNilValue(m_state->m_state, (lua_TValue*)&keyObj);
	return *this;
}

} // namespace LuaPlus

namespace LPCD
{
	using namespace LuaPlus;

	LUAPLUS_API void Push(lua_State* L, LuaObject& value);
	inline bool	Match(TypeWrapper<LuaObject>, lua_State* L, int idx)
		{  (void)L, (void)idx;  return true;  }
	LUAPLUS_API LuaObject		Get(TypeWrapper<LuaObject>, lua_State* L, int idx);

	template <typename Object, typename VarType>
	inline void PropertyCreate(LuaObject& metaTableObj, const char* varName, VarType Object::* var)
	{
		LuaObject propsObj = metaTableObj["__props"];
		if (propsObj.IsNil())
		{
			propsObj = metaTableObj.CreateTable("__props");
		}

		LuaObject varObj = propsObj.CreateTable(varName);

		LuaState* state = metaTableObj.GetState();

		varObj.Push();
		state->PushNumber(1);
		lpcd_pushmemberpropertygetclosure(*state, var);
		state->RawSet(-3);

		state->PushNumber(2);
		lpcd_pushmemberpropertysetclosure(*state, var);
		state->RawSet(-3);

		state->Pop();
	}


	inline void MetaTable_IntegratePropertySupport(LuaObject& metaTableObj)
	{
		metaTableObj.Register("__index", PropertyMetaTable_index);
		metaTableObj.Register("__newindex", PropertyMetaTable_newindex);
	}


	template <typename Object, typename VarType>
	void Register_MemberPropertyGetFunction(LuaObject& obj, const char* funcName, VarType Object::* var)
	{
		obj.Push();

		LuaState* state = obj.GetState();
		state->PushString(funcName);
		lpcd_pushmemberpropertygetclosure(*state, var);
		state->RawSet(-3);

		state->Pop();
	}

	template <typename Object, typename VarType>
	void Register_MemberPropertySetFunction(LuaObject& obj, const char* funcName, VarType Object::* var)
	{
		obj.Push();

		LuaState* state = obj.GetState();
		state->PushString(funcName);
		lpcd_pushmemberpropertysetclosure(*state, var);
		state->RawSet(-3);

		state->Pop();
	}


	template <typename VarType>
	void Register_GlobalPropertyGetFunction(const LuaObject& obj, const char* funcName, VarType* var)
	{
		obj.Push();

		LuaState* state = obj.GetState();
		state->PushString(funcName);
		lpcd_pushglobalpropertygetclosure(*state, var);
		state->RawSet(-3);

		state->Pop();
	}

	template <typename VarType>
	void Register_GlobalPropertySetFunction(const LuaObject& obj, const char* funcName, VarType* var)
	{
		obj.Push();

		LuaState* state = obj.GetState();
		state->PushString(funcName);
		lpcd_pushglobalpropertysetclosure(*state, var);
		state->RawSet(-3);

		state->Pop();
	}
} // namespace LPCD

#endif // LUAOBJECT_H
