//========================================================================
// ProcessManager.cpp : defines common game events
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
#include "ProcessManager.h"

//---------------------------------------------------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------------------------------------------------
ProcessManager::~ProcessManager(void)
{
    ClearAllProcesses();
}


//---------------------------------------------------------------------------------------------------------------------
// The process update tick.  Called every logic tick.  This function returns the number of process chains that 
// succeeded in the upper 32 bits and the number of process chains that failed or were aborted in the lower 32 bits.
//---------------------------------------------------------------------------------------------------------------------
unsigned int ProcessManager::UpdateProcesses(unsigned long deltaMs)
{
    unsigned short int successCount = 0;
    unsigned short int failCount = 0;

    ProcessList::iterator it = m_processList.begin();
    while (it != m_processList.end())
    {
        // grab the next process
        StrongProcessPtr pCurrProcess = (*it);

        // save the iterator and increment the old one in case we need to remove this process from the list
        ProcessList::iterator thisIt = it;
        ++it;

        // process is uninitialized, so initialize it
        if (pCurrProcess->GetState() == Process::UNINITIALIZED)
            pCurrProcess->VOnInit();

        // give the process an update tick if it's running
        if (pCurrProcess->GetState() == Process::RUNNING)
            pCurrProcess->VOnUpdate(deltaMs);

        // check to see if the process is dead
        if (pCurrProcess->IsDead())
        {
            // run the appropriate exit function
            switch (pCurrProcess->GetState())
            {
                case Process::SUCCEEDED :
                {
                    pCurrProcess->VOnSuccess();
                    StrongProcessPtr pChild = pCurrProcess->RemoveChild();
                    if (pChild)
                        AttachProcess(pChild);
                    else
                        ++successCount;  // only counts if the whole chain completed
                    break;
                }

                case Process::FAILED :
                {
                    pCurrProcess->VOnFail();
                    ++failCount;
                    break;
                }

                case Process::ABORTED :
                {
                    pCurrProcess->VOnAbort();
                    ++failCount;
                    break;
                }
            }

            // remove the process and destroy it
            m_processList.erase(thisIt);
        }
    }

    return ((successCount << 16) | failCount);
}


//---------------------------------------------------------------------------------------------------------------------
// Attaches the process to the process list so it can be run on the next update.
//---------------------------------------------------------------------------------------------------------------------
WeakProcessPtr ProcessManager::AttachProcess(StrongProcessPtr pProcess)
{
	m_processList.push_front(pProcess);
    return WeakProcessPtr(pProcess);
}

//---------------------------------------------------------------------------------------------------------------------
// Clears all processes (and DOESN'T run any exit code)
//---------------------------------------------------------------------------------------------------------------------
void ProcessManager::ClearAllProcesses(void)
{
    m_processList.clear();
}

//---------------------------------------------------------------------------------------------------------------------
// Aborts all processes.  If immediate == true, it immediately calls each ones OnAbort() function and destroys all 
// the processes.
//---------------------------------------------------------------------------------------------------------------------
void ProcessManager::AbortAllProcesses(bool immediate)
{
    ProcessList::iterator it = m_processList.begin();
    while (it != m_processList.end())
    {
        ProcessList::iterator tempIt = it;
        ++it;

        StrongProcessPtr pProcess = *tempIt;
        if (pProcess->IsAlive())
        {
            pProcess->SetState(Process::ABORTED);
            if (immediate)
            {
                pProcess->VOnAbort();
                m_processList.erase(tempIt);
            }
        }
    }
}


