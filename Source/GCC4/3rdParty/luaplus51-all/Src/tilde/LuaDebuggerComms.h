
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

#ifndef TILDE_INCLUDED_LuaDebuggerComms_h
#define TILDE_INCLUDED_LuaDebuggerComms_h

#include "tilde/LuaDebugger.h"

namespace tilde
{
	class LuaDebuggerComms;
	class SendMessageBuffer;
	class ReceiveMessageBuffer;

	//! Communications class for the lua debugger.
	class LuaDebuggerComms
	{
	public:
		LuaDebuggerComms(LuaDebuggerHost * connection);

		virtual ~LuaDebuggerComms();

		void RegisterState(const char* name, lua_State* lvm);

		struct DownloadedFile
		{
			DownloadedFile(int fileid, const char * name, int len)
				:
					m_fileID(fileid), m_name(name), m_length(len)
			{
				m_data = new u8 [len];
			}

			~DownloadedFile()
			{
				delete [] m_data;
			}

			int		m_fileID;
			String	m_name;
			int		m_length;
			u8		* m_data;
		};

		struct AutoCompleteResult
		{
			AutoCompleteResult(LuaDebuggerValue value, LuaType valueType)
				: m_value(value), m_valueType(valueType)
			{

			}

			LuaDebuggerValue	m_value;
			LuaType				m_valueType;
		};

		typedef std::vector<DownloadedFile *>	DownloadedFileCollection;

//		lua_State *			GetMainLVM() const	{ return m_mainLVM; }
		LuaDebugger *		GetDebugger() const	{ return m_debugger; }
		LuaDebuggerHost *	GetHost() const		{ return m_host; }

		bool	Open();
		void	Close();
		int		GetReceiveBufferCapacity() const;
		void	Receive(const u8 * data, int size);

		void SendExMessage(const char * command, const void * data, int datasize);
		void SendProfileResults(const char * varname);

		// LuaDebugger interface
		// ---------------------

		void	OnHookCall();
		void	OnHookReturn();
		void	OnHookLine();
		void	OnError(lua_State * lvm, const char * file, int line, const char * message);
		void	OnBreak(lua_State * lvm, const char * file, int line);
		void	OnIdle();
		void	OnRun();

		void	OnCached(LuaDebuggerObjectID value, LuaType luatype, const char * desc);

	private:

		LuaDebuggerComms(const LuaDebuggerComms &);
		LuaDebuggerComms & operator = (const LuaDebuggerComms &);
		
		typedef void (LuaDebuggerComms:: * MessageHandler)(int datalen);
		struct DebuggerMessageInfo
		{
			DebuggerMessage id;
			const char * name;
			MessageHandler handler;
		};

		static const DebuggerMessageInfo s_debuggerMessages [];

		LuaDebuggerObjectID	GetCurrentThreadID();

		void			SendStatusUpdate(lua_State * lvm, LuaDebuggerObjectID threadid, int stackFrame);

		void			FatalError(const char * message);

		void			SendMessages();
		void			ProcessMessages();

		void			IgnoreMessage(int datalen);

		void			ProcessMessage_AddBreakpoint(int datalen);
		void			ProcessMessage_RemoveBreakpoint(int datalen);
		void			ProcessMessage_ClearBreakpoints(int datalen);
		void			ProcessMessage_IgnoreError(int datalen);
		void			ProcessMessage_RetrieveStatus(int datalen);
		void			ProcessMessage_RetrieveThreads(int datalen);
		void			ProcessMessage_Run(int datalen);
		void			ProcessMessage_RetrieveLocals(int datalen);
		void			ProcessMessage_ExpandLocal(int datalen);
		void			ProcessMessage_CloseLocal(int datalen);

		void			ExpandVariable( VariableInfo * var );
		void			CloseVariable( VariableInfo * var );

		void			ProcessMessage_RetrieveWatches(int datalen);
		void			ProcessMessage_UpdateWatch(int datalen);
		void			ProcessMessage_AddWatch(int datalen);
		void			ProcessMessage_RemoveWatch(int datalen);
		void			ProcessMessage_ExpandWatch(int datalen);
		void			ProcessMessage_CloseWatch(int datalen);
		void			ProcessMessage_RunSnippet(int datalen);
		void			ProcessMessage_RetrieveAutocompleteOptions(int datalen);
		void			ProcessMessage_ExCommand(int datalen);

		void			CreateMessage_Connect();
		void			CreateMessage_Disconnect(const char * message);
		void			CreateMessage_StateUpdate(DebuggerTargetState mode, LuaDebuggerObjectID thread, const char * file, int line);
		void			CreateMessage_ErrorMessage(const char * message);
		void			CreateMessage_BreakpointAdded(int bkptid, bool success);
		void			CreateMessage_BreakpointRemoved(int bkptid);
		void			CreateMessage_CallstackUpdate(LuaDebuggerObjectID currentThread, const StackFrameCollection & stackFrames, int currentFrame);
		void			CreateMessage_ValueCached(LuaDebuggerObjectID value, LuaType luatype, const char * desc);

		void			WriteVariableExpansions(VariableInfo * root, int depth);
		void			WriteVariableInfo(VariableInfo * var, int depth);

		void			CreateMessage_LocalsUpdate(LuaDebuggerObjectID threadid, VariableInfo * rootvar, int stackIndex, bool cacheFlush);
		void			CreateMessage_WatchUpdate(LuaDebuggerObjectID threadid, WatchCollection * watches);
		void			CreateMessage_ThreadsUpdate();

		void			CreateMessage_SnippetResult(bool success, const char * output, const char * result);
		void			CreateMessage_AutocompleteOptions(int seqid, std::vector<AutoCompleteResult> * values, const char * message);

		void			CreateMessage_ExMessage(const char * command, const void * data, int datasize);

		DownloadedFile *FindFile(int fileid);


	private:
		typedef std::map<String, lua_State*> NameToLuaStateMap;
		NameToLuaStateMap		m_nameToLuaStateMap;
//		lua_State				* m_mainLVM;
		LuaDebuggerHost			* m_host;
		LuaDebugger				* m_debugger;
		SendMessageBuffer		* m_sendBuffer;
		ReceiveMessageBuffer	* m_receiveBuffer;

		DownloadedFileCollection	m_downloadedFiles;
		DownloadedFile				* m_downloadInProgress;
		int							m_downloadBytesReceived;

		DebuggerTargetState	m_lastStateUpdateMode;
		LuaDebuggerObjectID m_lastStateUpdateThread;
		int					m_lastStateUpdateLine;
		String				m_lastStateUpdateFile;
	};

}	// namespace tilde

#endif  // TILDE_INCLUDED_LuaDebuggerComms_h
