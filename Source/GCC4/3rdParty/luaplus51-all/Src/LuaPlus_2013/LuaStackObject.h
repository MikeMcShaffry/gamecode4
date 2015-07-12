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
#ifndef LUASTACKOBJECT_H
#define LUASTACKOBJECT_H

#include "LuaPlusInternal.h"
#include "LuaPlusCD.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
/**
	Representation of a Lua object residing on the Lua stack.
**/
class LUAPLUS_CLASS LuaStackObject
{
public:
	/**
	**/
	LuaStackObject() :
		m_state(NULL)
	{
		// Bad to have this... but useful for containers.
	}

	/**
		Various constructors accepting different parameters.
	**/
	LuaStackObject(LuaState* state, int stackIndex);

	/**
		Various constructors accepting different parameters.
	**/
	LuaStackObject(LuaState& state, int stackIndex);

	/**
		Copy constructor.
	**/
	LuaStackObject(const LuaStackObject& src) :
		m_state(src.m_state),
		m_stackIndex(src.m_stackIndex)
	{
	}

	/**
		Assignment operator.
	**/
	const LuaStackObject& operator=(const LuaStackObject& src)
	{
		m_state = src.m_state;
		m_stackIndex = src.m_stackIndex;
		return *this;
	}

	/**
		Retrieves the LuaState object associated with this LuaStackObject.
	**/
	LuaState* GetState() const;
	lua_State* GetCState() const;
	operator int() const				{  return m_stackIndex;  }

	bool operator==(const LuaStackObject& right) const;

	const char* GetTypeName() const;
	int GetType() const;

	bool IsNil() const;
	bool IsTable() const;
	bool IsUserData() const;
	bool IsCFunction() const;
	bool IsInteger() const;
	bool IsNumber() const;
	bool IsString() const;
	bool IsWString() const;
	bool IsFunction() const;
	bool IsNone() const;
	bool IsLightUserData() const;
	bool IsBoolean() const;
	bool IsThread() const;

	float GetFloat() const;
	double GetDouble() const;
	int GetInteger() const;
	lua_Number GetNumber() const;
	const char* GetString() const;
	const lua_WChar* GetWString() const;
	int StrLen() const;
	lua_CFunction GetCFunction() const;
	void* GetUserData() const;
	const void* GetLuaPointer() const;
	void* GetLightUserData() const;
	bool GetBoolean() const;
	lua_State* GetThread() const;

	void Push();
	void Pop();

	int Ref(int lock = 1);

	LuaStackObject GetMetaTable();
	void SetMetaTable();
	void SetMetaTable(LuaStackObject value);

	void SetTable();
	int GetCount();

	LuaStackObject CreateTable(const char* name, int narray = 0, int lnhash = 0);
	LuaStackObject CreateTable(int index, int narray = 0, int lnhash = 0);

	void SetNil(const char* name);
	void SetNil(int index);
	void SetBoolean(const char* name, bool value);
	void SetBoolean(int index, bool value);
	void SetInteger(const char* name, int value);
	void SetInteger(int index, int value);
	void SetNumber(const char* name, lua_Number value);
	void SetNumber(int index, lua_Number value);
	void SetString(const char* name, const char* value);
	void SetString(int index, const char* value);
	void SetWString(const char* name, const lua_WChar* value);
	void SetWString(int index, const lua_WChar* value);
	void SetUserData(const char* name, void* value);
	void SetUserData(int index, void* value);
	void SetLightUserData(int index, void* value);
	void SetLightUserData(const char* name, void* value);
	void SetObject(const char* name, LuaStackObject& value);
	void SetObject(int index, LuaStackObject& value);

	LuaStackObject GetByName(const char* name);
	LuaStackObject GetByIndex(int index);
	LuaStackObject GetByObject(LuaStackObject& obj);

	LuaStackObject operator[](const char* name) const;
	LuaStackObject operator[](int index) const;
	LuaStackObject operator[](LuaStackObject& obj) const;

//protected:
	friend class LuaState;

	LuaState* m_state;		//!< The parent state of this object.
	int m_stackIndex;		//!< The stack index representing this object.
};


/**
	Representation of a Lua object residing on the Lua stack.
**/
class LUAPLUS_CLASS LuaAutoObject : public LuaStackObject
{
public:
	/**
		Various constructors accepting different parameters.
	**/
//	LuaAutoObject(lua_State* state, int stackIndex) : LuaStackObject(state, stackIndex) {}

	/**
		Various constructors accepting different parameters.
	**/
	LuaAutoObject(LuaState* state, int stackIndex) : LuaStackObject(state, stackIndex) {}

	/**
		Various constructors accepting different parameters.
	**/
	LuaAutoObject(LuaState& state, int stackIndex) : LuaStackObject(state, stackIndex) {}

	/**
		Copy constructor.
	**/
	LuaAutoObject(const LuaStackObject& src) : LuaStackObject(src)
	{
		// No destruction necessary.
	}

	/**
		Assignment operator.
	**/
	const LuaAutoObject& operator=(const LuaStackObject& src);

	/**
		Assignment operator.
	**/
	const LuaAutoObject& operator=(const LuaAutoObject& src)
	{
		m_state = src.m_state;
		m_stackIndex = src.m_stackIndex;
		return *this;
	}

	~LuaAutoObject();
}; // LuaAutoObject


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class LUAPLUS_CLASS LuaStack
{
public:
	LuaStack(LuaState* state) : m_state(state) {}
	LuaStack(lua_State* L);
	~LuaStack() {}

	LuaStackObject operator[](int index)
	{
		return LuaStackObject(m_state, index);
	}

protected:
	LuaState* m_state;
};

} // namespace LuaPlus

#include "LuaState.h"

#ifdef LUAPLUS_ENABLE_INLINES
#include "LuaStackObject.inl"
#endif // LUAPLUS_ENABLE_INLINES

namespace LPCD {
	USING_NAMESPACE_LUA;

	using namespace LuaPlus;

	inline void Push(lua_State* L, LuaStackObject& value)			{  (void)L; value.Push();  }
	inline bool	Match(TypeWrapper<LuaStackObject>, lua_State* L, int idx)
		{  (void)L, (void)idx;  return true;  }
	inline LuaStackObject	Get(TypeWrapper<LuaStackObject>, lua_State* L, int idx)
		{  return LuaStackObject(LuaPlus::LuaState::CastState(L), idx);  }

	inline int LuaStateOldFunctionDispatcher(lua_State* L)
	{
		typedef int (*Functor)(LuaPlus::LuaState*, LuaPlus::LuaStackObject*);
		unsigned char* buffer = LPCD::GetFirstUpValueAsUserData(L);
		Functor& func = *(Functor*)(buffer);
		LuaPlus::LuaState* state = (LuaPlus::LuaState*)lua_getstateuserdata(L);
		int numArgs = lua_gettop(L);
		numArgs = 20;
		LuaPlus::LuaStackObject args[20 + 1];
		for (int i = 1; i <= numArgs; ++i)
		{
			args[i].m_state = state;
			args[i].m_stackIndex = i;
		}
 		return (*func)(state, args);
	}

	template <typename Callee>
	class LuaStateOldMemberDispatcherHelper
	{
	public:
		static inline int LuaStateOldMemberDispatcher(lua_State* L)
		{
			typedef int (Callee::*Functor)(LuaPlus::LuaState*, LuaPlus::LuaStackObject*);
 			unsigned char* buffer = LPCD::GetFirstUpValueAsUserData(L);
			Callee& callee = **(Callee**)buffer;
			Functor& func = *(Functor*)(buffer + sizeof(Callee*));
			LuaPlus::LuaState* state = (LuaPlus::LuaState*)lua_getstateuserdata(L);
			int numArgs = lua_gettop(L);
			numArgs = 20;
			LuaPlus::LuaStackObject args[20 + 1];
			for (int i = 1; i <= numArgs; ++i)
			{
				args[i].m_state = state;
				args[i].m_stackIndex = i;
			}
			return (callee.*func)(state, args);
		}
	};

} // namespace LPCD

#endif // LUASTACKOBJECT_H
