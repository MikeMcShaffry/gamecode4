
#pragma once
//========================================================================
// CriticalSection.h : Defines helper classes for multiprogramming
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
 
#include <windows.h>
 
class CriticalSection : public GCC_noncopyable
{
public:
	CriticalSection()
	{ 
		InitializeCriticalSection( &m_cs ); 
	}
 
	~CriticalSection()
	{
		DeleteCriticalSection( &m_cs ); 
	}
 
 
	void Lock()
    { 
		EnterCriticalSection( &m_cs ); 
	}

	void Unlock()
    { 
		LeaveCriticalSection( &m_cs ); 
	}
 
protected:
       // the critical section itself
    mutable CRITICAL_SECTION m_cs;
};
 
 
/*
 Description
      
       helper class
 
 
       allows automatic Locking/ Unlocking of a Resource,
       protected by a Critical Section:
       - locking when this object gets constructed
       - unlocking when this object is destructed
        (goes out of scope)
 
 
 Usage
 
 
       when you need protected access to a resource, do the following
       1. Create a Critical Section associated with the resource
       2. Embody the code accessing the resource in braces {}
       3. Construct an ScopedCriticalSection object
 
 
  Example:
       // we assume that m_CriticalSection
       // is a private variable, and we use it to protect
       // 'this' from being accessed while we need safe access to a resource
      
 
 
       // code that does not need resource locking
      
       {
              ScopedCriticalSection I_am_locked( m_cs);
             
              // code that needs thread locking
       }
      
       // code that does not need resource locking
 
 
*/
class ScopedCriticalSection : public GCC_noncopyable
{
public:
	ScopedCriticalSection( CriticalSection & csResource)
		: m_csResource( csResource)
	{
		m_csResource.Lock();
	}
  
	~ScopedCriticalSection()
	{
		m_csResource.Unlock();
	}
 
private:
       // the Critical Section for this resource
       CriticalSection & m_csResource;
};
 


// concurrent_queue was grabbed from 
// http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
// and was written by Anthony Williams
//
// class concurrent_queue					- Chapter 18, page 669
//

template<typename Data>
class concurrent_queue
{
private:
    std::queue<Data> the_queue;
	CriticalSection m_cs;
    //std::condition_variable the_condition_variable;
	HANDLE m_dataPushed;
public:
	concurrent_queue()
	{
		m_dataPushed = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

    void push(Data const& data)
    {
        //boost::mutex::scoped_lock lock(the_mutex);
		{
			ScopedCriticalSection locker(m_cs);
			the_queue.push(data);
		}
        //lock.unlock();
        //the_condition_variable.notify_one();
		PulseEvent(m_dataPushed);
    }

    bool empty() const
    {
        //boost::mutex::scoped_lock lock(the_mutex);
		ScopedCriticalSection locker(m_cs);
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        //boost::mutex::scoped_lock lock(the_mutex);
		ScopedCriticalSection locker(m_cs);
        if(the_queue.empty())
        {
            return false;
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data& popped_value)
    {
        //boost::mutex::scoped_lock lock(the_mutex);
		ScopedCriticalSection locker(m_cs);
        while(the_queue.empty())
        {
            //the_condition_variable.wait(lock);
			WaitForSingleObject(m_dataPushed);
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
    }

};



