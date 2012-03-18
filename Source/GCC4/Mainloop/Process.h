#pragma once
//========================================================================
// Process.h : defines common game events
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

class Process;
typedef shared_ptr<Process> StrongProcessPtr;
typedef weak_ptr<Process> WeakProcessPtr;

//---------------------------------------------------------------------------------------------------------------------
// Process class
// 
// Processes are ended by one of three methods: Success, Failure, or Aborted.
//		- Success means the process completed successfully.  If the process has a child, it will be attached to 
//		  the process mgr.
//		- Failure means the process started but failed in some way.  If the process has a child, it will be 
//		  aborted.
//		- Aborted processes are processes that are canceled while not submitted to the process mgr.  Depending 
//		  on the circumstances, they may or may not have gotten an OnInit() call.  For example, a process can 
//		  spawn another process and call AttachToParent() on itself.  If the new process fails, the child will
//		  get an Abort() call on it, even though its status is RUNNING.
//---------------------------------------------------------------------------------------------------------------------
class Process
{
	friend class ProcessManager;

public:
	enum State
	{
		// Processes that are neither dead nor alive
		UNINITIALIZED = 0,  // created but not running
		REMOVED,  // removed from the process list but not destroyed; this can happen when a process that is already running is parented to another process

		// Living processes
		RUNNING,  // initialized and running
		PAUSED,  // initialized but paused
		
		// Dead processes
		SUCCEEDED,  // completed successfully
		FAILED,  // failed to complete
		ABORTED,  // aborted; may not have started
	};
	
private:
	State m_state;  // the current state of the process
	StrongProcessPtr m_pChild;  // the child process, if any

public:
	// construction
    Process(void);
	virtual ~Process(void);
	
protected:
	// interface; these functions should be overridden by the subclass as needed
	virtual void VOnInit(void) { m_state = RUNNING; }  // called during the first update; responsible for setting the initial state (typically RUNNING)
	virtual void VOnUpdate(unsigned long deltaMs) = 0;  // called every frame
	virtual void VOnSuccess(void) { }  // called if the process succeeds (see below)
	virtual void VOnFail(void) { }  // called if the process fails (see below)
	virtual void VOnAbort(void) { }  // called if the process is aborted (see below)

public:
	// Functions for ending the process.
	inline void Succeed(void);
	inline void Fail(void);
	
	// pause
	inline void Pause(void);
	inline void UnPause(void);

	// accessors
	State GetState(void) const { return m_state; }
	bool IsAlive(void) const { return (m_state == RUNNING || m_state == PAUSED); }
	bool IsDead(void) const { return (m_state == SUCCEEDED || m_state == FAILED || m_state == ABORTED); }
	bool IsRemoved(void) const { return (m_state == REMOVED); }
	bool IsPaused(void) const { return m_state == PAUSED; }

	// child functions
	inline void AttachChild(StrongProcessPtr pChild);
	StrongProcessPtr RemoveChild(void);  // releases ownership of the child
	StrongProcessPtr PeekChild(void) { return m_pChild; }  // doesn't release ownership of the child

private:
	void SetState(State newState) { m_state = newState; }
};


//---------------------------------------------------------------------------------------------------------------------
// Inline function definitions
//---------------------------------------------------------------------------------------------------------------------
inline void Process::Succeed(void)
{
	GCC_ASSERT(m_state == RUNNING || m_state == PAUSED);
	m_state = SUCCEEDED;
}

inline void Process::Fail(void)
{
	GCC_ASSERT(m_state == RUNNING || m_state == PAUSED);
	m_state = FAILED;
}

inline void Process::AttachChild(StrongProcessPtr pChild)
{
	if (m_pChild)
		m_pChild->AttachChild(pChild);
	else
		m_pChild = pChild;
}

inline void Process::Pause(void)
{
	if (m_state == RUNNING)
		m_state = PAUSED;
	else
		GCC_WARNING("Attempting to pause a process that isn't running");
}

inline void Process::UnPause(void)
{
	if (m_state == PAUSED)
		m_state = RUNNING;
	else
		GCC_WARNING("Attempting to unpause a process that isn't paused");
}

/*
inline StrongProcessPtr Process::GetTopLevelProcess(void)
{
	if (m_pParent)
		return m_pParent->GetTopLevelProcess();
	else
		return this;
}
*/
