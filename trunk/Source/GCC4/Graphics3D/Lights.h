#pragma once
//========================================================================
// Lights.h - implements a simple light class for the GameCode4 scene graph
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


#include "D3DRenderer.h"
#include "Geometry.h"
#include "Scene.h"
#include "SceneNodes.h"

// Note: Light color is stored in the Material structure, which is already present in all SceneNodes.

//
// struct LightProperties				- Chapter 16, page 551
//
struct LightProperties
{
	float	m_Attenuation[3];  /* Attenuation coefficients */
	float	m_Range;
	float	m_Falloff;
	float	m_Theta;
	float	m_Phi;
};


//
// class LightNode						- Chapter 16, page 551
//
//    Note: In the book this class implements the LightNode in D3D11, but here it is a base
//    class. The derived classes make it possible to run the engine in D3D9 or D3D11.
//
class LightNode : public SceneNode
{
protected:
	LightProperties m_LightProps;

public:
	LightNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const LightProperties &props, const Mat4x4 *t);
};


class D3DLightNode9 : public LightNode
{
public:
	D3DLightNode9(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const LightProperties &lightProps, const Mat4x4 *t)
		: LightNode(actorId, renderComponent, lightProps,  t) { }

	D3DLIGHT9	m_d3dLight9;

	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VOnUpdate(Scene *, DWORD const elapsedMs);
};


class D3DLightNode11 : public LightNode
{
public:
	D3DLightNode11(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent,  const LightProperties &lightProps, const Mat4x4 *t)
		: LightNode(actorId, renderComponent, lightProps,  t) { }

	virtual HRESULT VOnRestore() { return S_OK; };
	virtual HRESULT VOnUpdate(Scene *, DWORD const elapsedMs);
};


struct ConstantBuffer_Lighting;


//
// class LightManager					- Chapter 16, 553
//
class LightManager
{
	friend class Scene;

protected:
	Lights					m_Lights;
	Vec4					m_vLightDir[MAXIMUM_LIGHTS_SUPPORTED];
    Color					m_vLightDiffuse[MAXIMUM_LIGHTS_SUPPORTED];
	Vec4					m_vLightAmbient;
public:
	void CalcLighting(Scene *pScene);
	void CalcLighting(ConstantBuffer_Lighting* pLighting, SceneNode *pNode);
	int GetLightCount(const SceneNode *node) { return m_Lights.size(); }
	const Vec4 *GetLightAmbient(const SceneNode *node) { return &m_vLightAmbient; }
	const Vec4 *GetLightDirection(const SceneNode *node) { return m_vLightDir; }
	const Color *GetLightDiffuse(const SceneNode *node) { return m_vLightDiffuse; }
};
