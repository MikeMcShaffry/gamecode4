
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

#ifndef TILDE_INCLUDED_LuaDebuggerHost_h
#define TILDE_INCLUDED_LuaDebuggerHost_h

#include <lua.h>

namespace tilde
{
	class SendMessageBuffer;
	class ReceiveMessageBuffer;

	// ---------------------------------------------------------------------------------------------------------------
	// LuaDebuggerHost
	// ---------------------------------------------------------------------------------------------------------------

	class LuaDebuggerHost
	{
	public:
		//! Transmits the specified buffer of data to the remote host.
		virtual void SendDebuggerData(const void * data, int size) = 0;

		//! Closes the connection to the remote host; usually caused by an internal error in the debugger.
		virtual void CloseDebuggerConnection() = 0;

		//! Attaches the hook function to the main thread, and any threads the client wants monitored.
		virtual bool AttachLuaHook(lua_State * vm, lua_Hook hook, int mask, int count) = 0;

		//! Removes the hook function from all threads.
		virtual void DetachLuaHook(lua_State * vm, lua_Hook hook) = 0;

		//! Attaches the hook function to capture printf()s
		virtual void AttachPrintfHook( void (* hook)(const char *) ) = 0;

		//! Removes the hook from capturing printf()s
		virtual void DetachPrintfHook( void (* hook)(const char *) ) = 0;

		//! Allows the client to process an extended command.
		virtual void ReceiveExCommand(const char * command, int datalen, ReceiveMessageBuffer * data) = 0;

		//! Called while the debugger is breaked.
		/**
		The client should at least check for incoming messages and forward them to the
		LuaDebuggerComms::Receive()	function.
		*/
		virtual void OnIdle() = 0;

		//! Called while the debugger is running.
		/**
		The client might want to check for messages if it's been a while since the last breakpoint
		was hit.
		*/
		virtual void OnRun() = 0;

		//! Retrieves the name of the specified function.
		/**
		The ar structure is already filled with all the information required "Snl", and the
		function is on the lvm stack at funcIndex.

		The returned string should be a statically allocated buffer that remains valid until
		the next time the function is called. The possible result formats are:
		name [what]
		name (file:line) [what]
		name (info) [what]
		where
		name = ar->name
		what = ar->what
		file = ar->source
		line = ar->linedefined
		info = any information the client might have about a C functions
		*/
		virtual const char * GetFunctionName(lua_State * lvm, int funcIndex, lua_Debug * ar) = 0;

		virtual const char *LookupSourceFile( const char *pTarget ) = 0;
		virtual const char *LookupTargetFile( const char *pSource ) = 0;

	};
} // namespace tilde

#endif // TILDE_INCLUDED_LuaDebuggerHost_h
