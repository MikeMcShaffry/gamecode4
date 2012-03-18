#pragma once
#ifndef __INCLUDED_SOURCE_SCRIPTLISTENER_H
#define __INCLUDED_SOURCE_SCRIPTLISTENER_H
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


#include "LuaStateManager.h"
#include "../EventManager/EventManager.h"
#include "LuaPlus.h"

using LuaPlus::LuaObject;

//--
// class ScriptEventListener				- Chapter 11, page 334
//
// The ScriptEventListener holds a script callback function that responds
// to a particular event.
class ScriptEventListener : public IEventListener
{
public:
	ScriptEventListener( LuaObject explicitHandlerFunction );

	virtual ~ScriptEventListener()
	{
	}

	virtual char const * GetName( void )
	{
		return "Script Listener";
	}

	virtual bool HandleEvent( ILuaable const & event );

	const LuaObject & GetHandlerFunction( void ) const
	{
		return m_HandlerFunction;
	}

protected:
	// This function is virtual as sub-classes may pass additional
	// parameters.
	virtual bool VCallLuaFunction( LuaObject & eventData );

	LuaObject m_HandlerFunction;
};

//--
// class ScriptActorEventListener					- Chapter 11, page 342
//
// The ScriptActorEventListener holds a script callback function tied
// to a specific actor, and when called, passes in the actor's script
// data.
class ScriptActorEventListener : public ScriptEventListener
{
public:
	ScriptActorEventListener( LuaObject explicitHandlerFunction, const ActorId actorID );

	virtual ~ScriptActorEventListener()
	{
	}

	virtual char const * GetName( void )
	{
		return "Script Actor Listener";
	}

	ActorId GetActorID( void ) const
	{
		return m_SrcActorID;
	}

private:

	// This will pass the event data object as well as
	// look up our actor's specific script data to pass.
	virtual bool VCallLuaFunction( LuaObject & eventData );

	// Our source actor.
	const ActorId m_SrcActorID;
};

#endif	//__INCLUDED_SOURCE_SCRIPTLISTENER_H