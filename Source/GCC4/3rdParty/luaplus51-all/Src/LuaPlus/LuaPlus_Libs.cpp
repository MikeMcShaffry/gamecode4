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
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lobject.h"
#include "src/lgc.h"
LUA_EXTERN_C_END
#include "LuaPlus.h"
#include "LuaState.h"
#include <string.h>
#ifdef WIN32
#if defined(WIN32) && !defined(_XBOX) && !defined(_XBOX_VER) && !defined(_WIN32_WCE)
#include <windows.h>
#elif defined(_XBOX) || defined(_XBOX_VER)
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

#ifdef _MSC_VER
#pragma warning(disable: 4100)
#endif // _MSC_VER

//-----------------------------------------------------------------------------
LUA_EXTERN_C_BEGIN
#include "src/lualib.h"
#include "src/lstate.h"
LUA_EXTERN_C_END

namespace LuaPlus {

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

#if 0

NOT AS FAST, BUT BETTER MEMORY MANAGEMENT.

class LuaStateOutString : public LuaStateOutFile
{
public:
	LuaStateOutString(size_t growBy = 10000)
		: m_tail(&m_head)
		, m_bufferDirty(true)
		, m_buffer(NULL)
		, m_bufferSize(0)
	{
		m_head.next = NULL;
	}

	virtual ~LuaStateOutString()
	{
		delete[] m_buffer;

		LineNode* node = m_head.next;
		while (node)
		{
			LineNode* oldNode = node;
			node = node->next;
			delete[] (unsigned char*)oldNode;
		}
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
			LineNode* newNode = (LineNode*)new unsigned char[sizeof(LineNode) + len];	// Already includes the +1 for \0
			newNode->next = NULL;
			m_tail->next = newNode;
			m_tail = newNode;

			newNode->len = len;
			strncpy(newNode->line, message, len);
			newNode->line[len] = 0;

			m_bufferSize += len;
			m_bufferDirty = true;
		}
	}

	const char* GetBuffer()
	{
		if (m_bufferDirty)
		{
			delete[] m_buffer;
			m_buffer = new char[m_bufferSize + 1];

			char* bufferPos = m_buffer;
			for (LineNode* node = m_head.next; node; node = node->next)
			{
				memcpy(bufferPos, node->line, node->len);
				bufferPos += node->len;
			}
			*bufferPos = 0;
		}

		return m_buffer;
	}

protected:
	struct LineNode
	{
		LineNode* next;
		size_t len;
		char line[1];
	};

	LineNode m_head;
	LineNode* m_tail;

	bool m_bufferDirty;
	char* m_buffer;
	size_t m_bufferSize;
};

#endif

} // namespace LuaPlus

#if LUAPLUS_DUMPOBJECT

LUA_EXTERN_C void luaplus_dumptable(lua_State* L, int index)
{
	LuaPlus::LuaState* state = lua_State_To_LuaState(L);
	LuaPlus::LuaObject valueObj(state, index);
	LuaPlus::LuaStateOutString stringFile;
	state->DumpObject(stringFile, NULL, valueObj, LuaPlus::LuaState::DUMP_ALPHABETICAL | LuaPlus::LuaState::DUMP_WRITEALL, 0, -1);
	state->PushString(stringFile.GetBuffer());
}

#endif // LUAPLUS_DUMPOBJECT

namespace LuaPlus {

/*static*/ LuaState* LuaState::Create(bool initStandardLibrary)
{
	LuaState* state = LuaState::Create();
	if (initStandardLibrary)
		state->OpenLibs();
	return state;
}

#if LUAPLUS_EXTENSIONS

LuaObject LuaState::CreateThread(LuaState* parentState)
{
    lua_State* L1 = lua_newthread(LuaState_to_lua_State(parentState));
    lua_TValue tobject;
#if LUA_REFCOUNT
    setnilvalue2n(L1, &tobject);
#else
    setnilvalue(&tobject);
#endif /* LUA_REFCOUNT */
    setthvalue(parentState->GetCState(), &tobject, L1);

	LuaObject retObj = LuaObject(lua_State_To_LuaState(L1), &tobject);
    setnilvalue(&tobject);
    lua_pop(LuaState_to_lua_State(parentState), 1);
    return retObj;
}

#endif // LUAPLUS_EXTENSIONS


#if LUAPLUS_DUMPOBJECT

// LuaDumpObject(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
int LS_LuaDumpObject( LuaState* state )
{
	LuaStateOutFile file;

	LuaStack args(state);
	LuaStackObject fileObj = args[1];
	if (fileObj.IsTable()  &&  state->GetTop() == 1)
	{
		LuaObject valueObj(fileObj);
		LuaObject nameObj;
		LuaStateOutString stringFile;
		state->DumpObject(stringFile, NULL, valueObj, LuaState::DUMP_ALPHABETICAL, 0, -1);
		state->PushString(stringFile.GetBuffer());
		return 1;
	}

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
			LuaStateOutString stringFile;
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
int LS_LuaDumpFile( LuaState* state )
{
	return LS_LuaDumpObject(state);
}


// LuaDumpGlobals(file, alphabetical, maxIndentLevel, writeAll)
int LS_LuaDumpGlobals(LuaState* state)
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

#endif // LUAPLUS_DUMPOBJECT


static int pmain (lua_State *L)
{
	luaL_openlibs(L);
	return 0;
}


/**
**/
void LuaState::OpenLibs()
{
#if LUAPLUS_INCLUDE_STANDARD_LIBRARY
	LuaAutoBlock autoBlock(this);
	lua_cpcall(LuaState_to_lua_State(this), &pmain, NULL);
#endif // LUAPLUS_INCLUDE_STANDARD_LIBRARY
}

} // namespace LuaPlus
