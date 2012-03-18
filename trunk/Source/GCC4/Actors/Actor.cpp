//========================================================================
// Actor.cpp - Implements the Actor class
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
#include "Actor.h"
#include "ActorComponent.h"
#include "../Utilities/String.h"


//---------------------------------------------------------------------------------------------------------------------
// Actor
// Chapter 6, page 165
//---------------------------------------------------------------------------------------------------------------------
Actor::Actor(ActorId id)
{
    m_id = id;
    m_type = "Unknown";

	// [mrmike] added post press - this is an editor helper
	m_resource = "Unknown";
}

Actor::~Actor(void)
{
    GCC_LOG("Actor", std::string("Destroying Actor ") + ToStr(m_id));
    GCC_ASSERT(m_components.empty());  // [rez] if this assert fires, the actor was destroyed without calling Actor::Destroy()
}

bool Actor::Init(TiXmlElement* pData)
{
    GCC_LOG("Actor", std::string("Initializing Actor ") + ToStr(m_id));

	m_type = pData->Attribute("type");
	m_resource = pData->Attribute("resource");
    return true;
}

void Actor::PostInit(void)
{
    for (ActorComponents::iterator it = m_components.begin(); it != m_components.end(); ++it)
    {
        it->second->VPostInit();
    }
}

void Actor::Destroy(void)
{
    m_components.clear();
}

void Actor::Update(int deltaMs)
{
    for (ActorComponents::iterator it = m_components.begin(); it != m_components.end(); ++it)
    {
        it->second->VUpdate(deltaMs);
    }
}

std::string Actor::ToXML()
{
    TiXmlDocument outDoc;

    // Actor element
    TiXmlElement* pActorElement = GCC_NEW TiXmlElement("Actor");
    pActorElement->SetAttribute("type", m_type.c_str());
	pActorElement->SetAttribute("resource", m_resource.c_str());

    // components
    for (auto it = m_components.begin(); it != m_components.end(); ++it)
    {
        StrongActorComponentPtr pComponent = it->second;
        TiXmlElement* pComponentElement = pComponent->VGenerateXml();
        pActorElement->LinkEndChild(pComponentElement);
    }

    outDoc.LinkEndChild(pActorElement);
	TiXmlPrinter printer;
	outDoc.Accept(&printer);

	return printer.CStr();
}


void Actor::AddComponent(StrongActorComponentPtr pComponent)
{
    std::pair<ActorComponents::iterator, bool> success = m_components.insert(std::make_pair(pComponent->VGetId(), pComponent));
    GCC_ASSERT(success.second);
}

