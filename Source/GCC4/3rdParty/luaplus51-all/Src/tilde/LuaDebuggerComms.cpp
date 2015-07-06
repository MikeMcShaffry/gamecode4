
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

#include "tilde/LuaDebuggerComms.h"

#include <stdlib.h>

#include "lua.h"
#include "lauxlib.h"

#include "tilde/LuaDebuggerProtocol.h"
#include "tilde/ReceiveMessageBuffer.h"
#include "tilde/SendMessageBuffer.h"

namespace tilde
{
	const LuaDebuggerComms::DebuggerMessageInfo LuaDebuggerComms::s_debuggerMessages [] = 
	{
		{ DebuggerMessage_Connect,				"Connect",			&LuaDebuggerComms::IgnoreMessage },
		{ DebuggerMessage_Disconnect,			"Disconnect",		&LuaDebuggerComms::IgnoreMessage },
		{ DebuggerMessage_Run,					"Run",				&LuaDebuggerComms::ProcessMessage_Run },
		{ DebuggerMessage_Reset,				"Reset",			&LuaDebuggerComms::IgnoreMessage },
		{ DebuggerMessage_AddBreakpoint,		"AddBreakpoint",	&LuaDebuggerComms::ProcessMessage_AddBreakpoint },
		{ DebuggerMessage_RemoveBreakpoint,		"RemoveBreakpoint",	&LuaDebuggerComms::ProcessMessage_RemoveBreakpoint },
		{ DebuggerMessage_ClearBreakpoints,		"ClearBreakpoints",	&LuaDebuggerComms::ProcessMessage_ClearBreakpoints },
		{ DebuggerMessage_IgnoreError,			"IgnoreError",		&LuaDebuggerComms::ProcessMessage_IgnoreError },
		{ DebuggerMessage_RetrieveStatus,		"RetrieveStatus",	&LuaDebuggerComms::ProcessMessage_RetrieveStatus },
		{ DebuggerMessage_RetrieveThreads,		"RetrieveThreads",	&LuaDebuggerComms::ProcessMessage_RetrieveThreads },
		{ DebuggerMessage_RetrieveLocals,		"RetrieveLocals",	&LuaDebuggerComms::ProcessMessage_RetrieveLocals },
		{ DebuggerMessage_ExpandLocal,			"ExpandLocal",		&LuaDebuggerComms::ProcessMessage_ExpandLocal },
		{ DebuggerMessage_CloseLocal,			"CloseLocal",		&LuaDebuggerComms::ProcessMessage_CloseLocal },
		{ DebuggerMessage_RetrieveWatches,		"RetrieveWatches",	&LuaDebuggerComms::ProcessMessage_RetrieveWatches },
		{ DebuggerMessage_UpdateWatch,			"UpdateWatch",		&LuaDebuggerComms::ProcessMessage_UpdateWatch },
		{ DebuggerMessage_AddWatch,				"AddWatch",			&LuaDebuggerComms::ProcessMessage_AddWatch },
		{ DebuggerMessage_RemoveWatch,			"RemoveWatch",		&LuaDebuggerComms::ProcessMessage_RemoveWatch },
		{ DebuggerMessage_ExpandWatch,			"ExpandWatch",		&LuaDebuggerComms::ProcessMessage_ExpandWatch },
		{ DebuggerMessage_CloseWatch,			"CloseWatch",		&LuaDebuggerComms::ProcessMessage_CloseWatch },
		{ DebuggerMessage_ClearWatches,			"ClearWatches",		&LuaDebuggerComms::IgnoreMessage },
		{ DebuggerMessage_RunSnippet,			"RunSnippet",		&LuaDebuggerComms::ProcessMessage_RunSnippet },
		{ DebuggerMessage_RetrieveAutocompleteOptions,		"RetrieveAutocompleteOptions",		&LuaDebuggerComms::ProcessMessage_RetrieveAutocompleteOptions },
		{ DebuggerMessage_ExCommand,			"ExCommand",		&LuaDebuggerComms::ProcessMessage_ExCommand },
	};

	LuaDebuggerComms::LuaDebuggerComms(LuaDebuggerHost * host) 
		: 
//			m_mainLVM(NULL),
			m_host(host),
			m_debugger(NULL),
			m_sendBuffer(NULL),
			m_receiveBuffer(NULL),
			m_downloadInProgress(NULL),
			m_downloadBytesReceived(0),
			m_lastStateUpdateMode(DebuggerTargetState_Connecting),
			m_lastStateUpdateThread(0),
			m_lastStateUpdateLine(-1)
	{
		m_sendBuffer = new SendMessageBuffer(TILDE_SEND_BUFFER_SIZE, TILDE_SEND_BUFFER_ALIGN);
		m_receiveBuffer = new ReceiveMessageBuffer(TILDE_RECEIVE_BUFFER_SIZE, TILDE_RECEIVE_BUFFER_ALIGN);
		m_debugger = new LuaDebugger(this);

		for(int index = 0; index < sizeof(s_debuggerMessages) / sizeof(DebuggerMessageInfo); ++ index)
			TILDE_ASSERT_MSG(s_debuggerMessages[index].id == (DebuggerMessage) index, "The entries in the s_debuggerMessages table are out of order; entry %d (%s) is incorrect", s_debuggerMessages[index].id, s_debuggerMessages[index].name);
	}

	LuaDebuggerComms::~LuaDebuggerComms()
	{
		TILDE_ASSERT(!m_debugger->IsConnected() || m_debugger->IsShutdown());

		delete m_debugger;
		delete m_receiveBuffer;
		delete m_sendBuffer;
	}

	static void InitialiseThreadTable(lua_State* lvm)
	{
		// Create the cache
		lua_newtable(lvm);
		int cacheIndex = lua_gettop(lvm);

		// Create an entry for the main thread
		lua_pushthread(lvm);
		lua_newtable(lvm);
		int mainTableIndex = lua_gettop(lvm);

		lua_pushstring(lvm, "name");
		lua_pushstring(lvm, "<<main>>");
		lua_settable(lvm, mainTableIndex);

		lua_pushstring(lvm, "threadid");
		lua_pushnumber(lvm, 0);
		lua_settable(lvm, mainTableIndex);

		lua_pushstring(lvm, "state");
		lua_pushnumber(lvm, 0);	// Ready
		lua_settable(lvm, mainTableIndex);

		// Register the main thread in the ThreadTable
		lua_settable(lvm, cacheIndex);

		// Put the ThreadTable into the registry
		lua_pushstring(lvm, "ThreadTable");
		lua_pushvalue(lvm, cacheIndex);
		lua_settable(lvm, LUA_REGISTRYINDEX);

		// Put the ThreadTable into the globals as well
		lua_pushstring(lvm, "ThreadTable");
		lua_pushvalue(lvm, cacheIndex);
		lua_settable(lvm, LUA_GLOBALSINDEX);

		// Pop the cache and metatable from the stack
		lua_pop(lvm, 1);
	}


	void LuaDebuggerComms::RegisterState(const char* name, lua_State* lvm)
	{
		m_nameToLuaStateMap[name] = lvm;
//		m_mainLVM = lvm;
		InitialiseThreadTable(lvm);
	}

	bool LuaDebuggerComms::Open()
	{
		m_lastStateUpdateMode = DebuggerTargetState_Connecting;
		m_lastStateUpdateThread = 0;
		m_lastStateUpdateLine = -1;

		for (NameToLuaStateMap::iterator it = m_nameToLuaStateMap.begin(); it != m_nameToLuaStateMap.end(); ++it)
		{
			if(!m_debugger->Connect((*it).second))
			{
				CreateMessage_Disconnect("The lua debugger cannot attach to the main virtual machine. There may be another debugger connected, or a profiler active.\n");
				SendMessages();
				return false;
			}
		}

		CreateMessage_Connect();
		SendMessages();

		// Wait for the host to set a run mode, and give it a chance to update everything it wants
		while(m_debugger->GetExecutionMode() == DebuggerExecutionMode_Connecting)
		{
			m_host->OnIdle();
			OsSleep(10);
			if(!m_debugger->IsConnected())
				return false;
		}

		return true;
	}

	void LuaDebuggerComms::Close()
	{
		if(m_debugger->IsConnected())
			m_debugger->Disconnect();

		for(DownloadedFileCollection::iterator iter = m_downloadedFiles.begin(); iter != m_downloadedFiles.end(); ++iter)
		{
			delete *iter;
		}
		m_downloadedFiles.clear();
	}


	int LuaDebuggerComms::GetReceiveBufferCapacity() const
	{
		return m_receiveBuffer->GetSpaceAvailable();
	}

	void LuaDebuggerComms::Receive(const u8 * data, int size)
	{
		m_receiveBuffer->Write(data, size);

		TILDE_PRINT("LuaDebuggerComms::OnReceive(); received %d bytes\n", size);

		ProcessMessages();
	}

	void LuaDebuggerComms::SendExMessage(const char * command, const void * data, int datasize)
	{
		CreateMessage_ExMessage(command, data, datasize);
		SendMessages();
	}

	void LuaDebuggerComms::FatalError(const char * message)
	{
		CreateMessage_ErrorMessage(message);
		SendMessages();
		m_host->CloseDebuggerConnection();
	}

	void LuaDebuggerComms::SendMessages()
	{
		while(!m_sendBuffer->IsEmpty())
		{
			int size;
			void * buffer = m_sendBuffer->RawRead(size);

			TILDE_PRINT("LuaDebuggerComms::SendMessages(); sending %d bytes\n", size);
			m_host->SendDebuggerData(buffer, size);
		}
	}

	void LuaDebuggerComms::ProcessMessages()
	{
		while(m_debugger->IsConnected() && !m_debugger->IsShutdown())
		{
			// If there's a download in progress, skim off the bytes until it's complete
			if(m_downloadInProgress != NULL)
			{
				int downloadsize = m_receiveBuffer->Read(m_downloadInProgress->m_data + m_downloadBytesReceived, m_downloadInProgress->m_length - m_downloadBytesReceived);

				m_downloadBytesReceived += downloadsize;
				TILDE_ASSERT(m_downloadBytesReceived <= m_downloadInProgress->m_length);
				if(m_downloadBytesReceived == m_downloadInProgress->m_length)
				{
					m_downloadInProgress = NULL;
					m_downloadBytesReceived = 0;
					m_receiveBuffer->AlignTo(4);
				}
			}

			int avail = m_receiveBuffer->GetDataAvailable();
			if(avail < sizeof(int))
				break;

			int msglen = m_receiveBuffer->Peek<int>();
			if(msglen > avail)
				break;

			// There's a complete message here to process
			m_receiveBuffer->Skip(sizeof(int));
			DebuggerMessage cmd = (DebuggerMessage) m_receiveBuffer->Read<int>();
			const DebuggerMessageInfo * msgInfo = (cmd < 0 || cmd > sizeof(s_debuggerMessages)/sizeof(DebuggerMessageInfo)) ? NULL : &s_debuggerMessages[(int) cmd];

			TILDE_PRINT("LuaDebuggerComms::ProcessMessages(); received msg length %d, command %d (%s)\n", msglen, cmd, msgInfo ? msgInfo->name : "UNKNOWN");

			int beforeavail = m_receiveBuffer->GetDataAvailable();
			int datalen = msglen - sizeof(DebuggerMessageHeader);

			if(msgInfo == NULL)
				IgnoreMessage(datalen);
			else
			{
				MessageHandler handler = msgInfo->handler;
				(this->*handler)(datalen);
			}

			// Make sure the entire message was parsed from the buffer
			int afteravail = m_receiveBuffer->GetDataAvailable();
			TILDE_ASSERT(afteravail == beforeavail - datalen);
		}
	}

	void LuaDebuggerComms::IgnoreMessage(int datalen)
	{
		m_receiveBuffer->Skip(datalen);
	}

	void LuaDebuggerComms::ProcessMessage_AddBreakpoint(int datalen)
	{
		char filename[256];
		int bkptid = m_receiveBuffer->Read<int>();
		int filenamelen = m_receiveBuffer->ReadString(filename, sizeof(filename));	
		int line = m_receiveBuffer->Read<int>();

		bool success = m_debugger->AddBreakpoint(bkptid, filename, line);

		CreateMessage_BreakpointAdded(bkptid, success);
		SendMessages();
	}

	void LuaDebuggerComms::ProcessMessage_RemoveBreakpoint(int datalen)
	{
		int bkptid = m_receiveBuffer->Read<int>();

		bool success = m_debugger->RemoveBreakpoint(bkptid);

		CreateMessage_BreakpointRemoved(bkptid);
		SendMessages();
	}

	void LuaDebuggerComms::ProcessMessage_ClearBreakpoints(int datalen)
	{
		m_debugger->ClearBreakpoints();
	}

	void LuaDebuggerComms::ProcessMessage_IgnoreError(int datalen)
	{
		char filename[256];
		int filenamelen = m_receiveBuffer->ReadString(filename, sizeof(filename));
		int line = m_receiveBuffer->Read<int>();

		m_debugger->IgnoreError(filename, line);
	}

	void LuaDebuggerComms::ProcessMessage_RetrieveStatus(int datalen)
	{
		LuaDebuggerObjectID threadid = m_receiveBuffer->Read<LuaDebuggerObjectID>();
		int stackIndex = m_receiveBuffer->Read<int>();

		lua_State * lvm = m_debugger->LookupThreadID(threadid);
		if(lvm != NULL)
		{
			SendStatusUpdate(lvm, threadid, stackIndex);
		}
	}

	void LuaDebuggerComms::ProcessMessage_RetrieveThreads(int datalen)
	{
		m_debugger->UpdateThreads();
		CreateMessage_ThreadsUpdate();
		SendMessages();
	}

	void LuaDebuggerComms::ProcessMessage_Run(int datalen)
	{
		DebuggerExecutionMode mode = (DebuggerExecutionMode) m_receiveBuffer->Read<int>();
		m_debugger->SetExecutionMode(mode);
	}

	void LuaDebuggerComms::ProcessMessage_RetrieveLocals(int datalen)
	{
		LuaDebuggerObjectID threadid = m_receiveBuffer->Read<LuaDebuggerObjectID>();
		int stackIndex = m_receiveBuffer->Read<int>();

		lua_State * lvm = m_debugger->LookupThreadID(threadid);
		if(lvm != NULL)
		{
			bool cacheFlush = m_debugger->UpdateLocalVariables(lvm, stackIndex);
			CreateMessage_LocalsUpdate(threadid, m_debugger->GetLocalVariables(), stackIndex, cacheFlush);

			SendMessages();
		}
	}

	void LuaDebuggerComms::ProcessMessage_ExpandLocal(int datalen)
	{
		ExpandVariable(m_debugger->GetLocalVariables());
	}

	void LuaDebuggerComms::ProcessMessage_CloseLocal(int datalen)
	{
		CloseVariable(m_debugger->GetLocalVariables());
	}

	void LuaDebuggerComms::ProcessMessage_RetrieveWatches(int datalen)
	{
		LuaDebuggerObjectID threadid = m_receiveBuffer->Read<LuaDebuggerObjectID>();
		int stackIndex = m_receiveBuffer->Read<int>();

		if(!m_debugger->GetWatchVariables()->empty())
		{
			lua_State * lvm = m_debugger->LookupThreadID(threadid);
			if(lvm != NULL)
			{
				m_debugger->UpdateWatchVariables(lvm, stackIndex);
				CreateMessage_WatchUpdate(threadid, m_debugger->GetWatchVariables());

				SendMessages();
			}
		}
	}

	void LuaDebuggerComms::ProcessMessage_UpdateWatch(int datalen)
	{
		int watchID = m_receiveBuffer->Read<int>();
		LuaDebuggerObjectID threadid = m_receiveBuffer->Read<LuaDebuggerObjectID>();
		int stackIndex = m_receiveBuffer->Read<int>();

		if(!m_debugger->GetWatchVariables()->empty())
		{
			lua_State * lvm = m_debugger->LookupThreadID(threadid);
			if(lvm != NULL)
			{
				m_debugger->UpdateWatchVariable(watchID, lvm, stackIndex);
				CreateMessage_WatchUpdate(threadid, m_debugger->GetWatchVariables());

				SendMessages();
			}
		}
	}

	void LuaDebuggerComms::ProcessMessage_AddWatch(int datalen)
	{
		char expr[1024];
		m_receiveBuffer->ReadString(expr, sizeof(expr));
		int watchid = m_receiveBuffer->Read<int>();

		m_debugger->AddWatch(expr, watchid);
	}

	void LuaDebuggerComms::ProcessMessage_RemoveWatch(int datalen)
	{
		int watchid = m_receiveBuffer->Read<int>();

		m_debugger->RemoveWatch(watchid);
	}

	void LuaDebuggerComms::ProcessMessage_ExpandWatch(int datalen)
	{
		int watchid = m_receiveBuffer->Read<int>();

		LuaWatchInfo * watch = m_debugger->FindWatch(watchid);
		if(watch != NULL)
			ExpandVariable(&watch->m_variable);
		else
			m_receiveBuffer->Skip(datalen - sizeof(int));
	}

	void LuaDebuggerComms::ProcessMessage_CloseWatch(int datalen)
	{
		int watchid = m_receiveBuffer->Read<int>();

		LuaWatchInfo * watch = m_debugger->FindWatch(watchid);
		if(watch != NULL)
			CloseVariable(&watch->m_variable);
		else
			m_receiveBuffer->Skip(datalen - sizeof(int));
	}

	static lua_State * RunSnippetPrintfHook_lvm = NULL;
	static bool RunSnippetPrintfHook_guard = false;
	static void RunSnippetPrintfHook(const char * msg)
	{
		LuaDebugger::stack_restore_point restore(RunSnippetPrintfHook_lvm, 8);

		if(RunSnippetPrintfHook_guard)
			return;

		RunSnippetPrintfHook_guard = true;

		lua_pushstring(RunSnippetPrintfHook_lvm, "LuaDebugger");
		lua_gettable(RunSnippetPrintfHook_lvm, LUA_REGISTRYINDEX);
		int registryIndex = lua_gettop(RunSnippetPrintfHook_lvm);

		lua_pushstring(RunSnippetPrintfHook_lvm, "RunSnippetOutput");
		lua_gettable(RunSnippetPrintfHook_lvm, registryIndex);

		lua_pushstring(RunSnippetPrintfHook_lvm, msg);
		lua_concat(RunSnippetPrintfHook_lvm, 2);

		lua_pushstring(RunSnippetPrintfHook_lvm, "RunSnippetOutput");
		lua_pushvalue(RunSnippetPrintfHook_lvm, -2);
		lua_settable(RunSnippetPrintfHook_lvm, registryIndex);

		RunSnippetPrintfHook_guard = false;
	}

	void LuaDebuggerComms::ProcessMessage_RunSnippet(int datalen)
	{
		TILDE_PRINT("LuaDebuggerComms::ProcessMessage_RunSnippet(); executing script...\n");

		lua_State* snippetlvm = m_nameToLuaStateMap.begin()->second;
		LuaDebugger::stack_restore_point restore(snippetlvm, 8);

		// Set up buffers for print spam
		RunSnippetPrintfHook_lvm = snippetlvm;

		lua_pushstring(snippetlvm, "LuaDebugger");
		lua_gettable(snippetlvm, LUA_REGISTRYINDEX);
		int registryIndex = lua_gettop(snippetlvm);

		lua_pushstring(snippetlvm, "RunSnippetOutput");
		lua_pushstring(snippetlvm, "");
		lua_settable(snippetlvm, registryIndex);

		// Get the message arguments
		LuaDebuggerObjectID threadid = m_receiveBuffer->Read<LuaDebuggerObjectID>();
		int stackFrame = m_receiveBuffer->Read<int>();
		m_receiveBuffer->PushString(snippetlvm);
		int snippetIndex = lua_gettop(snippetlvm);

		lua_State * lvm = threadid == 0 ? snippetlvm : m_debugger->LookupThreadID(threadid);
		if(lvm != NULL)
		{
			const char * buffer = lua_tostring(snippetlvm, snippetIndex);
			if(buffer[0] == '=')
			{
				lua_pushstring(snippetlvm, "return ");
				lua_pushstring(snippetlvm, buffer + 1);
				lua_concat(snippetlvm, 2);
				lua_remove(snippetlvm, snippetIndex);
			}

			// Set hooks
			m_debugger->EnableHook(false);
			m_host->AttachPrintfHook(&RunSnippetPrintfHook);

			int resultsIndex = lua_gettop(snippetlvm);
			int results = m_debugger->CallChunk(lvm, stackFrame, "Tilde console snippet", true);	// Pops "return xxx" and pushes result

			// Print the results into the top item on the stack
			if(results > 0)
			{
				lua_getglobal(snippetlvm, "tostring");
				int tostringIndex = lua_gettop(snippetlvm);

				lua_pushstring(snippetlvm, "");
				int accumulatorIndex = lua_gettop(snippetlvm);

				for(int i = 0; i < results; i++)
				{
					if(i > 0)
						lua_pushstring(snippetlvm, ", ");

					lua_pushvalue(snippetlvm, tostringIndex);
					lua_pushvalue(snippetlvm, resultsIndex + i);
					lua_call(snippetlvm, 1, 1);

					lua_concat(snippetlvm, i == 0 ? 2 : 3);
				}

				// Replace the results with the concatenated string
				lua_insert(snippetlvm, resultsIndex);
				lua_pop(snippetlvm, lua_gettop(snippetlvm) - resultsIndex);
			}
			else if(results == 0)
				lua_pushstring(snippetlvm, "");

			// Restore hooks
			m_host->DetachPrintfHook(&RunSnippetPrintfHook);
			m_debugger->EnableHook(true);

			// Get the output/result strings
			lua_pushstring(snippetlvm, "RunSnippetOutput");
			lua_gettable(snippetlvm, registryIndex);
			const char * outputString = lua_tostring(snippetlvm, -1);
			const char * resultString = lua_tostring(snippetlvm, resultsIndex);

			CreateMessage_SnippetResult(results >= 0, outputString, resultString);
		}
		else
		{
			CreateMessage_SnippetResult(false, "", "Could not find thread context");
		}

		// Update the program state now, in case anything changed
		if(m_debugger->GetBreakedThread())
		{
			// Send locals
			bool cacheFlush = m_debugger->UpdateLocalVariables(lvm, stackFrame);
			CreateMessage_LocalsUpdate(threadid, m_debugger->GetLocalVariables(), stackFrame, cacheFlush);

			// Send watches
			m_debugger->UpdateWatchVariables(lvm, stackFrame);
			CreateMessage_WatchUpdate(threadid, m_debugger->GetWatchVariables());

		}
		// Send threads
		m_debugger->UpdateThreads();
		CreateMessage_ThreadsUpdate();

		SendMessages();
	}

	void LuaDebuggerComms::ProcessMessage_RetrieveAutocompleteOptions(int datalen)
	{
		TILDE_PRINT("LuaDebuggerComms::ProcessMessage_RetrieveAutocompleteOptions(); executing script...\n");

		lua_State* autocompletelvm = m_nameToLuaStateMap.begin()->second;
		LuaDebugger::stack_restore_point restore(autocompletelvm, 8);

		// Get the message arguments
		int seqid = m_receiveBuffer->Read<int>();
		LuaDebuggerObjectID threadid = m_receiveBuffer->Read<LuaDebuggerObjectID>();
		int stackFrame = m_receiveBuffer->Read<int>();
		m_receiveBuffer->PushString(autocompletelvm);
		int snippetIndex = lua_gettop(autocompletelvm);

		lua_State * lvm = threadid == 0 ? autocompletelvm : m_debugger->LookupThreadID(threadid);
		if(lvm != NULL)
		{
			const char * buffer = lua_tostring(autocompletelvm, snippetIndex);
			lua_pushstring(autocompletelvm, "return ");
			lua_pushstring(autocompletelvm, buffer);
			lua_concat(autocompletelvm, 2);
			lua_remove(autocompletelvm, snippetIndex);

			// Set hooks
			m_debugger->EnableHook(false);

			int resultsIndex = lua_gettop(autocompletelvm);
			int results = m_debugger->CallChunk(lvm, stackFrame, "Tilde console autocomplete expression", false);	// Pops "return xxx" and pushes result

			// We should get a single table as the result
			if(results == 1 && lua_istable(lvm, resultsIndex))
			{
				std::vector<AutoCompleteResult> values;
				lua_pushnil(lvm);
				while(lua_next(lvm, resultsIndex) != 0)
				{
					int keyIndex = lua_gettop(lvm) - 1;
					int valueIndex = lua_gettop(lvm);
					if(lua_isstring(lvm, keyIndex) || lua_isnumber(lvm, keyIndex))
						values.push_back(AutoCompleteResult(m_debugger->LookupValue(lvm, keyIndex), (LuaType) lua_type(lvm, valueIndex)));

					lua_pop(lvm, 1);
				}			

				CreateMessage_AutocompleteOptions(seqid, &values, "");
			}
			else if(results == -1)
			{
				CreateMessage_AutocompleteOptions(seqid, NULL, lua_tostring(lvm, resultsIndex));
			}
			else
			{
				CreateMessage_AutocompleteOptions(seqid, NULL, "Expression does not evaluate to a table");
			}

			// Restore hooks
			m_debugger->EnableHook(true);
		}
		else
		{
			CreateMessage_AutocompleteOptions(seqid, NULL, "Could not find thread context");
		}
		SendMessages();
	}

	void LuaDebuggerComms::ProcessMessage_ExCommand(int datalen)
	{
		char command[256];
		int before = m_receiveBuffer->GetDataAvailable();
		m_receiveBuffer->ReadString(command, sizeof(command));
		int after = m_receiveBuffer->GetDataAvailable();
		m_host->ReceiveExCommand(command, datalen - (before - after), m_receiveBuffer);
	}

	LuaDebuggerComms::DownloadedFile * LuaDebuggerComms::FindFile(int fileid)
	{
		for(DownloadedFileCollection::iterator iter = m_downloadedFiles.begin(); iter != m_downloadedFiles.end(); ++iter)
		{
			if((*iter)->m_fileID == fileid)
				return *iter;
		}
		return NULL;	
	}

	void LuaDebuggerComms::ExpandVariable(VariableInfo * var)
	{
		int pathlength = m_receiveBuffer->Read<int>();
		for(int index = 0; index < pathlength; ++index)
		{
			LuaDebuggerValue value = m_receiveBuffer->ReadValue();
			VariableInfo temp(value);
			VariableExpansions::iterator iter = var->m_expansions.find(&temp);
			if(iter == var->m_expansions.end())
			{
				// Skip over remaining entries in the path
				for(++index; index < pathlength; ++index)
					m_receiveBuffer->ReadValue();
				return;
			}

			var = *iter;
		}
		var->Expand();
	}

	void LuaDebuggerComms::CloseVariable(VariableInfo * var)
	{
		int pathlength = m_receiveBuffer->Read<int>();
		for(int index = 0; index < pathlength; ++index)
		{
			LuaDebuggerValue value = m_receiveBuffer->ReadValue();
			VariableInfo temp(value);
			VariableExpansions::iterator iter = var->m_expansions.find(&temp);
			if(iter == var->m_expansions.end())
			{
				// Skip over remaining entries in the path
				for(++index; index < pathlength; ++index)
					m_receiveBuffer->ReadValue();
				return;
			}

			var = *iter;
		}
		var->Close();
	}

	void LuaDebuggerComms::OnHookCall()
	{
	//	ProcessMessages();
	}

	void LuaDebuggerComms::OnHookReturn()
	{
	//	ProcessMessages();
	}

	void LuaDebuggerComms::OnHookLine()
	{
	//	ProcessMessages();
	}

	void LuaDebuggerComms::OnError(lua_State * lvm, const char * file, int line, const char * message)
	{
		// Find out which thread we're in
		lua_pushthread(lvm);
		LuaDebuggerObjectID thread = m_debugger->LookupValue(lvm, lua_gettop(lvm)).GetObjectID();
		lua_pop(lvm, 1);

		// Send state update
		CreateMessage_StateUpdate(DebuggerTargetState_Error, thread, file, line);

		// Send error message
		CreateMessage_ErrorMessage(message);

		// Send full status update
		SendStatusUpdate(lvm, thread, -1);
	}

	void LuaDebuggerComms::OnBreak(lua_State *lvm, const char * file, int line)
	{
		// Find out which thread we're in
		lua_pushthread(lvm);
		LuaDebuggerObjectID thread = m_debugger->LookupValue(lvm, lua_gettop(lvm)).GetObjectID();
		lua_pop(lvm, 1);

		// Translate the filename
		const char *pSourceFilename = m_host->LookupSourceFile(file);

		// Send state update
		CreateMessage_StateUpdate(DebuggerTargetState_Breaked, thread, pSourceFilename, line);

		// Send full status update
		SendStatusUpdate(lvm, thread, -1);
	}

	void LuaDebuggerComms::SendStatusUpdate(lua_State * lvm, LuaDebuggerObjectID threadid, int stackFrame)
	{
		// Send callstack
		StackFrameCollection stackFrames;
		if(m_debugger->GetCallstack(lvm, stackFrames, stackFrame))
			CreateMessage_CallstackUpdate(threadid, stackFrames, stackFrame);

		// Send locals
		bool cacheFlush = m_debugger->UpdateLocalVariables(lvm, stackFrame);
		CreateMessage_LocalsUpdate(threadid, m_debugger->GetLocalVariables(), stackFrame, cacheFlush);

		// Send watches
		m_debugger->UpdateWatchVariables(lvm, stackFrame);
		CreateMessage_WatchUpdate(threadid, m_debugger->GetWatchVariables());

		// Send threads
		m_debugger->UpdateThreads();
		CreateMessage_ThreadsUpdate();

		SendMessages();
	}

	void LuaDebuggerComms::OnIdle()
	{
		m_host->OnIdle();
	}

	void LuaDebuggerComms::OnRun()
	{
		m_host->OnRun();

		// Send state update
		CreateMessage_StateUpdate(DebuggerTargetState_Running, 0, NULL, 0);
		SendMessages();
	}

	void LuaDebuggerComms::OnCached(LuaDebuggerObjectID value, LuaType luatype, const char * desc)
	{
		// Send the value details
		CreateMessage_ValueCached(value, luatype, desc);
		SendMessages();
	}

	void LuaDebuggerComms::CreateMessage_Connect()
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_Connect);
		m_sendBuffer->Write<int>(LUA_DEBUGGER_PROTOCOL_VERSION);
		m_sendBuffer->Write<int>((int) sizeof(LuaDebuggerObjectID));
		m_sendBuffer->Write<int>((int) sizeof(lua_Number));
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_Connect()");
	}

	void LuaDebuggerComms::CreateMessage_Disconnect(const char * message)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_Disconnect);
		m_sendBuffer->WriteString(message);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_Disconnect()");
	}

	void LuaDebuggerComms::CreateMessage_StateUpdate(DebuggerTargetState mode, LuaDebuggerObjectID thread, const char * file, int line)
	{
		// Only send message if something changed since last time
		if(
				m_lastStateUpdateMode != mode 
			||	m_lastStateUpdateThread != thread
			||	m_lastStateUpdateLine != line 
			|| (file == NULL && !m_lastStateUpdateFile.empty()) 
			|| (file != NULL && strcmp(file, m_lastStateUpdateFile.c_str()) != 0)
		)
		{
			m_lastStateUpdateMode = mode;
			m_lastStateUpdateThread = thread;
			m_lastStateUpdateLine = line;
			if(file == NULL)
				m_lastStateUpdateFile.clear();
			else
				m_lastStateUpdateFile = file;

			m_sendBuffer->Begin();
			m_sendBuffer->Write<int>(TargetMessage_StateUpdate);
			m_sendBuffer->Write<int>(mode);
			m_sendBuffer->Write<LuaDebuggerObjectID>(thread);
			m_sendBuffer->Write<int>(line);
			m_sendBuffer->WriteString(file);
			m_sendBuffer->End("LuaDebuggerComms::CreateMessage_StateUpdate()");
		}
	}

	void LuaDebuggerComms::CreateMessage_ErrorMessage(const char * message)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_ErrorMessage);
		m_sendBuffer->WriteString(message);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_ErrorMessage()");
	}

	void LuaDebuggerComms::CreateMessage_BreakpointAdded(int bkptid, bool success)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_BreakpointAdded);
		m_sendBuffer->Write<int>(bkptid);
		m_sendBuffer->Write<int>(success ? 1 : 0);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_BreakpointAdded()");
	}

	void LuaDebuggerComms::CreateMessage_BreakpointRemoved(int bkptid)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_BreakpointRemoved);
		m_sendBuffer->Write<int>(bkptid);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_BreakpointRemoved()");
	}

	void LuaDebuggerComms::CreateMessage_CallstackUpdate(LuaDebuggerObjectID threadid, const StackFrameCollection & stackFrames, int currentFrame)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_CallstackUpdate);
		m_sendBuffer->Write<LuaDebuggerObjectID>(threadid);
		m_sendBuffer->Write<int>(currentFrame);
		m_sendBuffer->Write<int>((int) stackFrames.size());

		for(StackFrameCollection::const_iterator iter = stackFrames.begin(); iter != stackFrames.end(); ++iter)
		{
			StackFrameInfo * frame = *iter;
			
			// Translate the filename
			const char *pSourceFilename = m_host->LookupSourceFile(frame->m_source.c_str());

			m_sendBuffer->WriteString(frame->m_function.c_str());
			m_sendBuffer->WriteString(pSourceFilename);
			m_sendBuffer->Write<int>(frame->m_line);
		}

		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_CallstackUpdate()");
	}

	void LuaDebuggerComms::CreateMessage_ValueCached(LuaDebuggerObjectID value, LuaType luatype, const char * desc)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_ValueCached);
		m_sendBuffer->Write<int>(luatype);
		m_sendBuffer->Write<LuaDebuggerObjectID>(value);
		m_sendBuffer->WriteString(desc);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_ValueCached()");
	}

	void LuaDebuggerComms::WriteVariableExpansions(VariableInfo * root, int depth)
	{
		// Reserve space to write the count at the end
		int expansionCountPos = m_sendBuffer->Write<int>(0);

		int expansionCount = 0;
		for(VariableExpansions::iterator iter = root->m_expansions.begin(); iter != root->m_expansions.end(); ++iter)
		{
			VariableInfo * var = *iter;

			// Output the full variable info if it has been modified, become invalid, or it has some expansions
			if(var->m_modified || !var->m_valid || var->m_expanded)
			{
				WriteVariableInfo(var, depth);
				++expansionCount;
			}
		}

		// Backfill the count
		m_sendBuffer->Write<int>(expansionCount, expansionCountPos);
	}

	void LuaDebuggerComms::WriteVariableInfo(VariableInfo * var, int depth)
	{
		m_sendBuffer->WriteValue(var->m_key);
		m_sendBuffer->WriteValue(var->m_value);

		int flags = 
				(var->m_valid ? VarInfoFlag_Valid : 0) 
			|	(var->m_expanded ? VarInfoFlag_Expanded : 0) 
			|	(var->m_hasEntries ? VarInfoFlag_HasEntries : 0)
			|	(var->m_hasMetadata ? VarInfoFlag_HasMetadata : 0); 
		
		m_sendBuffer->Write<short>(flags);
		m_sendBuffer->Write<short>(var->m_class);

		WriteVariableExpansions(var, depth + 1);
	}

	void LuaDebuggerComms::CreateMessage_LocalsUpdate(LuaDebuggerObjectID threadid, VariableInfo * rootvar, int stackIndex, bool cacheFlush)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_LocalsUpdate);
		m_sendBuffer->Write<LuaDebuggerObjectID>(threadid);
		m_sendBuffer->Write<int>(stackIndex);
		m_sendBuffer->Write<int>(cacheFlush ? 1 : 0);
		WriteVariableExpansions(rootvar, 1);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_LocalsUpdate()");
	}

	void LuaDebuggerComms::CreateMessage_ThreadsUpdate()
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_ThreadsUpdate);
		int countpos = m_sendBuffer->Write<int>(0);

		int count = 0;
		for(ThreadCollection::iterator iter = m_debugger->GetThreads()->begin(); iter != m_debugger->GetThreads()->end(); ++iter)
		{
			ThreadInfo * info = *iter;

			if(info->m_modified || !info->m_valid)
			{
				m_sendBuffer->Write<LuaDebuggerObjectID>(info->m_thread.GetObjectID());
				m_sendBuffer->Write<LuaDebuggerObjectID>(info->m_parent.GetObjectID());
				m_sendBuffer->WriteString(info->m_name.c_str());
				m_sendBuffer->Write<int>(info->m_threadID);
				m_sendBuffer->WriteString(info->m_currentFunction.c_str());

				m_sendBuffer->Write<int>(info->m_state);
				m_sendBuffer->Write<short>(info->m_stackSize);
				m_sendBuffer->Write<short>(info->m_stackUsed);
				m_sendBuffer->Write<lua_Number>(info->m_peakTime);
				m_sendBuffer->Write<lua_Number>(info->m_averageTime);
				
				m_sendBuffer->Write<short>(info->m_modified);
				m_sendBuffer->Write<short>(info->m_valid);

				TILDE_PRINT("Writing thread %s %d/%d %d %f %f\n", info->m_currentFunction.c_str(), info->m_thread.GetObjectID(), info->m_parent.GetObjectID(), info->m_state, info->m_peakTime, info->m_averageTime);

				++ count;
			}
		}

		m_sendBuffer->Write<int>(count, countpos);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_ThreadsUpdate()");
	}

	void LuaDebuggerComms::CreateMessage_WatchUpdate(LuaDebuggerObjectID threadid, WatchCollection * watches)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_WatchUpdate);
		m_sendBuffer->Write<LuaDebuggerObjectID>(threadid);
		int countpos = m_sendBuffer->Write<int>(0);

		int count = 0;
		for(WatchCollection::iterator iter = watches->begin(); iter != watches->end(); ++iter)
		{
			LuaWatchInfo * watch = *iter;
			VariableInfo * var = &watch->m_variable;

			if(var->m_modified || !var->m_valid || var->m_expanded)
			{
				m_sendBuffer->Write<int>(watch->m_watchid);
				WriteVariableInfo(&watch->m_variable, 1);
				++ count;
			}
		}

		m_sendBuffer->Write<int>(count, countpos);
		m_sendBuffer->End("LuaDebuggerComms::CreateMessage_WatchUpdate()");
	}

	void LuaDebuggerComms::CreateMessage_ExMessage(const char * command, const void * data, int datasize)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_ExMessage);
		m_sendBuffer->WriteString(command);
		m_sendBuffer->Write<int>(datasize);
		m_sendBuffer->RawWrite(data, datasize);
		m_sendBuffer->End("CreateMessage_ExMessage()");
	}


	void LuaDebuggerComms::CreateMessage_SnippetResult(bool success, const char * output, const char * result)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_SnippetResult);
		m_sendBuffer->Write<int>(success);
		m_sendBuffer->WriteString(output);
		m_sendBuffer->WriteString(result);
		m_sendBuffer->End("CreateMessage_SnippetOutput()");
	}

	void LuaDebuggerComms::CreateMessage_AutocompleteOptions(int seqid, std::vector<AutoCompleteResult> * values, const char * message)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_AutocompleteOptions);
		m_sendBuffer->Write<int>(seqid);
		m_sendBuffer->Write<int>(values == NULL ? -1 : (int) values->size());
		if(values != NULL)
		{
			for(std::vector<AutoCompleteResult>::iterator iter = values->begin(); iter != values->end(); ++iter)
			{
				m_sendBuffer->WriteValue(iter->m_value);
				m_sendBuffer->Write<int>(iter->m_valueType);
			}
		}
		m_sendBuffer->WriteString(message);
		m_sendBuffer->End("CreateMessage_AutocompleteOptions()");
	}

	static char escapeBuffer[1024];
	const char * XMLEscapeAttribute(const char * str)
	{
		char const * in = str;
		char * out = escapeBuffer;
		char c;
		while( (c = *in++) != '\0' && out < escapeBuffer + sizeof(escapeBuffer) - 8)
		{
			switch(c)
			{
			case '\"':
				strcpy(out, "&quot;");		
				out += 6;			
				break;

			default:
				*out++ = c;
				break;
			}
		}
		*out = '\0';

		return escapeBuffer;
	}

	void LuaDebuggerComms::SendProfileResults(const char * varname)
	{
		m_sendBuffer->Begin();
		m_sendBuffer->Write<int>(TargetMessage_UploadFile);
		m_sendBuffer->WriteString("generalPurposeProfile.luaprof.xml");

		m_sendBuffer->BeginString();

		m_sendBuffer->AppendString("<luaprofile>\n");

		{
			lua_State* profilelvm = m_nameToLuaStateMap.begin()->second;
			lua_State * lvm = profilelvm;
			lua_pushstring(lvm, varname);
			lua_rawget(lvm, LUA_GLOBALSINDEX);

			if(lua_istable(lvm, -1))
			{
				int profileIndex = lua_gettop(lvm);

				lua_pushstring(lvm, "funcs");
				lua_gettable(lvm, -2);
				if(lua_istable(lvm, -1))
				{
					int funcsIndex = lua_gettop(lvm);

					lua_pushnil(lvm);
					while(lua_next(lvm, funcsIndex) != 0)
					{
						int keyIndex = lua_gettop(lvm) - 1;
						int resultsIndex = lua_gettop(lvm);

						m_sendBuffer->AppendString("\t<function name=\"%s\" ", XMLEscapeAttribute(lua_tostring(lvm, keyIndex)));

						lua_pushnil(lvm);
						while(lua_next(lvm, resultsIndex) != 0)
						{
							lua_pushvalue(lvm, lua_gettop(lvm) - 1);
							const char * value = lua_tostring(lvm, -2);
							const char * key = lua_tostring(lvm, -1);
							m_sendBuffer->AppendString("%s=\"%s\" ", key, value);
							lua_pop(lvm, 2);
						}

						m_sendBuffer->AppendString("/>\n");
						lua_pop(lvm, 1);
					}
				}

				// Pop the funcs table
				lua_pop(lvm, 1);
			}

			// Pop the profile table
			lua_pop(lvm, 1);
		}

		m_sendBuffer->AppendString("</luaprofile>\n");

		m_sendBuffer->EndString();

		m_sendBuffer->End("LuaDebuggerComms::DownloadProfileResults()");

		SendMessages();
	}

}	// namespace tilde
