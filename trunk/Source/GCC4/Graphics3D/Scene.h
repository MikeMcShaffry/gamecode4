#pragma once
//========================================================================
// File: Scene.h - implements the container class for 3D Graphics scenes
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


#include "Geometry.h"
#include "SceneNodes.h"

// Forward declarations
////////////////////////////////////////////////////
//
// SceneActorMap Description				- Chapter 16, page Y
//
//   An STL map that allows fast lookup of a scene node given an ActorId.
//
////////////////////////////////////////////////////

typedef std::map<ActorId, shared_ptr<ISceneNode> > SceneActorMap;


////////////////////////////////////////////////////
//
// Scene Description						- Chapter 16, page 536
//
// A heirarchical container of scene nodes, which
// are classes that implemente the ISceneNode interface
//
////////////////////////////////////////////////////

class CameraNode;
class SkyNode;
class LightNode;
class LightManager;

class Scene
{
protected:
	shared_ptr<SceneNode>  	m_Root;
	shared_ptr<CameraNode> 	m_Camera;
	shared_ptr<IRenderer>	m_Renderer;

	ID3DXMatrixStack 		*m_MatrixStack;
	AlphaSceneNodes 		m_AlphaSceneNodes;
	SceneActorMap 			m_ActorMap;

	LightManager			*m_LightManager;

	void RenderAlphaPass();

public:
	Scene(shared_ptr<IRenderer> renderer);
	virtual ~Scene();

	HRESULT OnRender();
	HRESULT OnRestore();
	HRESULT OnLostDevice();
	HRESULT OnUpdate(const int deltaMilliseconds);
	shared_ptr<ISceneNode> FindActor(ActorId id);
	bool AddChild(ActorId id, shared_ptr<ISceneNode> kid);
	bool RemoveChild(ActorId id);

    // event delegates
    void NewRenderComponentDelegate(IEventDataPtr pEventData);
    void ModifiedRenderComponentDelegate(IEventDataPtr pEventData);			// added post-press!
    void DestroyActorDelegate(IEventDataPtr pEventData);
    void MoveActorDelegate(IEventDataPtr pEventData);

	void SetCamera(shared_ptr<CameraNode> camera) { m_Camera = camera; }
	const shared_ptr<CameraNode> GetCamera() const { return m_Camera; }


	void PushAndSetMatrix(const Mat4x4 &toWorld)
	{
		// Note this code carefully!!!!! It is COMPLETELY different
		// from some DirectX 9 documentation out there....
		// Scene::PushAndSetMatrix - Chapter 16, page 541

		m_MatrixStack->Push();
		m_MatrixStack->MultMatrixLocal(&toWorld);
		Mat4x4 mat = GetTopMatrix();
		m_Renderer->VSetWorldTransform(&mat);
	}

	void PopMatrix() 
	{
		// Scene::PopMatrix - Chapter 16, page 541
		m_MatrixStack->Pop(); 
		Mat4x4 mat = GetTopMatrix();
		m_Renderer->VSetWorldTransform(&mat);
	}

	const Mat4x4 GetTopMatrix() 
	{ 
		// Scene::GetTopMatrix - Chapter 16, page 541
		return static_cast<const Mat4x4>(*m_MatrixStack->GetTop()); 
	}

	LightManager *GetLightManager() { return m_LightManager; }

	void AddAlphaSceneNode(AlphaSceneNode *asn) { m_AlphaSceneNodes.push_back(asn); }

	HRESULT Pick(RayCast *pRayCast) { return m_Root->VPick(this, pRayCast); }

	shared_ptr<IRenderer> GetRenderer() { return m_Renderer; }
};

