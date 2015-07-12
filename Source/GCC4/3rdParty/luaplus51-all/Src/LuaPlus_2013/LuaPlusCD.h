///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2007 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUSCD_H
#define LUAPLUSCD_H

#include "LuaLink.h"

LUA_EXTERN_C_BEGIN
#include "src/lua.h"
#include "src/lauxlib.h"
LUA_EXTERN_C_END

#include <stdlib.h>
#include <string.h>

// LuaPlus Call Dispatcher
namespace LPCD
{
	struct LuaLightUserData
	{
		LuaLightUserData(const void* value) :
			m_value(value)
		{
		}

		const void* m_value;
	};


	struct LuaUserData
	{
		LuaUserData(const void* value) :
			m_value(value)
		{
		}

		const void* m_value;
	};

	struct LuaNil
	{
	};

	inline void Push(lua_State* L, bool value)				{  lua_pushboolean(L, value);  }
	inline void Push(lua_State* L, char value)				{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, unsigned char value)		{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, short value)				{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, unsigned short value)	{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, int value)				{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, unsigned int value)		{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, long value)				{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, unsigned long value)		{  lua_pushnumber(L, value);  }
	inline void Push(lua_State* L, double value)			{  lua_pushnumber(L, (lua_Number)value);  }
	inline void Push(lua_State* L, float value)				{  lua_pushnumber(L, (lua_Number)value);  }
	inline void Push(lua_State* L, const char* value)		{  lua_pushstring(L, value);  }
	inline void Push(lua_State* L, const LuaNil&)			{  lua_pushnil(L);  }
	inline void Push(lua_State* L, lua_CFunction value)		{  lua_pushcclosure(L, value, 0);  }
	inline void Push(lua_State* L, const void* value)		{  lua_pushlightuserdata(L, (void*)value);  }
	inline void Push(lua_State* L, const LuaLightUserData& value)	{  lua_pushlightuserdata(L, (void*)value.m_value);  }
    inline void Push(lua_State* L, const LuaUserData& value){  lua_boxpointer(L, (void*)value.m_value);  }

	template<class T> struct TypeWrapper {};

	inline bool	Match(TypeWrapper<bool>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TBOOLEAN;  }
	inline bool	Match(TypeWrapper<char>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<unsigned char>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<short>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<unsigned short>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<int>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<unsigned int>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<long>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<unsigned long>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<float>, lua_State* L, int idx)
		{  int type = lua_type(L, idx);  return type == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<double>, lua_State* L, int idx)
		{  int type = lua_type(L, idx);  return type == LUA_TNUMBER;  }
	inline bool	Match(TypeWrapper<const char*>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TSTRING;  }
	inline bool	Match(TypeWrapper<lua_State*>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TNONE;  }
	inline bool	Match(TypeWrapper<void*>, lua_State* L, int idx)
		{  return lua_type(L, idx) == LUA_TLIGHTUSERDATA;  }

	inline void				Get(TypeWrapper<void>, lua_State*, int)
		{  }
	inline bool				Get(TypeWrapper<bool>, lua_State* L, int idx)
		{  return lua_toboolean(L, idx) != 0;  }
	inline char				Get(TypeWrapper<char>, lua_State* L, int idx)
		{  return static_cast<char>(lua_tonumber(L, idx));  }
	inline unsigned char	Get(TypeWrapper<unsigned char>, lua_State* L, int idx)
		{  return static_cast<unsigned char>(lua_tonumber(L, idx));  }
	inline short			Get(TypeWrapper<short>, lua_State* L, int idx)
		{  return static_cast<short>(lua_tonumber(L, idx));  }
	inline unsigned short	Get(TypeWrapper<unsigned short>, lua_State* L, int idx)
		{  return static_cast<unsigned short>(lua_tonumber(L, idx));  }
	inline int				Get(TypeWrapper<int>, lua_State* L, int idx)
		{  return static_cast<int>(lua_tonumber(L, idx));  }
	inline unsigned int		Get(TypeWrapper<unsigned int>, lua_State* L, int idx)
		{  return static_cast<unsigned int>(lua_tonumber(L, idx));  }
	inline long				Get(TypeWrapper<long>, lua_State* L, int idx)
		{  return static_cast<long>(lua_tonumber(L, idx));  }
	inline unsigned long	Get(TypeWrapper<unsigned long>, lua_State* L, int idx)
		{  return static_cast<unsigned long>(lua_tonumber(L, idx));  }
	inline float			Get(TypeWrapper<float>, lua_State* L, int idx)
		{  return static_cast<float>(lua_tonumber(L, idx));  }
	inline double			Get(TypeWrapper<double>, lua_State* L, int idx)
		{  return static_cast<double>(lua_tonumber(L, idx));  }
	inline const char*		Get(TypeWrapper<const char*>, lua_State* L, int idx)
		{  return static_cast<const char*>(lua_tostring(L, idx));  }
	inline LuaNil			Get(TypeWrapper<LuaNil>, lua_State* L, int idx)
		{  (void)L, (void)idx;  return LuaNil();  }
	inline lua_CFunction	Get(TypeWrapper<lua_CFunction>, lua_State* L, int idx)
		{  return static_cast<lua_CFunction>(lua_tocfunction(L, idx));  }
	inline void*			Get(TypeWrapper<void*>, lua_State* L, int idx)
		{  return static_cast<void*>(lua_touserdata(L, idx));  }
	inline lua_State*		Get(TypeWrapper<lua_State*>, lua_State* L, int /*idx*/)
		{  return L;  }

	//////////////////////////////////////////////////////////////////////////

	#define luaL_argassert(arg, _index_) if (!Match(TypeWrapper<P##arg>(), L, _index_)) \
				luaL_argerror(L, _index_, "bad argument")

	template<class RT>
	struct ReturnSpecialization
	{
		static int Call(RT (*func)(), lua_State* L, int /*index*/)
		{
			RT ret = func();
			Push(L, ret);
			return 1;
		}


		template <typename P1>
		static int Call(RT (*func)(P1), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);

			RT ret = func(
				Get(TypeWrapper<P1>(), L, index + 0)
			);
			Push(L, ret);
			return 1;
		}


		template <typename P1, typename P2>
		static int Call(RT (*func)(P1, P2), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);

			RT ret = func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1)
			);
			Push(L, ret);
			return 1;
		}


		template <typename P1, typename P2, typename P3>
		static int Call(RT (*func)(P1, P2, P3), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);

			RT ret = func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2)
			);
			Push(L, ret);
			return 1;
		}


		template <typename P1, typename P2, typename P3, typename P4>
		static int Call(RT (*func)(P1, P2, P3, P4), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);

			RT ret = func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3)
			);
			Push(L, ret);
			return 1;
		}


		template <typename P1, typename P2, typename P3, typename P4,
					typename P5>
		static int Call(RT (*func)(P1, P2, P3, P4, P5), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);

			RT ret = func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4)
			);
			Push(L, ret);
			return 1;
		}


		template <typename P1, typename P2, typename P3, typename P4,
					typename P5, typename P6>
		static int Call(RT (*func)(P1, P2, P3, P4, P5, P6), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);

			RT ret = func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5)
			);
			Push(L, ret);
			return 1;
		}


		template <typename P1, typename P2, typename P3, typename P4,
					typename P5, typename P6, typename P7>
		static int Call(RT (*func)(P1, P2, P3, P4, P5, P6, P7), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);
			luaL_argassert(7, index + 6);

			RT ret = func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5),
				Get(TypeWrapper<P7>(), L, index + 6)
			);
			Push(L, ret);
			return 1;
		}


		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		template <typename Callee>
		static int Call(Callee& callee, RT (Callee::*func)(), lua_State* L, int /*index*/)
		{
			RT ret = (callee.*func)();
			Push(L, ret);
			return 1;
		}


		template <typename Callee, typename P1>
		static int Call(Callee& callee, RT (Callee::*func)(P1), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);

			RT ret = (callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0)
			);
			Push(L, ret);
			return 1;
		}


		template <typename Callee, typename P1, typename P2>
		static int Call(Callee& callee, RT (Callee::*func)(P1, P2), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);

			RT ret = (callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1)
			);
			Push(L, ret);
			return 1;
		}

		template <typename Callee, typename P1, typename P2, typename P3>
		static int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);

			RT ret = (callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2)
			);
			Push(L, ret);
			return 1;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4>
		static int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);

			RT ret = (callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3)
			);
			Push(L, ret);
			return 1;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4, typename P5>
		static int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4, P5), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);

			RT ret = (callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4)
			);
			Push(L, ret);
			return 1;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4, typename P5, typename P6>
		static int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4, P5, P6), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);

			RT ret = (callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5)
			);
			Push(L, ret);
			return 1;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4, typename P5, typename P6, typename P7>
		static int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4, P5, P6, P7), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);
			luaL_argassert(7, index + 6);

			RT ret = (callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5),
				Get(TypeWrapper<P7>(), L, index + 6)
			);
			Push(L, ret);
			return 1;
		}
	};


	template<>
	struct ReturnSpecialization<void>
	{
		static int Call(void (*func)(), lua_State* L, int /*index*/)
		{
			(void)L;
			func();
			return 0;
		}


		template <typename P1>
		static int Call(void (*func)(P1), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);

			func(
				Get(TypeWrapper<P1>(), L, index + 0)
			);
			return 0;
		}


		template <typename P1, typename P2>
		static int Call(void (*func)(P1, P2), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);

			func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1)
			);
			return 0;
		}


		template <typename P1, typename P2, typename P3>
		static int Call(void (*func)(P1, P2, P3), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);

			func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2)
			);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4>
		static int Call(void (*func)(P1, P2, P3, P4), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);

			func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3)
			);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		static int Call(void (*func)(P1, P2, P3, P4, P5), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);

			func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4)
			);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
		static int Call(void (*func)(P1, P2, P3, P4, P5, P6), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);

			func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5)
			);
			return 0;
		}

		template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
		static int Call(void (*func)(P1, P2, P3, P4, P5, P6, P7), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);
			luaL_argassert(7, index + 6);

			func(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5),
				Get(TypeWrapper<P7>(), L, index + 6)
			);
			return 0;
		}

		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		template <typename Callee>
		static int Call(Callee& callee, void (Callee::*func)(), lua_State* /*L*/, int /*index*/)
		{
			(callee.*func)();
			return 0;
		}


		template <typename Callee, typename P1>
		static int Call(Callee& callee, void (Callee::*func)(P1), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);

			(callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0)
			);
			return 0;
		}


		template <typename Callee, typename P1, typename P2>
		static int Call(Callee& callee, void (Callee::*func)(P1, P2), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);

			(callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1)
			);
			return 0;
		}

		template <typename Callee, typename P1, typename P2, typename P3>
		static int Call(Callee& callee, void (Callee::*func)(P1, P2, P3), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);

			(callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2)
			);
			return 0;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4>
		static int Call(Callee& callee, void (Callee::*func)(P1, P2, P3, P4), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);

			(callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3)
			);
			return 0;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4, typename P5>
		static int Call(Callee& callee, void (Callee::*func)(P1, P2, P3, P4, P5), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);

			(callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4)
			);
			return 0;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4, typename P5, typename P6>
		static int Call(Callee& callee, void (Callee::*func)(P1, P2, P3, P4, P5, P6), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);

			(callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5)
			);
			return 0;
		}

		template <typename Callee, typename P1, typename P2, typename P3,
					typename P4, typename P5, typename P6, typename P7>
		static int Call(Callee& callee, void (Callee::*func)(P1, P2, P3, P4, P5, P6, P7), lua_State* L, int index)
		{
			luaL_argassert(1, index + 0);
			luaL_argassert(2, index + 1);
			luaL_argassert(3, index + 2);
			luaL_argassert(4, index + 3);
			luaL_argassert(5, index + 4);
			luaL_argassert(6, index + 5);
			luaL_argassert(7, index + 6);

			(callee.*func)(
				Get(TypeWrapper<P1>(), L, index + 0),
				Get(TypeWrapper<P2>(), L, index + 1),
				Get(TypeWrapper<P3>(), L, index + 2),
				Get(TypeWrapper<P4>(), L, index + 3),
				Get(TypeWrapper<P5>(), L, index + 4),
				Get(TypeWrapper<P6>(), L, index + 5),
				Get(TypeWrapper<P7>(), L, index + 6)
			);
			return 0;
		}
	};


	template <typename RT>
	int Call(RT (*func)(), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename RT, typename P1>
	int Call(RT (*func)(P1), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename RT, typename P1, typename P2>
	int Call(RT (*func)(P1, P2), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename RT, typename P1, typename P2, typename P3>
	int Call(RT (*func)(P1, P2, P3), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename RT, typename P1, typename P2, typename P3, typename P4>
	int Call(RT (*func)(P1, P2, P3, P4), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename RT, typename P1, typename P2, typename P3, typename P4,
				typename P5>
	int Call(RT (*func)(P1, P2, P3, P4, P5), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename RT, typename P1, typename P2, typename P3, typename P4,
				typename P5, typename P6>
	int Call(RT (*func)(P1, P2, P3, P4, P5, P6), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename RT, typename P1, typename P2, typename P3, typename P4,
				typename P5, typename P6, typename P7>
	int Call(RT (*func)(P1, P2, P3, P4, P5, P6, P7), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(func, L, index);
	}


	template <typename Callee, typename RT>
	int Call(Callee& callee, RT (Callee::*func)(), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	template <typename Callee, typename RT, typename P1>
	int Call(Callee& callee, RT (Callee::*func)(P1), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	template <typename Callee, typename RT, typename P1, typename P2>
	int Call(Callee& callee, RT (Callee::*func)(P1, P2), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	template <typename Callee, typename RT, typename P1, typename P2, typename P3>
	int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	template <typename Callee, typename RT, typename P1, typename P2, typename P3,
				typename P4>
	int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	template <typename Callee, typename RT, typename P1, typename P2, typename P3,
				typename P4, typename P5>
	int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4, P5), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	template <typename Callee, typename RT, typename P1, typename P2, typename P3,
				typename P4, typename P5, typename P6>
	int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4, P5, P6), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	template <typename Callee, typename RT, typename P1, typename P2, typename P3,
				typename P4, typename P5, typename P6, typename P7>
	int Call(Callee& callee, RT (Callee::*func)(P1, P2, P3, P4, P5, P6, P7), lua_State* L, int index)
	{
		return ReturnSpecialization<RT>::Call(callee, func, L, index);
	}


	inline unsigned char* GetFirstUpValueAsUserData(lua_State* L)
	{
		void* buffer;

#ifndef FAST_DISPATCH
		buffer = lua_touserdata(L, lua_upvalueindex(1));
#else // FAST_DISPATCH
#endif // FAST_DISPATCH

		return (unsigned char*)buffer;
	}


	// The Helper class is needed for less fortunate compiler template implementations.
	template <typename Func>
	class DirectCallFunctionDispatchHelper
	{
	public:
		static inline int DirectCallFunctionDispatcher(lua_State* L)
		{
 			unsigned char* buffer = GetFirstUpValueAsUserData(L);
			return Call(*(Func*)(buffer), L, 1);
		}
	};


	template <typename Callee, typename Func>
	class DirectCallMemberDispatcherHelper
	{
	public:
		static inline int DirectCallMemberDispatcher(lua_State* L)
		{
 			unsigned char* buffer = GetFirstUpValueAsUserData(L);
			return Call(**(Callee**)buffer, *(Func*)(buffer + sizeof(Callee*)), L, 1);
		}
	};

	inline int lua_StateFunctionDispatcher(lua_State* L)
	{
		typedef int (*Functor)(lua_State*);
 		unsigned char* buffer = GetFirstUpValueAsUserData(L);
		Functor& func = *(Functor*)(buffer);
		return (*func)(L);
	}


	template <typename Callee>
	class lua_StateMemberDispatcherHelper
	{
	public:
		static inline int lua_StateMemberDispatcher(lua_State* L)
		{
			typedef int (Callee::*Functor)(lua_State*);
 			unsigned char* buffer = GetFirstUpValueAsUserData(L);
			Callee& callee = **(Callee**)buffer;
			Functor& func = *(Functor*)(buffer + sizeof(Callee*));
			return (callee.*func)(L);
		}
	};
} // namespace LPCD


template <typename Func>
inline void lua_pushdirectclosure(lua_State* L, Func func, unsigned int nupvalues)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(func));
	memcpy(buffer, &func, sizeof(func));
	lua_pushcclosure(L, LPCD::DirectCallFunctionDispatchHelper<Func>::DirectCallFunctionDispatcher, nupvalues + 1);
}


template <typename Callee, typename Func>
inline void lua_pushdirectclosure(lua_State* L, const Callee& callee, Func func, unsigned int nupvalues)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(Callee*) + sizeof(func));
	const void* pCallee = &callee;
	memcpy(buffer, &pCallee, sizeof(Callee*));
	memcpy(buffer + sizeof(Callee*), &func, sizeof(func));
	lua_pushcclosure(L, LPCD::DirectCallMemberDispatcherHelper<Callee, Func>::DirectCallMemberDispatcher, nupvalues + 1);
}


inline void lua_pushfunctorclosure(lua_State* L, int (*func)(lua_State*), unsigned int nupvalues)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(func));
	memcpy(buffer, &func, sizeof(func));
	lua_pushcclosure(L, LPCD::lua_StateFunctionDispatcher, nupvalues + 1);
}


template <typename Callee>
inline void lua_pushfunctorclosureex(lua_State* L, const Callee& callee, int (Callee::*func)(lua_State*), unsigned int nupvalues)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(Callee*) + sizeof(func));
	const void* pCallee = &callee;
	memcpy(buffer, &pCallee, sizeof(Callee*));
	memcpy(buffer + sizeof(Callee*), &func, sizeof(func));
	lua_pushcclosure(L, LPCD::lua_StateMemberDispatcherHelper<Callee>::lua_StateMemberDispatcher, nupvalues + 1);
}


///////////////////////////////////////////////////////////////////////////////

namespace LPCD
{
	inline void* GetObjectUserData(lua_State* L)
	{
		int type = lua_type(L, 1);
		if (type == LUA_TUSERDATA)
			return lua_unboxpointer(L, 1);
		else if (type == LUA_TTABLE)
		{
			lua_pushvalue(L, 1);
			lua_pushstring(L, "__object");
			lua_rawget(L, -2);
			if (!lua_isuserdata(L, -1))
				luaL_error(L, "The table does not have a userdata member called __object.");
			void* ret = lua_touserdata(L, -1);
			lua_pop(L, 1);
			return ret;
		}
		else
		{
			luaL_argerror(L, 1, "must be userdata or a table with a userdata member called __object");
		}

		return NULL;
	}

	template <typename Callee>
	class Object_MemberDispatcherHelper
	{
	public:
		static inline int Object_MemberDispatcher(lua_State* L)
		{
			typedef int (Callee::*Functor)(lua_State*);
 			unsigned char* buffer = GetFirstUpValueAsUserData(L);
			Functor& func = *(Functor*)(buffer);
			Callee& callee = *(Callee*)GetObjectUserData(L);
			return (callee.*func)(L);
		}
	};

	template <typename Callee, typename Func, int startIndex>
	class DirectCallObjectMemberDispatcherHelper
	{
	public:
		static inline int DirectCallMemberDispatcher(lua_State* L)
		{
 			unsigned char* buffer = GetFirstUpValueAsUserData(L);
			Callee& callee = *(Callee*)GetObjectUserData(L);
			return Call(callee, *(Func*)buffer, L, startIndex);
		}
	};

	inline int PropertyMetaTable_newindex(lua_State* L)
	{
													// t k v
		lua_getmetatable(L, 1);						// t k v m
		lua_pushstring(L, "__props");				// t k v m p
		lua_rawget(L, -2);							// t k v m pt
		if (lua_istable(L, -1))
		{
			lua_pushvalue(L, 2);					// t k v m pt k
			lua_rawget(L, -2);						// t k v m pt prop
			if (lua_isnil(L, -1))
			{
				luaL_argerror(L, 1, "The property is not available.");
			}

			lua_rawgeti(L, -1, 2);					// t k v m pt prop setf
			lua_pushvalue(L, 1);					// t k v m pt prop setf t
			lua_pushvalue(L, 3);					// t k v m pt prop setf t v
			lua_call(L, 2, 1);
			return 1;
		}

		return 0;
	}

	// function gettable_event (table, key)
	inline int PropertyMetaTable_index(lua_State* L)
	{
													// t v
		lua_getmetatable(L, 1);						// t v m
		lua_pushvalue(L, 2);						// t v m v
		lua_rawget(L, -2);							// t v m lookup
		if (!lua_isnil(L, -1))
			return 1;

		lua_pop(L, 1);								// t v m
		lua_pushstring(L, "__props");				// t k v m __props
		lua_rawget(L, -2);							// t k v m pt

		if (lua_istable(L, -1))
		{
			lua_pushvalue(L, 2);					// t k v m pt k
			lua_rawget(L, -2);						// t k v m pt prop
			if (lua_isnil(L, -1))
			{
				luaL_argerror(L, 1, "The property is not available.");
			}

			lua_rawgeti(L, -1, 1);					// t k v m pt prop getf
			lua_pushvalue(L, 1);					// t k v m pt prop getf t
			lua_call(L, 1, 1);
			return 1;
		}

		return 0;
	}

	template <typename Object, typename VarType>
	class PropertyMemberHelper
	{
	public:
		static int PropertyGet(lua_State* L)
		{
			void* offset = lua_touserdata(L, lua_upvalueindex(1));

			Object* obj = (Object*)LPCD::GetObjectUserData(L);

			LPCD::Push(L, *(VarType*)((unsigned char*)obj + (unsigned int)offset));

			return 1;
		}

		static int PropertySet(lua_State* L)
		{
			void* offset = lua_touserdata(L, lua_upvalueindex(1));

			Object* obj = (Object*)LPCD::GetObjectUserData(L);

			if (!Match(TypeWrapper<VarType>(), L, 2))
				luaL_argerror(L, 2, "bad argument");

			*(VarType*)((unsigned char*)obj + (unsigned int)offset) = LPCD::Get(LPCD::TypeWrapper<VarType>(), L, 2);

			return 1;
		}
	};

	template <typename VarType>
	class PropertyGlobalHelper
	{
	public:
		static int PropertyGet(lua_State* L)
		{
			void* offset = lua_touserdata(L, lua_upvalueindex(1));

			LPCD::Push(L, *(VarType*)offset);

			return 1;
		}

		static int PropertySet(lua_State* L)
		{
			void* offset = lua_touserdata(L, lua_upvalueindex(1));

			if (!Match(TypeWrapper<VarType>(), L, 1))
				luaL_argerror(L, 2, "bad argument");

			*(VarType*)offset = LPCD::Get(LPCD::TypeWrapper<VarType>(), L, 1);

			return 1;
		}
	};
} // namespace LPCD


template <typename Callee>
inline void lua_pushobjectfunctorclosure(lua_State* L, int (Callee::*func)(lua_State*), unsigned int nupvalues)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(func));
	memcpy(buffer, &func, sizeof(func));
	lua_pushcclosure(L, LPCD::Object_MemberDispatcherHelper<Callee>::Object_MemberDispatcher, nupvalues + 1);
}


template <typename Callee, typename Func>
inline void lua_pushobjectdirectclosure(lua_State* L, const Callee* callee, Func func, unsigned int nupvalues)
{
	unsigned char* buffer = (unsigned char*)lua_newuserdata(L, sizeof(func));
	memcpy(buffer, &func, sizeof(func));
	lua_pushcclosure(L, LPCD::DirectCallObjectMemberDispatcherHelper<Callee, Func, 2>::DirectCallMemberDispatcher, nupvalues + 1);
}


template <typename Object, typename VarType>
inline void lpcd_pushmemberpropertygetclosure(lua_State* L, VarType Object::* var)
{
	lua_pushlightuserdata(L, (void*)&(((Object*)0)->*var));
	lua_pushcclosure(L, &LPCD::PropertyMemberHelper<Object, VarType>::PropertyGet, 1);
}


template <typename Object, typename VarType>
inline void lpcd_pushmemberpropertysetclosure(lua_State* L, VarType Object::* var)
{
	lua_pushlightuserdata(L, (void*)&(((Object*)0)->*var));
	lua_pushcclosure(L, &LPCD::PropertyMemberHelper<Object, VarType>::PropertySet, 1);
}


template <typename VarType>
inline void lpcd_pushglobalpropertygetclosure(lua_State* L, VarType* var)
{
	lua_pushlightuserdata(L, (void*)var);
	lua_pushcclosure(L, &LPCD::PropertyGlobalHelper<VarType>::PropertyGet, 1);
}


template <typename VarType>
inline void lpcd_pushglobalpropertysetclosure(lua_State* L, VarType* var)
{
	lua_pushlightuserdata(L, (void*)var);
	lua_pushcclosure(L, &LPCD::PropertyGlobalHelper<VarType>::PropertySet, 1);
}

#endif // LUAPLUS_LPCD_H

