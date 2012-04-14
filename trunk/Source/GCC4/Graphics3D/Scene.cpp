//========================================================================
// File: Scene.cpp - implements the container class for 3D Graphics scenes
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

#include "../GameCode4/GameCode.h"
#include "../EventManager/EventManager.h"
#include "../EventManager/Events.h"
#include "../Utilities/String.h"

#include "Geometry.h"
#include "Lights.h"


////////////////////////////////////////////////////
// Scene Implementation
////////////////////////////////////////////////////


//
// Scene::Scene						- Chapter 16, page 539
//
//    Note: The shared_ptr<IRenderer> was added to allow for both D3D9 and D3D11 renderer implementations.
//          The book only describes D3D11, so to find all the differences, just search for m_Renderer!
//
Scene::Scene(shared_ptr<IRenderer> renderer)
{
	m_Root.reset(GCC_NEW RootNode());
	m_Renderer = renderer;
	m_LightManager = GCC_NEW LightManager;

	D3DXCreateMatrixStack(0, &m_MatrixStack);

	// [mrmike] - event delegates were added post-press
    IEventManager* pEventMgr = IEventManager::Get();
    pEventMgr->VAddListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EvtData_New_Render_Component::sk_EventType);
    pEventMgr->VAddListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EvtData_Destroy_Actor::sk_EventType);
    pEventMgr->VAddListener(MakeDelegate(this, &Scene::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);
    pEventMgr->VAddListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EvtData_Modified_Render_Component::sk_EventType);
}

//
// Scene::~Scene					- Chapter 16, page 539
//
Scene::~Scene()
{
	// [mrmike] - event delegates were added post-press!
    IEventManager* pEventMgr = IEventManager::Get();
    pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::NewRenderComponentDelegate), EvtData_New_Render_Component::sk_EventType);
    pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::DestroyActorDelegate), EvtData_Destroy_Actor::sk_EventType);
    pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);

    pEventMgr->VRemoveListener(MakeDelegate(this, &Scene::ModifiedRenderComponentDelegate), EvtData_Modified_Render_Component::sk_EventType);

	SAFE_RELEASE(m_MatrixStack);
	SAFE_DELETE(m_LightManager);
}

//
// Scene::OnRender					- Chapter 16, page 539
//
HRESULT Scene::OnRender()
{
	// The render passes usually go like this 
	// 1. Static objects & terrain
	// 2. Actors (dynamic objects that can move)
	// 3. The Sky
	// 4. Anything with Alpha

	if (m_Root && m_Camera)
	{
		// The scene root could be anything, but it
		// is usually a SceneNode with the identity
		// matrix

		m_Camera->SetViewTransform(this);

		m_LightManager->CalcLighting(this);

		if (m_Root->VPreRender(this)==S_OK)
		{
			m_Root->VRender(this);
			m_Root->VRenderChildren(this);
			m_Root->VPostRender(this);
		}
		RenderAlphaPass();
	}

	return S_OK;
}	

//
// Scene::OnLostDevice						- not in the book
//
//    All Scene nodes implement VOnLostDevice, which is called in the D3D9 renderer.
//
HRESULT Scene::OnLostDevice()
{
	if (m_Root)
	{
		return m_Root->VOnLostDevice(this);
	}
	return S_OK;
}

//
// Scene::OnRestore					- Chapter 16, page 540
//
HRESULT Scene::OnRestore()
{
	if (!m_Root)
		return S_OK;

	HRESULT hr;
	V_RETURN(m_Renderer->VOnRestore());

	return m_Root->VOnRestore(this);
}





bool Scene::AddChild(ActorId id, shared_ptr<ISceneNode> kid)
{ 
	if (id != INVALID_ACTOR_ID)
	{
		// This allows us to search for this later based on actor id
		m_ActorMap[id] = kid;	
	}

	shared_ptr<LightNode> pLight = dynamic_pointer_cast<LightNode>(kid);
	if (pLight != NULL && m_LightManager->m_Lights.size()+1 < MAXIMUM_LIGHTS_SUPPORTED)
	{
		m_LightManager->m_Lights.push_back(pLight);
	}
	return m_Root->VAddChild(kid); 
}

bool Scene::RemoveChild(ActorId id)
{
	if (id == INVALID_ACTOR_ID)
		return false;
	shared_ptr<ISceneNode> kid = FindActor(id);
	shared_ptr<LightNode> pLight = dynamic_pointer_cast<LightNode>(kid);
	if (pLight != NULL)
	{
		m_LightManager->m_Lights.remove(pLight);
	}
	m_ActorMap.erase(id);
	return m_Root->VRemoveChild(id); 
}



void Scene::NewRenderComponentDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_New_Render_Component> pCastEventData = static_pointer_cast<EvtData_New_Render_Component>(pEventData);

    ActorId actorId = pCastEventData->GetActorId();
    shared_ptr<SceneNode> pSceneNode(pCastEventData->GetSceneNode());

    // FUTURE WORK: Add better error handling here.		
    if (FAILED(pSceneNode->VOnRestore(this)))
    {
		std::string error = "Failed to restore scene node to the scene for actorid " + ToStr(actorId);
        GCC_ERROR(error);
        return;
    }

    AddChild(actorId, pSceneNode);
}

void Scene::ModifiedRenderComponentDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_Modified_Render_Component> pCastEventData = static_pointer_cast<EvtData_Modified_Render_Component>(pEventData);

    ActorId actorId = pCastEventData->GetActorId();
	if (actorId == INVALID_ACTOR_ID)
	{
		GCC_ERROR("Scene::ModifiedRenderComponentDelegate - unknown actor id!");
		return;
	}

	if (g_pApp->GetGameLogic()->GetState()==BGS_LoadingGameEnvironment)
		return;

	shared_ptr<ISceneNode> pSceneNode = FindActor(actorId);
    // FUTURE WORK: Add better error handling here.		
    if (!pSceneNode  || FAILED(pSceneNode->VOnRestore(this)))
    {
        GCC_ERROR("Failed to restore scene node to the scene for actorid " + ToStr(actorId));
    }
}

void Scene::DestroyActorDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_Destroy_Actor> pCastEventData = static_pointer_cast<EvtData_Destroy_Actor>(pEventData);
    RemoveChild(pCastEventData->GetId());
}

void Scene::MoveActorDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_Move_Actor> pCastEventData = static_pointer_cast<EvtData_Move_Actor>(pEventData);

    ActorId id = pCastEventData->GetId();
    Mat4x4 transform = pCastEventData->GetMatrix();

    shared_ptr<ISceneNode> pNode = FindActor(id);
    if (pNode)
    {
        pNode->VSetTransform(&transform);
    }
}



//
// Scene::OnUpdate					- Chapter 16, page 540
//
HRESULT Scene::OnUpdate(const int deltaMilliseconds)
{
	if (!m_Root)
		return S_OK;

	static DWORD lastTime = timeGetTime();
	DWORD elapsedTime = 0;
	DWORD now = timeGetTime();

	elapsedTime = now - lastTime;
	lastTime = now;

	return m_Root->VOnUpdate(this, elapsedTime);
}

//
// Scene::FindActor					- Chapter 16, page 542
//
shared_ptr<ISceneNode> Scene::FindActor(ActorId id)
{
	SceneActorMap::iterator i = m_ActorMap.find(id);
	if (i==m_ActorMap.end())
	{
		return shared_ptr<ISceneNode>();
	}

	return i->second;
}


//
// Scene::RenderAlphaPass			- Chapter 16, page 543
//
void Scene::RenderAlphaPass()
{
	shared_ptr<IRenderState> alphaPass = m_Renderer->VPrepareAlphaPass();

	m_AlphaSceneNodes.sort();
	while (!m_AlphaSceneNodes.empty())
	{
		AlphaSceneNodes::reverse_iterator i = m_AlphaSceneNodes.rbegin();
		PushAndSetMatrix((*i)->m_Concat);
		(*i)->m_pNode->VRender(this);
		delete (*i);
		PopMatrix();
		m_AlphaSceneNodes.pop_back();
	}
}



