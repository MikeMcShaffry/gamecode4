///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAOBJECT_H
#define LUAOBJECT_H

#include "LuaStateCD.h"

#if LUAPLUS_EXTENSIONS

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
class LuaObject
{
	friend class LuaState;
public:
	LUAPLUS_CLASS_API LuaObject();
	LUAPLUS_CLASS_API LuaObject(lua_State* L) throw();
	LUAPLUS_CLASS_API LuaObject(LuaState* state) throw();
	LUAPLUS_CLASS_API LuaObject(lua_State* L, int stackIndex) throw();
	LUAPLUS_CLASS_API LuaObject(LuaState* state, int stackIndex) throw();
	LUAPLUS_CLASS_API LuaObject(lua_State* L, const lua_TValue* obj);
	LUAPLUS_CLASS_API LuaObject(LuaState* state, const lua_TValue* obj);
	LUAPLUS_CLASS_API LuaObject(const LuaObject& src) throw();
	LUAPLUS_CLASS_API LuaObject(const LuaStackObject& src) throw();
	LUAPLUS_CLASS_API LuaObject& operator=(const LuaObject& src) throw();
	LUAPLUS_CLASS_API LuaObject& operator=(const LuaStackObject& src) throw();

/*	template <typename T>
	LuaObject& operator=(const T& value)
	{
		assert(L);
		LCD::Assign(*this, value);
		return *this;
	}*/

	LUAPLUS_CLASS_API ~LuaObject();

	LUAPLUS_CLASS_API void Reset();

	/**
		Retrieves the LuaState object associated with this LuaObject.
	**/
	LuaState* GetState() const;
	lua_State* GetCState() const;
	lua_TValue* GetTObject() const;

	bool operator==(const LuaObject& right) const;
	bool operator<(const LuaObject& right) const;

	LUAPLUS_CLASS_API const char* TypeName() const;
	LUAPLUS_CLASS_API int Type() const;

	LUAPLUS_CLASS_API bool IsNil() const;
	LUAPLUS_CLASS_API bool IsTable() const;
	LUAPLUS_CLASS_API bool IsUserData() const;
	LUAPLUS_CLASS_API bool IsCFunction() const;
	LUAPLUS_CLASS_API bool IsInteger() const;
	LUAPLUS_CLASS_API bool IsNumber() const;
	LUAPLUS_CLASS_API bool IsString() const;
	LUAPLUS_CLASS_API bool IsConvertibleToInteger() const;
	LUAPLUS_CLASS_API bool IsConvertibleToNumber() const;
	LUAPLUS_CLASS_API bool IsConvertibleToString() const;
#if LUA_WIDESTRING
	LUAPLUS_CLASS_API bool IsWString() const;
	LUAPLUS_CLASS_API bool IsConvertibleToWString() const;
#endif /* LUA_WIDESTRING */
	LUAPLUS_CLASS_API bool IsFunction() const;
	LUAPLUS_CLASS_API bool IsNone() const;
	LUAPLUS_CLASS_API bool IsLightUserData() const;
	LUAPLUS_CLASS_API bool IsBoolean() const;

	LUAPLUS_CLASS_API int ToInteger();
	LUAPLUS_CLASS_API lua_Number ToNumber();
	LUAPLUS_CLASS_API const char* ToString();
#if LUA_WIDESTRING
	LUAPLUS_CLASS_API const lua_WChar* ToWString();
#endif /* LUA_WIDESTRING */
	LUAPLUS_CLASS_API size_t ToStrLen();

	LUAPLUS_CLASS_API int GetInteger() const;
	LUAPLUS_CLASS_API float GetFloat() const;
	LUAPLUS_CLASS_API double GetDouble() const;
	LUAPLUS_CLASS_API lua_Number GetNumber() const;
	LUAPLUS_CLASS_API const char* GetString() const;
#if LUA_WIDESTRING
	LUAPLUS_CLASS_API const lua_WChar* GetWString() const;
#endif /* LUA_WIDESTRING */
	LUAPLUS_CLASS_API size_t StrLen();
	LUAPLUS_CLASS_API lua_CFunction GetCFunction() const;
	LUAPLUS_CLASS_API void* GetUserData();
	LUAPLUS_CLASS_API const void* GetLuaPointer();
	LUAPLUS_CLASS_API void* GetLightUserData() const;
	LUAPLUS_CLASS_API bool GetBoolean() const;

	LUAPLUS_CLASS_API LuaStackObject Push() const;

	LUAPLUS_CLASS_API LuaObject GetMetaTable() const;
	LUAPLUS_CLASS_API void SetMetaTable(const LuaObject& valueObj);

	LUAPLUS_CLASS_API int GetN() const;
	LUAPLUS_CLASS_API void SetN(int n);

	LUAPLUS_CLASS_API void Insert(LuaObject& obj);
	LUAPLUS_CLASS_API void Insert(int index, LuaObject& obj);
	LUAPLUS_CLASS_API void Remove(int index = -1);
	LUAPLUS_CLASS_API void Sort();

	LUAPLUS_CLASS_API size_t GetCount() const;
	LUAPLUS_CLASS_API size_t GetTableCount() const;

	LUAPLUS_CLASS_API LuaObject Clone() const;
	LUAPLUS_CLASS_API void DeepClone(LuaObject& outObj) const;

	LUAPLUS_CLASS_API LuaObject CreateTable(const char* key, int narray = 0, int lnhash = 0);
	LUAPLUS_CLASS_API LuaObject CreateTable(int key, int narray = 0, int lnhash = 0);
	LUAPLUS_CLASS_API LuaObject CreateTable(LuaObject& key, int narray = 0, int lnhash = 0);

	template <typename KeyT, typename ValueT> LuaObject& Set(const KeyT& key, const ValueT& value);
	template <typename KeyT> LuaObject& SetNil(const KeyT& key);

	LUAPLUS_CLASS_API LuaObject& SetNil(const char* key);
	LUAPLUS_CLASS_API LuaObject& SetNil(int key);
	LUAPLUS_CLASS_API LuaObject& SetNil(LuaObject& key);
	LUAPLUS_CLASS_API LuaObject& SetBoolean(const char* key, bool value);
	LUAPLUS_CLASS_API LuaObject& SetBoolean(int key, bool value);
	LUAPLUS_CLASS_API LuaObject& SetBoolean(LuaObject& key, bool value);
	LUAPLUS_CLASS_API LuaObject& SetInteger(const char* key, int value);
	LUAPLUS_CLASS_API LuaObject& SetInteger(int key, int value);
	LUAPLUS_CLASS_API LuaObject& SetInteger(LuaObject& key, int value);
	LUAPLUS_CLASS_API LuaObject& SetNumber(const char* key, lua_Number value);
	LUAPLUS_CLASS_API LuaObject& SetNumber(int key, lua_Number value);
	LUAPLUS_CLASS_API LuaObject& SetNumber(LuaObject& key, lua_Number value);
	LUAPLUS_CLASS_API LuaObject& SetString(const char* key, const char* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& SetString(int key, const char* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& SetString(LuaObject& key, const char* value, int len = -1);
#if LUA_WIDESTRING
	LUAPLUS_CLASS_API LuaObject& SetWString(const char* key, const lua_WChar* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& SetWString(int key, const lua_WChar* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& SetWString(LuaObject& key, const lua_WChar* value, int len = -1);
#endif /* LUA_WIDESTRING */
	LUAPLUS_CLASS_API LuaObject& SetUserData(const char* key, void* value);
	LUAPLUS_CLASS_API LuaObject& SetUserData(int key, void* value);
	LUAPLUS_CLASS_API LuaObject& SetUserData(LuaObject& key, void* value);
	LUAPLUS_CLASS_API LuaObject& SetLightUserData(const char* key, void* value);
	LUAPLUS_CLASS_API LuaObject& SetLightUserData(int key, void* value);
	LUAPLUS_CLASS_API LuaObject& SetLightUserData(LuaObject& key, void* value);
	LUAPLUS_CLASS_API LuaObject& SetObject(const char* key, LuaObject& value);
	LUAPLUS_CLASS_API LuaObject& SetObject(int key, LuaObject& value);
	LUAPLUS_CLASS_API LuaObject& SetObject(LuaObject& key, LuaObject& value);

	LUAPLUS_CLASS_API LuaObject& RawSetNil(const char* key);
	LUAPLUS_CLASS_API LuaObject& RawSetNil(int key);
	LUAPLUS_CLASS_API LuaObject& RawSetNil(LuaObject& key);
	LUAPLUS_CLASS_API LuaObject& RawSetBoolean(const char* key, bool value);
	LUAPLUS_CLASS_API LuaObject& RawSetBoolean(int key, bool value);
	LUAPLUS_CLASS_API LuaObject& RawSetBoolean(LuaObject& key, bool value);
	LUAPLUS_CLASS_API LuaObject& RawSetInteger(const char* key, int value);
	LUAPLUS_CLASS_API LuaObject& RawSetInteger(int key, int value);
	LUAPLUS_CLASS_API LuaObject& RawSetInteger(LuaObject& key, int value);
	LUAPLUS_CLASS_API LuaObject& RawSetNumber(const char* key, lua_Number value);
	LUAPLUS_CLASS_API LuaObject& RawSetNumber(int key, lua_Number value);
	LUAPLUS_CLASS_API LuaObject& RawSetNumber(LuaObject& key, lua_Number value);
	LUAPLUS_CLASS_API LuaObject& RawSetString(const char* key, const char* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& RawSetString(int key, const char* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& RawSetString(LuaObject& key, const char* value, int len = -1);
#if LUA_WIDESTRING
	LUAPLUS_CLASS_API LuaObject& RawSetWString(const char* key, const lua_WChar* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& RawSetWString(int key, const lua_WChar* value, int len = -1);
	LUAPLUS_CLASS_API LuaObject& RawSetWString(LuaObject& key, const lua_WChar* value, int len = -1);
#endif /* LUA_WIDESTRING */
	LUAPLUS_CLASS_API LuaObject& RawSetUserData(const char* key, void* value);
	LUAPLUS_CLASS_API LuaObject& RawSetUserData(int key, void* value);
	LUAPLUS_CLASS_API LuaObject& RawSetUserData(LuaObject& key, void* value);
	LUAPLUS_CLASS_API LuaObject& RawSetLightUserData(const char* key, void* value);
	LUAPLUS_CLASS_API LuaObject& RawSetLightUserData(int key, void* value);
	LUAPLUS_CLASS_API LuaObject& RawSetLightUserData(LuaObject& key, void* value);
	LUAPLUS_CLASS_API LuaObject& RawSetObject(const char* key, LuaObject& value);
	LUAPLUS_CLASS_API LuaObject& RawSetObject(int key, LuaObject& value);
	LUAPLUS_CLASS_API LuaObject& RawSetObject(LuaObject& key, LuaObject& value);

	LUAPLUS_CLASS_API void AssignNil(LuaState* state);
	LUAPLUS_CLASS_API void AssignBoolean(LuaState* state, bool value);
	LUAPLUS_CLASS_API void AssignInteger(LuaState* state, int value);
	LUAPLUS_CLASS_API void AssignNumber(LuaState* state, lua_Number value);
	LUAPLUS_CLASS_API void AssignString(LuaState* state, const char* value, int len = -1);
#if LUA_WIDESTRING
	LUAPLUS_CLASS_API void AssignWString(LuaState* state, const lua_WChar* value, int len = -1);
#endif /* LUA_WIDESTRING */
	LUAPLUS_CLASS_API void AssignUserData(LuaState* state, void* value);
	LUAPLUS_CLASS_API void AssignLightUserData(LuaState* state, void* value);
	LUAPLUS_CLASS_API void AssignObject(LuaObject& value);		// Should this function be removed??
	LUAPLUS_CLASS_API LuaObject& AssignNewTable(LuaState* state, int narray = 0, int nrec = 0);
	LUAPLUS_CLASS_API void AssignTObject(LuaState* state, const lua_TValue* value);

	void AssignCFunction(lua_CFunction func, int nupvalues = 0);
	void AssignCFunction(int (*func)(LuaState*), int nupvalues = 0);

	template <class Callee>
	void AssignCFunction(const Callee& callee, int (Callee::*func)(LuaState*), int nupvalues = 0)
	{
		const void* pCallee = &callee;
		AssignCFunctionHelper(LPCD::LuaStateMemberDispatcherHelper<Callee>::LuaStateMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <class Callee>
	void AssignCFunctionObjectFunctor(const char* funcName, int (Callee::*func)(LuaState*), int nupvalues = 0)
	{
		AssignCFunctionHelper(LPCD::Object_MemberDispatcher_to_LuaStateHelper<Callee>::Object_MemberDispatcher_to_LuaState, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Func>
	inline void AssignCFunctionDirect(Func func, unsigned int nupvalues = 0)
	{
		AssignCFunctionHelper(LPCD::DirectCallFunctionDispatchHelper<Func>::DirectCallFunctionDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline void AssignCFunctionDirect(const Callee& callee, Func func, unsigned int nupvalues = 0)
	{
		const void* pCallee = &callee;
		AssignCFunctionHelper(LPCD::DirectCallMemberDispatcherHelper<Callee, Func>::DirectCallMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
	}

	template <typename Callee, typename Func>
	inline void AssignCFunctionObjectDirect(const Callee* callee, Func func, unsigned int nupvalues = 0)
	{
		AssignCFunctionHelper(LPCD::DirectCallObjectMemberDispatcherHelper<Callee, Func, 2>::DirectCallMemberDispatcher, nupvalues, NULL, 0, &func, sizeof(func));
	}


	LUAPLUS_CLASS_API LuaObject operator[](const char* name) const;
	LUAPLUS_CLASS_API LuaObject operator[](int index) const;
	LUAPLUS_CLASS_API LuaObject operator[](const LuaStackObject& obj) const;
	LUAPLUS_CLASS_API LuaObject operator[](const LuaObject& obj) const;
	LUAPLUS_CLASS_API LuaObject Get(const char* name) const;
	LUAPLUS_CLASS_API LuaObject Get(int index) const;
	LUAPLUS_CLASS_API LuaObject Get(const LuaStackObject& obj) const;
	LUAPLUS_CLASS_API LuaObject Get(const LuaObject& obj) const;
	LUAPLUS_CLASS_API LuaObject GetByName(const char* name) const;
	LUAPLUS_CLASS_API LuaObject GetByIndex(int index) const;
	LUAPLUS_CLASS_API LuaObject GetByObject(const LuaStackObject& obj) const;
	LUAPLUS_CLASS_API LuaObject GetByObject(const LuaObject& obj) const;

	LUAPLUS_CLASS_API LuaObject RawGet(const char* name) const;
	LUAPLUS_CLASS_API LuaObject RawGet(int index) const;
	LUAPLUS_CLASS_API LuaObject RawGet(const LuaStackObject& obj) const;
	LUAPLUS_CLASS_API LuaObject RawGet(const LuaObject& obj) const;
	LUAPLUS_CLASS_API LuaObject RawGetByName(const char* name) const;
	LUAPLUS_CLASS_API LuaObject RawGetByIndex(int index) const;
	LUAPLUS_CLASS_API LuaObject RawGetByObject(const LuaStackObject& obj) const;
	LUAPLUS_CLASS_API LuaObject RawGetByObject(const LuaObject& obj) const;

	LUAPLUS_CLASS_API LuaObject Lookup(const char* key) const;

	LUAPLUS_CLASS_API void Register(const char* funcName, lua_CFunction func, int nupvalues = 0);

	LUAPLUS_CLASS_API void Register(const char* funcName, int (*func)(LuaState*), int nupvalues = 0);

	template <class Callee>
	void Register(const char* funcName, const Callee& callee, int (Callee::*func)(LuaState*), int nupvalues = 0)
	{
		const void* pCallee = &callee;
		RegisterHelper(funcName, LPCD::LuaStateMemberDispatcherHelper<Callee>::LuaStateMemberDispatcher, nupvalues, &pCallee, sizeof(Callee*), &func, sizeof(func));
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

	LUAPLUS_CLASS_API void Unregister(const char* funcName);

protected:
	void SetNilHelper(lua_TValue& obj);
	LUAPLUS_CLASS_API void RegisterHelper(const char* funcName, lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc);
	LUAPLUS_CLASS_API void AssignCFunctionHelper(lua_CFunction function, int nupvalues, const void* callee, unsigned int sizeofCallee, void* func, unsigned int sizeofFunc);

	LUAPLUS_CLASS_API LuaObject& SetTableHelper(const char* key, lua_TValue* valueObj);
	LUAPLUS_CLASS_API LuaObject& SetTableHelper(int key, lua_TValue* valueObj);
	LUAPLUS_CLASS_API LuaObject& SetTableHelper(const lua_TValue* keyObj, const lua_TValue* valueObj);
	LUAPLUS_CLASS_API LuaObject& SetTableHelper(const LuaObject& key, lua_TValue* valueObj);
	LUAPLUS_CLASS_API LuaObject& RawSetTableHelper(const char* key, lua_TValue* valueObj);
	LUAPLUS_CLASS_API LuaObject& RawSetTableHelper(int key, lua_TValue* valueObj);
	LUAPLUS_CLASS_API LuaObject& RawSetTableHelper(const lua_TValue* keyObj, const lua_TValue* valueObj);
	LUAPLUS_CLASS_API LuaObject& RawSetTableHelper(const LuaObject& key, lua_TValue* valueObj);

private:
	// Private functions and data
	void AddToUsedList(lua_State* L);
	void AddToUsedList(lua_State* L, const lua_TValue& obj);
	void RemoveFromUsedList();

	LuaObject* m_next;		   // only valid when in free list
	LuaObject* m_prev;		   // only valid when in used list
#if defined(BUILDING_LUAPLUS)
	lua_TValue m_object;
#else
	LP_lua_TValue m_object;
#endif
	lua_State* L;
};


namespace detail
{
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const LuaArgNil&);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, bool value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, char value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, unsigned char value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, short value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, unsigned short value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, int value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, unsigned int value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, float value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, double value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const char* value);
#if LUA_WIDESTRING
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const lua_WChar* value);
#endif /* LUA_WIDESTRING */
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, const LuaObject& value);
	LUAPLUS_CLASS_API void AssignNewTObject(lua_State* L, lua_TValue* obj, void* value);

	LUAPLUS_CLASS_API void SetNilValue(lua_State* L, lua_TValue* obj);
}

template <typename T>
inline LuaObject& LuaObject::SetNil(const T& key)
{
	luaplus_assert(L  &&  IsTable());
	LP_lua_TValue keyObj;
	LuaArgNil argNil;
	detail::AssignNewTObject(L, (lua_TValue*)&keyObj, argNil);
	detail::AssignNewTObject(L, (lua_TValue*)&keyObj, key);
	LP_lua_TValue valueObj;
	detail::AssignNewTObject(L, (lua_TValue*)&valueObj, LuaArgNil());
	detail::AssignNewTObject(L, (lua_TValue*)&valueObj, LuaArgNil());
	SetTableHelper((lua_TValue*)&keyObj, (lua_TValue*)&valueObj);
	detail::SetNilValue(L, (lua_TValue*)&keyObj);
	return *this;
}


template <typename KeyT, typename ValueT>
LuaObject& LuaObject::Set(const KeyT& key, const ValueT& value)
{
	luaplus_assert(L  &&  IsTable());
	LP_lua_TValue keyObj;
	detail::AssignNewTObject(L, (lua_TValue*)&keyObj, LuaArgNil());
	detail::AssignNewTObject(L, (lua_TValue*)&keyObj, key);
	LP_lua_TValue valueObj;
	detail::AssignNewTObject(L, (lua_TValue*)&valueObj, LuaArgNil());
	detail::AssignNewTObject(L, (lua_TValue*)&valueObj, value);
	SetTableHelper((lua_TValue*)&keyObj, (lua_TValue*)&valueObj);
	detail::SetNilValue(L, (lua_TValue*)&valueObj);
	detail::SetNilValue(L, (lua_TValue*)&keyObj);
	return *this;
}

} // namespace LuaPlus

namespace LPCD
{
	using namespace LuaPlus;

	LUAPLUS_CLASS_API void Push(lua_State* L, LuaPlus::LuaObject& value);
	inline bool	Match(TypeWrapper<LuaPlus::LuaObject>, lua_State* L, int idx)
		{  (void)L, (void)idx;  return true;  }
	LUAPLUS_CLASS_API LuaPlus::LuaObject Get(TypeWrapper<LuaPlus::LuaObject>, lua_State* L, int idx);

	template <typename Object, typename VarType>
	inline void PropertyCreate(LuaPlus::LuaObject& metaTableObj, const char* varName, VarType Object::* var, bool read = true, bool write = true) {
		LuaObject propsObj = metaTableObj["__props"];
		if (propsObj.IsNil()) {
			propsObj = metaTableObj.CreateTable("__props");
		}

		LuaObject varObj = propsObj.CreateTable(varName);

		lua_State* L = metaTableObj.GetCState();

		varObj.Push();

		if (read) {
			lua_pushnumber(L, 1);
			lpcd_pushmemberpropertygetclosure(L, var);
			lua_rawset(L, -3);
		}

		if (write) {
			lua_pushnumber(L, 2);
			lpcd_pushmemberpropertysetclosure(L, var);
			lua_rawset(L, -3);
		}

		lua_pop(L, 1);
	}


	inline void MetaTable_IntegratePropertySupport(LuaPlus::LuaObject& metaTableObj)
	{
		metaTableObj.Register("__index", PropertyMetaTable_index);
		metaTableObj.Register("__newindex", PropertyMetaTable_newindex);
	}


	template <typename Object, typename VarType>
	void Register_MemberPropertyGetFunction(LuaPlus::LuaObject& obj, const char* funcName, VarType Object::* var)
	{
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushmemberpropertygetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}

	template <typename Object, typename VarType>
	void Register_MemberPropertySetFunction(LuaPlus::LuaObject& obj, const char* funcName, VarType Object::* var)
	{
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushmemberpropertysetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}


	template <typename VarType>
	void Register_GlobalPropertyGetFunction(const LuaPlus::LuaObject& obj, const char* funcName, VarType* var)
	{
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushglobalpropertygetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}

	template <typename VarType>
	void Register_GlobalPropertySetFunction(const LuaPlus::LuaObject& obj, const char* funcName, VarType* var)
	{
		obj.Push();

		lua_State* L = obj.GetCState();
		lua_pushstring(L, funcName);
		lpcd_pushglobalpropertysetclosure(L, var);
		lua_rawset(L, -3);

		lua_pop(L, 1);
	}
} // namespace LPCD

#endif // LUAPLUS_EXTENSIONS

#endif // LUAOBJECT_H
