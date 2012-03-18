//========================================================================
// Process.cpp : defines common game events
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

#include "GameCodeStd.h"
#include "Process.h"
//#include "BleachUtils/Misc/macros.h"


//---------------------------------------------------------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------------------------------------------------------
Process::Process(void)
{
	m_state = UNINITIALIZED;
	//m_pParent = NULL;
	//m_pChild = NULL;
}


//---------------------------------------------------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------------------------------------------------
Process::~Process(void)
{
	if (m_pChild)
	{
		m_pChild->VOnAbort();
	}
}


/*
//---------------------------------------------------------------------------------------------------------------------
// Attaches this process to the passed in parent.  It's perfectly valid for this process to have been run and you can
// safely call it from within this process' OnUpdate() function.
//---------------------------------------------------------------------------------------------------------------------
bool Process::AttachToParent(Process* pParent)
{
	// If the process is alive, we need to mark it to be removed from the process list without being destroyed.  If 
	// the process is dead, kick and error and don't attach it.  If the process has already been marked for removal,
	// it means we're trying to double-attach this process, which is an error.
	if (IsAlive())
	{
		m_state = REMOVED;
	}
	else if (IsDead())
	{
		GCC_ERROR("Attempting to attach a dead process to a parent");
		return false;
	}
	else if (IsRemoved())
	{
		GCC_ERROR("Attempting to attach a process that is already being removed to another parent");
		return false;
	}

	// Attach this process to the new parent.  It will be attached at the very end of the child tree.
	pParent->AttachChild(this);

	return true;
}
*/


//---------------------------------------------------------------------------------------------------------------------
// Removes the child from this process.  This releases ownership of the child to the caller and completely removes it
// from the process chain.
//---------------------------------------------------------------------------------------------------------------------
StrongProcessPtr Process::RemoveChild(void)
{
	if (m_pChild)
	{
        StrongProcessPtr pChild = m_pChild;  // this keeps the child from getting destroyed when we clear it
		m_pChild.reset();
		//pChild->SetParent(NULL);
        return pChild;
	}

	return StrongProcessPtr();
}

/*
//---------------------------------------------------------------------------------------------------------------------
// Cancels the top level process in this chain by calling its Fail() function.
//---------------------------------------------------------------------------------------------------------------------
void Process::CancelTopLevelProcess(void)
{
	StrongProcessPtr pTop = GetTopLevelProcess();
	pTop->Fail();
}
*/
