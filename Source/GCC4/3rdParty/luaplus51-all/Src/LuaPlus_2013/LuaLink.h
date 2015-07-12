///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2007 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUALINK_H
#define LUALINK_H

#ifndef LUAPLUS_DLL
#ifndef LUAPLUS_LIB
#define LUAPLUS_LIB
#endif // LUAPLUS_LIB
#endif // LUAPLUS_DLL

#ifndef LUAPLUS_INCLUDE_STANDARD_LIBRARY
#define LUAPLUS_INCLUDE_STANDARD_LIBRARY 1
#endif // LUAPLUS_INCLUDE_STANDARD_LIBRARY

#undef LUA_API
#undef LUALIB_API

#ifdef _MSC_VER
	#ifndef LUAPLUS_LIB
		#ifdef LUAPLUS_BUILDDLL
			#define LUA_API __declspec(dllexport)
			#define LUAPLUS_CLASS __declspec(dllexport)
			#define LUAPLUS_API	__declspec(dllexport)
		#else
			#define LUA_API __declspec(dllimport)
			#define LUAPLUS_CLASS __declspec(dllimport)
			#define LUAPLUS_API	__declspec(dllimport)
			#ifdef _DEBUG
				#pragma comment(lib, "LuaPlusD_1100.lib")
			#else // _DEBUG
				#pragma comment(lib, "LuaPlus_1100.lib")
			#endif // _DEBUG
		#endif

		#ifdef LUAMODULE_BUILDDLL
			#define LUAMODULE_API __declspec(dllexport)
		#else
			#define LUAMODULE_API __declspec(dllimport)
		#endif
	#else //!LUASTATEDLL
		#define LUA_API
		#define LUAPLUS_CLASS
		#define LUAPLUS_API extern
		#define LUAMODULE_API 
	#endif
#else // !_MSC_VER
	#ifdef __cplusplus
		#define LUA_API extern "C"
	#else
		#define LUA_API extern
	#endif
    #define LUAPLUS_CLASS
    #define LUAPLUS_API extern
#endif // _MSC_VER

#if defined(__APPLE__)  ||  defined(macintosh)
#define LUA_USE_MACOSX
#endif

#ifdef LUAPLUS_USE_NAMESPACES
#define NAMESPACE_LUA_BEGIN namespace Lua {
#define NAMESPACE_LUA_END } // namespace Lua
#define USING_NAMESPACE_LUA using namespace Lua;
#define NAMESPACE_LUA_PREFIX Lua::
#define LUA_EXTERN_C
#define LUA_EXTERN_C_BEGIN
#define LUA_EXTERN_C_END
#endif /* LUAPLUS_USE_NAMESPACES */

#ifndef NAMESPACE_LUA_BEGIN
#define NAMESPACE_LUA_BEGIN
#define NAMESPACE_LUA_END
#define USING_NAMESPACE_LUA
#define NAMESPACE_LUA_PREFIX
#define LUA_EXTERN_C extern "C"
#ifdef __cplusplus
#define LUA_EXTERN_C_BEGIN extern "C" {
#define LUA_EXTERN_C_END }
#else
#define LUA_EXTERN_C_BEGIN
#define LUA_EXTERN_C_END
#endif
#endif /* NAMESPACE_LUA_BEGIN */

#define LUALIB_API LUA_API

#endif /* LUALINK_H */

