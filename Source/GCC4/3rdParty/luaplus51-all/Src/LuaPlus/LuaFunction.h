///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAFUNCTION_H
#define LUAFUNCTION_H

#include "LuaPlusInternal.h"
#include "LuaAutoBlock.h"

#if LUAPLUS_EXTENSIONS

namespace LuaPlus {

#define LUAFUNCTION_PRECALL() \
		lua_State* L = functionObj.GetCState(); \
		LuaAutoBlock autoBlock(L); \
		functionObj.Push();

#define LUAFUNCTION_POSTCALL(numArgs) \
		if (lua_pcall(L, numArgs, 1, 0)) { \
			const char* errorString = lua_tostring(L, -1);  (void)errorString; \
			luaplus_assert(0); \
		} \
		return LPCD::Get(LPCD::TypeWrapper<RT>(), L, -1);


/**
**/
template <typename RT>
class LuaFunction
{
public:
	LuaFunction(LuaObject& _functionObj)
		: functionObj(_functionObj) {
		luaplus_assert(functionObj.IsFunction());
	}

	LuaFunction(LuaState* state, const char* functionName) {
		functionObj = state->GetGlobals()[functionName];
		luaplus_assert(functionObj.IsFunction());
	}

	RT operator()() {
		LUAFUNCTION_PRECALL();
		LUAFUNCTION_POSTCALL(0);
	}

	template <typename P1>
	RT operator()(P1 p1) {
		LUAFUNCTION_PRECALL();
		LPCD::Push(L, p1);
		LUAFUNCTION_POSTCALL(1);
	}

	template <typename P1, typename P2>
	RT operator()(P1 p1, P2 p2) {
		LUAFUNCTION_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LUAFUNCTION_POSTCALL(2);
	}

	template <typename P1, typename P2, typename P3>
	RT operator()(P1 p1, P2 p2, P3 p3) {
		LUAFUNCTION_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LUAFUNCTION_POSTCALL(3);
	}

	template <typename P1, typename P2, typename P3, typename P4>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
		LUAFUNCTION_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LUAFUNCTION_POSTCALL(4);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
		LUAFUNCTION_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LUAFUNCTION_POSTCALL(5);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
		LUAFUNCTION_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);
		LUAFUNCTION_POSTCALL(6);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	RT operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
		LUAFUNCTION_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);
		LPCD::Push(L, p7);
		LUAFUNCTION_POSTCALL(7);
	}

protected:
	LuaObject functionObj;
};


#define LUAFUNCTIONVOID_PRECALL() \
		lua_State* L = functionObj.GetCState(); \
		LuaAutoBlock autoBlock(L); \
		functionObj.Push();

#define LUAFUNCTIONVOID_POSTCALL(numArgs) \
		if (lua_pcall(L, numArgs, 1, 0)) { \
			const char* errorString = lua_tostring(L, -1); (void)errorString;\
			luaplus_assert(0); \
		}


/**
**/
class LuaFunctionVoid
{
public:
	LuaFunctionVoid(const LuaObject& functionObj)
		: functionObj(functionObj) {
		luaplus_assert(functionObj.IsFunction());
	}

	LuaFunctionVoid(LuaState* state, const char* functionName) {
		functionObj = state->GetGlobals()[functionName];
		luaplus_assert(functionObj.IsFunction());
	}

	void operator()() {
		LUAFUNCTIONVOID_PRECALL();
		LUAFUNCTIONVOID_POSTCALL(0);
	}

	template <typename P1>
	void operator()(P1 p1) {
		LUAFUNCTIONVOID_PRECALL();
		LPCD::Push(L, p1);
		LUAFUNCTIONVOID_POSTCALL(1);
	}

	template <typename P1, typename P2>
	void operator()(P1 p1, P2 p2) {
		LUAFUNCTIONVOID_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LUAFUNCTIONVOID_POSTCALL(2);
	}

	template <typename P1, typename P2, typename P3>
	void operator()(P1 p1, P2 p2, P3 p3) {
		LUAFUNCTIONVOID_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LUAFUNCTIONVOID_POSTCALL(3);
	}

	template <typename P1, typename P2, typename P3, typename P4>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4) {
		LUAFUNCTIONVOID_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LUAFUNCTIONVOID_POSTCALL(4);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) {
		LUAFUNCTIONVOID_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LUAFUNCTIONVOID_POSTCALL(5);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) {
		LUAFUNCTIONVOID_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);
		LUAFUNCTIONVOID_POSTCALL(6);
	}

	template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7) {
		LUAFUNCTIONVOID_PRECALL();
		LPCD::Push(L, p1);
		LPCD::Push(L, p2);
		LPCD::Push(L, p3);
		LPCD::Push(L, p4);
		LPCD::Push(L, p5);
		LPCD::Push(L, p6);
		LPCD::Push(L, p7);
		LUAFUNCTIONVOID_POSTCALL(7);
	}

protected:
	LuaObject functionObj;
};

} // namespace LuaPlus

#endif // LUAPLUS_EXTENSIONS

#endif // LUAFUNCTION_H
