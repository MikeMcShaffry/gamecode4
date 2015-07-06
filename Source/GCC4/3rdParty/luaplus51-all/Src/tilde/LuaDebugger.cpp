
/****************************************************************************

Tilde

Copyright (c) 2008 Tantalus Media Pty

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

****************************************************************************/

#include "tilde/LuaDebugger.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "lauxlib.h"
#include "lobject.h"
#include "lstate.h"
#include "lapi.h"

#include "tilde/HostConfig.h"
#include "tilde/LuaDebuggerComms.h"


namespace tilde
{
	LuaDebugger::LuaDebugger(LuaDebuggerComms * listener)
		:
			m_mainlvm(NULL),
			m_shutdown(false),
			m_enabled(true),
			m_listener(listener),
			m_previousExecutionMode(DebuggerExecutionMode_Connecting),
			m_executionMode(DebuggerExecutionMode_Connecting),
			m_breakedlvm(NULL),
			m_stepNestedCalls(0),
			m_localNestedCalls(0),
			m_nextValueID(1),
			m_chunkCallInProgress(NULL),
			m_normalizedFilename(NULL),
			m_normalizedFilenameLength(0)
	{
	}

	LuaDebugger::~LuaDebugger(void)
	{
		delete[] m_normalizedFilename;
	}


	void LuaDebugger::HookCallback(lua_State * lvm, lua_Debug * ar)
	{
		LuaDebugger * debugger = (LuaDebugger *) LuaDebugger::GetRegistryEntry(lvm, "instance");

		if(!debugger)
			return;

		if(debugger->m_shutdown)
		{
			debugger->Shutdown();
			return;
		}

		// Ignore hook if we're not enabled, or if we're already breaked (Accessing some debug data might trigger metaevents on a different lvm)
		if(!debugger->m_enabled || debugger->m_breakedlvm != NULL)
			return;

		debugger->m_breakedlvm = lvm;

		switch(ar->event)
		{

		case LUA_HOOKCALL:
			debugger->OnLuaCallback_HookCall();
			break;

		case LUA_HOOKRET:
		case LUA_HOOKTAILRET:
			debugger->OnLuaCallback_HookReturn();
			break;

		case LUA_HOOKLINE:
			debugger->OnLuaCallback_HookLine();
			break;

		case LUA_HOOKERROR:
			debugger->OnLuaCallback_HookError();
			break;
		}

		debugger->m_breakedlvm = NULL;

		// Wait for all operations to be complete before shutting ourselves down
		if(debugger->IsShutdown())
			debugger->Shutdown();
	}

	bool LuaDebugger::Connect(lua_State * lvm)
	{
		TILDE_ASSERT(m_mainlvm == NULL);
		TILDE_ASSERT(lvm != NULL);

		if(m_listener->GetHost()->AttachLuaHook(lvm, &LuaDebugger::HookCallback, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKERROR, 0) == false)
			return false;

		m_mainlvm = lvm;
		m_breakedlvm = NULL;

		InitialiseRegistry(lvm);
		InitialiseValueCache(lvm);
		InitialiseIDCache(lvm);

		return true;
	}

	void LuaDebugger::Disconnect()
	{
		// Postpone until later, so we don't shutdown while processing messages or in a debug hook
		m_shutdown = true;
	}

	void LuaDebugger::Shutdown()
	{
		if(m_mainlvm != NULL)
		{
			m_listener->GetHost()->DetachLuaHook(m_mainlvm, &LuaDebugger::HookCallback);

			ClearRegistry();
			ClearBreakpoints();
			ClearIgnoredErrors();
			ClearWatches();
			ClearCaches();
		}

		m_mainlvm = NULL;
		m_breakedlvm = NULL;
		m_shutdown = false;
	}

	void LuaDebugger::ClearCaches()
	{
		m_localVariableCache.Clear();

		for(ThreadCollection::iterator iter = m_threads.begin(); iter != m_threads.end(); ++iter)
		{
			delete *iter;
		}

		m_threads.clear();
	}

	DebuggerExecutionMode LuaDebugger::GetExecutionMode() const
	{
		return m_executionMode;
	}

	void LuaDebugger::SetExecutionMode(DebuggerExecutionMode mode)
	{
		// If the user wants to 'continue' from a breakpoint, then return to the previous mode
		if(mode == DebuggerExecutionMode_Continue && (m_executionMode == DebuggerExecutionMode_Break || m_executionMode == DebuggerExecutionMode_Error))
			m_executionMode = m_previousExecutionMode;

		// Disallow stepping over or stepping while running
		else if(
			(mode == DebuggerExecutionMode_StepOver && (m_executionMode != DebuggerExecutionMode_Break && m_executionMode != DebuggerExecutionMode_Error))
			||
			(mode == DebuggerExecutionMode_StepOut && (m_executionMode != DebuggerExecutionMode_Break && m_executionMode != DebuggerExecutionMode_Error))
			||
			(mode == DebuggerExecutionMode_Error)
			)
			return;
		else
		{
			m_stepNestedCalls = 0;
			m_previousExecutionMode = mode;
			m_executionMode = mode;
		}
	}

	bool LuaDebugger::AddBreakpoint(int bkptid, const char * file, int line)
	{
		TILDE_PRINT("LuaDebugger::AddBreakpoint(%d, \"%s\", %d)\n", bkptid, file, line);

		const char *pTarget = m_listener->GetHost()->LookupTargetFile(file);

		BreakpointInfo * newinfo = new BreakpointInfo(bkptid, NormalizedFilename(pTarget), line);
		m_breakpoints.push_back(newinfo);
		
		m_breakpointMap.insert(std::make_pair(BreakpointKey(newinfo->m_fileName.c_str(), line), newinfo));

		return true;
	}

	bool LuaDebugger::RemoveBreakpoint(int bkptid)
	{
		for(BreakpointCollection::iterator iter = m_breakpoints.begin(); iter != m_breakpoints.end(); ++iter)
		{
			BreakpointInfo * info = *iter;
			if(info->m_bkptid == bkptid)
			{
				TILDE_PRINT("LuaDebugger::RemoveBreakpoint(%d, \"%s\", %d)\n", bkptid, info->m_fileName.c_str(), info->m_line);

				BreakpointMap::iterator mapiter = m_breakpointMap.find(BreakpointKey(info->m_fileName.c_str(), info->m_line));
				m_breakpointMap.erase(mapiter);
				m_breakpoints.erase(iter);
				delete info;
				return true;
			}
		}
		return false;
	}

	void LuaDebugger::ClearBreakpoints()
	{
		TILDE_PRINT("LuaDebugger::ClearBreakpoints()\n");

		m_breakpointMap.clear();

		for(BreakpointCollection::iterator iter = m_breakpoints.begin(); iter != m_breakpoints.end(); ++iter)
		{
			delete *iter;
		}

		m_breakpoints.clear();
	}

	int LuaDebugger::FindBreakpoint(const char * file, int line)
	{
		BreakpointMap::iterator iter = m_breakpointMap.find(BreakpointKey(NormalizedFilename(file), line));
		if(iter == m_breakpointMap.end())
			return 0;
		else
			return iter->second->m_bkptid;
	}

	void LuaDebugger::IgnoreError(const char * file, int line)
	{
		TILDE_PRINT("LuaDebugger::IgnoreError(\"%s\", %d)\n", file, line);

		IgnoredErrorInfo * newinfo = new IgnoredErrorInfo(file, line);
		m_ignoredErrors.push_back(newinfo);

		m_ignoredErrorMap.insert(std::make_pair(IgnoredErrorKey(newinfo->m_fileName.c_str(), line), newinfo));

	}

	bool LuaDebugger::IsIgnoredError(const char * file, int line)
	{
		IgnoredErrorMap::iterator iter = m_ignoredErrorMap.find(IgnoredErrorKey(file, line));
		return iter != m_ignoredErrorMap.end();
	}

	void LuaDebugger::ClearIgnoredErrors()
	{
		TILDE_PRINT("LuaDebugger::ClearIgnoredErrors()\n");

		m_ignoredErrorMap.clear();

		for(IgnoredErrorCollection::iterator iter = m_ignoredErrors.begin(); iter != m_ignoredErrors.end(); ++iter)
		{
			delete *iter;
		}

		m_ignoredErrors.clear();
	}

	void LuaDebugger::AddWatch(const char * expr, int watchid)
	{
		LuaWatchInfo * watch = new LuaWatchInfo(expr, watchid);
		m_watches.push_back(watch);
	}

	void LuaDebugger::RemoveWatch(int watchid)
	{
		for(WatchCollection::iterator iter = m_watches.begin(); iter != m_watches.end(); ++iter)
		{
			LuaWatchInfo * watch = *iter;
			if(watch->m_watchid == watchid)
			{
				delete watch;
				m_watches.erase(iter);
				break;
			}
		}
	}

	LuaWatchInfo * LuaDebugger::FindWatch(int watchid)
	{
		for(WatchCollection::iterator iter = m_watches.begin(); iter != m_watches.end(); ++iter)
		{
			LuaWatchInfo * watch = *iter;
			if(watch->m_watchid == watchid)
			{
				return watch;
			}
		}
		return NULL;
	}

	void LuaDebugger::ClearWatches()
	{
		for(WatchCollection::iterator iter = m_watches.begin(); iter != m_watches.end(); ++iter)
		{
			delete *iter;
		}
		m_watches.clear();
	}

	// This function pushes a new string onto the stack, and returns it.
	const char * LuaDebugger::ToString(lua_State * lvm, int index)
	{
		LuaType type = (LuaType) lua_type(lvm, index);

		switch(type)
		{
			case LuaType_NIL:
			{
				lua_pushstring(lvm, "nil");
				return lua_tostring(lvm, -1);
			}

			case LuaType_BOOLEAN:
			{
				bool val = lua_toboolean(lvm, index) != 0;
				lua_pushstring(lvm, val ? "true" : "false");
				return lua_tostring(lvm, -1);
			}

			case LuaType_LIGHTUSERDATA:
			{
				return lua_pushfstring(lvm, "LIGHTUSERDATA %p", lua_topointer(lvm, index));
			}

			case LuaType_NUMBER:
			case LuaType_STRING:
			{
				lua_pushvalue(lvm, index);
				return lua_tostring(lvm, -1);
			}

			case LuaType_TABLE:
			{
				return lua_pushfstring(lvm, "TABLE %p", lua_topointer(lvm, index));
			}

			case LuaType_FUNCTION:
			{
				lua_Debug debugInfo;

				// Make a duplicate so lua_getinfo can push it off without breaking anything
				lua_pushvalue(lvm, index);						
				if(lua_getinfo(lvm, ">Snl", &debugInfo))
				{
					const char * name = m_listener->GetHost()->GetFunctionName(lvm, index, &debugInfo);
					lua_pushstring(lvm, name);
					return lua_tostring(lvm, -1);
				}
				lua_pushstring(lvm, "");
				return lua_tostring(lvm, -1);
			}

			case LuaType_USERDATA:
			{
				return lua_pushfstring(lvm, "USERDATA %p", lua_topointer(lvm, index));
			}

			case LuaType_THREAD:
			{
				// Look it up in the thread table to get the name
				lua_pushstring(lvm, "ThreadTable");
				lua_gettable(lvm, LUA_REGISTRYINDEX);
				lua_pushvalue(lvm, index);
				lua_gettable(lvm, -2);
				int infoIndex = lua_gettop(lvm);

				if(lua_isuserdata(lvm, infoIndex) || lua_istable(lvm, infoIndex))
				{
					lua_pushstring(lvm, "name");
					lua_gettable(lvm, infoIndex);
					int nameIndex = lua_gettop(lvm);

					if(!lua_isnil(lvm, nameIndex))
					{
						const char * result = lua_pushfstring(lvm, "THREAD %s", lua_tostring(lvm, nameIndex));
						lua_remove(lvm, nameIndex);
						lua_remove(lvm, infoIndex);
						return result;
					}
				}

				// Otherwise find the name of the starting function
				lua_State * threadlvm = lua_tothread(lvm, index);
				lua_Debug debugInfo;
				int level = 0;
				while(lua_getstack(threadlvm, level, &debugInfo) == 1)
					++level;
				--level;
				lua_getstack(threadlvm, level, &debugInfo);
				lua_getinfo(threadlvm, "fnSl", &debugInfo);
				const char * name = m_listener->GetHost()->GetFunctionName(threadlvm, lua_gettop(threadlvm), &debugInfo);
				lua_pop(threadlvm, 1);
				return lua_pushfstring(lvm, "THREAD %s", name);
			}

			default:
			{
				lua_pushstring(lvm, "ERROR");
				return lua_tostring(lvm, -1);
			}
		}
	}


	// This function does not push anything onto any stacks
	bool LuaDebugger::GetCallstack(lua_State * lvm, StackFrameCollection & result, int & firstLuaFrame)
	{
		lua_Debug debugInfo;
		memset(&debugInfo, 0, sizeof(lua_Debug));

		result.clear();

		firstLuaFrame = -1;
		int level = 0;
		while(lua_getstack(lvm, level, &debugInfo) == 1)
		{
			lua_getinfo(lvm, "fnSl", &debugInfo);
			const char * name = m_listener->GetHost()->GetFunctionName(lvm, lua_gettop(lvm), &debugInfo);
			lua_pop(lvm, 1);
			StackFrameInfo * frameInfo = new StackFrameInfo(level, name, debugInfo.source, debugInfo.currentline);
			result.push_back(frameInfo);

			if(firstLuaFrame == -1 && debugInfo.what[0] != 'C')
				firstLuaFrame = level;

			++level;
		}

		if(firstLuaFrame == -1)
			firstLuaFrame = 0;

		return true;
	}

	// Leaves two items on the stack; at index -1 is the result, and at -2 is the input expression
	int LuaDebugger::CallChunk(lua_State * lvm, int stackDepth, const char * name, bool verboseError)
	{
		int chunkIndex = lua_gettop(m_mainlvm);

		// Compile the expression into a function
		int result = -1;
		const char * script = lua_tostring(m_mainlvm, chunkIndex);
		if(luaL_loadbuffer(m_mainlvm, script, strlen(script), name) == 0)
		{
			int functionIndex = lua_gettop(m_mainlvm);

			// Create a new function environment so we can access locals/upvalues/environment of the original context
			SetChunkEnvironment(lvm, stackDepth, functionIndex);

			// Push the error handler under the compiled function
			int errorFunctionIndex = functionIndex;
			lua_pushboolean(m_mainlvm, verboseError);
			lua_pushcclosure(m_mainlvm, &LuaDebugger::ChunkErrorFunction, 1);
			lua_insert(m_mainlvm, errorFunctionIndex);
			++functionIndex;

			// This is where the results start
			int resultsIndex = lua_gettop(m_mainlvm);

			// Push the current function's varargs 
			int varArgs = 0;
			lua_Debug debugInfo;
			memset(&debugInfo, 0, sizeof(lua_Debug));
			if(lua_getstack(lvm, stackDepth, &debugInfo) == 1 && lua_getinfo(lvm, "f", &debugInfo))
			{
				int runningFuncIndex = lua_gettop(lvm);
				while(lua_getvararg(lvm, &debugInfo, varArgs + 1))
				{
					++varArgs;
				}
				lua_xmove(lvm, m_mainlvm, varArgs);
				lua_remove(lvm, runningFuncIndex);
			}

			// Invoke it
			m_chunkCallInProgress = lua_topointer(m_mainlvm, functionIndex);
			if(lua_pcall(m_mainlvm, varArgs, LUA_MULTRET, errorFunctionIndex) == 0)
			{
				result = lua_gettop(m_mainlvm) - resultsIndex + 1;
			}

			// Restore state and pop error function
			m_chunkCallInProgress = NULL;
			lua_remove(m_mainlvm, errorFunctionIndex);
		}
		else
		{
			lua_pushstring(m_mainlvm, "<<compile error: ");
			lua_insert(m_mainlvm, -2);
			lua_pushstring(m_mainlvm, ">>");
			lua_concat(m_mainlvm, 3);
		}

		// Remove the input value, leaving only the output(s) behind
		lua_remove(m_mainlvm, chunkIndex);

		return result;
	}


	bool LuaDebugger::UpdateWatchVariables(lua_State * lvm, int stackDepth)
	{
		for(WatchCollection::iterator iter = m_watches.begin(); iter != m_watches.end(); ++iter)
		{
			UpdateWatch(*iter, lvm, stackDepth);
		}

		// Do a full garbage collection in case the computations needed a lot of workspace.
		if(!m_watches.empty())
			lua_gc(lvm, LUA_GCCOLLECT, 0);

		return true;
	}

	bool LuaDebugger::UpdateWatchVariable(int watchid, lua_State * lvm, int stackDepth)
	{
		LuaWatchInfo * watchinfo = FindWatch(watchid);
		if(watchinfo != NULL)
		{
			UpdateWatch(watchinfo, lvm, stackDepth);

			// Do a full garbage collection in case the computations needed a lot of workspace.
			lua_gc(lvm, LUA_GCCOLLECT, 0);
		}

		return true;
	}

	void LuaDebugger::UpdateWatch(LuaWatchInfo * watch, lua_State * lvm, int stackDepth)
	{
		stack_restore_point restore(m_mainlvm, 6);

		watch->m_variable.Invalidate();

		lua_pushstring(m_mainlvm, watch->m_expression.c_str());
		int exprIndex = lua_gettop(m_mainlvm);

		lua_pushstring(m_mainlvm, "return ");
		lua_pushvalue(m_mainlvm, exprIndex);
		lua_concat(m_mainlvm, 2);

		int results = CallChunk(lvm, stackDepth, "Tilde watch expression", false);	// Pops "return xxx" and pushes result(s)
		bool success = results >= 0;
		if(results == 0)
		{
			lua_pushnil(m_mainlvm);
			++results;
		}
		else if (results < 0)
			results = 1;
		int valueIndex = lua_gettop(m_mainlvm) - results + 1;

		LuaDebuggerValue key = LookupValue(m_mainlvm, exprIndex);
		LuaDebuggerValue value = LookupValue(m_mainlvm, valueIndex);

		watch->m_variable.m_key = key;
		if(	watch->m_variable.m_value != value)
		{
			watch->m_variable.m_modified = true;
			watch->m_variable.m_value = value;
		}
		watch->m_variable.m_valid = success;
		watch->m_variable.m_class = VariableClass_Watch;

		UpdateExpansions(&watch->m_variable, m_mainlvm, valueIndex);

		lua_pop(m_mainlvm, 2);	// Remove the expression string and return value/error message
	}

	bool LuaDebugger::UpdateLocalVariables(lua_State * lvm, int stackFrame)
	{
		stack_restore_point restore(lvm, 4);

		m_localVariableCache.Invalidate();
		m_localVariableCache.m_valid = true;
		m_localVariableCache.m_expanded	= true;
		m_localVariableCache.m_hasEntries = true;
		m_localVariableCache.m_hasMetadata = false;

		VariableInfo * rootvar = &m_localVariableCache;
		lua_Debug debugInfo;
		if(lua_getstack(lvm, stackFrame, &debugInfo) == 1)
		{
			// Get the upvalues
			if(lua_getinfo(lvm, "uf", &debugInfo) && debugInfo.nups > 0)
			{
				int funcIndex = lua_gettop(lvm);
				for(int index = 1; index <= debugInfo.nups; ++index)
				{
					const char * name;
					if( (name = lua_getupvalue(lvm, funcIndex, index)) != NULL)
					{
						int valueIndex = lua_gettop(lvm);

						lua_pushstring(lvm, name);
						int keyIndex = lua_gettop(lvm);

						UpdateVariable(rootvar, lvm, keyIndex, valueIndex, VariableClass_Upvalue);

						lua_pop(lvm, 2);		// Remove value and name
					}
				}
 			}
			lua_pop(lvm, 1);		// lua_getinfo("f") always pushes something onto the stack, even if it 'fails'	

			// Get all the varargs
			int index = 1;
			while(lua_getvararg(lvm, &debugInfo, index))
			{
				int valueIndex = lua_gettop(lvm);

				lua_pushfstring(lvm, "...[%d]", index);
				int keyIndex = lua_gettop(lvm);

				UpdateVariable(rootvar, lvm, keyIndex, valueIndex, VariableClass_Local);
				lua_pop(lvm, 2);
				++index;
			}

			// Get all the local variables
			index = 1;
			const char * name;
			while( (name = lua_getlocal(lvm, &debugInfo, index++)) != NULL)
			{
				if(name[0] != '(')
				{
					int valueIndex = lua_gettop(lvm);

					lua_pushstring(lvm, name);
					int keyIndex = lua_gettop(lvm);

					UpdateVariable(rootvar, lvm, keyIndex, valueIndex, VariableClass_Local);
				}
				lua_pop(lvm, 1);
			}

		}
		return false;
	}

	bool LuaDebugger::GetTableRecursive(VariableInfo * rootvar, lua_State * lvm, int tableIndex)
	{
		// Expects a table value on the top of the stack
		stack_balanced_check balancedCheck(lvm, 4);

		lua_pushnil(lvm);						
		while(lua_next(lvm, tableIndex))				
		{
			int keyIndex = lua_gettop(lvm) - 1;
			int valueIndex = lua_gettop(lvm);

			UpdateVariable(rootvar, lvm, keyIndex, valueIndex, VariableClass_TableEntry);

			lua_pop(lvm, 1);					// Pop the value
		}
		return true;
	}

	void LuaDebugger::UpdateVariable(VariableInfo * rootvar, lua_State * lvm, int keyIndex, int valueIndex, VariableClass varclass)
	{
		LuaDebuggerValue key = LookupValue(lvm, keyIndex);
		LuaDebuggerValue value = LookupValue(lvm, valueIndex);

		VariableInfo * varInfo = rootvar->UpdateValue(key, value, varclass);

		UpdateExpansions(varInfo, lvm, valueIndex);
	}

	void LuaDebugger::UpdateExpansions(VariableInfo * rootvar, lua_State * lvm, int objectIndex)
	{
		stack_balanced_check balancedCheck(lvm, 4);

		bool oldHasEntries = rootvar->m_hasEntries;
		bool oldHasMetadata = rootvar->m_hasMetadata;
		rootvar->m_hasEntries = false;
		rootvar->m_hasMetadata = false;

		LuaType objectType = (LuaType) lua_type(lvm, objectIndex);

		// Insert the metatable
		if(objectType == LuaType_USERDATA || objectType == LuaType_TABLE)
		{
			if(lua_getmetatable(lvm, objectIndex))
			{
				rootvar->m_hasMetadata = true;
				if(rootvar->m_expanded)
				{
					int valueIndex = lua_gettop(lvm);

					LuaDebuggerValue key = LuaDebuggerValue(LuaType_TILDE_METATABLE);
					LuaDebuggerValue value = LookupValue(lvm, valueIndex);
					VariableInfo * varInfo = rootvar->UpdateValue(key, value, VariableClass_TableEntry);
					UpdateExpansions(varInfo, lvm, valueIndex);
				}
				lua_pop(lvm, 1);
			}
		}

		if(objectType == LuaType_USERDATA || objectType == LuaType_FUNCTION)
		{
			lua_getfenv(lvm, objectIndex);
			int valueIndex = lua_gettop(lvm);

			lua_pushvalue(lvm, LUA_GLOBALSINDEX);
			if(!lua_equal(lvm, -1, -2))
			{
				rootvar->m_hasMetadata = true;
				if(rootvar->m_expanded)
				{
					LuaDebuggerValue key = LuaDebuggerValue(LuaType_TILDE_ENVIRONMENT);
					LuaDebuggerValue value = LookupValue(lvm, valueIndex);
					VariableInfo * varInfo = rootvar->UpdateValue(key, value, VariableClass_TableEntry);
					UpdateExpansions(varInfo, lvm, valueIndex);
				}
			}
			lua_pop(lvm, 2);
		}

		if(objectType == LuaType_FUNCTION)
		{
			lua_Debug debugInfo;
			memset(&debugInfo, 0, sizeof(lua_Debug));
			lua_pushvalue(lvm, objectIndex);
			if(lua_getinfo(lvm, ">u", &debugInfo) && debugInfo.nups > 0)
			{
				rootvar->m_hasMetadata = true;
				if(rootvar->m_expanded)
				{
					LuaDebuggerValue key = LuaDebuggerValue(LuaType_TILDE_UPVALUES);
					LuaDebuggerValue value = LuaDebuggerValue(LuaType_NUMBER, (lua_Number) debugInfo.nups);
					VariableInfo * varInfo = rootvar->UpdateValue(key, value, VariableClass_TableEntry);

					varInfo->m_hasEntries = true;
					if(varInfo->m_expanded)
						GetUpvalues(varInfo, lvm, objectIndex);
				}
			}
		}

		if(objectType == LuaType_TABLE)
		{
			lua_pushnil(lvm);
			if(lua_next(lvm, objectIndex))
			{
				rootvar->m_hasEntries = true;
				lua_pop(lvm, 2);
				if(rootvar->m_expanded)
					GetTableRecursive(rootvar, lvm, objectIndex);
			}
		}

		if(!rootvar->m_hasEntries && !rootvar->m_hasMetadata)
			rootvar->m_expanded = false;

		if(oldHasEntries != rootvar->m_hasEntries || oldHasMetadata != rootvar->m_hasMetadata)
			rootvar->m_modified = true;
	}

	void LuaDebugger::GetUpvalues(VariableInfo * rootvar, lua_State * lvm, int functionIndex)
	{
		stack_balanced_check balancedCheck(lvm, 4);

		int index = 1;
		const char * name;
		while( (name = lua_getupvalue(lvm, functionIndex, index)) != NULL )
		{
			int valueIndex = lua_gettop(lvm);

			// C functions don't have names for upvalues, so generate one
			if(name[0] == '\0')
				lua_pushfstring(lvm, "[%d]", index);
			else
				lua_pushstring(lvm, name);
			int keyIndex = lua_gettop(lvm);

			UpdateVariable(rootvar, lvm, keyIndex, valueIndex, VariableClass_TableEntry);

			lua_pop(lvm, 2);
			++index;
		}
	}

	bool LuaDebugger::UpdateThreads()
	{
		// Flush and reset the cache
		for(ThreadCollection::iterator iter = m_threads.begin(); iter != m_threads.end(); )
		{
			ThreadCollection::iterator nextiter = iter;
			++nextiter;

			if((*iter)->m_valid == false)
			{
				delete *iter;
				m_threads.erase(iter);
			}
			else
			{
				(*iter)->m_valid = false;
				(*iter)->m_modified = false;
			}
			iter = nextiter;
		}

		lua_pushstring(m_mainlvm, "ThreadTable");
		lua_gettable(m_mainlvm, LUA_REGISTRYINDEX);
		int threadTable = lua_gettop(m_mainlvm);
		TILDE_ASSERT_MSG(lua_istable(m_mainlvm, threadTable), "Expecting to find table 'ThreadTable' in registry");

		lua_pushnil(m_mainlvm);
		while(lua_next(m_mainlvm, threadTable) != 0)
		{
			int threadIndex = lua_gettop(m_mainlvm) - 1;
			int dataIndex = lua_gettop(m_mainlvm);

			lua_State * threadlvm = lua_tothread(m_mainlvm, threadIndex);
			bool dataValid = lua_isuserdata(m_mainlvm, dataIndex) || lua_istable(m_mainlvm, dataIndex);

			lua_pushthread(threadlvm);

			LuaDebuggerValue thread = LookupValue(m_mainlvm, threadIndex);
			LuaDebuggerValue parent(LuaType_NIL, (LuaDebuggerObjectID) 0);

			const char * name = "";
			int threadid = -1;
			String startFunction;
			String currentFunction;
			int state = -1;
			lua_Number peakTime = -1.0f;
			lua_Number averageTime = -1.0f;

			int stackSize = threadlvm->stacksize;
			int stackUsed = (int) ((char *)threadlvm->top - (char *)threadlvm->stack) / (int)sizeof(TValue);

			// Get the currently executing function
			lua_Debug debugInfo;
			if(lua_getstack(threadlvm, 0, &debugInfo))
			{
				lua_getinfo(threadlvm, "fnSl", &debugInfo);
				currentFunction = m_listener->GetHost()->GetFunctionName(threadlvm, lua_gettop(threadlvm), &debugInfo);
				lua_pop(threadlvm, 1);
			}

			if(dataValid)
			{
				int stackpos = lua_gettop(m_mainlvm);
				lua_pushstring(m_mainlvm, "name");
				lua_gettable(m_mainlvm, dataIndex);
				if(!lua_isnil(m_mainlvm, -1))
					name = lua_tostring(m_mainlvm, -1);
				lua_pop(m_mainlvm, 1);
				TILDE_ASSERT(stackpos == lua_gettop(m_mainlvm));

				stackpos = lua_gettop(m_mainlvm);
				lua_pushstring(m_mainlvm, "threadid");
				lua_gettable(m_mainlvm, dataIndex);
				if(!lua_isnil(m_mainlvm, -1))
					threadid = (int) lua_tonumber(m_mainlvm, -1);
				lua_pop(m_mainlvm, 1);
				TILDE_ASSERT(stackpos == lua_gettop(m_mainlvm));

				lua_pushstring(m_mainlvm, "state");
				lua_gettable(m_mainlvm, dataIndex);
				if(!lua_isnil(m_mainlvm, -1))
					state = (int) lua_tonumber(m_mainlvm, -1);
				lua_pop(m_mainlvm, 1);

				lua_pushstring(m_mainlvm, "peak_time");
				lua_gettable(m_mainlvm, dataIndex);
				if(!lua_isnil(m_mainlvm, -1))
					peakTime = lua_tonumber(m_mainlvm, -1);
				lua_pop(m_mainlvm, 1);

				lua_pushstring(m_mainlvm, "average_time");
				lua_gettable(m_mainlvm, dataIndex);
				if(!lua_isnil(m_mainlvm, -1))
					averageTime = lua_tonumber(m_mainlvm, -1);
				lua_pop(m_mainlvm, 1);

				lua_pushstring(m_mainlvm, "parent");
				lua_gettable(m_mainlvm, dataIndex);
				if(!lua_isnil(m_mainlvm, -1))
					parent = LookupValue(m_mainlvm, lua_gettop(m_mainlvm));
				lua_pop(m_mainlvm, 1);
			}

			// Update the entry in the threads cache
			ThreadInfo temp(thread);
			ThreadCollection::iterator iter = m_threads.find(&temp);
			if(iter == m_threads.end())
			{
				m_threads.insert(new ThreadInfo(thread, parent, name, threadid, startFunction, currentFunction, state, stackSize,stackUsed, peakTime, averageTime));
			}
			else
			{
				(*iter)->UpdateValue(parent, currentFunction, state, stackSize, stackUsed, peakTime, averageTime);
			}

			// Pop the thread off the stack
			lua_pop(threadlvm, 1);

			// End of iteration
			lua_pop(m_mainlvm, 1);
		}

		// Remove the ThreadTable from the stack
		lua_pop(m_mainlvm, 1);

		return true;
	}

	void * LuaDebugger::GetRegistryEntry(lua_State * lvm, const char * name)
	{
		stack_restore_point restore(lvm, 4);

		void * result = NULL;

		lua_pushstring(lvm, "LuaDebugger");
		lua_gettable(lvm, LUA_REGISTRYINDEX);
		int registryIndex = lua_gettop(lvm);

		if(lua_istable(lvm, registryIndex))
		{
			lua_pushstring(lvm, name);
			lua_gettable(lvm, registryIndex);

			result = lua_touserdata(lvm, -1);
			lua_pop(lvm, 1);
		}

		lua_pop(lvm, 1);

		return result;
	}

	void LuaDebugger::InitialiseRegistry(lua_State* lvm)
	{
		stack_restore_point restore(lvm, 4);

		// Create the root registry entry
		lua_pushstring(lvm, "LuaDebugger");
		lua_createtable(lvm, 0, 4);
		int registryIndex = lua_gettop(lvm);

		lua_pushstring(lvm, "instance");
		lua_pushlightuserdata(lvm, this);
		lua_settable(lvm, registryIndex);

		lua_settable(lvm, LUA_REGISTRYINDEX);
	}

	void LuaDebugger::ClearRegistry()
	{
		stack_restore_point restore(m_mainlvm, 4);

		lua_pushstring(m_mainlvm, "LuaDebugger");
		lua_pushnil(m_mainlvm);
		lua_settable(m_mainlvm, LUA_REGISTRYINDEX);
	}

	void LuaDebugger::SetChunkEnvironment(lua_State * lvm, int stackDepth, int functionIndex)
	{
		stack_restore_point restore(m_mainlvm, 6);

		// Create the environment table
		lua_createtable(m_mainlvm, 0, 2);										//     +1
		int envIndex = lua_gettop(m_mainlvm);

		// Create the metatable
		lua_createtable(m_mainlvm, 0, 5);										//     +1
		int metatableIndex = lua_gettop(m_mainlvm);

		lua_pushstring(m_mainlvm, "__index");									//     +1
		lua_pushcfunction(m_mainlvm, &LuaDebugger::ChunkEnvironment_index);		//     +1
		lua_settable(m_mainlvm, metatableIndex);								// -2

		lua_pushstring(m_mainlvm, "__newindex");								//     +1
		lua_pushcfunction(m_mainlvm, &LuaDebugger::ChunkEnvironment_newindex);	//     +1
		lua_settable(m_mainlvm, metatableIndex);								// -2

		// Store the stack index
		lua_pushstring(m_mainlvm, "frame");										//     +1
		lua_pushnumber(m_mainlvm, stackDepth);									//     +1
		lua_settable(m_mainlvm, metatableIndex);								// -2

		// Store the thread
		lua_pushstring(m_mainlvm, "thread");									//     +1
		lua_pushthread(lvm);
		lua_xmove(lvm, m_mainlvm, 1);											//     +1
		lua_settable(m_mainlvm, metatableIndex);								// -2

		// Store the function
		lua_pushstring(m_mainlvm, "function");									//     +1
		lua_pushvalue(m_mainlvm, functionIndex);								//     +1
		lua_settable(m_mainlvm, metatableIndex);								// -2

		// Store the original function environment
		lua_pushstring(m_mainlvm, "env");										//     +1
		lua_Debug debugInfo;
		memset(&debugInfo, 0, sizeof(lua_Debug));
		if(lua_getstack(lvm, stackDepth, &debugInfo) == 1 && lua_getinfo(lvm, "f", &debugInfo))			//     +1
		{
			int funcIndex = lua_gettop(lvm);
			lua_getfenv(lvm, funcIndex);																//     +1
			lua_xmove(lvm, m_mainlvm, 1);										//     +1				// -1
			lua_remove(lvm, funcIndex);																	// -1
		}
		else
		{
			// If there's nothing running (ie. we're evaluating 'live' on the main thread)
			// fall back on the globals table.
			lua_pushvalue(m_mainlvm, LUA_GLOBALSINDEX);							//     +1
		}
		int parentEnvIndex = lua_gettop(m_mainlvm);
		TILDE_ASSERT_MSG(lua_istable(m_mainlvm, parentEnvIndex), "The chunk environment's 'env' field was expected to be a table; got %s", lua_typename(m_mainlvm, lua_type(m_mainlvm, parentEnvIndex)));
		lua_settable(m_mainlvm, metatableIndex);								// -2

		// Assign the metatable to the environment
		lua_setmetatable(m_mainlvm, envIndex);									// -1

		// Set our environment for the function
		lua_setfenv(m_mainlvm, functionIndex);									// -1
	}

	int LuaDebugger::ChunkErrorFunction(lua_State * lvm)
	{
		// Arguments: errorMessage

		bool verbose = lua_toboolean(lvm, lua_upvalueindex(1)) != 0;

		if(!verbose)
		{
			lua_pushstring(lvm, "<<evaluation error: ");
			lua_insert(lvm, 1);
			lua_pushstring(lvm, ">>");
			lua_concat(lvm, 3);
		}
		else
		{
			lua_pushstring(lvm, "<<evaluation error: ");
			lua_insert(lvm, 1);
			lua_pushstring(lvm, ">>\n");

			// push "debug.traceback"
			lua_pushliteral(lvm, "debug");
			lua_gettable(lvm, LUA_GLOBALSINDEX);
			lua_pushliteral(lvm, "traceback");
			lua_gettable(lvm, -2);
			lua_remove(lvm, -2);

			// Call it with no error message
			lua_pcall(lvm, 0, 1, 0);
			lua_concat(lvm, 4);
		}

		return 1;
	}

	int LuaDebugger::ChunkEnvironment_index( lua_State * lvm )
	{
		// Arguments: table, key

		TILDE_ASSERT_MSG(lua_isstring(lvm, 2), "This code assumes only strings are indexed into a function environment");
		const char * key = lua_tostring(lvm, 2);

		// Find the extra 'arguments' in the metatable

		lua_getmetatable(lvm, 1);
		int metatableIndex = lua_gettop(lvm);

		// Get the original environment table
		lua_pushstring(lvm, "env");
		lua_rawget(lvm, metatableIndex);
		int envIndex = lua_gettop(lvm);

		// Get the chunk function that was originally called
		lua_pushstring(lvm, "function");
		lua_rawget(lvm, metatableIndex);
		int functionIndex = lua_gettop(lvm);

		// If we're not actually evaluating a chunk right now, we must be running a function closure that was
		// created during a chunk evaluation. The closure has been stored and executed at some later time, after
		// the chunk call completed.
		LuaDebugger * debugger = (LuaDebugger *) LuaDebugger::GetRegistryEntry(lvm, "instance");
		if(debugger->m_chunkCallInProgress != lua_topointer(lvm, functionIndex))
		{
			// Replace the running function's environment so we don't have to do this again
			lua_Debug debugInfo;
			if(lua_getstack(lvm, 0, &debugInfo) == 1 && lua_getinfo(lvm, "f", &debugInfo))
			{
				int funcIndex = lua_gettop(lvm);
				lua_pushvalue(lvm, envIndex);
				lua_setfenv(lvm, funcIndex);
				lua_pop(lvm, 1);
			}

			// Look the key up in the environment; this might trigger an error
			lua_pushvalue(lvm, 2);
			lua_gettable(lvm, envIndex);

			// Don't bother removing the other stuff on the stack, lua will do it for us
			return 1;
		}

		// Get the current stack frame
		lua_pushstring(lvm, "frame");
		lua_rawget(lvm, metatableIndex);
		int frame = (int) lua_tointeger(lvm, -1);

		// Get the thread
		lua_pushstring(lvm, "thread");
		lua_rawget(lvm, metatableIndex);
		lua_State * threadlvm = lua_tothread(lvm, -1);

		//
		// Check in locals first
		//

		// If we're examining the main thread we need to skip over the watch expression call, and the metatable __index call
		if(threadlvm == lvm)
			frame += 2;

		lua_Debug debugInfo;
		memset(&debugInfo, 0, sizeof(lua_Debug));
		if(lua_getstack(threadlvm, frame, &debugInfo) == 1 && lua_getinfo(threadlvm, "uf", &debugInfo))			//		+1
		{
			int funcIndex = lua_gettop(threadlvm);

			// Look in the upvalues
			if(debugInfo.nups > 0)
			{
				for(int index = 1; index <= debugInfo.nups; ++index)
				{
					const char * name = lua_getupvalue(threadlvm, funcIndex, index);								//		+1
					if(name != NULL && strcmp(name, key) == 0)
					{
						// Move the value across
						lua_xmove(threadlvm, lvm, 1);																// -1

						// Pop the function off the stack
						lua_remove(threadlvm, funcIndex);															// -1

						// Don't bother removing the other stuff on the stack, lua will do it for us
						return 1;
					}	
					lua_pop(threadlvm, 1);																			// -1
				}
			}

			// Check in the locals
			int index = 1;
			const char * name;
			while( (name = lua_getlocal(threadlvm, &debugInfo, index++)) != NULL)									//		+1
			{
				if(strcmp(name, key) == 0)
				{
					// Move the value across
					lua_xmove(threadlvm, lvm, 1);																	// -1

					// Pop the function off the stack
					lua_remove(threadlvm, funcIndex);																// -1

					// Don't bother removing the other stuff on the stack, lua will do it for us
					return 1;
				}
				lua_pop(threadlvm, 1);																				// -1
			}

			// This might trigger an error
			lua_pushvalue(lvm, 2);
			lua_gettable(lvm, envIndex);

			// If it's nil, leave it as the return value anyway
			return 1;
		}
		else
		{
			// There's no lua function running, so just look in the globals
			// This might trigger an error
			lua_getglobal(lvm, key);
			return 1;
		}

	}

	int LuaDebugger::ChunkEnvironment_newindex( lua_State * lvm )
	{
		// Arguments: table, key, value

		TILDE_ASSERT_MSG(lua_isstring(lvm, 2), "This code assumes only strings are indexed into a function environment");
		const char * key = lua_tostring(lvm, 2);

		// Find the extra 'arguments' in the metatable

		lua_getmetatable(lvm, 1);
		int metatableIndex = lua_gettop(lvm);

		// Get the original environment table
		lua_pushstring(lvm, "env");
		lua_rawget(lvm, metatableIndex);
		int envIndex = lua_gettop(lvm);

		// Get the chunk function that was originally called
		lua_pushstring(lvm, "function");
		lua_rawget(lvm, metatableIndex);
		int functionIndex = lua_gettop(lvm);

		// If we're not actually evaluating a chunk right now, we must be running a function closure that was
		// created during a chunk evaluation. The closure has been stored and executed at some later time, after
		// the chunk call completed.
		LuaDebugger * debugger = (LuaDebugger *) LuaDebugger::GetRegistryEntry(lvm, "instance");
		if(debugger->m_chunkCallInProgress != lua_topointer(lvm, functionIndex))
		{
			// Replace the running function's environment so we don't have to do this again
			lua_Debug debugInfo;
			if(lua_getstack(lvm, 0, &debugInfo) == 1 && lua_getinfo(lvm, "f", &debugInfo))
			{
				int funcIndex = lua_gettop(lvm);
				lua_pushvalue(lvm, envIndex);
				lua_setfenv(lvm, funcIndex);
				lua_pop(lvm, 1);
			}

			// This might trigger an error
			lua_pushvalue(lvm, 2);
			lua_pushvalue(lvm, 3);
			lua_settable(lvm, envIndex);

			// Don't bother removing the other stuff on the stack, lua will do it for us
			return 0;
		}

		// Get the current stack frame
		lua_pushstring(lvm, "frame");
		lua_rawget(lvm, metatableIndex);
		int frame = (int) lua_tointeger(lvm, -1);

		// Get the thread
		lua_pushstring(lvm, "thread");
		lua_rawget(lvm, metatableIndex);
		lua_State * threadlvm = lua_tothread(lvm, -1);

		//
		// Check in locals first
		//

		// If we're examining the main thread we need to skip over the watch expression call, and the metatable __index call
		if(threadlvm == lvm)
			frame += 2;

		lua_Debug debugInfo;
		memset(&debugInfo, 0, sizeof(lua_Debug));
		if(lua_getstack(threadlvm, frame, &debugInfo) == 1 && lua_getinfo(threadlvm, "uf", &debugInfo))			//		+1
		{
			int funcIndex = lua_gettop(threadlvm);

			// Look in the upvalues
			if(debugInfo.nups > 0)
			{
				for(int index = 1; index <= debugInfo.nups; ++index)
				{
					const char * name = lua_getupvalue(threadlvm, funcIndex, index);								//		+1
					if(name != NULL && strcmp(name, key) == 0)
					{
						// Move the value across
						lua_pushvalue(lvm, 3);
						lua_xmove(lvm, threadlvm, 1);																// -1
						lua_setupvalue(threadlvm, funcIndex, index);

						// Pop the function off the stack
						lua_remove(threadlvm, funcIndex);															// -1

						// Don't bother removing the other stuff on the stack, lua will do it for us
						return 0;
					}	
					lua_pop(threadlvm, 1);																			// -1
				}
			}

			// Check in the locals
			int index = 1;
			const char * name;
			while( (name = lua_getlocal(threadlvm, &debugInfo, index)) != NULL)									//		+1
			{
				if(strcmp(name, key) == 0)
				{
					// Move the value across
					lua_pushvalue(lvm, 3);
					lua_xmove(lvm, threadlvm, 1);																	// -1
					lua_setlocal(threadlvm, &debugInfo, index);

					// Pop the function off the stack
					lua_remove(threadlvm, funcIndex);																// -1

					// Don't bother removing the other stuff on the stack, lua will do it for us
					return 0;
				}
				lua_pop(threadlvm, 1);																				// -1
				++index;
			}

			// Pop the function off the stack
			lua_remove(threadlvm, funcIndex);																		// -1

			// This might trigger an error
			lua_pushvalue(lvm, 2);
			lua_pushvalue(lvm, 3);
			lua_settable(lvm, envIndex);

			// Don't bother removing the other stuff on the stack, lua will do it for us
			return 0;
		}
		else
		{
			// There's no lua function running, so just look in the globals
			// This might trigger an error
			lua_pushvalue(lvm, 2);
			lua_pushvalue(lvm, 3);
			lua_settable(lvm, LUA_GLOBALSINDEX);
		}


		return 0;
	}

	void LuaDebugger::InitialiseValueCache(lua_State* lvm)
	{
		stack_restore_point restore(lvm, 8);

		// Create the metatable
		lua_createtable(lvm, 0, 1);
		int metatableIndex = lua_gettop(lvm);
		lua_pushstring(lvm, "__mode");
		lua_pushstring(lvm, "k");
		lua_settable(lvm, metatableIndex);

		// Create the cache
		lua_newtable(lvm);
		int cacheIndex = lua_gettop(lvm);

		// Assign the metatable
		lua_pushvalue(lvm, metatableIndex);
		lua_setmetatable(lvm, cacheIndex);

		// Put it into the registry
		lua_pushstring(lvm, "LuaDebugger");
		lua_gettable(lvm, LUA_REGISTRYINDEX);
		int registryIndex = lua_gettop(lvm);

		lua_pushstring(lvm, "LuaDebugerValueCache");
		lua_pushvalue(lvm, cacheIndex);
		lua_settable(lvm, registryIndex);

		// Pop the cache, metatable and registry from the stack
		lua_pop(lvm, 3);
	}

	void LuaDebugger::InitialiseIDCache(lua_State* lvm)
	{
		stack_restore_point restore(lvm, 8);

		// Create the metatable
		lua_createtable(lvm, 0, 1);
		int metatableIndex = lua_gettop(lvm);
		lua_pushstring(lvm, "__mode");
		lua_pushstring(lvm, "v");
		lua_settable(lvm, metatableIndex);

		// Create the cache
		lua_newtable(lvm);
		int cacheIndex = lua_gettop(lvm);

		// Assign the metatable
		lua_pushvalue(lvm, metatableIndex);
		lua_setmetatable(lvm, cacheIndex);

		// Put it into the registry
		lua_pushstring(lvm, "LuaDebugger");
		lua_gettable(lvm, LUA_REGISTRYINDEX);
		int registryIndex = lua_gettop(lvm);

		lua_pushstring(lvm, "LuaDebugerIDCache");
		lua_pushvalue(lvm, cacheIndex);
		lua_settable(lvm, registryIndex);

		// Pop the cache, metatable and registry table from the stack
		lua_pop(lvm, 3);
	}

	LuaDebuggerValue LuaDebugger::LookupValue(lua_State * lvm, int valueIndex)
	{
		stack_restore_point restore(lvm, 8);

		LuaType type = (LuaType) lua_type(lvm, valueIndex);

		if(type == LuaType_NIL)
		{
			return LuaDebuggerValue(type, (LuaDebuggerObjectID) 0);
		}
		else if(type == LuaType_BOOLEAN)
		{
			return LuaDebuggerValue(type, (LuaDebuggerObjectID) lua_toboolean(lvm, valueIndex));
		}
		else if(type == LuaType_NUMBER)
		{
			return LuaDebuggerValue(type, lua_tonumber(lvm, valueIndex));
		}
		else
		{
			TILDE_ASSERT(type == LuaType_LIGHTUSERDATA || type == LuaType_STRING || type == LuaType_TABLE || type == LuaType_FUNCTION || type == LuaType_USERDATA || type == LuaType_THREAD
					|| type == LuaType_WSTRING
					);

			// Get the cache table
			lua_pushstring(lvm, "LuaDebugger");
			lua_gettable(lvm, LUA_REGISTRYINDEX);
			int registryIndex = lua_gettop(lvm);

			lua_pushstring(lvm, "LuaDebugerValueCache"); 
			lua_gettable(lvm, registryIndex);
			int cacheIndex = lua_gettop(lvm);

			lua_pushstring(lvm, "LuaDebugerIDCache"); 
			lua_gettable(lvm, registryIndex);
			int idCacheIndex = lua_gettop(lvm);

			// Lookup the value in the table
			lua_pushvalue(lvm, valueIndex);
			lua_gettable(lvm, cacheIndex);

			LuaDebuggerObjectID valueID;

			// If it's not there already, then populate it
			if(lua_isnil(lvm, -1))
			{
				valueID = m_nextValueID++;
				lua_pushvalue(lvm, valueIndex);
				lua_pushlightuserdata(lvm, (void *) valueID);
				lua_settable(lvm, cacheIndex);

				lua_pushlightuserdata(lvm, (void *) valueID);
				lua_pushvalue(lvm, valueIndex);
				lua_settable(lvm, idCacheIndex);

				// Tell the debugger about the new value
				const char * str = ToString(lvm, valueIndex);
				m_listener->OnCached(valueID, (LuaType) lua_type(lvm, valueIndex), str);
				lua_pop(lvm, 1);
			}
			else
			{
				lua_pushvalue(lvm, valueIndex);
				lua_gettable(lvm, cacheIndex);
				valueID = (LuaDebuggerObjectID) lua_touserdata(lvm, -1);
			}

			// Pop the cache and registry off the stack
			lua_pop(lvm, 2);

			return LuaDebuggerValue(type, valueID);
		}
	}

	lua_State * LuaDebugger::LookupThreadID(LuaDebuggerObjectID valueid)
	{
		lua_State * lvm = m_mainlvm;

		stack_restore_point restore(lvm, 4);

		if(LookupID(valueid))
		{
			int valueIndex = lua_gettop(lvm);
			if(lua_isthread(lvm, valueIndex))
			{
				return lua_tothread(lvm, valueIndex);
			}
		}

		return NULL;
	}

	// Upon success, pushes the corresponding object onto the main lvm stack
	bool LuaDebugger::LookupID(LuaDebuggerObjectID valueid)
	{
		lua_State * lvm = m_mainlvm;

		// Get the cache table
		lua_pushstring(lvm, "LuaDebugger");
		lua_gettable(lvm, LUA_REGISTRYINDEX);
		int registryIndex = lua_gettop(lvm);

		lua_pushstring(lvm, "LuaDebugerIDCache"); 
		lua_gettable(lvm, registryIndex);
		int cacheIndex = lua_gettop(lvm);

		// Lookup the value in the cache
		lua_pushlightuserdata(lvm, (void *) valueid);
		lua_gettable(lvm, cacheIndex);
		int valueIndex = lua_gettop(lvm);

		if(lua_isnil(lvm, valueIndex))
		{
			lua_pop(lvm, 3);
			return false;
		}
		else
		{
			lua_remove(lvm, cacheIndex);
			lua_remove(lvm, registryIndex);
			return true;
		}	
	}

	void LuaDebugger::OnLuaCallback_HookCall()
	{
		if(m_executionMode == DebuggerExecutionMode_StepOver || m_executionMode == DebuggerExecutionMode_StepOut)
		{
			++m_stepNestedCalls;
		}

		++m_localNestedCalls;

		m_listener->OnHookCall();
	}

	void LuaDebugger::OnLuaCallback_HookReturn()
	{
		if(m_executionMode == DebuggerExecutionMode_StepOver || m_executionMode == DebuggerExecutionMode_StepOut)
		{
			if(m_stepNestedCalls == 0)
			{
				m_previousExecutionMode = DebuggerExecutionMode_Go;
				m_executionMode = DebuggerExecutionMode_Break;
			}
			else
			{
				--m_stepNestedCalls;
			}
		}

		// If we returned from the last 'local' function, then clear the local watches
		if(m_localNestedCalls > 0)
		{
			--m_localNestedCalls;
		}

		m_listener->OnHookReturn();
	}

	void LuaDebugger::OnLuaCallback_HookLine()
	{
		stack_restore_point restore(m_breakedlvm, 4);

		lua_Debug debugInfo;
		memset(&debugInfo, 0, sizeof(lua_Debug));

		if(lua_getstack(m_breakedlvm, 0, &debugInfo) == 1)
		{
			lua_getinfo(m_breakedlvm, "Sl", &debugInfo);
		}

		// If we were stepping over a single line, and we're not in a nested call, then break
		if(m_executionMode == DebuggerExecutionMode_StepOver && m_stepNestedCalls == 0)
		{
			m_previousExecutionMode = DebuggerExecutionMode_Go;
			m_executionMode = DebuggerExecutionMode_Break;
		}

		// Have we hit a string compiled as Lua code?  If so, step into it.
		if (debugInfo.source[0] != '@')
		{
			if (m_executionMode == DebuggerExecutionMode_Break)
			{
				m_previousExecutionMode = DebuggerExecutionMode_Break;
				m_executionMode = DebuggerExecutionMode_StepInto;
			}
		}

		// Have we hit a breakpoint?
		else if(m_executionMode != DebuggerExecutionMode_Break && IsBreakpoint(debugInfo.source, debugInfo.currentline))
		{
			m_previousExecutionMode = m_executionMode;
			m_executionMode = DebuggerExecutionMode_Break;
		}

		MainHook(debugInfo);
	}

	void LuaDebugger::OnLuaCallback_HookError()
	{
		stack_restore_point restore(m_breakedlvm, 4);

		lua_Debug debugInfo;
		memset(&debugInfo, 0, sizeof(lua_Debug));

		// See if it's an error marked to be ignored, skipping over C functions.
		int level = 0;
		while(lua_getstack(m_breakedlvm, level++, &debugInfo) == 1)
		{
			lua_getinfo(m_breakedlvm, "Sl", &debugInfo);
			if(debugInfo.what[0] != 'C')
			{
				if(IsIgnoredError(debugInfo.source, debugInfo.currentline))
					return;
				else
					break;
			}			
		}

		if(lua_getstack(m_breakedlvm, 0, &debugInfo) == 1)
		{
			lua_getinfo(m_breakedlvm, "Sl", &debugInfo);
		}

		m_previousExecutionMode = m_executionMode;
		m_executionMode = DebuggerExecutionMode_Error;

		MainHook(debugInfo);
	}

	void LuaDebugger::MainHook(lua_Debug & debugInfo)
	{
		// Have we stopped?
		if(m_executionMode == DebuggerExecutionMode_Break || m_executionMode == DebuggerExecutionMode_Error)
		{
			m_localNestedCalls = 0;
			
			// We stopped, so inform the listener
			if(m_executionMode == DebuggerExecutionMode_Break)
				m_listener->OnBreak(m_breakedlvm, debugInfo.source, debugInfo.currentline);
			else
				m_listener->OnError(m_breakedlvm, debugInfo.source, debugInfo.currentline, lua_tostring(m_breakedlvm, lua_gettop(m_breakedlvm)));
		}

		// Keep processing events until we are allowed to run again
		while(1)
		{
			DebuggerExecutionMode prevMode = m_executionMode;
			m_listener->OnHookLine();

			if(m_executionMode != DebuggerExecutionMode_Break && m_executionMode != DebuggerExecutionMode_Error)
				break;

			// We were running but now we've stopped
			if(prevMode != DebuggerExecutionMode_Break && m_executionMode != DebuggerExecutionMode_Error)
			{
				m_listener->OnBreak(m_breakedlvm, debugInfo.source, debugInfo.currentline);
				m_localNestedCalls = 0;
			}

			m_listener->OnIdle();

			if(m_shutdown)
				return;
		} 

		m_listener->OnRun();

		// Break immediately on next line if we're stepping into
		if(m_executionMode == DebuggerExecutionMode_StepInto)
		{		
			m_previousExecutionMode = DebuggerExecutionMode_Go;
			m_executionMode = DebuggerExecutionMode_Break;
		}
	}

	const char *LuaDebugger::NormalizedFilename(const char* filenameToNormalize)
	{
		size_t filenameToNormalizeLength = strlen(filenameToNormalize);
		if (filenameToNormalizeLength > m_normalizedFilenameLength)
		{
			delete[] m_normalizedFilename;
			m_normalizedFilename = new char[filenameToNormalizeLength + 1];
			m_normalizedFilenameLength = filenameToNormalizeLength;
		}
		char* dest = m_normalizedFilename;
		const char* src = filenameToNormalize;
		while (*src)
		{
			if (*src == '\\')
				*dest = '/';
			else
				*dest = tolower(*src);
			++dest;
			++src;
		}
		*dest = 0;
		return m_normalizedFilename;
	}

}	// namespace tilde
