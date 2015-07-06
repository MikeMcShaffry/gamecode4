///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
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
class LuaStackObject
{
public:
	/**
	**/
	LuaStackObject() :
		L(NULL)
	{
		// Bad to have this... but useful for containers.
	}

	/**
		Various constructors accepting different parameters.
	**/
	LuaStackObject(lua_State* L, int _stackIndex);

	/**
		Various constructors accepting different parameters.
	**/
	LuaStackObject(LuaState* state, int _stackIndex);

	/**
		Copy constructor.
	**/
	LuaStackObject(const LuaStackObject& src)
		: L(src.L)
		, m_stackIndex(src.m_stackIndex)
	{
	}

	/**
		Assignment operator.
	**/
	const LuaStackObject& operator=(const LuaStackObject& src)
	{
		L = src.L;
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
#if LUA_WIDESTRING
	const lua_WChar* GetWString() const;
#endif /* LUA_WIDESTRING */
	size_t StrLen() const;
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
	size_t GetCount();

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
#if LUA_WIDESTRING
	void SetWString(const char* name, const lua_WChar* value);
	void SetWString(int index, const lua_WChar* value);
#endif /* LUA_WIDESTRING */
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

	lua_State* L;			//!< The parent state of this object.
	int m_stackIndex;		//!< The stack index representing this object.
};


/**
	Representation of a Lua object residing on the Lua stack.
**/
class LuaAutoObject : public LuaStackObject
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
		L = src.L;
		m_stackIndex = src.m_stackIndex;
		return *this;
	}

	~LuaAutoObject();
}; // LuaAutoObject


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class LuaStack
{
public:
	LuaStack(LuaState* state);
	LuaStack(lua_State* L);
	~LuaStack();
	int Count() const			{  return m_numStack;  }

	LuaStackObject operator[](int index);

protected:
	lua_State* L;
	int m_numStack;
};

} // namespace LuaPlus

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
		{  return LuaStackObject(lua_State_To_LuaState(L), idx);  }
} // namespace LPCD

#endif // LUASTACKOBJECT_H
