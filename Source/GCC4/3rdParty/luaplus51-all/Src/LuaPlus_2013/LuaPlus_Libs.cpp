///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2005 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://wwhiz.com/LuaPlus/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define BUILDING_LUAPLUS
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#define LUA_CORE
#include "src/lobject.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaCall.h"
#include <string.h>
#ifdef WIN32
#if defined(WIN32) && !defined(_XBOX) && !defined(_WIN32_WCE)
#include <windows.h>
#elif defined(_XBOX)
#include <xtl.h>
#endif // WIN32
#undef GetObject
#endif // WIN32
#if defined(__GNUC__)
	#include <new>
#else
	#include <new.h>
#endif

#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

#include "LuaPlusFunctions.h"

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif // _MSC_VER

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN
#include "src/lualib.h"
#include "src/lstate.h"
LUA_EXTERN_C_END

namespace LuaPlus {

/*static*/ LuaState* LuaState::Create(bool initStandardLibrary)
{
	LuaState* state = LuaState::Create();
	if (initStandardLibrary)
		state->OpenLibs();
	return state;
}

#if 0
/*static*/ LuaState* LuaState::CreateMT()
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "LuaState", 0);
	::new(state) LuaState(true);
	return state;
}
#endif

#if 0
/**
**/
LuaState::LuaState(bool multithreaded)
{
	m_state = lua_newstate(luaHelper_defaultAlloc, luaHelper_ud);
	m_ownState = true;

	lua_atpanic(m_state, FatalError);

//jj    luaX_setnewthreadhandler( newthread_handler );
//jj    luaX_setfreethreadhandler( freethread_handler );

#ifdef LUA_MTSUPPORT
	if (multithreaded)
	{
/*#ifdef WIN32
		// What about clean up?
		CRITICAL_SECTION* cs = new CRITICAL_SECTION;
		::InitializeCriticalSection(cs);
		lua_setlockfunctions(m_state, LSLock, LSUnlock, cs);
#endif // WIN32*/
	}
#endif // LUA_MTSUPPORT

//    luastateopen_thread(m_state);
}
#endif

#if 0
/*static*/ LuaState* LuaState::CreateMT(bool initStandardLibrary)
{
	lua_Alloc reallocFunc;
	void* data;
	lua_getdefaultallocfunction(&reallocFunc, &data);
	LuaState* state = (LuaState*)(*reallocFunc)(data, NULL, 0, sizeof(LuaState), "LuaState", 0);
	::new(state) LuaState(true);
	state->OpenLibs();
	return state;
}
#endif

LuaObject LuaState::CreateThread(LuaState* parentState)
{
    lua_State* L1 = lua_newthread(parentState->GetCState());
    lua_TValue tobject;
    setnilvalue2n(L1, &tobject);
    setthvalue(parentState->GetCState(), &tobject, L1);

	LuaObject retObj = LuaObject((LuaState*)lua_getstateuserdata(L1), &tobject);
    setnilvalue(&tobject);
    lua_pop(parentState->GetCState(), 1);
    return retObj;
}


static int LS_LOG( lua_State* L )
{
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "towstring");
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s = NULL;
		const lua_WChar *ws = NULL;
		if (lua_type(L, i) == LUA_TSTRING)
		{
			s = lua_tostring(L, -1);
		}
		else if (lua_type(L, i) != LUA_TWSTRING)
		{
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  /* get result */
			if (s == NULL)
				return luaL_error(L, "`tostring' must return a string to `print'");
		}
		else
		{
			lua_pushvalue(L, -2);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			ws = lua_towstring(L, -1);  /* get result */
			if (ws == NULL)
				return luaL_error(L, "`tostring' must return a string to `print'");
		}
		if (i>1)
		{
#ifdef WIN32
			OutputDebugStringA("\t");
#else
			fputs("\t", stdout);
#endif
		}
		if (s)
		{
#ifdef WIN32
			OutputDebugStringA(s);
#else
			fputs(s, stdout);
#endif
		}
		else if (ws)
		{
            wchar_t out[512];
            wchar_t* outEnd = out + sizeof(out) - 2;
            while (*ws) {
                wchar_t* outPos = out;
                while (*ws && outPos != outEnd) {
                *outPos++ = *ws++;
                }
                *outPos++ = 0;
#ifdef WIN32
			    OutputDebugStringW(out);
#else
    			fputws(out, stdout);
#endif
            }
		}
		lua_pop(L, 1);  /* pop result */
	}

#ifdef WIN32
	OutputDebugStringA("\n");
#else
	fputs("\n", stdout);
#endif

	return 0;
}


static int LS_ALERT( lua_State* L )
{
	const char* err = lua_tostring(L, 1);
#ifdef WIN32
	OutputDebugStringA(err);
    OutputDebugStringA("\n");
#else // !WIN32
	puts(err);
#endif // WIN32

	return 0;
}


#if 0

/**
**/
static void LSLock(void* data)
{
#ifdef WIN32
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)data;
	::EnterCriticalSection(cs);
#endif // WIN32
}


/**
**/
static void LSUnlock(void* data)
{
#ifdef WIN32
	CRITICAL_SECTION* cs = (CRITICAL_SECTION*)data;
	::LeaveCriticalSection(cs);
#endif // WIN32
}

#endif

class LuaStateOutString : public LuaStateOutFile
{
public:
	LuaStateOutString(size_t growBy = 10000) :
		m_buffer(NULL),
		m_growBy(growBy),
		m_curPos(0),
		m_size(0)
	{
	}

	virtual ~LuaStateOutString()
	{
		free(m_buffer);
	}

	virtual void Print(const char* str, ...)
	{
		char message[ 800 ];
		va_list arglist;

		va_start( arglist, str );
		vsprintf( message, str, arglist );
		va_end( arglist );

		size_t len = strlen(message);
		if (len != 0)
		{
			if (m_curPos + len + 1 > m_size)
			{
				size_t newSize = m_size;
				while (newSize < m_curPos + len + 1)
					newSize += m_growBy;
				m_buffer = (char*)realloc(m_buffer, newSize);
				m_size = newSize;
			}

			strncpy(m_buffer + m_curPos, message, len);
			m_curPos += len;
			m_buffer[m_curPos] = 0;
		}
	}

	const char* GetBuffer() const
	{
		return m_buffer;
	}

protected:
	char* m_buffer;
	size_t m_growBy;
	size_t m_curPos;
	size_t m_size;
};


// LuaDumpObject(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
static int LS_LuaDumpObject( LuaState* state )
{
	LuaStateOutFile file;
	LuaStateOutString stringFile;

	LuaStack args(state);
	LuaStackObject fileObj = args[1];
	const char* fileName = NULL;
	if ( fileObj.IsUserData() )
	{
		FILE* stdioFile = (FILE *)fileObj.GetUserData();
		file.Assign( stdioFile );
	}
	else if ( fileObj.IsString() )
	{
		fileName = fileObj.GetString();
	}

	LuaObject nameObj = args[2];
	LuaObject valueObj = args[3];
	LuaStackObject alphabeticalObj = args[4];
	LuaStackObject indentLevelObj = args[5];
	LuaStackObject maxIndentLevelObj = args[6];
	LuaStackObject writeAllObj = args[7];
	bool writeAll = writeAllObj.IsBoolean() ? writeAllObj.GetBoolean() : false;
	bool alphabetical = alphabeticalObj.IsBoolean() ? alphabeticalObj.GetBoolean() : true;
	unsigned int maxIndentLevel = maxIndentLevelObj.IsInteger() ? (unsigned int)maxIndentLevelObj.GetInteger() : 0xFFFFFFFF;

	unsigned int flags = (alphabetical ? LuaState::DUMP_ALPHABETICAL : 0) | (writeAll ? LuaState::DUMP_WRITEALL : 0);

	if (fileName)
	{
		if (strcmp(fileName, ":string") == 0)
		{
			state->DumpObject(stringFile, nameObj, valueObj, flags, indentLevelObj.GetInteger(), maxIndentLevel);
			state->PushString(stringFile.GetBuffer());
			return 1;
		}
		else
		{
			state->DumpObject(fileName, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
		}
	}
	else
	{
		state->DumpObject(file, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
	}

	return 0;
}


// LuaDumpFile(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
static int LS_LuaDumpFile( LuaState* state )
{
	return LS_LuaDumpObject(state);
}


// LuaDumpGlobals(file, alphabetical, maxIndentLevel, writeAll)
static int LS_LuaDumpGlobals(LuaState* state)
{
	LuaStateOutFile file;

	LuaStack args(state);
	LuaStackObject fileObj = args[1];
	const char* fileName = NULL;
	if ( fileObj.IsUserData() )
	{
		FILE* stdioFile = (FILE *)fileObj.GetUserData();
		file.Assign( stdioFile );
	}
	else if ( fileObj.IsString() )
	{
		fileName = fileObj.GetString();
	}

	LuaStackObject alphabeticalObj = args[2];
	LuaStackObject maxIndentLevelObj = args[3];
	LuaStackObject writeAllObj = args[4];
	bool alphabetical = alphabeticalObj.IsBoolean() ? alphabeticalObj.GetBoolean() : true;
	unsigned int maxIndentLevel = maxIndentLevelObj.IsInteger() ? (unsigned int)maxIndentLevelObj.GetInteger() : 0xFFFFFFFF;
	bool writeAll = writeAllObj.IsBoolean() ? writeAllObj.GetBoolean() : false;

	unsigned int flags = (alphabetical ? LuaState::DUMP_ALPHABETICAL : 0) | (writeAll ? LuaState::DUMP_WRITEALL : 0);

	if (fileName)
	{
		state->DumpGlobals(fileName, flags, maxIndentLevel);
	}
	else
	{
		state->DumpGlobals(file, flags, maxIndentLevel);
	}

	return 0;
}


static int pmain (lua_State *L)
{
    lua_pushcfunction(L, luaopen_base);
    lua_pushstring(L, "");
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_package);
    lua_pushstring(L, LUA_LOADLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_table);
    lua_pushstring(L, LUA_TABLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_io);
    lua_pushstring(L, LUA_IOLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_os);
    lua_pushstring(L, LUA_OSLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_string);
    lua_pushstring(L, LUA_STRLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_math);
    lua_pushstring(L, LUA_MATHLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_debug);
    lua_pushstring(L, LUA_DBLIBNAME);
    lua_call(L, 1, 0);

    lua_pushcfunction(L, luaopen_wstring);
    lua_pushstring(L, LUA_WSTRLIBNAME);
    lua_call(L, 1, 0);

#if 0
	luaopen_wstring(L);
	lua_settop(L, 0);
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_replace(L, LUA_ENVIRONINDEX);  /* restore environment */
#endif

	return 0;
}


/**
**/
void LuaState::OpenLibs()
{
#if LUAPLUS_INCLUDE_STANDARD_LIBRARY
	LuaAutoBlock autoBlock(this);
	lua_cpcall(m_state, &pmain, NULL);

#ifdef LUA_MTSUPPORT
//        luaopen_thread(m_state);
#endif // LUA_MTSUPPORT

	ScriptFunctionsRegister(this);

    GetGlobals().Register("LuaDumpGlobals", LS_LuaDumpGlobals);
	GetGlobals().Register("LuaDumpObject", LS_LuaDumpObject);
	GetGlobals().Register("LuaDumpFile", LS_LuaDumpFile);

	GetGlobals().Register("LOG", LS_LOG);
	GetGlobals().Register("_ALERT", LS_ALERT);
#endif // LUAPLUS_INCLUDE_STANDARD_LIBRARY
}

} // namespace LuaPlus
