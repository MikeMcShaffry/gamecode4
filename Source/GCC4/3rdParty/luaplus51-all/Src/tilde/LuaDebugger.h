
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

#ifndef TILDE_INCLUDED_LuaDebugger_h
#define TILDE_INCLUDED_LuaDebugger_h

#include <vector>
#include <map>
#include <set>

#include "lua.h"

#include "tilde/HostConfig.h"
#include "tilde/LuaDebuggerProtocol.h"
#include "tilde/LuaDebuggerHost.h"

namespace tilde
{
	class LuaDebuggerComms;

	// ---------------------------------------------------------------------------------------------------------------
	// LuaDebuggerValue
	// ---------------------------------------------------------------------------------------------------------------

	class LuaDebuggerValue
	{
	public:
		LuaDebuggerValue()
		{
			m_type = LuaType_NONE;
			m_value.objectid = 0;
		}

		LuaDebuggerValue(LuaType type)
		{
			m_type = type;
			m_value.objectid = 0;
		}

		LuaDebuggerValue(LuaType type, LuaDebuggerObjectID value)
		{
			m_type = type;
			m_value.objectid = value;
		}

		LuaDebuggerValue(LuaType type, lua_Number value)
		{
			m_type = type;
			m_value.number = value;
		}

		int compare(const LuaDebuggerValue & rhs) const
		{
			int diff = (int) m_type - (int) rhs.m_type;
			if(diff != 0)
				return diff;
			else if (m_type == LuaType_NUMBER)
			{
				lua_Number ndiff = this->m_value.number - rhs.m_value.number;
				if(ndiff < 0.0f)		return -1;
				else if(ndiff > 0.0f)	return 1;
				else					return 0;
			}
			else
			{
				return (int) (this->m_value.objectid - rhs.m_value.objectid);
			}
		}

		LuaType					GetType() const			{ return m_type; }
		LuaDebuggerObjectID		GetObjectID() const		{ return m_value.objectid; }
		lua_Number				GetNumber() const		{ return m_value.number; }

	private:
		LuaType m_type;

		union
		{
			LuaDebuggerObjectID		objectid;
			lua_Number				number;
		} m_value;
	};

	inline int operator - (const LuaDebuggerValue & lhs, const LuaDebuggerValue & rhs)
	{
		return lhs.compare(rhs);
	}

	inline bool operator < (const LuaDebuggerValue & lhs, const LuaDebuggerValue & rhs)
	{
		return lhs.compare(rhs) < 0;
	}

	inline bool operator == (const LuaDebuggerValue & lhs, const LuaDebuggerValue & rhs)
	{
		return lhs.compare(rhs) == 0;
	}

	inline bool operator != (const LuaDebuggerValue & lhs, const LuaDebuggerValue & rhs)
	{
		return lhs.compare(rhs) != 0;
	}

	// ---------------------------------------------------------------------------------------------------------------
	// BreakpointInfo
	// ---------------------------------------------------------------------------------------------------------------

	struct BreakpointKey
	{
		BreakpointKey(const char * file, int line)
			: m_fileName(file), m_line(line)
		{}

		struct LessThanComparator
		{
			bool operator() (const BreakpointKey & lhs, const BreakpointKey & rhs) const
			{
				int diff = strcmp(lhs.m_fileName, rhs.m_fileName);
				if(diff < 0)
					return true;
				else if(diff > 0)
					return false;
				else
					return lhs.m_line < rhs.m_line;
			}
		};

		const char	* m_fileName;
		int			m_line;
	};

	struct BreakpointInfo
	{
		BreakpointInfo(int bkptid, const char * file, int line)
			: m_enabled(true), m_bkptid(bkptid), m_fileName(file), m_line(line)
		{}

		struct LessThanComparator
		{
			bool operator() (const BreakpointInfo * lhs, const BreakpointInfo * rhs) const
			{
				int diff = lhs->m_fileName.compare(rhs->m_fileName);
				if(diff < 0)
					return true;
				else if(diff > 0)
					return false;
				else
					return lhs->m_line < rhs->m_line;
			}
		};

		bool	m_enabled;
		int		m_bkptid;
		String	m_fileName;
		int		m_line;
	};

	typedef std::vector<struct BreakpointInfo *, StlAllocator<struct BreakpointInfo *> >	BreakpointCollection;
	typedef std::map<BreakpointKey, BreakpointInfo *, BreakpointKey::LessThanComparator, StlAllocator<std::pair<BreakpointKey, BreakpointInfo *> > >	BreakpointMap;

	// ---------------------------------------------------------------------------------------------------------------
	// IgnoredErrorInfo
	// ---------------------------------------------------------------------------------------------------------------

	struct IgnoredErrorKey
	{
		IgnoredErrorKey(const char * file, int line)
			: m_fileName(file), m_line(line)
		{}

		struct LessThanComparator
		{
			bool operator() (const IgnoredErrorKey & lhs, const IgnoredErrorKey & rhs) const
			{
#if defined(_MSC_VER)
				int diff = stricmp(lhs.m_fileName, rhs.m_fileName);
#else
				int diff = strcasecmp(lhs.m_fileName, rhs.m_fileName);
#endif // _MSC_VER
				if(diff < 0)
					return true;
				else if(diff > 0)
					return false;
				else
					return lhs.m_line < rhs.m_line;
			}
		};

		const char	* m_fileName;
		int			m_line;
	};

	struct IgnoredErrorInfo
	{
		IgnoredErrorInfo(const char * file, int line)
			: m_fileName(file), m_line(line)
		{}

		struct LessThanComparator
		{
			bool operator() (const IgnoredErrorInfo * lhs, const IgnoredErrorInfo * rhs) const
			{
				int diff = lhs->m_fileName.compare(rhs->m_fileName);
				if(diff < 0)
					return true;
				else if(diff > 0)
					return false;
				else
					return lhs->m_line < rhs->m_line;
			}
		};

		String	m_fileName;
		int		m_line;
	};

	typedef std::vector<struct IgnoredErrorInfo *, StlAllocator<struct IgnoredErrorInfo *> > IgnoredErrorCollection;
	typedef std::map<IgnoredErrorKey, IgnoredErrorInfo *, IgnoredErrorKey::LessThanComparator, StlAllocator<std::pair<IgnoredErrorKey, IgnoredErrorInfo *> > > IgnoredErrorMap;

	// ---------------------------------------------------------------------------------------------------------------
	// StackFrameInfo
	// ---------------------------------------------------------------------------------------------------------------

	struct StackFrameInfo
	{
		StackFrameInfo(int depth, const char * function, const char * source, int line)
			: m_depth(depth), m_function(function), m_source(source), m_line(line)
		{

		}

		int		m_depth;
		String	m_function;
		String	m_source;
		int		m_line;
	};

	typedef std::vector<StackFrameInfo *, StlAllocator<StackFrameInfo *> > StackFrameCollection;

	// ---------------------------------------------------------------------------------------------------------------
	// VariableInfo
	// ---------------------------------------------------------------------------------------------------------------

	class VariableInfoLessThanComparator
	{
	public:
		inline bool operator() (const class VariableInfo * lhs, const class VariableInfo * rhs) const;
	};

	typedef std::set<class VariableInfo *, VariableInfoLessThanComparator, StlAllocator<class VariableInfo *> > VariableExpansions;

	enum VariableClass
	{
		VariableClass_INVALID,

		VariableClass_Local,
		VariableClass_Upvalue,
		VariableClass_Watch,
		VariableClass_TableEntry
	};

	class VariableInfo
	{
	public:

		VariableInfo()
			: m_expanded(false), m_hasEntries(false), m_hasMetadata(false), m_valid(false), m_modified(true), m_class(VariableClass_INVALID)
		{}

		VariableInfo(LuaDebuggerValue key, LuaDebuggerValue value, VariableClass varclass)
			: m_key(key), m_value(value), m_expanded(false), m_hasEntries(false), m_hasMetadata(false), m_valid(false), m_modified(true), m_class(varclass)
		{}

		VariableInfo(LuaDebuggerValue key)
			: m_key(key), m_expanded(false), m_hasEntries(false), m_hasMetadata(false), m_valid(false), m_modified(true)
		{}

		~VariableInfo()
		{
			for(VariableExpansions::iterator iter = m_expansions.begin(); iter != m_expansions.end(); ++iter)
				delete *iter;
		}

		void Expand()
		{
			// Also mark the node modified, so we can detect the ones that need to be sent
			m_modified = true;
			m_expanded = true;
		}

		void Close()
		{
			// Also mark the node modified, so we can detect the ones that need to be sent
			m_modified = true;
			m_expanded = false;
		}

		void Invalidate()
		{
			// Mark as not modified so we can detect the ones that have been opened/closed or changed
			m_modified = false;

			// Mark invalid so we can detect the ones that are out of scope
			m_valid = false;

			// We'll re-open it later if need be
			m_hasEntries = false;
			m_hasMetadata = false;

			VariableExpansions::iterator iter = m_expansions.begin();
			while(iter != m_expansions.end())
			{
				VariableExpansions::iterator nextiter = iter;
				++nextiter;
				if( !(*iter)->m_valid )
					m_expansions.erase(iter);
				else
					(*iter)->Invalidate();
				iter = nextiter;
			}
		}

		void Clear()
		{
			for(VariableExpansions::iterator iter = m_expansions.begin(); iter != m_expansions.end(); ++iter)
				delete *iter;

			m_expansions.clear();
		}

		//! Updates the value of an entry 'name' inside this collections expansions
		/**
			\return			The entry for the variable in the expansions
			\param	name	The name of the variable to update
			\param	type	The type of the variable
			\param	value	The value of the variable
		*/
		VariableInfo * UpdateValue(LuaDebuggerValue key, LuaDebuggerValue value, VariableClass varclass)
		{
			VariableInfo temp(key);
			VariableExpansions::iterator iter = m_expansions.find(&temp);
			VariableInfo * result;

			if(iter == m_expansions.end())
			{
				result = new VariableInfo(key, value, varclass);
				m_expansions.insert(result);
			}
			else
			{
				result = *iter;
				if(result->m_value != value || result->m_class != varclass)
				{
					result->m_modified = true;
					result->m_value = value;
					result->m_class = varclass;
				}
			}
			result->m_valid = true;
			return result;
		}


		LuaDebuggerValue	m_key;
		LuaDebuggerValue	m_value;
		VariableClass		m_class;

		bool m_expanded;					//!< Has this node been expanded?
		bool m_hasEntries;					//!< Are there table entries inside this node?
		bool m_hasMetadata;					//!< Is there metadata attached to this node?
		bool m_valid;						//!< Is the variable currently in scope?
		bool m_modified;					//!< Has the value changed since the last time it was retrieved?
		VariableExpansions	m_expansions;	//!< Variables under this one that have been expanded
	};

	bool VariableInfoLessThanComparator::operator() (const VariableInfo * lhs, const VariableInfo * rhs) const
	{
		return lhs->m_key < rhs->m_key;
	}

	typedef VariableInfo VariableCollection;


	// ---------------------------------------------------------------------------------------------------------------
	// WatchInfo
	// ---------------------------------------------------------------------------------------------------------------

	struct LuaWatchInfoLessThanComparator
	{
		inline bool operator() (const struct LuaWatchInfo * lhs, const struct LuaWatchInfo * rhs) const;
	};

	typedef std::vector<struct LuaWatchInfo *, StlAllocator<struct LuaWatchInfo *> > WatchCollection;

	struct LuaWatchInfo
	{
		LuaWatchInfo(const char * expr, int watchid)
			: m_expression(expr), m_watchid(watchid)
		{}

		~LuaWatchInfo()
		{
		}

		String				m_expression;		//!< Expression to evaluate
		int					m_watchid;			//!< The client's watch id
		VariableInfo		m_variable;			//!< The result of the evaluation
	};

	bool LuaWatchInfoLessThanComparator::operator() (const LuaWatchInfo * lhs, const LuaWatchInfo * rhs) const
	{
		return lhs->m_watchid < rhs->m_watchid;
	}

	// ---------------------------------------------------------------------------------------------------------------
	// ThreadInfo
	// ---------------------------------------------------------------------------------------------------------------

	class ThreadInfo
	{
	public:

		ThreadInfo(LuaDebuggerValue thread)
			: 
				m_modified(true), 
				m_valid(true), 
				m_thread(thread), 
				m_parent(),
				m_name(),
				m_threadID(0),
				m_startFunction(), 
				m_currentFunction(), 
				m_state(0),
				m_stackSize(0),
				m_stackUsed(0),
				m_peakTime(0.0f),
				m_averageTime(0.0f)
		{
		}

		ThreadInfo(LuaDebuggerValue thread, LuaDebuggerValue parent, const char * name, int threadid, const String & startFunction, const String & currentFunction, int state, int stackSize, int stackUsed, lua_Number peakTime, lua_Number averageTime)
			: 
				m_modified(true), 
				m_valid(true), 
				m_thread(thread), 
				m_parent(parent), 
				m_name(name),
				m_threadID(threadid),
				m_startFunction(startFunction), 
				m_currentFunction(currentFunction), 
				m_state(state),
				m_stackSize(stackSize),
				m_stackUsed(stackUsed),
				m_peakTime(peakTime), 
				m_averageTime(averageTime)
		{
		}

		~ThreadInfo()
		{
		}

		void UpdateValue(LuaDebuggerValue parent, const String & currentFunction, int state, int stackSize, int stackUsed, lua_Number peakTime, lua_Number averageTime)
		{
			m_valid = true;
			if(parent != m_parent || currentFunction != m_currentFunction || state != m_state || stackSize != m_stackSize || stackUsed != m_stackUsed || peakTime != m_peakTime || averageTime != m_averageTime)
			{
				m_modified = true;
				m_parent = parent;
				m_currentFunction = currentFunction;
				m_state = state;
				m_stackSize = stackSize;
				m_stackUsed = stackUsed;
				m_peakTime = peakTime;
				m_averageTime = averageTime;
			}
		}

		bool				m_modified;
		bool				m_valid;

		LuaDebuggerValue	m_thread;
		LuaDebuggerValue	m_parent;
		String				m_name;
		int					m_threadID;
		String				m_startFunction;
		String				m_currentFunction;
		int					m_state;
		int					m_stackSize;
		int					m_stackUsed;
		lua_Number			m_peakTime;
		lua_Number			m_averageTime;
	};

	class ThreadInfoLessThanComparator
	{
	public:
		inline bool operator() (const ThreadInfo * lhs, const ThreadInfo * rhs) const
		{
			return lhs->m_thread < rhs->m_thread;
		}
	};

	typedef std::set<ThreadInfo *, ThreadInfoLessThanComparator, StlAllocator<ThreadInfo *> > ThreadCollection;

	// ---------------------------------------------------------------------------------------------------------------
	// LuaDebugger
	// ---------------------------------------------------------------------------------------------------------------

	class LuaDebugger
	{
		friend void LuaCallback_Hook(lua_State * lvm, lua_Debug * ar);

	public:
		class stack_restore_point 
		{
		public:

			/// Create the restore point using the stack in 'vm' and check that the
			/// stack is big enough to accommodate 'growby' new objects.

			explicit stack_restore_point(lua_State * vm, int growby = 0) : lvm(vm)
			{
				top = lua_gettop(lvm);

				if (growby > 0)
					lua_checkstack(lvm, growby);
			}

			~stack_restore_point()
			{
				lua_settop(lvm, top);
			}

		private:
			lua_State * lvm;
			unsigned int top;
		};

		class stack_balanced_check
		{
		public:

			/// Create the restore point using the stack in 'vm' and check that the
			/// stack is big enough to accommodate 'growby' new objects.

			explicit stack_balanced_check(lua_State * vm, int growby = 0) : lvm(vm)
			{
				top = lua_gettop(lvm);

				if (growby > 0)
					lua_checkstack(lvm, growby);
			}

			~stack_balanced_check()
			{
				TILDE_ASSERT_MSG(lua_gettop(lvm) == top, "Stack not balanced; expecting %d got %d", top, lua_gettop(lvm));
			}

		private:
			lua_State * lvm;
			unsigned int top;
		};

	public:
		LuaDebugger(LuaDebuggerComms * listener);
		~LuaDebugger(void);

		bool Connect(lua_State * lvm);
		void Disconnect();
		void Shutdown();
		void EnableHook(bool enabled)		{ m_enabled = enabled; }

		bool	IsShutdown() const			{ return m_shutdown; }
		bool	IsConnected() const			{ return m_mainlvm != NULL; }

		static void *	GetRegistryEntry(lua_State * lvm, const char * name);

		DebuggerExecutionMode	GetExecutionMode() const;
		void					SetExecutionMode(DebuggerExecutionMode mode);

		lua_State * GetBreakedThread(void) const			{ return m_breakedlvm; }

		//
		// Runtime configuration
		//
		bool AddBreakpoint(int bkptid, const char * file, int line);
		bool RemoveBreakpoint(int bkptid);
		void ClearBreakpoints();
		bool IsBreakpoint(const char * file, int line)		{ return FindBreakpoint(file, line) > 0; }
		int	 FindBreakpoint(const char * file, int line);

		void AddWatch(const char * expr, int watchid);
		void RemoveWatch(int watchid);
		LuaWatchInfo * FindWatch(int watchid);
		void ClearWatches();

		void IgnoreError(const char * file, int line);
		bool IsIgnoredError(const char * file, int line);
		void ClearIgnoredErrors();

		int CallChunk(lua_State * lvm, int stackDepth, const char * name, bool verboseError);

		//
		// Retrieval methods
		//

		bool				UpdateLocalVariables(lua_State * lvm, int stackDepth);
		bool				UpdateWatchVariables(lua_State * lvm, int stackDepth);
		bool				UpdateWatchVariable(int watchid, lua_State * lvm, int stackDepth);
		bool				UpdateThreads();

		VariableInfo *		GetLocalVariables()		{ return &m_localVariableCache; }
		WatchCollection *	GetWatchVariables()		{ return &m_watches; }
		ThreadCollection *	GetThreads()			{ return &m_threads; }

		bool GetCallstack(lua_State * lvm, StackFrameCollection & result, int & firstLuaFrame);

		LuaDebuggerValue		LookupValue(lua_State * lvm, int stackIndex);
		lua_State *				LookupThreadID(LuaDebuggerObjectID valueid);

	private:


		bool LookupID(LuaDebuggerObjectID valueid);

		void UpdateWatch(LuaWatchInfo * watch, lua_State * lvm, int stackDepth);

		void InitialiseRegistry(lua_State* lvm);
		void InitialiseValueCache(lua_State* lvm);
		void InitialiseIDCache(lua_State* lvm);
		void ClearRegistry();

		void ClearCaches();

		void SetChunkEnvironment(lua_State * lvm, int stackDepth, int functionIndex);
		static int ChunkErrorFunction(lua_State * lvm);
		static int ChunkEnvironment_index(lua_State * lvm);
		static int ChunkEnvironment_newindex(lua_State * lvm);

		static void HookCallback(lua_State * lvm, lua_Debug * ar);
		void OnLuaCallback_HookCall();
		void OnLuaCallback_HookReturn();
		void OnLuaCallback_HookLine();
		void OnLuaCallback_HookError();

		void MainHook(lua_Debug & debugInfo);

		bool					GetTableRecursive(VariableInfo * rootvar, lua_State * lvm, int tableIndex);
		void					GetUpvalues(VariableInfo * rootvar, lua_State * lvm, int functionIndex);
		void					UpdateVariable(VariableInfo * rootvar, lua_State * lvm, int keyIndex, int valueIndex, VariableClass varclass);
		void					UpdateExpansions(VariableInfo * rootvar, lua_State * lvm, int valueIndex);

		const char *			ToString(lua_State * lvm, int index);

		const char *			NormalizedFilename(const char* filenameToNormalize);

	private:

		lua_State			* m_mainlvm;

		bool				m_shutdown;
		bool				m_enabled;

		LuaDebuggerComms	* m_listener;

		BreakpointCollection	m_breakpoints;
		BreakpointMap			m_breakpointMap;

		IgnoredErrorCollection	m_ignoredErrors;
		IgnoredErrorMap			m_ignoredErrorMap;

		DebuggerExecutionMode	m_previousExecutionMode;
		DebuggerExecutionMode	m_executionMode;
		lua_State				* m_breakedlvm;
		int						m_stepNestedCalls;
		int						m_localNestedCalls;

		VariableInfo			m_localVariableCache;
		int						m_localVariableCacheFrame;
		WatchCollection			m_watches;
		ThreadCollection		m_threads;

		LuaDebuggerObjectID		m_nextValueID;

		void const *			m_chunkCallInProgress;

		char *					m_normalizedFilename;
		size_t					m_normalizedFilenameLength;

	};

}	// namespace tilde

#endif // TILDE_INCLUDED_LuaDebugger_h
