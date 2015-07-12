///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && !defined(_XBOX)
#include <windows.h>
#elif defined(_XBOX)
#include <xtl.h>
#endif // WIN32

#ifdef WIN32
#ifndef _WIN32_WCE
#include <io.h>
#endif // _WIN32_WCE
#endif // WIN32

#define LUA_CORE

#include "LuaPlus.h"
#include "LuaPlusFunctions.h"
LUA_EXTERN_C_BEGIN
#include "src/lua.h"
#include "src/lauxlib.h"
#include "src/lfunc.h"
#include "src/lobject.h"
#include "src/lstring.h"
#include "src/lmem.h"
#include "src/lopcodes.h"
#include "src/lundump.h"
LUA_EXTERN_C_END

#include <stdlib.h>

USING_NAMESPACE_LUA

#ifdef _MSC_VER
#pragma warning( disable: 4100 )
#endif // _MSC_VER

namespace LuaPlus {

#ifdef WIN32
static unsigned __int64 s_ticksPerSecond;
static float s_secondsPerTick;
#endif

static int LS_GetTickCount( LuaState* state )
{
#ifdef WIN32
	if (s_ticksPerSecond == 0)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&s_ticksPerSecond);
		s_secondsPerTick = 1.0f / (float) (__int64)s_ticksPerSecond;
	}

	unsigned __int64 ticks;
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
	state->PushNumber((double)(__int64)ticks * s_secondsPerTick);
#else
	state->PushNumber(0);
#endif
	return 1;
}


#define LUA_PATH	"LUA_PATH"

#ifndef LUA_PATH_SEP
#define LUA_PATH_SEP	";"
#endif

const char *getpath (lua_State *L) {
  const char *path;
  lua_getglobal(L, LUA_PATH);  /* try global variable */
  path = lua_tostring(L, -1);
  lua_pop(L, 1);
  if (path) return path;
#ifndef _XBOX
  path = getenv(LUA_PATH);  /* else try environment variable */
  if (path) return path;
#endif /* _XBOX */
  return LUA_PATH_DEFAULT;  /* else use default */
}


void ScriptFunctionsRegister( LuaState* state )
{
	LuaObject globalsObj = state->GetGlobals();
	globalsObj.Register("GetTickCount",		LS_GetTickCount);

#if defined(_MSC_VER)  &&  defined(WIN32)  &&  !defined(_XBOX)
#ifdef _DEBUG
	const char* luaplusdllName = "LuaPlusD_1100.dll";
#else // _DEBUG
	const char* luaplusdllName = "LuaPlus_1100.dll";
#endif // _DEBUG

	char modulePath[MAX_PATH];
	GetModuleFileNameA(GetModuleHandleA(luaplusdllName), modulePath, _MAX_PATH);
	char* slashPtr = strrchr( modulePath, '\\' );
	slashPtr++;
	*slashPtr = 0;
#else
	char modulePath[300];
	strcpy(modulePath, "");
#endif // _MSC_VER

    const char* luaPath = getpath(state->GetCState());	(void)luaPath;
	state->PushString("LUA_SOPATH=");
	state->PushString(modulePath);
	state->PushString("modules/");
	state->Concat(3);
	LuaObject modulePathObj(state, -1);
#if !defined(_XBOX)  &&  !defined(_XBOX_VER)  &&  !defined(PLATFORM_PS3)
#if defined(_MSC_VER)
	_putenv(modulePathObj.GetString());
#else
	putenv((char*)modulePathObj.GetString());
#endif // _MSC_VER
#endif // _XBOX
	state->Pop();

	state->PushString("LUA_CPATH=");
	state->PushString(modulePath);
	state->PushString("modules/");
#ifdef _DEBUG
    state->PushString("?.dlld");
#else
    state->PushString("?.dll");
#endif
	state->Concat(4);
	LuaObject modulePath2Obj(state, -1);
#if !defined(_XBOX)  &&  !defined(_XBOX_VER)  &&  !defined(PLATFORM_PS3)
#if defined(_MSC_VER)
	_putenv(modulePath2Obj.GetString());
#else
	putenv((char*)modulePath2Obj.GetString());
#endif // _MSC_VER
#endif // _XBOX
	state->Pop();

	{
		LuaObject packageObj = state->GetGlobal("package");
		LuaObject pathObj = packageObj["path"];

		state->PushString(pathObj.GetString());
		state->PushString(LUA_PATH_SEP);
		state->PushString(modulePath);
		state->PushString("modules/?.lua");
		state->Concat(4);
		LuaObject newPathObj(state, -1);
		packageObj.SetString("path", newPathObj.GetString());
		state->Pop();
	}

	{
		LuaObject packageObj = state->GetGlobal("package");
		LuaObject pathObj = packageObj["cpath"];

		state->PushString(modulePath);
		state->PushString("modules/");
#ifdef _DEBUG
	    state->PushString("?.dlld");
#else
	    state->PushString("?.dll");
#endif
		state->PushString(LUA_PATH_SEP);
		state->PushString(pathObj.GetString());
		state->Concat(5);

		LuaObject newPathObj(state, -1);
		packageObj.SetString("cpath", newPathObj.GetString());
		state->Pop();
	}
}

} // namespace LuaPlus





/*
--[[function io.lines(file, openType)
	if type( file ) == "string" then
		if file:sub( 1, 1 ) == "|" then
			file = io.popen( file:sub( 2 ), openType )
		else
			file = io.open( file, openType )
		end
	end

	return function ()
		return file:read() or (assert(file:close()) and nil), i
	end
end


function DirProcessFiles(path, wildcard, func)
	if type(func) ~= "function" then
		return
	end

	path = path:gsub("/", "\\")
	if path:sub(path:len()) ~= '\\' then
		path = path .. "\\"
	end

	local handle = FileFind.First(path .. wildcard)

	while 1 do
		local fileName = handle:GetFileName()
		local isDirectory = handle:IsDirectory()
		func(path, fileName)
		if not handle:Next() then
			break
		end
	end

	handle:Close()
end


function DirProcessFilesRecursive(path, func, ext)
	if ext then
		ext = ext:lower()
	end

	if type(func) ~= "function" then
		return
	end

	path = path:gsub("/", "\\")
	if path:sub(path:len()) ~= '\\' then
		path = path .. "\\"
	end

	function ReadDirectory(path)
		local handle = FileFind.First(path .. "*.*")

		while handle do
			local fileName = handle:GetFileName()
			local isDirectory = handle:IsDirectory()
			if isDirectory then
				if fileName ~= "." and fileName ~= ".." then
					ReadDirectory(path .. fileName .. "\\")
				end
			else
				if ext then
					local dotPos, endPos, foundExt = fileName:find("%.(%w*)$")
					if foundExt then
						foundExt = foundExt:lower()
						if ext == foundExt then
							func(path, fileName)
						end
					end
				else
					func(path, fileName)
				end
			end
			if not handle:Next() then
				break
			end
		end

		if handle then
			handle:Close()
		end
	end

	ReadDirectory(path)
end
]]
*/
