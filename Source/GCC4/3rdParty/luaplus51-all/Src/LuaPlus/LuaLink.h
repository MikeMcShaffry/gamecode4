///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
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
			#define LUAPLUS_CLASS_API __declspec(dllexport)
		#else
//			#define LUA_API __declspec(dllimport)
//			#define LUAPLUS_CLASS __declspec(dllimport)
//			#define LUAPLUS_API	__declspec(dllimport)
//			#define LUAPLUS_CLASS_API __declspec(dllimport)
			#define LUA_API
			#define LUAPLUS_CLASS
			#define LUAPLUS_API
			#define LUAPLUS_CLASS_API
		#endif

		#ifdef LUAMODULE_BUILDDLL
			#define LUAMODULE_API __declspec(dllexport)
		#else
			#define LUAMODULE_API __declspec(dllimport)
		#endif
	#else // LUAPLUS_LIB
		#define LUA_API
		#define LUAPLUS_CLASS
		#define LUAPLUS_CLASS_API
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
    #define LUAPLUS_CLASS_API
    #define LUAPLUS_API extern
	#ifdef LUAMODULE_BUILDDLL
		#define LUAMODULE_API
	#else
		#define LUAMODULE_API
	#endif
#endif // _MSC_VER

#if defined(PLATFORM_IOS)
#elif defined(__APPLE__)  ||  defined(macintosh)
#define LUA_USE_MACOSX
#endif

#if defined(LUAPLUS_ALL)  &&  !defined(LUAPLUS_USE_NAMESPACES)
#define NAMESPACE_LUA_BEGIN
#define NAMESPACE_LUA_END
#define USING_NAMESPACE_LUA
#define NAMESPACE_LUA_PREFIX
#define LUA_EXTERN_C
#define LUA_EXTERN_C_BEGIN
#define LUA_EXTERN_C_END
#endif /* LUAPLUS_ALL */

#if 0
#ifdef LUAPLUS_USE_NAMESPACES
#define NAMESPACE_LUA_BEGIN namespace Lua {
#define NAMESPACE_LUA_END } // namespace Lua
#define USING_NAMESPACE_LUA using namespace Lua;
#define NAMESPACE_LUA_PREFIX Lua::
#define LUA_EXTERN_C
#define LUA_EXTERN_C_BEGIN
#define LUA_EXTERN_C_END
#endif /* LUAPLUS_USE_NAMESPACES */
#endif

#if defined(LUAPLUS_CPP)
#define NAMESPACE_LUA_BEGIN
#define NAMESPACE_LUA_END
#define USING_NAMESPACE_LUA
#define NAMESPACE_LUA_PREFIX
#define LUA_EXTERN_C
#define LUA_EXTERN_C_BEGIN
#define LUA_EXTERN_C_END
#endif /* LUAPLUS_ALL */

#ifndef NAMESPACE_LUA_BEGIN
#define NAMESPACE_LUA_BEGIN
#define NAMESPACE_LUA_END
#define USING_NAMESPACE_LUA
#define NAMESPACE_LUA_PREFIX
#if defined(__cplusplus)
#define LUA_EXTERN_C extern "C"
#define LUA_EXTERN_C_BEGIN extern "C" {
#define LUA_EXTERN_C_END }
#else
#define LUA_EXTERN_C
#define LUA_EXTERN_C_BEGIN
#define LUA_EXTERN_C_END
#endif
#endif /* NAMESPACE_LUA_BEGIN */

#endif /* LUALINK_H */

