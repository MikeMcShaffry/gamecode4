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
#ifndef LUASTATE_H
#define LUASTATE_H

#include "LuaPlusInternal.h"

void LuaState_UserStateOpen_Internal(lua_State* L);
void LuaState_UserStateThread_Internal(lua_State* L, lua_State* L1);
void LuaState_UserStateFree_Internal(lua_State* L);

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

typedef const char * (*luaplus_Reader) (lua_State *L, void *ud, size_t *sz);

/**
	A lua_State wrapper.
**/
class LUAPLUS_CLASS LuaState
{
public:
	enum DumpObjectTypes
	{
		DUMP_ALPHABETICAL		= 0x00000001,
		DUMP_WRITEALL			= 0x00000002,
		DUMP_WRITETABLEPOINTERS = 0x00000004,
	};


	///////////////////////////////////////////////////////////////////////////
	static LuaState* Create();
//	static LuaState* CreateMT();
	static LuaState* Create(bool initStandardLibrary);
//	static LuaState* CreateMT(bool initStandardLibrary);
	static LuaObject CreateThread(LuaState* parentState);
	static LuaState* CastState(lua_State* L);
	static void Destroy(LuaState* state);

	lua_CFunction AtPanic(lua_CFunction panicf);

	void OpenLibs();

	// Stack functions.
	LuaStackObject Stack(int index);
	LuaStackObject StackTop();
	int GetTop();
	void SetTop(int index);
	void PushValue(int index);
	void PushValue(LuaStackObject& object);
	void Remove(int index);
	void Insert(int index);
	void Replace(int index);
	int CheckStack(int size);
	void XMove(LuaState* to, int n);

	// access functions
	int Equal(int index1, int index2);
	int Equal(const LuaObject& o1, const LuaObject& o2);
	int RawEqual(int index1, int index2);
	int LessThan(int index1, int index2);
	int LessThan(const LuaObject& o1, const LuaObject& o2);

	// push functions (C -> stack)
	LuaStackObject PushNil();
	LuaStackObject PushNumber(lua_Number n);
	LuaStackObject PushInteger(int n);
	LuaStackObject PushLString(const char *s, size_t len);
	LuaStackObject PushLWString(const lua_WChar* s, size_t len);
	LuaStackObject PushString(const char *s);
	LuaStackObject PushWString(const lua_WChar* s);
	LuaStackObject PushVFString(const char* fmt, va_list argp);
	LuaStackObject PushFString(const char* fmt, ...);
	
	LuaStackObject PushCClosure(lua_CFunction fn, int n);
	LuaStackObject PushCClosure(int (*f)(LuaState*), int n);

	LuaStackObject PushCFunction(lua_CFunction f);
	LuaStackObject PushBoolean(bool value);
	LuaStackObject PushLightUserData(void* p);
	LuaStackObject PushThread();

	LuaStackObject CreateTable(int narr = 0, int nrec = 0);
	LuaStackObject NewUserData(size_t size);

	// get functions (Lua -> stack)
	void GetTable(int index);
	void RawGet(int index);
	void RawGetI(int index, int n);
	LuaStackObject GetMetaTable(int objindex);

	// LuaPlus ---->
	LuaObject GetGlobals() throw();
	LuaStackObject GetGlobals_Stack();					// Backward compatible.
	LuaObject GetRegistry();
	LuaStackObject GetRegistry_Stack();
	LuaObject GetGlobal(const char *name);
	LuaStackObject GetGlobal_Stack(const char *name);
	// set functions(stack -> Lua)
	void SetTable(int index);
	void RawSet(int index);
	void RawSetI(int index, int n);
	void SetMetaTable(int index);


	// `load' and `call' functions (load and run Lua code)
	void Call(int nargs, int nresults);
	int PCall(int nargs, int nresults, int errfunc);
	int CPCall(lua_CFunction func, void* ud);
	int Load(luaplus_Reader reader, void *dt, const char *chunkname);
	int WLoad(luaplus_Reader reader, void *data, const char *chunkname);

	int Dump(lua_Chunkwriter writer, void* data, int strip, char endian);

	// coroutine functions
	int CoYield(int nresults);
	int CoResume(int narg);
	int CoStatus();

	// garbage-collection function and options
	int GC(int what, int data);

	// miscellaneous functions
	int Error();

	int Next(int index);

	void Concat(int n);

	lua_Alloc GetAllocF(void **ud);
	void SetAllocF(lua_Alloc f, void *ud);
	
	// Helper functions
	void Pop();
	void Pop(int amount);

	// debug functions
	int GetStack(int level, lua_Debug* ar);
	int GetInfo(const char* what, lua_Debug* ar);
	const char* GetLocal(const lua_Debug* ar, int n);
	const char* SetLocal(const lua_Debug* ar, int n);
	const char* GetUpValue(int funcindex, int n);
	const char* SetUpValue(int funcindex, int n);

	int SetHook(lua_Hook func, int mask, int count);
	lua_Hook GetHook();
	int GetHookMask();
	int GetHookCount();

	// lauxlib functions.
	int TypeError(int narg, const char* tname);
	int ArgError(int numarg, const char* extramsg);
	const char* CheckLString(int numArg, size_t* len);
	const char* OptLString(int numArg, const char *def, size_t* len);
	lua_Number CheckNumber(int numArg);
	lua_Number OptNumber(int nArg, lua_Number def);

	lua_Integer CheckInteger(int numArg);
	lua_Integer OptInteger(int nArg, lua_Integer def);

	void CheckStack(int sz, const char* msg);
	void CheckType(int narg, int t);
	void CheckAny(int narg);

	LuaStackObject NewMetaTable(const char* tname);
	void* CheckUData(int ud, const char* tname);

	int Where(int lvl);
	void Error(const char* fmt, ...);

	int CheckOption(int narg, const char *def, const char *const lst[]);

	int Ref(int t);
	void Unref(int t, int ref);
	
	int LoadFile(const char* filename);
	int LoadBuffer(const char* buff, size_t size, const char* name);
	int LoadString(const char* str);

	const char* GSub(const char *s, const char *p, const char *r);

	const char* FindTable(int idx, const char *fname, int szhint);

	void ArgCheck(bool condition, int numarg, const char* extramsg);
	const char* CheckString(int numArg);
	const char* OptString(int numArg, const char* def);
	int CheckInt(int numArg);
	int OptInt(int numArg, int def);
	long CheckLong(int numArg);
	long OptLong(int numArg, int def);

	int DoFile(const char *filename);
	int DoFile(const char *filename, LuaObject& fenvObj);

	int DoString(const char *str);
	int DoString(const char *str, LuaObject& fenvObj);

	int DoBuffer(const char *buff, size_t size, const char *name);
	int DoBuffer(const char *buff, size_t size, const char *name, LuaObject& fenvObj);

	int LoadWBuffer(const lua_WChar* buff, size_t size, const char* name);
	int LoadWString(const lua_WChar* str);

	int DoWString(const lua_WChar* str, const char* name);
	int DoWBuffer(const lua_WChar* buff, size_t size, const char *name);

	LuaObject NewUserDataBox(void* u);
	LuaStackObject NewUserDataBox_Stack(void* u);

	bool DumpObject(const char* filename, const char* name, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);
	bool DumpObject(const char* filename, LuaObject& key, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);

	bool DumpObject(LuaStateOutFile& file, const char* name, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);
	bool DumpObject(LuaStateOutFile& file, LuaObject& key, LuaObject& value, unsigned int flags = DUMP_ALPHABETICAL,
					int indentLevel = 0, unsigned int maxIndentLevel = 0xffffffff);

	bool DumpGlobals(const char* filename, unsigned int flags = DUMP_ALPHABETICAL, unsigned int maxIndentLevel = 0xFFFFFFFF);
	bool DumpGlobals(LuaStateOutFile& file, unsigned int flags = DUMP_ALPHABETICAL, unsigned int maxIndentLevel = 0xFFFFFFFF);

	operator lua_State*()						{  return m_state;  }
	lua_State* GetCState()						{  return m_state;  }

	// Extra
	LuaStackObject BoxPointer(void* u);
	void* UnBoxPointer(int stackIndex);

	int UpValueIndex(int i);

	LuaObject GetLocalByName( int level, const char* name );

	// LuaPlus ------->
	void SetFEnv( int index );
	LuaStackObject GetDefaultMetaTable(int type);
	void SetDefaultMetaTable(int type);

protected:
	friend class LuaObject;
	friend void ::LuaState_UserStateOpen_Internal(lua_State* L);
	friend void ::LuaState_UserStateThread_Internal(lua_State* L, lua_State* L1);
	friend void ::LuaState_UserStateFree_Internal(lua_State* L);

//	LuaState();
	LuaState(lua_State* L);
	LuaState(lua_State* L, lua_State* L1);
	~LuaState();
	LuaState& operator=(LuaState& src);		// Not implemented.

    static void newthread_handler(lua_State *L, lua_State *L1);
    static void freethread_handler(lua_State *L, lua_State *L1);

	bool CallFormatting(LuaObject& tableObj, LuaStateOutFile& file, int indentLevel,
			bool writeAll, bool alphabetical, bool writeTablePointers,
			unsigned int maxIndentLevel);

	static void LuaPlusGCFunction(void* s);

	lua_State* m_state;
	bool m_ownState;
};


class LuaStateAuto
{
public:
    operator LuaState*()							{  return m_state;  }
    operator const LuaState*() const				{  return m_state;  }
    operator LuaState*() const						{  return m_state;  }
    LuaState& operator*()							{  return *m_state; }
    const LuaState& operator*() const				{  return *m_state; }
    LuaState* operator->()							{  return m_state;  }
    const LuaState* operator->() const				{  return m_state;  }
	LuaState* Get() const							{  return m_state;  }

	LuaStateAuto() : m_state(NULL) {}
    LuaStateAuto(LuaState* newState) : m_state(newState) {}
	LuaStateAuto& operator=(LuaState* newState)
	{
		Assign(newState);
		return *this;
	}

	~LuaStateAuto()
	{
		Assign(NULL);
	}

    LuaState* m_state;

protected:
    LuaStateAuto(const LuaStateAuto&);					// Not implemented.
    LuaStateAuto& operator=(const LuaStateAuto&);		// Not implemented.

	void Assign(LuaState* state)
	{
		if (m_state)
			LuaState::Destroy(m_state);
		m_state = state;
	}
};


class LuaStateOwner : public LuaStateAuto
{
public:
    LuaStateOwner()
	{
		m_state = LuaState::Create();
	}

    LuaStateOwner(bool initStandardLibrary)
	{
		m_state = LuaState::Create(initStandardLibrary);
	}

    LuaStateOwner(bool initStandardLibrary, bool multithreaded)
	{
		if (!multithreaded)
			m_state = LuaState::Create(initStandardLibrary);
//		else
//			m_state = LuaState::CreateMT(initStandardLibrary);
	}

    LuaStateOwner(LuaState* newState) : LuaStateAuto(newState) {}
	LuaStateOwner& operator=(LuaState* newState)
	{
		Assign(newState);
		return *this;
	}

	~LuaStateOwner()
	{
		Assign(NULL);
	}

private:
    LuaStateOwner(const LuaStateOwner&);				// Not implemented.
    LuaStateOwner& operator=(const LuaStateOwner&);		// Not implemented.
};


} // namespace LuaPlus



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifdef LUAPLUS_ENABLE_INLINES
#include "LuaState.inl"
#endif // LUAPLUS_ENABLE_INLINES

namespace LPCD
{
	using namespace LuaPlus;

	inline void Push(lua_State* L, const lua_WChar* value)			{  lua_pushwstring(L, value);  }
	LUAPLUS_API void Push(lua_State* L, int (*value)(LuaState*));

	inline bool	Match(TypeWrapper<const lua_WChar*>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TWSTRING;  }
	inline bool	Match(TypeWrapper<LuaState*>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNONE;  }

	inline const lua_WChar*	Get(TypeWrapper<const lua_WChar*>, lua_State* L, int idx)
		{  return static_cast<const lua_WChar*>(lua_towstring(L, idx));  }
	inline LuaState*		Get(TypeWrapper<LuaState*>, lua_State* L, int /*idx*/)
		{  return LuaState::CastState(L);  }

	inline int LuaStateFunctionDispatcher(lua_State* L)
	{
		typedef int (*Functor)(LuaPlus::LuaState*);
		unsigned char* buffer = LPCD::GetFirstUpValueAsUserData(L);
		Functor& func = *(Functor*)(buffer);
 		return (*func)((LuaPlus::LuaState*)lua_getstateuserdata(L));
	}

	template <typename Callee>
	class LuaStateMemberDispatcherHelper
	{
	public:
		static inline int LuaStateMemberDispatcher(lua_State* L)
		{
			typedef int (Callee::*Functor)(LuaPlus::LuaState*);
 			unsigned char* buffer = LPCD::GetFirstUpValueAsUserData(L);
			Callee& callee = **(Callee**)buffer;
			Functor& func = *(Functor*)(buffer + sizeof(Callee*));
			return (callee.*func)((LuaPlus::LuaState*)lua_getstateuserdata(L));
		}
	};

	template <typename Callee>
	class Object_MemberDispatcher_to_LuaStateHelper
	{
	public:
		static inline int Object_MemberDispatcher_to_LuaState(lua_State* L)
		{
			typedef int (Callee::*Functor)(LuaPlus::LuaState*);
 			unsigned char* buffer = GetFirstUpValueAsUserData(L);
			Functor& func = *(Functor*)(buffer);
			Callee& callee = *(Callee*)GetObjectUserData(L);
			return (callee.*func)(((LuaPlus::LuaState*)lua_getstateuserdata(L)));
		}
	};
} // namespace LPCD


#endif // LUASTATE_H
