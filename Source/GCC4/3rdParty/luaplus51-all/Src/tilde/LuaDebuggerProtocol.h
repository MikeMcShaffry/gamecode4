
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

#ifndef TILDE_INCLUDED_LuaDebuggerProtocol_h
#define TILDE_INCLUDED_LuaDebuggerProtocol_h

#define LUA_DEBUGGER_PROTOCOL_VERSION	18

namespace tilde
{
	enum DebuggerExecutionMode
	{
		DebuggerExecutionMode_Connecting = -1,			//!< We're still connecting, don't do anything yet

		DebuggerExecutionMode_Break,					//!< Don't run
		DebuggerExecutionMode_Error,					//!< The VM has stopped due to an error
		DebuggerExecutionMode_Go,						//!< Run until a breakpoint is hit/program terminates
		DebuggerExecutionMode_StepOver,					//!< Step over a single line, even if it is a function call
		DebuggerExecutionMode_StepInto,					//!< Step a single line, or into a function call
		DebuggerExecutionMode_StepOut,					//!< Step out of the current function

		DebuggerExecutionMode_Continue					//!< Continue with the last execution mode (protocol command only)
	};

	enum DebuggerTargetState
	{
		DebuggerTargetState_Connecting = -1,		//!< We're still connecting, don't do anything yet

		DebuggerTargetState_Connected,		// Connected but the machine hasn't been started yet
		DebuggerTargetState_Breaked,			// Machine has stopped (hit a breakpoint)
		DebuggerTargetState_Error,			// Machine has stopped because of an error
		DebuggerTargetState_Running,			// Machine is running
		DebuggerTargetState_Finished,			// Machine has finished running
		DebuggerTargetState_Disconnected		// Disconnected from machine
	};

	enum LuaType
	{
		// Lua 5.1.3 public types
		LuaType_NONE			= LUA_TNONE,
		LuaType_NIL				= LUA_TNIL,
		LuaType_BOOLEAN			= LUA_TBOOLEAN,
		LuaType_LIGHTUSERDATA	= LUA_TLIGHTUSERDATA,
		LuaType_NUMBER			= LUA_TNUMBER,
		LuaType_STRING			= LUA_TSTRING,
		LuaType_TABLE			= LUA_TTABLE,
		LuaType_FUNCTION		= LUA_TFUNCTION,
		LuaType_USERDATA		= LUA_TUSERDATA,
		LuaType_THREAD			= LUA_TTHREAD,
		LuaType_WSTRING			= LUA_TWSTRING,

		// Special Tilde types
		LuaType_TILDE_METATABLE		= LUA_TNONE - 1,
		LuaType_TILDE_ENVIRONMENT	= LUA_TNONE - 2,
		LuaType_TILDE_UPVALUES		= LUA_TNONE - 3
	};

	enum VarInfoFlag
	{
		VarInfoFlag_Valid		= 0x0001,
		VarInfoFlag_Expanded	= 0x0002,
		VarInfoFlag_HasEntries	= 0x0004,
		VarInfoFlag_HasMetadata	= 0x0008,
	};

	// From debugger -> target
	enum DebuggerMessage
	{
		DebuggerMessage_Connect,
		DebuggerMessage_Disconnect,
		DebuggerMessage_Run,
		DebuggerMessage_Reset,
		DebuggerMessage_AddBreakpoint,			// AddBreakpoint(int bkptid, string file, int line)
		DebuggerMessage_RemoveBreakpoint,		// RemoveBreakpoint(int bkptid)
		DebuggerMessage_ClearBreakpoints,
		DebuggerMessage_IgnoreError,
		DebuggerMessage_RetrieveStatus,
		DebuggerMessage_RetrieveThreads,
		DebuggerMessage_RetrieveLocals,
		DebuggerMessage_ExpandLocal,
		DebuggerMessage_CloseLocal,
		DebuggerMessage_RetrieveWatches,
		DebuggerMessage_UpdateWatch,
		DebuggerMessage_AddWatch,
		DebuggerMessage_RemoveWatch,
		DebuggerMessage_ExpandWatch,
		DebuggerMessage_CloseWatch,
		DebuggerMessage_ClearWatches,
		DebuggerMessage_RunSnippet,
		DebuggerMessage_RetrieveAutocompleteOptions,
		DebuggerMessage_ExCommand,

	};

	// From target -> debugger
	enum TargetMessage
	{
		TargetMessage_Connect,
		TargetMessage_Disconnect,
		TargetMessage_ValueCached,
		TargetMessage_DebugPrint,
		TargetMessage_ErrorMessage,
		TargetMessage_StateUpdate,
		TargetMessage_CallstackUpdate,
		TargetMessage_LocalsUpdate,
		TargetMessage_WatchUpdate,
		TargetMessage_ThreadsUpdate,
		TargetMessage_BreakpointAdded,
		TargetMessage_BreakpointRemoved,
		TargetMessage_UploadFile,
		TargetMessage_ExMessage,
		TargetMessage_SnippetResult,
		TargetMessage_AutocompleteOptions
	};

	enum DebuggerBreakpointState
	{
		DebuggerBreakpointState_Enabled,
		DebuggerBreakpointState_Disabled
	};

	struct DebuggerMessageHeader
	{
		int	m_length;
		int	m_command;
	};

}	// namespace tilde

#endif // TILDE_INCLUDED_LuaDebuggerProtocol_h
