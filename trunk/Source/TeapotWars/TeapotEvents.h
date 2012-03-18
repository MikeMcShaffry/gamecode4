#pragma once
//========================================================================
// TeapotEvents.h : defines game-specific events for TeapotWars
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

#include "../GCC4/EventManager/EventManager.h"
#include "../GCC4/EventManager/Events.h"
#include "../GCC4/LUAScripting/ScriptEvent.h"


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_Fire_Weapon				- Chapter 10, 279
// Nasty game dependent events here. We'll move this back into game specific TeapotEvents.h when real streaming code 
// gets written so Network.cpp doesn't have to know about all events.
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Fire_Weapon : public ScriptEvent
{
    ActorId m_id;

public:
    static const EventType sk_EventType;
    virtual const EventType & VGetEventType() const
    {
        return sk_EventType;
    }

    EvtData_Fire_Weapon(void)
    {
        m_id = INVALID_ACTOR_ID;
    }

    EvtData_Fire_Weapon( ActorId id )
        : m_id( id )
    {
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr ( GCC_NEW EvtData_Fire_Weapon (m_id) );
    }

    virtual void VSerialize( std::ostrstream & out ) const
    {
        out << m_id << " ";
    }

    virtual void VDeserialize( std::istrstream & in )
    {
        in >> m_id;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Fire_Weapon";
    }

    virtual bool VBuildEventFromScript(void)
    {
        if (m_eventData.IsTable())
        {
            LuaPlus::LuaObject temp = m_eventData.GetByName("id");
            if (temp.IsInteger())
            {
                m_id = temp.GetInteger();
            }
            else
            {
                GCC_ERROR("Invalid id sent to EvtData_Fire_Weapon; type = " + std::string(temp.TypeName()));
                return false;
            }
            return true;
        }

        return false;
    }

	ActorId GetActorId(void) const
    {
        return m_id;
    }

	virtual void VBuildEventData(void);

    EXPORT_FOR_SCRIPT_EVENT(EvtData_Fire_Weapon);
};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_StartThrust				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EvtData_StartThrust : public ScriptEvent
{
    ActorId m_id;
    float m_acceleration;

public:
    static const EventType sk_EventType;
    virtual const EventType& VGetEventType( void ) const
    {
        return sk_EventType;
    }

    EvtData_StartThrust(void) :
        m_id(INVALID_ACTOR_ID),
        m_acceleration(0)
    {
        //
    }

    EvtData_StartThrust(ActorId id, float acceleration) :
        m_id(id),
        m_acceleration(acceleration)
    {
        //
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr (GCC_NEW EvtData_StartThrust(m_id, m_acceleration));
    }

    virtual void VSerialize(std::ostrstream & out) const
    {
        out << m_id << " ";
        out << m_acceleration << " ";
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
        in >> m_acceleration;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_ThrustStart";
    }

    ActorId GetActorId(void) const
    {
        return m_id;
    }

    float GetAcceleration(void) const
    {
        return m_acceleration;
    }

    void Set(ActorId id)
    {
        m_id = id;
    }

    virtual bool VBuildEventFromScript(void)
    {
        if (m_eventData.IsTable())
        {
            // ID
            LuaPlus::LuaObject temp = m_eventData.GetByName("id");
            if (temp.IsInteger())
            {
                m_id = temp.GetInteger();
            }
            else
            {
                GCC_ERROR("Invalid id sent to EvtData_StartThrust; type = " + std::string(temp.TypeName()));
                return false;
            }

            // acceleration
            temp = m_eventData.GetByName("acceleration");
            if (temp.IsNumber())
                m_acceleration = temp.GetFloat();
            else
                m_acceleration = 5.0f;  // something reasonable

            return true;
        }

        return false;
    }

    EXPORT_FOR_SCRIPT_EVENT(EvtData_StartThrust);
};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_EndThrust				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EvtData_EndThrust : public ScriptEvent
{
    ActorId m_id;

public:
    static const EventType sk_EventType;
    virtual const EventType & VGetEventType( void ) const
    {
        return sk_EventType;
    }

    EvtData_EndThrust(void) :
        m_id(INVALID_ACTOR_ID)
    {
        //
    }

    EvtData_EndThrust(ActorId id) :
        m_id(id)
    {
        //
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr (GCC_NEW EvtData_EndThrust(m_id));
    }

    virtual void VSerialize(std::ostrstream & out) const
    {
        out << m_id << " ";
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_EndThrust";
    }

    ActorId GetActorId(void) const
    {
        return m_id;
    }

    void Set(ActorId id)
    {
        m_id = id;
    }

    virtual bool VBuildEventFromScript(void)
    {
        if (m_eventData.IsTable())
        {
            // ID
            LuaPlus::LuaObject temp = m_eventData.GetByName("id");
            if (temp.IsInteger())
            {
                m_id = temp.GetInteger();
            }
            else
            {
                GCC_ERROR("Invalid id sent to EvtData_EndThrust; type = " + std::string(temp.TypeName()));
                return false;
            }

            return true;
        }

        return false;
    }

    EXPORT_FOR_SCRIPT_EVENT(EvtData_EndThrust);
};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_StartSteer				             - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EvtData_StartSteer : public ScriptEvent
{
    ActorId m_id;
    float m_acceleration;

public:
    static const EventType sk_EventType;
    virtual const EventType & VGetEventType( void ) const
    {
        return sk_EventType;
    }

    EvtData_StartSteer(void) :
        m_id(INVALID_ACTOR_ID),
        m_acceleration(0)
    {
        //
    }

    EvtData_StartSteer(ActorId id, float acceleration) :
        m_id(id),
        m_acceleration(acceleration)
    {
        //
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr(GCC_NEW EvtData_StartSteer(m_id, m_acceleration));
    }

    virtual void VSerialize(std::ostrstream & out) const
    {
        out << m_id << " ";
        out << m_acceleration << " ";
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
        in >> m_acceleration;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_StartSteer";
    }

    ActorId GetActorId(void) const
    {
        return m_id;
    }

    float GetAcceleration(void) const
    {
        return m_acceleration;
    }

    void Set(ActorId id)
    {
        m_id = id;
    }

    virtual bool VBuildEventFromScript(void)
    {
        if (m_eventData.IsTable())
        {
            // ID
            LuaPlus::LuaObject temp = m_eventData.GetByName("id");
            if (temp.IsInteger())
            {
                m_id = temp.GetInteger();
            }
            else
            {
                GCC_ERROR("Invalid id sent to EvtData_StartSteer; type = " + std::string(temp.TypeName()));
                return false;
            }

            // acceleration
            temp = m_eventData.GetByName("acceleration");
            if (temp.IsNumber())
                m_acceleration = temp.GetFloat();
            else
                m_acceleration = 5.0f;  // something reasonable

            return true;
        }

        return false;
    }

    EXPORT_FOR_SCRIPT_EVENT(EvtData_StartSteer);
};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_EndSteer				        - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EvtData_EndSteer : public ScriptEvent
{
    ActorId m_id;

public:
    static const EventType sk_EventType;
    virtual const EventType & VGetEventType( void ) const
    {
        return sk_EventType;
    }

    EvtData_EndSteer(void) :
        m_id(INVALID_ACTOR_ID)
    {
        //
    }

    EvtData_EndSteer(ActorId id) :
        m_id(id)
    {
        //
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr (GCC_NEW EvtData_EndThrust(m_id));
    }

    virtual void VSerialize(std::ostrstream & out) const
    {
        out << m_id << " ";
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_EndThrust";
    }

    ActorId GetActorId(void) const
    {
        return m_id;
    }

    void Set(ActorId id)
    {
        m_id = id;
    }

    virtual bool VBuildEventFromScript(void)
    {
        if (m_eventData.IsTable())
        {
            // ID
            LuaPlus::LuaObject temp = m_eventData.GetByName("id");
            if (temp.IsInteger())
            {
                m_id = temp.GetInteger();
            }
            else
            {
                GCC_ERROR("Invalid id sent to EvtData_EndSteer; type = " + std::string(temp.TypeName()));
                return false;
            }

            return true;
        }

        return false;
    }

    EXPORT_FOR_SCRIPT_EVENT(EvtData_EndSteer);
};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_Gameplay_UI_Update				- Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Gameplay_UI_Update : public ScriptEvent
{
    std::string m_gameplayUiString;

public:
    static const EventType sk_EventType;

    EvtData_Gameplay_UI_Update(void) { }
    EvtData_Gameplay_UI_Update(const std::string& gameplayUiString)
        : m_gameplayUiString(gameplayUiString)
    {
    }

    virtual const EventType& VGetEventType(void) const
    {
        return sk_EventType;
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr(GCC_NEW EvtData_Gameplay_UI_Update(m_gameplayUiString));
    }

    virtual void VSerialize(std::ostrstream& out) const
    {
        out << m_gameplayUiString;
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_gameplayUiString;
    }

    const std::string& GetUiString(void) const
    {
        return m_gameplayUiString;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Gameplay_UI_Update";
    }

    virtual bool VBuildEventFromScript(void);

    EXPORT_FOR_SCRIPT_EVENT(EvtData_Gameplay_UI_Update);
};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_Set_Controlled_Actor				- Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EvtData_Set_Controlled_Actor : public ScriptEvent
{
    ActorId m_id;

public:
    static const EventType sk_EventType;

    EvtData_Set_Controlled_Actor(void) { }
    EvtData_Set_Controlled_Actor(ActorId actorId)
        : m_id(actorId)
    {
    }

    virtual const EventType& VGetEventType(void) const
    {
        return sk_EventType;
    }

    virtual IEventDataPtr VCopy() const
    {
        return IEventDataPtr(GCC_NEW EvtData_Set_Controlled_Actor(m_id));
    }

    virtual void VSerialize(std::ostrstream& out) const
    {
        out << m_id;
    }

    virtual void VDeserialize(std::istrstream& in)
    {
        in >> m_id;
    }

    const ActorId& GetActorId(void) const
    {
        return m_id;
    }

    virtual const char* GetName(void) const
    {
        return "EvtData_Set_Controlled_Actor";
    }

    virtual bool VBuildEventFromScript(void);

    EXPORT_FOR_SCRIPT_EVENT(EvtData_Set_Controlled_Actor);
};


//---------------------------------------------------------------------------------------------------------------------
// class EvtData_ScriptEventTest				             - Chapter 10, 279
//---------------------------------------------------------------------------------------------------------------------
class EvtData_ScriptEventTest_ToLua : public ScriptEvent
{
    int m_num;

public:
    static const EventType sk_EventType;

    EvtData_ScriptEventTest_ToLua(void) { m_num = 0; }
    EvtData_ScriptEventTest_ToLua(int num) { m_num = num; }

    virtual const EventType& VGetEventType(void) const { return sk_EventType; }

    virtual IEventDataPtr VCopy(void) const
    {
        shared_ptr<EvtData_ScriptEventTest_ToLua> pCopy(GCC_NEW EvtData_ScriptEventTest_ToLua(m_num));
        pCopy->m_eventData = m_eventData;
        return pCopy;
    }

    virtual const char* GetName(void) const { return "EvtData_ScriptEventTest_ToLua"; }

protected:
    virtual void VBuildEventData(void);
    //virtual bool VBuildEventFromScript(void);

    EXPORT_FOR_SCRIPT_EVENT(EvtData_ScriptEventTest_ToLua);
};

class EvtData_ScriptEventTest_FromLua : public ScriptEvent
{
    int m_num;

public:
    static const EventType sk_EventType;

    EvtData_ScriptEventTest_FromLua(void) { m_num = 0; }
    EvtData_ScriptEventTest_FromLua(int num) { m_num = num; }

    virtual const EventType& VGetEventType(void) const { return sk_EventType; }

    virtual IEventDataPtr VCopy(void) const
    {
        shared_ptr<EvtData_ScriptEventTest_FromLua> pCopy(GCC_NEW EvtData_ScriptEventTest_FromLua(m_num));
        pCopy->m_eventData = m_eventData;
        return pCopy;
    }

    virtual const char* GetName(void) const { return "EvtData_ScriptEventTest_FromLua"; }

    int GetNum(void) { return m_num; }

protected:
    //virtual void VBuildEventData(void);
    virtual bool VBuildEventFromScript(void);

    EXPORT_FOR_SCRIPT_EVENT(EvtData_ScriptEventTest_FromLua);
};


void RegisterTeapotScriptEvents(void);
