#pragma once
//========================================================================
// Events.h : defines common game events
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


#include "EventManager.h"
#include "../GameCode4/GameCode.h"
#include "../LUAScripting/ScriptEvent.h"

// Auxiliary data decls ...
//
// data that is passed per-event in the userData parameter
// 
// ( for some, but not all, events )

//-- new object notification


void RegisterEngineScriptEvents(void);


//---------------------------------------------------------------------------------------------------------------------
// EvtData_New_Actor - This event is sent out when an actor is *actually* created.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_New_Actor : public BaseEventData
{
	ActorId m_actorId;
    GameViewId m_viewId;

public:
	static const EventType sk_EventType;

	EvtData_New_Actor(void) 
	{
		m_actorId = INVALID_ACTOR_ID;
		m_viewId = gc_InvalidGameViewId;
	}

    explicit EvtData_New_Actor(ActorId actorId, GameViewId viewId = gc_InvalidGameViewId) 
        : m_actorId(actorId),
          m_viewId(viewId)
	{
	}

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_actorId;
		in >> m_viewId;
    }

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy(void) const
	{
		return IEventDataPtr(GCC_NEW EvtData_New_Actor(m_actorId, m_viewId));
	}

	virtual void VSerialize(std::ostrstream& out) const
	{
		out << m_actorId << " ";
		out << m_viewId << " ";
	}


    virtual const char* GetName(void) const
    {
        return "EvtData_New_Actor";
    }

	const ActorId GetActorId(void) const
	{
		return m_actorId;
	}

    GameViewId GetViewId(void) const
    {
        return m_viewId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Destroy_Actor - sent when actors are destroyed	
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Destroy_Actor : public BaseEventData
{
    ActorId m_id;

public:
	static const EventType sk_EventType;

    explicit EvtData_Destroy_Actor(ActorId id = INVALID_ACTOR_ID)
        : m_id(id)
    {
        //
    }

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy(void) const
	{
		return IEventDataPtr ( GCC_NEW EvtData_Destroy_Actor ( m_id ) );
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id;
	}

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Destroy_Actor";
    }

    ActorId GetId(void) const { return m_id; }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Move_Actor - sent when actors are moved
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Move_Actor : public BaseEventData
{
    ActorId m_id;
    Mat4x4 m_matrix;

public:
	static const EventType sk_EventType;

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

    EvtData_Move_Actor(void)
    {
        m_id = INVALID_ACTOR_ID;
    }

	EvtData_Move_Actor(ActorId id, const Mat4x4& matrix)
        : m_id(id), m_matrix(matrix)
	{
        //
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id << " ";
		for (int i=0; i<4; ++i)
		{
			for (int j=0; j<4; ++j)
			{
				out << m_matrix.m[i][j] << " ";
			}
		}
	}

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
        for (int i=0; i<4; ++i)
        {
            for (int j=0; j<4; ++j)
            {
                in >> m_matrix.m[i][j];
            }
        }
    }

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr(GCC_NEW EvtData_Move_Actor(m_id, m_matrix));
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Move_Actor";
    }

    ActorId GetId(void) const
    {
        return m_id;
    }

    const Mat4x4& GetMatrix(void) const
    {
        return m_matrix;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_New_Render_Component - This event is sent out when an actor is *actually* created.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_New_Render_Component : public BaseEventData
{
    ActorId m_actorId;
    shared_ptr<SceneNode> m_pSceneNode;

public:
    static const EventType sk_EventType;

    EvtData_New_Render_Component(void) 
    {
        m_actorId = INVALID_ACTOR_ID;
    }

    explicit EvtData_New_Render_Component(ActorId actorId, shared_ptr<SceneNode> pSceneNode) 
        : m_actorId(actorId),
          m_pSceneNode(pSceneNode)
    {
    }

    virtual void VSerialize(std::ostrstream& out) const
    {
        GCC_ERROR(GetName() + std::string(" should not be serialzied!"));
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        GCC_ERROR(GetName() + std::string(" should not be serialzied!"));
    }

    virtual const EventType& VGetEventType(void) const
    {
        return sk_EventType;
    }

    virtual IEventDataPtr VCopy(void) const
    {
        return IEventDataPtr(GCC_NEW EvtData_New_Render_Component(m_actorId, m_pSceneNode));
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_New_Render_Component";
    }

    const ActorId GetActorId(void) const
    {
        return m_actorId;
    }

    shared_ptr<SceneNode> GetSceneNode(void) const
    {
        return m_pSceneNode;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Modified_Render_Component - This event is sent out when a render component is changed
//   NOTE: This class is not described in the book!
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Modified_Render_Component : public BaseEventData
{
    ActorId m_id;

public:
	static const EventType sk_EventType;

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

    EvtData_Modified_Render_Component(void)
    {
        m_id = INVALID_ACTOR_ID;
    }

	EvtData_Modified_Render_Component(ActorId id)
        : m_id(id)
	{
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id;
	}

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
    }

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr(GCC_NEW EvtData_Modified_Render_Component(m_id));
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Modified_Render_Component";
    }

    ActorId GetActorId(void) const
    {
        return m_id;
    }
};



//---------------------------------------------------------------------------------------------------------------------
// EvtData_Environment_Loaded - this event is sent when a new game is started
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Environment_Loaded : public BaseEventData
{
public:
	static const EventType sk_EventType;

    EvtData_Environment_Loaded(void) { }
	virtual const EventType& VGetEventType(void) const	{ return sk_EventType; }
	virtual IEventDataPtr VCopy(void) const
		{ return IEventDataPtr( GCC_NEW EvtData_Environment_Loaded( ) ); }
    virtual const char* GetName(void) const  { return "EvtData_Environment_Loaded";  }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Environment_Loaded - this event is sent when a client has loaded its environment
//   This is special because we only want this event to go from client to server, and stop there. The
//   EvtData_Environment_Loaded is received by server and proxy logics alike. Thy to do this with just the above 
//   event and you'll get into an endless loop of the EvtData_Environment_Loaded event making infinite round trips
//   from client to server.
//
// FUTURE_WORK: It would be an interesting idea to add a "Private" type of event that is addressed only to a specific 
//              listener. Of course, that might be a really dumb idea too - someone will have to try it!
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Remote_Environment_Loaded : public BaseEventData
{
public:
	static const EventType sk_EventType;

    EvtData_Remote_Environment_Loaded(void) { }
	virtual const EventType& VGetEventType(void) const	{ return sk_EventType; }
	virtual IEventDataPtr VCopy(void) const
		{ return IEventDataPtr( GCC_NEW EvtData_Remote_Environment_Loaded( ) ); }
    virtual const char* GetName(void) const  { return "EvtData_Remote_Environment_Loaded";  }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Request_Start_Game - this is sent by the authoritative game logic to all views so they will load a game level.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Request_Start_Game : public BaseEventData
{

public:
	static const EventType sk_EventType;

    EvtData_Request_Start_Game(void) { }

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Request_Start_Game( ) );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Request_Start_Game";
    }
};


/**** HOLY CRAP THIS ISN"T USED ANYMORE????
//---------------------------------------------------------------------------------------------------------------------
// EvtData_Game_State - sent whenever the game state is changed (look for "BGS_" to see the different states)
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Game_State : public BaseEventData
{
    BaseGameState m_gameState;
	std::string m_parameter;

public:
	static const EventType sk_EventType;

	EvtData_Game_State(void)
    {
        m_gameState = BGS_Invalid;
    }
    
    explicit EvtData_Game_State(const BaseGameState gameState, const std::string &parameter)
		: m_gameState(gameState), m_parameter(parameter)
	{
	}

    virtual const EventType & VGetEventType( void ) const
    {
        return sk_EventType;
    }

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Game_State( m_gameState, m_parameter ) );
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		const int tempVal = static_cast< int >( m_gameState );
		out << tempVal << " ";
		out << m_parameter;
	}

    virtual void VDeserialize(std::istrstream &in)
    {
        int tempVal;
        in >> tempVal;
        m_gameState = static_cast<BaseGameState>( tempVal );
		in >> m_parameter;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Game_State";
    }

    BaseGameState GetGameState(void) const
    {
        return m_gameState;
    }

	const std::string GetParameter(void) const 
	{
		return m_parameter;
	}
};
**********************/


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Remote_Client						- Chapter 19, page 687
// 
//   Sent whenever a new client attaches to a game logic acting as a server				
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Remote_Client : public BaseEventData
{
    int m_socketId;
    int m_ipAddress;

public:
	static const EventType sk_EventType;

    EvtData_Remote_Client(void)
    {
        m_socketId = 0;
        m_ipAddress = 0;
    }

	EvtData_Remote_Client( const int socketid, const int ipaddress )
		: m_socketId( socketid ), m_ipAddress(ipaddress)
	{
	}

    virtual const EventType & VGetEventType( void ) const
    {
        return sk_EventType;
    }

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr( GCC_NEW EvtData_Remote_Client( m_socketId, m_ipAddress ) );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Remote_Client";
    }

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_socketId << " ";
		out << m_ipAddress;
	}

    virtual void VDeserialize( std::istrstream &in )
    {
        in >> m_socketId;
        in >> m_ipAddress;
    }

    int GetSocketId(void) const
    {
        return m_socketId;
    }

    int GetIpAddress(void) const
    {
        return m_ipAddress;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Update_Tick - sent by the game logic each game tick
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Update_Tick : public BaseEventData
{
    int m_DeltaMilliseconds;

public:
	static const EventType sk_EventType;

    explicit EvtData_Update_Tick( const int deltaMilliseconds )
        : m_DeltaMilliseconds( deltaMilliseconds )
    {
    }

	virtual const EventType& VGetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_Update_Tick ( m_DeltaMilliseconds ) );
	}

	virtual void VSerialize( std::ostrstream & out )
	{
		GCC_ERROR("You should not be serializing update ticks!");
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Update_Tick";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Network_Player_Actor_Assignment - sent by the server to the clients when a network view is assigned a player number
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Network_Player_Actor_Assignment : public BaseEventData
{
    ActorId m_ActorId;
    int m_SocketId;

public:
	static const EventType sk_EventType;

	EvtData_Network_Player_Actor_Assignment()
	{
		m_ActorId = INVALID_ACTOR_ID;
		m_SocketId = -1;
	}

    explicit EvtData_Network_Player_Actor_Assignment( const ActorId actorId, const int socketId )
        : m_ActorId( actorId ), m_SocketId(socketId)

    {
    }

	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy() const
	{
		 return IEventDataPtr( GCC_NEW EvtData_Network_Player_Actor_Assignment( m_ActorId, m_SocketId ) ) ;
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Network_Player_Actor_Assignment";
    }


	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_ActorId << " ";
		out << m_SocketId;
	}

    virtual void VDeserialize( std::istrstream &in )
    {
        in >> m_ActorId;
        in >> m_SocketId;
    }

    ActorId GetActorId(void) const
    {
        return m_ActorId;
    }

    ActorId GetSocketId(void) const
    {
        return m_SocketId;
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Decompress_Request - sent to a multithreaded game event listener to decompress something in the resource file
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Decompress_Request : public BaseEventData
{
    std::wstring m_zipFileName;
    std::string m_fileName;

public:
	static const EventType sk_EventType;

    explicit EvtData_Decompress_Request( std::wstring zipFileName, std::string filename  )
        : m_zipFileName( zipFileName ),
        m_fileName ( filename )
    {
    }

	virtual const EventType& VGetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_Decompress_Request ( m_zipFileName, m_fileName ) );
	}

	virtual void VSerialize( std::ostrstream & out )
	{
		GCC_ERROR( "You should not be serializing decompression requests!" );
	}

    const std::wstring& GetZipFilename(void) const
    {
        return m_zipFileName;
    }

    const std::string& GetFilename(void) const
    {
        return m_fileName;
    }
    virtual const char* GetName(void) const
    {
        return "EvtData_Decompress_Request";
    }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Decompression_Progress - sent by the decompression thread to report progress
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Decompression_Progress : public BaseEventData
{
    int m_progress;
    std::wstring m_zipFileName;
    std::string m_fileName;
    void *m_buffer;

public:
	static const EventType sk_EventType;

    EvtData_Decompression_Progress( int progress, std::wstring zipFileName, std::string filename, void *buffer )
        : m_progress(progress),
        m_zipFileName( zipFileName ),
        m_fileName ( filename ),
        m_buffer (buffer)
    {
    }

	virtual const EventType & VGetEventType( void ) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr (GCC_NEW EvtData_Decompression_Progress ( m_progress, m_zipFileName, m_fileName, m_buffer ) );
	}

	virtual void VSerialize( std::ostrstream & out )
	{
		GCC_ERROR( "You should not be serializing decompression progress events!" );
	}

    virtual const char* GetName(void) const
    {
        return "EvtData_Decompression_Progress";
    }

};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_Request_New_Actor				
// This event is sent by a server asking Client proxy logics to create new actors from their local resources.
// It can be sent from script or via code.
// This event is also sent from the server game logic to client logics AFTER it has created a new actor. The logics will allow follow suit to stay in sync.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Request_New_Actor : public BaseEventData
{
    std::string m_actorResource;
	bool m_hasInitialTransform;
	Mat4x4 m_initialTransform;
	ActorId m_serverActorId;
    GameViewId m_viewId;

public:
	static const EventType sk_EventType;

	EvtData_Request_New_Actor()
	{
		m_actorResource = "";
		m_hasInitialTransform = false;
		m_initialTransform = Mat4x4::g_Identity;
		m_serverActorId = -1;
		m_viewId = gc_InvalidGameViewId;
	}

    explicit EvtData_Request_New_Actor(const std::string &actorResource, const Mat4x4 *initialTransform=NULL, const ActorId serverActorId = INVALID_ACTOR_ID, const GameViewId viewId = gc_InvalidGameViewId)
    {
        m_actorResource = actorResource;
		if (initialTransform)
		{
			m_hasInitialTransform = true;
			m_initialTransform = *initialTransform;
		}
		else
			m_hasInitialTransform = false;

		m_serverActorId = serverActorId;
        m_viewId = viewId;
    }

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	virtual void VDeserialize( std::istrstream & in )
	{
		in >> m_actorResource;
		in >> m_hasInitialTransform;
		if (m_hasInitialTransform)
		{
			for (int i=0; i<4; ++i)
			{
				for (int j=0; j<4; ++j)
				{
					in >> m_initialTransform.m[i][j];
				}
			}
		}
		in >> m_serverActorId;
		in >> m_viewId;
	}

	virtual IEventDataPtr VCopy() const
	{	 
		return IEventDataPtr (GCC_NEW EvtData_Request_New_Actor(m_actorResource, (m_hasInitialTransform) ? &m_initialTransform : NULL, m_serverActorId, m_viewId));
	}

	virtual void VSerialize( std::ostrstream & out ) const
	{
		out << m_actorResource << " ";
		out << m_hasInitialTransform << " ";
		if (m_hasInitialTransform)
		{
			for (int i=0; i<4; ++i)
			{
				for (int j=0; j<4; ++j)
				{
					out << m_initialTransform.m[i][j] << " ";
				}
			}		
		}
		out << m_serverActorId << " ";
		out << m_viewId << " ";
	}

    virtual const char* GetName(void) const { return "EvtData_Request_New_Actor";  }

    const std::string &GetActorResource(void) const { return m_actorResource;  }
	const Mat4x4 *GetInitialTransform(void) const { return (m_hasInitialTransform) ? &m_initialTransform : NULL; }
	const ActorId GetServerActorId(void) const 	{ return m_serverActorId; }
    GameViewId GetViewId(void) const { return m_viewId; }
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_Request_Destroy_Actor - sent by any system requesting that the game logic destroy an actor	
//    FUTURE WORK - This event shouldn't really exist - subsystems should never ask the game logic to destroy something through an event, should they?
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Request_Destroy_Actor : public ScriptEvent
{
    ActorId m_actorId;

public:
    static const EventType sk_EventType;

    EvtData_Request_Destroy_Actor()
    {
        m_actorId = INVALID_ACTOR_ID;
    }

    EvtData_Request_Destroy_Actor(ActorId actorId)
    {
        m_actorId = actorId;
    }

    virtual const EventType& VGetEventType(void) const
    {
        return sk_EventType;
    }

    virtual void VDeserialize( std::istrstream & in )
    {
        in >> m_actorId;
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr (GCC_NEW EvtData_Request_Destroy_Actor(m_actorId));
    }

    virtual void VSerialize( std::ostrstream & out ) const
    {
        out << m_actorId;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Request_Destroy_Actor";
    }

    ActorId GetActorId(void) const
    {
        return m_actorId;
    }

    virtual bool VBuildEventFromScript(void)
    {
        if (m_eventData.IsInteger())
        {
            m_actorId = m_eventData.GetInteger();
            return true;
        }
        return false;
    }

    EXPORT_FOR_SCRIPT_EVENT(EvtData_Request_Destroy_Actor);
};


//---------------------------------------------------------------------------------------------------------------------
// EvtData_PlaySound - sent by any system wishing for a HumanView to play a sound
//---------------------------------------------------------------------------------------------------------------------
class EvtData_PlaySound : public ScriptEvent
{
    std::string m_soundResource;

public:
    static const EventType sk_EventType;

    EvtData_PlaySound(void) { }
    EvtData_PlaySound(const std::string& soundResource)
        : m_soundResource(soundResource)
    {
    }

    virtual const EventType& VGetEventType(void) const
    {
        return sk_EventType;
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr(GCC_NEW EvtData_PlaySound(m_soundResource));
    }

    virtual void VSerialize(std::ostrstream& out) const
    {
        out << m_soundResource;
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_soundResource;
    }

    const std::string& GetResource(void) const
    {
        return m_soundResource;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_PlaySound";
    }

    virtual bool VBuildEventFromScript(void);

    EXPORT_FOR_SCRIPT_EVENT(EvtData_PlaySound);
};


