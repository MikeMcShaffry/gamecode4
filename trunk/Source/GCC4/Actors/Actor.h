#pragma once
//========================================================================
// Actor.h - Defines the Actor class
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

class TiXmlElement;
typedef std::string ActorType;

//---------------------------------------------------------------------------------------------------------------------
// Actor class
// Chapter 6, page 165
//---------------------------------------------------------------------------------------------------------------------
class Actor
{
    friend class ActorFactory;

public:

    typedef std::map<ComponentId, StrongActorComponentPtr> ActorComponents;

private:
    ActorId m_id;					// unique id for the actor
    ActorComponents m_components;	// all components this actor has
    ActorType m_type;

	// [mrmike] - these were added post press as editor helpers, but will also be great for save game files if we ever make them.
	std::string m_resource;			// the XML file from which this actor was initialized (considered the "Archetype" file)

public:
    explicit Actor(ActorId id);
    ~Actor(void);

    bool Init(TiXmlElement* pData);
    void PostInit(void);
    void Destroy(void);
    void Update(int deltaMs);

    // editor functions
    //bool SaveActorFromEditor(const char* path);      //[mrmike] - we can delete this.
	std::string Actor::ToXML();

    // accessors
    ActorId GetId(void) const { return m_id; }
    ActorType GetType(void) const { return m_type; }

    // template function for retrieving components
    template <class ComponentType>
    weak_ptr<ComponentType> GetComponent(ComponentId id)
    {
        ActorComponents::iterator findIt = m_components.find(id);
        if (findIt != m_components.end())
        {
            StrongActorComponentPtr pBase(findIt->second);
            shared_ptr<ComponentType> pSub(static_pointer_cast<ComponentType>(pBase));  // cast to subclass version of the pointer
            weak_ptr<ComponentType> pWeakSub(pSub);  // convert strong pointer to weak pointer
            return pWeakSub;  // return the weak pointer
        }
        else
        {
            return weak_ptr<ComponentType>();
        }
    }

    template <class ComponentType>
    weak_ptr<ComponentType> GetComponent(const char *name)
    {
		ComponentId id = ActorComponent::GetIdFromName(name);
        ActorComponents::iterator findIt = m_components.find(id);
        if (findIt != m_components.end())
        {
            StrongActorComponentPtr pBase(findIt->second);
            shared_ptr<ComponentType> pSub(static_pointer_cast<ComponentType>(pBase));  // cast to subclass version of the pointer
            weak_ptr<ComponentType> pWeakSub(pSub);  // convert strong pointer to weak pointer
            return pWeakSub;  // return the weak pointer
        }
        else
        {
            return weak_ptr<ComponentType>();
        }
    }

	const ActorComponents* GetComponents() { return &m_components; }

    void AddComponent(StrongActorComponentPtr pComponent);
};


