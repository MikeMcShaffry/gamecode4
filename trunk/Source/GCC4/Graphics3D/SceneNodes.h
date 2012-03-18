#pragma once
//========================================================================
// SceneNodes.h - defines the base class for the 3D graphics scene graph, and
//                  derived classes like RootNode, Alpha scene nodes, Camera, Grids, etc.                  
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
#include "Material.h"
#include "Shaders.h"

// Forward declarations
class SceneNode;
class Scene;
class RayCast;
class MovementController;
class IResourceExtraData;
class ActorComponent;
class BaseRenderComponent; 

// FUTURE WORK - Smart pointers don't work right....going to use a naked pointer for now!
typedef BaseRenderComponent* WeakBaseRenderComponentPtr;

////////////////////////////////////////////////////
//
// AlphaType					- Chapter X, page Y
//
//   This enum defines the different types of alpha blending
//   types that can be set on a scene node.
//
//	
////////////////////////////////////////////////////

enum AlphaType
{
	AlphaOpaque,
	AlphaTexture,
	AlphaMaterial,
	AlphaVertex
};


//////////////////////////////////////////////////////////////////////
//  class SceneNodeProperties	- Chapter 16, page 527
//
//   This class is contained in the SceneNode class, and gains
//   easy access to common scene node properties such as its ActorId
//   or render pass, with a single ISceneNode::VGet() method.
//
//////////////////////////////////////////////////////////////////////

class SceneNodeProperties
{
	friend class SceneNode;

protected:
	ActorId                 m_ActorId;
	std::string				m_Name;
	Mat4x4					m_ToWorld, m_FromWorld;
	float					m_Radius;
	RenderPass				m_RenderPass;
	Material				m_Material;
	AlphaType				m_AlphaType;

	void SetAlpha(const float alpha) 
		{ m_AlphaType=AlphaMaterial; m_Material.SetAlpha(alpha); } 

public:
    SceneNodeProperties(void);
	const ActorId &ActorId() const { return m_ActorId; }
	Mat4x4 const &ToWorld() const { return m_ToWorld; }
	Mat4x4 const &FromWorld() const { return m_FromWorld; }
	void Transform(Mat4x4 *toWorld, Mat4x4 *fromWorld) const;

	const char * Name() const { return m_Name.c_str(); }		

	bool HasAlpha() const { return m_Material.HasAlpha(); }						
	float Alpha() const { return m_Material.GetAlpha(); }
	AlphaType AlphaType() const { return m_AlphaType; }

	RenderPass RenderPass() const { return m_RenderPass; }
	float Radius() const { return m_Radius; }

	Material GetMaterial() const { return m_Material; }
};

//////////////////////////////////////////////////////////////
//
// SceneNodeList						- Chapter 16, page 529
//
//   Every scene node has a list of its children - this 
//   is implemented as a vector since it is expected that
//   we won't add/delete nodes very often, and we'll want 
//   fast random access to each child.
//
//////////////////////////////////////////////////////////////

typedef std::vector<shared_ptr<ISceneNode> > SceneNodeList;


//////////////////////////////////////////////////////////////
//
// SceneNode							- Chapter 16, page 529
//
//   Implements ISceneNode. Forms the base class for any node
//   that can exist in the 3D scene graph managed by class Scene.
//
//////////////////////////////////////////////////////////////

class SceneNode : public ISceneNode
{
	friend class Scene;

protected:
	SceneNodeList			m_Children;
	SceneNode				*m_pParent;
	SceneNodeProperties		m_Props;
	WeakBaseRenderComponentPtr	m_RenderComponent;

public:
	SceneNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Mat4x4 *to, const Mat4x4 *from=NULL);

	virtual ~SceneNode();

	virtual const SceneNodeProperties* const VGet() const { return &m_Props; }

	virtual void VSetTransform(const Mat4x4 *toWorld, const Mat4x4 *fromWorld=NULL);

	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VOnUpdate(Scene *, DWORD const elapsedMs);

	virtual HRESULT VPreRender(Scene *pScene);
	virtual bool VIsVisible(Scene *pScene) const;
	virtual HRESULT VRender(Scene *pScene) { return S_OK; } 
	virtual HRESULT VRenderChildren(Scene *pScene);
	virtual HRESULT VPostRender(Scene *pScene);

	virtual bool VAddChild(shared_ptr<ISceneNode> kid);
	virtual bool VRemoveChild(ActorId id);
	virtual HRESULT VOnLostDevice(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);

	void SetAlpha(float alpha);
	float GetAlpha() const { return m_Props.Alpha(); }

	Vec3 GetPosition() const { return m_Props.m_ToWorld.GetPosition(); }
	void SetPosition(const Vec3 &pos) { m_Props.m_ToWorld.SetPosition(pos); }

	const Vec3 GetWorldPosition() const;					// [mrmike] added post-press to respect ancestor's position 

	Vec3 GetDirection() const { return m_Props.m_ToWorld.GetDirection(); }

	void SetRadius(const float radius) { m_Props.m_Radius = radius; }
	void SetMaterial(const Material &mat) { m_Props.m_Material = mat; }
};

//
// class D3DSceneNode9						- not described in the book
// class D3DSceneNode11						- not described in the book
//
// The book only describes a D3D11 render implementations. The engine code implements D3D9 as well as 
//    D3D11, by creating different renderers and scene nodes based on the settings of PlayerOptions.xml.  
//
class D3DSceneNode9 : public SceneNode
{
public:
	D3DSceneNode9(const ActorId actorId, 
	WeakBaseRenderComponentPtr renderComponent,
	RenderPass renderPass, 
	const Mat4x4 *t)
	 : SceneNode(actorId, renderComponent, renderPass, t) { }

	virtual HRESULT VRender(Scene *pScene);
};

class D3DSceneNode11 : public SceneNode
{
public:
	virtual HRESULT VRender(Scene *pScene) { return S_OK; }
};


////////////////////////////////////////////////////
//
// AlphaSceneNode Description						- Chapter 16, page 535
// AlphaSceneNodes Description						- Chapter 16, page 535
//
// A list of scene nodes that need to be drawn in the alpha pass;
// the list is defined as an STL list
////////////////////////////////////////////////////

struct AlphaSceneNode
{
	shared_ptr<ISceneNode> m_pNode;
	Mat4x4 m_Concat;
	float m_ScreenZ;

	// For the STL sort...
	bool const operator <(AlphaSceneNode const &other) { return m_ScreenZ < other.m_ScreenZ; }
};

typedef std::list<AlphaSceneNode *> AlphaSceneNodes;


////////////////////////////////////////////////////
//
// SceneActorMap Description
//
//   An STL map that allows fast lookup of a scene node given an ActorId.
//
////////////////////////////////////////////////////

typedef std::map<ActorId, shared_ptr<ISceneNode> > SceneActorMap;


////////////////////////////////////////////////////
//
// Scene Description
//
// A heirarchical container of scene nodes, which
// are classes that implemente the ISceneNode interface
//
////////////////////////////////////////////////////

class CameraNode;
class SkyNode;


////////////////////////////////////////////////////
//
// class RootNode					- Chapter 16, page 545
//
//    This is the root node of the scene graph.
//
////////////////////////////////////////////////////

class RootNode : public SceneNode
{
public:
	RootNode();
	virtual bool VAddChild(shared_ptr<ISceneNode> kid);
	virtual HRESULT VRenderChildren(Scene *pScene);
	virtual bool VRemoveChild(ActorId id);
	virtual bool VIsVisible(Scene *pScene) const { return true; }
};


////////////////////////////////////////////////////
//
// class CameraNode					- Chapter 16, page 548
//
//    A camera node controls the D3D view transform and holds the view
//    frustum definition
//
////////////////////////////////////////////////////

class CameraNode : public SceneNode
{
public:
	CameraNode(Mat4x4 const *t, Frustum const &frustum) 
	 : SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_0, t),
	   m_Frustum(frustum),
	   m_bActive(true),
	   m_DebugCamera(false),
	   m_pTarget(shared_ptr<SceneNode>()),
	   m_CamOffsetVector( 0.0f, 1.0f, -10.0f, 0.0f )
	{ 
	}

	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual bool VIsVisible(Scene *pScene) const { return m_bActive; }

	const Frustum &GetFrustum() { return m_Frustum; }
	void SetTarget(shared_ptr<SceneNode> pTarget)
		{ m_pTarget = pTarget; }
	void ClearTarget() { m_pTarget = shared_ptr<SceneNode>(); }
	shared_ptr<SceneNode> GetTarget() { return m_pTarget; }

	Mat4x4 GetWorldViewProjection(Scene *pScene);
	HRESULT SetViewTransform(Scene *pScene);

	Mat4x4 GetProjection() { return m_Projection; }
	Mat4x4 GetView() { return m_View; }

	void SetCameraOffset( const Vec4 & cameraOffset )
	{
		m_CamOffsetVector = cameraOffset;
	}

protected:

	Frustum			m_Frustum;
    Mat4x4			m_Projection;
	Mat4x4			m_View;
	bool			m_bActive;
	bool			m_DebugCamera;
	shared_ptr<SceneNode> m_pTarget;
	Vec4			m_CamOffsetVector;	//Direction of camera relative to target.
};




////////////////////////////////////////////////////
//
// D3DGrid9 Description
//
//    A slightly modified version of Grid from 
//       the beginning 3D chapter
//
////////////////////////////////////////////////////

class D3DGrid9 : public SceneNode
{
protected:
	shared_ptr<ResHandle>	m_handle;			// the resource handle for the grid texture for the axes planes
	LPDIRECT3DVERTEXBUFFER9 m_pVerts;			// the grid verts
	LPDIRECT3DINDEXBUFFER9	m_pIndices;			// the grid index
	DWORD					m_numVerts;
	DWORD					m_numPolys;
//    std::string             m_textureResource;
//    int                     m_squares;

public:
	bool					m_bTextureHasAlpha;

    D3DGrid9(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, /* const std::string& name, const char* textureResource, int squares, const Color &diffuseColor, */ const Mat4x4* pMatrix);
	virtual ~D3DGrid9();
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);

	bool VHasAlpha() const { return m_bTextureHasAlpha; }
};



class D3DGrid11 : public SceneNode
{
protected:
	DWORD					m_numVerts;
	DWORD					m_numPolys;

//    int                     m_squares;

	ID3D11Buffer*               m_pIndexBuffer;
	ID3D11Buffer*               m_pVertexBuffer;

	GameCode4_Hlsl_VertexShader		m_VertexShader;
	GameCode4_Hlsl_PixelShader		m_PixelShader;

public:
	bool					m_bTextureHasAlpha;
	
    D3DGrid11(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, /* const std::string& name, const char* textureResource, int squares, const Color &diffuseColor, */ const Mat4x4* pMatrix);
	virtual ~D3DGrid11();
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VOnUpdate(Scene *pScene, DWORD const elapsedMs) { return S_OK; }
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast) { return E_FAIL; }

	bool VHasAlpha() const { return m_bTextureHasAlpha; }
};



////////////////////////////////////////////////////
//
// ArrowNode Description
//
//   Graphical object that looks like an arrow - they
//     aren't discussed in the book per se but you see them
//     in a lot of the figures!
//
////////////////////////////////////////////////////

class ArrowNode : public SceneNode
{
protected:
	ID3DXMesh *m_shaft;
	ID3DXMesh *m_cone;
	Mat4x4 m_coneTrans;
	Mat4x4 m_shaftTrans;
	
public:
	ArrowNode(std::string name, WeakBaseRenderComponentPtr renderComponent, const float length, const Mat4x4 *t, const Color &color);

	virtual ~ArrowNode() { SAFE_RELEASE(m_shaft); SAFE_RELEASE(m_cone); }
	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);
};


////////////////////////////////////////////////////
//
// TestObject Description
//
//    Implements a test object that has no symmetry on X, Y, and Z axes.
//    Really useful to make sure you haven't flipped left-handed versus
//    right handed coordinate systems or something like that
//
////////////////////////////////////////////////////

//class TriangleIterator;
//struct TestObjectParams;


class TestObject : public SceneNode
{
protected:
	LPDIRECT3DVERTEXBUFFER9 m_pVerts;			
	DWORD					m_numVerts;
	DWORD					m_numPolys;

	DWORD					m_color;
	bool					m_squashed;

public:
	//TestObject(TestObjectParams const &p, std::string name);
    TestObject(std::string name);
	virtual ~TestObject();
	HRESULT VOnRestore(Scene *pScene);
	HRESULT VRender(Scene *pScene);

//	TriangleIterator *CreateTriangleIterator();

	static WORD g_TestObjectIndices[][3];
	static Vec3 g_CubeVerts[];
	static Vec3 g_SquashedCubeVerts[];
};

