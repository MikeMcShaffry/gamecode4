#pragma once
//========================================================================
// File: Mesh.h - classes to render meshes in D3D9 and D3D11
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


#include <SDKMesh.h>

#include "Geometry.h"
#include "../ResourceCache/ResCache.h"


//
// class D3DSdkMeshResourceExtraData11					- Chapter 16, page 561
//
class D3DSdkMeshResourceExtraData11 : public IResourceExtraData
{
	friend class SdkMeshResourceLoader;

public:
	D3DSdkMeshResourceExtraData11() { };
	virtual ~D3DSdkMeshResourceExtraData11() { }
	virtual std::string VToString() { return "D3DSdkMeshResourceExtraData11"; }

	CDXUTSDKMesh                m_Mesh11;	
};

//
// class SdkMeshResourceLoader							- Chapter 16, page 561
//
class SdkMeshResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return false; }
	virtual unsigned int VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.sdkmesh"; }
};




////////////////////////////////////////////////////
//
// D3DMeshNode9 Description
//
//    Attaches a ID3DXMesh Mesh object to the GameCode4 scene graph
//    This is a creature of D3D9, and not described in the 4th edition.
//    In the 3rd edition this was called D3DMeshNode
//
////////////////////////////////////////////////////

class D3DMeshNode9 : public D3DSceneNode9
{
protected:
	ID3DXMesh *m_pMesh;
	std::wstring m_XFileName;

public:
	D3DMeshNode9(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		ID3DXMesh *Mesh, 
		RenderPass renderPass, 
		const Mat4x4 *t);

	D3DMeshNode9(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		std::wstring xFileName, 
		RenderPass renderPass,  
		const Mat4x4 *t); 

	virtual ~D3DMeshNode9() { SAFE_RELEASE(m_pMesh); }
	HRESULT VRender(Scene *pScene);
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);
	
	float CalcBoundingSphere();
};


////////////////////////////////////////////////////
//
// D3DShaderMeshNode9 Description
//
//    Attaches a ID3DXMesh Mesh object to the GameCode4 scene graph and renders it with a shader.
//    This is a creature of D3D9, and not described in the 4th edition.
//    In the 3rd edition this was called D3DShaderMeshNode
//
////////////////////////////////////////////////////

class D3DShaderMeshNode9 : public D3DMeshNode9
{
protected:
	ID3DXEffect *m_pEffect;
	std::string m_fxFileName;

public:
	D3DShaderMeshNode9(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		ID3DXMesh *Mesh, 
		std::string fxFileName,
		RenderPass renderPass, 
		const Mat4x4 *t); 

	D3DShaderMeshNode9(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		std::wstring xFileName, 
		std::string fxFileName,
		RenderPass renderPass, 
		const Mat4x4 *t); 

	virtual ~D3DShaderMeshNode9() { SAFE_RELEASE(m_pEffect); }
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VOnLostDevice(Scene *pScene);
	HRESULT VRender(Scene *pScene);
};

////////////////////////////////////////////////////
//
// class TeapotMeshNode9				- Chapter X, page Y
//
////////////////////////////////////////////////////

class D3DTeapotMeshNode9 : public D3DShaderMeshNode9
{
public:
	D3DTeapotMeshNode9(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, std::string fxFileName, RenderPass renderPass, const Mat4x4 *t);
	virtual HRESULT VOnRestore(Scene *pScene);
};


class D3DShaderMeshNode11 : public SceneNode
{
public:
	D3DShaderMeshNode11(const ActorId actorId, 
		WeakBaseRenderComponentPtr renderComponent,
		std::string sdkMeshFileName, 
		RenderPass renderPass, 
		const Mat4x4 *t);

	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VOnLostDevice(Scene *pScene) { return S_OK; }
	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);

protected:
	std::string					m_sdkMeshFileName;

	GameCode4_Hlsl_VertexShader		m_VertexShader;
	GameCode4_Hlsl_PixelShader		m_PixelShader;

	float CalcBoundingSphere(CDXUTSDKMesh *mesh11);			// this was added post press.
};


class D3DTeapotMeshNode11 : public D3DShaderMeshNode11
{
public:
	D3DTeapotMeshNode11(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass,const Mat4x4 *t)
		: D3DShaderMeshNode11(actorId, renderComponent, "art\\teapot.sdkmesh", renderPass, t) { }
	bool VIsVisible() { return true; }
};



