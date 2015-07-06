///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2010 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef BUILDING_LUAPLUS
#define BUILDING_LUAPLUS
#endif
#if defined(WIN32) && !defined(_XBOX) && !defined(_XBOX_VER)
#include <windows.h>
#elif defined(_XBOX) || defined(_XBOX_VER)
#include <xtl.h>
#endif // WIN32

#ifdef WIN32
#ifndef _WIN32_WCE
#include <io.h>
#endif // _WIN32_WCE
#elif defined(__APPLE__)
#include <sys/param.h>
#include <mach-o/dyld.h>
#endif // WIN32

#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lobject.h"
LUA_EXTERN_C_END

#include "LuaPlus.h"
LUA_EXTERN_C_BEGIN
#include "src/lua.h"
#include "src/lauxlib.h"
LUA_EXTERN_C_END

#include <stdlib.h>
#include <assert.h>

USING_NAMESPACE_LUA

#ifdef _MSC_VER
#pragma warning( disable: 4100 )
#endif // _MSC_VER

namespace LuaPlus {

int LS_LuaDumpObject( LuaState* state );
int LS_LuaDumpFile( LuaState* state );
int LS_LuaDumpGlobals(LuaState* state);

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


void ScriptFunctionsRegister(struct lua_State* L)
{
#if LUAPLUS_EXTENSIONS
	LuaState* state = lua_State_To_LuaState(L);
	LuaObject globalsObj = state->GetGlobals();
	globalsObj.Register("GetTickCount",		LS_GetTickCount);

#if defined(_MSC_VER)  &&  defined(WIN32)  &&  !defined(_XBOX)  &&  !defined(_XBOX_VER)  &&  !defined(PLATFORM_PS3)
#elif defined(__APPLE__)
	char modulePath[MAXPATHLEN + 1];
	unsigned int path_len = MAXPATHLEN;
    _NSGetExecutablePath(modulePath, &path_len);
	char* slashPtr = strrchr(modulePath, '/');
	slashPtr++;
	*slashPtr = 0;
#endif // _MSC_VER
#endif // LUAPLUS_EXTENSIONS

#if LUAPLUS_DUMPOBJECT
    state->GetGlobals().Register("LuaDumpGlobals", LS_LuaDumpGlobals);
	state->GetGlobals().Register("LuaDumpObject", LS_LuaDumpObject);
	state->GetGlobals().Register("LuaDumpFile", LS_LuaDumpFile);
#endif // LUAPLUS_DUMPOBJECT
}

} // namespace LuaPlus


LUA_EXTERN_C void LuaPlus_ScriptFunctionsRegister(lua_State* L)
{
	LuaPlus::ScriptFunctionsRegister(L);
}





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
