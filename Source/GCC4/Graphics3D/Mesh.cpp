//========================================================================
// Mesh.cpp - classes to render meshes in D3D9 and D3D11
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

#include <SDKmisc.h>

#include "../GameCode4/GameCode.h"
#include "../ResourceCache/ResCache.h"
#include "D3DRenderer.h"
#include "Lights.h"
#include "Mesh.h"
#include "Raycast.h"
#include "SceneNodes.h"


shared_ptr<IResourceLoader> CreateSdkMeshResourceLoader()
{
	return shared_ptr<IResourceLoader>(GCC_NEW SdkMeshResourceLoader());
}

unsigned int SdkMeshResourceLoader::VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize)
{
	// The raw data of the SDK Mesh file is needed by the CDXUTMesh class, so we're going to keep it around.
	return rawSize;
}

//
// SdkMeshResourceLoader::VLoadResource						- Chapter 16, page 561
//
bool SdkMeshResourceLoader::VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle)
{
	GameCodeApp::Renderer renderer = GameCodeApp::GetRendererImpl();
	if (renderer == GameCodeApp::Renderer_D3D9)
	{
		GCC_ASSERT(0 && "This is not supported in D3D9");
	}
	else if (renderer == GameCodeApp::Renderer_D3D11)
	{
		shared_ptr<D3DSdkMeshResourceExtraData11> extra = shared_ptr<D3DSdkMeshResourceExtraData11>(GCC_NEW D3DSdkMeshResourceExtraData11());

		// Load the Mesh
	    if (SUCCEEDED ( extra->m_Mesh11.Create( DXUTGetD3D11Device(), (BYTE *)rawBuffer, (UINT)rawSize, true ) ) )
		{
			handle->SetExtra(shared_ptr<D3DSdkMeshResourceExtraData11>(extra));
		}

		return true;
	}

	GCC_ASSERT(0 && "Unsupported Renderer in SdkMeshResourceLoader::VLoadResource");
	return false;
}




////////////////////////////////////////////////////
// MeshNode Implementation
////////////////////////////////////////////////////

//
// D3DMeshNode9::D3DMeshNode9				- 3rd Edition, Chapter 14, page 504
//
D3DMeshNode9::D3DMeshNode9(const ActorId actorId, 
	WeakBaseRenderComponentPtr renderComponent,
	ID3DXMesh *Mesh, 
	RenderPass renderPass, 
	const Mat4x4 *t )
 : D3DSceneNode9(actorId, renderComponent, renderPass, t) 
{ 
	m_pMesh = Mesh; 
	if (m_pMesh)
	{
		// Added post press - not all Mesh modes have Meshes. Some are just effects!
		m_pMesh->AddRef(); 
	}
}

//
// MeshNode::MeshNode					-  3rd Edition, Chapter 14, page 505
//
D3DMeshNode9::D3DMeshNode9(const ActorId actorId, 
	WeakBaseRenderComponentPtr renderComponent,
	std::wstring xFileName, 
	RenderPass renderPass,  
	const Mat4x4 *t)
 : D3DSceneNode9(actorId, renderComponent, renderPass, t) 
{ 
	m_pMesh = NULL;
	m_XFileName = xFileName;
}

//
// MeshNode::CalcBoundingSphere				-  3rd Edition, Chapter 14, page 507
//
float D3DMeshNode9::CalcBoundingSphere()
{
    D3DXVECTOR3* pData;
    D3DXVECTOR3 vCenter;
    FLOAT fObjectRadius;
	HRESULT hr;
	V( m_pMesh->LockVertexBuffer( 0, ( LPVOID* )&pData ) );
    V( D3DXComputeBoundingSphere( pData, m_pMesh->GetNumVertices(),
                                  D3DXGetFVFVertexSize( m_pMesh->GetFVF() ), &vCenter, &fObjectRadius ) );
    V( m_pMesh->UnlockVertexBuffer() );

	return fObjectRadius;
}


//
// MeshNode::VRender				-  3rd Edition, Chapter 14, page 505
//
HRESULT D3DMeshNode9::VRender(Scene *pScene)
{
	if (S_OK != D3DSceneNode9::VRender(pScene) )
		return E_FAIL;

	return m_pMesh->DrawSubset(0); 
} 

//
// MeshNode::VOnRestore				-  3rd Edition, Chapter 14, page 506
//
// This function loads the Mesh and ensures the Mesh has normals; it also optimizes the 
// Mesh for the graphics card's vertex cache, which improves performance by organizing 
// the internal triangle list for less cache misses.
//
HRESULT D3DMeshNode9::VOnRestore(Scene *pScene)
{
	if (m_XFileName.empty())
	{
		SetRadius(CalcBoundingSphere());
		return D3DSceneNode9::VOnRestore(pScene);
	}

	// Change post press - release the Mesh only if we have a valid Mesh file name to load.
	// Otherwise we likely created it on our own, and needs to be kept.
	SAFE_RELEASE(m_pMesh);

    WCHAR str[MAX_PATH];
    HRESULT hr;

    // Load the Mesh with D3DX and get back a ID3DXMesh*.  For this
    // sample we'll ignore the X file's embedded materials since we know 
    // exactly the model we're loading.  See the Mesh samples such as
    // "OptimizedMesh" for a more generic Mesh loading example.
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, m_XFileName.c_str() ) );

    V_RETURN( D3DXLoadMeshFromX(str, D3DXMESH_MANAGED, DXUTGetD3D9Device(), NULL, NULL, NULL, NULL, &m_pMesh) );

    DWORD *rgdwAdjacency = NULL;

    // Make sure there are normals which are required for lighting
    if( !(m_pMesh->GetFVF() & D3DFVF_NORMAL) )
    {
        ID3DXMesh* pTempMesh;
        V( m_pMesh->CloneMeshFVF( m_pMesh->GetOptions(), 
                                  m_pMesh->GetFVF() | D3DFVF_NORMAL, 
                                  DXUTGetD3D9Device(), &pTempMesh ) );
        V( D3DXComputeNormals( pTempMesh, NULL ) );

        SAFE_RELEASE( m_pMesh );
        m_pMesh = pTempMesh;
    }

    // Optimize the Mesh for this graphics card's vertex cache 
    // so when rendering the Mesh's triangle list the vertices will 
    // cache hit more often so it won't have to re-execute the vertex shader 
    // on those vertices so it will improve perf.     

    rgdwAdjacency = GCC_NEW DWORD[m_pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    V( m_pMesh->ConvertPointRepsToAdjacency(NULL, rgdwAdjacency) );
    V( m_pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL) );
    
	SAFE_DELETE_ARRAY(rgdwAdjacency);

	SetRadius(CalcBoundingSphere());

    return D3DSceneNode9::VOnRestore(pScene);
}

HRESULT D3DMeshNode9::VPick(Scene *pScene, RayCast *pRayCast)
{
	if (SceneNode::VPick(pScene, pRayCast)==E_FAIL)
		return E_FAIL;

	pScene->PushAndSetMatrix(m_Props.ToWorld());
	HRESULT hr = pRayCast->Pick(pScene, m_Props.ActorId(), m_pMesh);
	pScene->PopMatrix();

	return hr;
}


////////////////////////////////////////////////////
// D3DShaderMeshNode9 Implementation
////////////////////////////////////////////////////

//
// ShaderMeshNode::ShaderMeshNode				- 3rd Edition, Chapter 14, page 517
//
D3DShaderMeshNode9::D3DShaderMeshNode9(const ActorId actorId, 
	WeakBaseRenderComponentPtr renderComponent,
	ID3DXMesh *Mesh, 
	std::string fxFileName,			// used to be effect pointer - changed to fix a VOnRestore crash.
	RenderPass renderPass, 
	const Mat4x4 *t)
 : D3DMeshNode9(actorId, renderComponent, Mesh, renderPass, t) 
{ 
	m_pEffect = NULL;
	m_fxFileName = fxFileName;
}

//
// ShaderMeshNode::ShaderMeshNode				- 3rd Edition, Chapter 14, page 517
//
D3DShaderMeshNode9::D3DShaderMeshNode9(const ActorId actorId, 
	WeakBaseRenderComponentPtr renderComponent,
	std::wstring xFileName, 
	std::string fxFileName,
	RenderPass renderPass, 
	const Mat4x4 *t)
 : D3DMeshNode9(actorId, renderComponent, xFileName, renderPass, t) 
{
	m_pEffect = NULL;
	m_fxFileName = fxFileName;
}

//
// ShaderMeshNode::VOnRestore					- very similar to MeshNode::VOnRestore
//
HRESULT D3DShaderMeshNode9::VOnRestore(Scene *pScene)
{
	SAFE_RELEASE(m_pEffect);

	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_DEBUG | D3DXSHADER_NO_PRESHADER;
	HRESULT hr;

    Resource resource(m_fxFileName);
    shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);  // this actually loads the XML file from the zip file
	V ( D3DXCreateEffect( DXUTGetD3D9Device(), pResourceHandle->Buffer(), pResourceHandle->Size(), NULL, NULL, dwShaderFlags, NULL, &m_pEffect, NULL ) );
	return D3DMeshNode9::VOnRestore(pScene);
}

//
// ShaderMeshNode::VRender						- 3rd Edition, Chapter 14, page 517				
//
HRESULT D3DShaderMeshNode9::VRender(Scene *pScene)
{
	if (S_OK != D3DSceneNode9::VRender(pScene) )
		return E_FAIL;

	HRESULT hr;

    // Update the effect's variables.  Instead of using strings, it would 
    // be more efficient to cache a handle to the parameter by calling 
    // ID3DXEffect::GetParameterByName

	Mat4x4 worldViewProj = pScene->GetCamera()->GetWorldViewProjection(pScene);
	Mat4x4 world = pScene->GetTopMatrix();

	D3DXCOLOR ambient =  m_Props.GetMaterial().GetAmbient();
    V_RETURN( m_pEffect->SetValue( "g_MaterialAmbientColor", &ambient, sizeof( D3DXCOLOR ) ) );
	D3DXCOLOR diffuse =  m_Props.GetMaterial().GetDiffuse();
    V_RETURN( m_pEffect->SetValue( "g_MaterialDiffuseColor", &diffuse, sizeof( D3DXCOLOR ) ) );

	V( m_pEffect->SetMatrix( "g_mWorldViewProjection", &worldViewProj ) );
    V( m_pEffect->SetMatrix( "g_mWorld", &world ) );
    V( m_pEffect->SetFloat( "g_fTime", ( float )1.0f ) );

	int count = pScene->GetLightManager()->GetLightCount(this);
	if (count)
	{
		// Light 0 is the only one we use for ambient lighting. The rest are ignored in this simple shader.
		V( m_pEffect->SetValue( "g_LightAmbient", pScene->GetLightManager()->GetLightAmbient(this), sizeof( D3DXVECTOR4 ) * 1 ) );
		V( m_pEffect->SetValue( "g_LightDir", pScene->GetLightManager()->GetLightDirection(this), sizeof( D3DXVECTOR4 ) * MAXIMUM_LIGHTS_SUPPORTED ) );
		V( m_pEffect->SetValue( "g_LightDiffuse", pScene->GetLightManager()->GetLightDiffuse(this), sizeof( D3DXVECTOR4 ) * MAXIMUM_LIGHTS_SUPPORTED ) );
		V( m_pEffect->SetInt( "g_nNumLights", count ) );
	}

	float alpha = m_Props.GetMaterial().GetAlpha();
	V( m_pEffect->SetFloat( "g_fAlpha", alpha  ) );	
	V( m_pEffect->SetTechnique( (alpha < 1.0f ) ? "RenderSceneWithAlpha" : "RenderScene" ) );

    // Apply the technique contained in the effect 
	UINT iPass, cPasses;

    V( m_pEffect->Begin( &cPasses, 0 ) );

    for( iPass = 0; iPass < cPasses; iPass++ )
    {
        V( m_pEffect->BeginPass( iPass ) );

        // The effect interface queues up the changes and performs them 
        // with the CommitChanges call. You do not need to call CommitChanges if 
        // you are not setting any parameters between the BeginPass and EndPass.
        // V( g_pEffect->CommitChanges() );

        // Render the Mesh with the applied technique
        V( m_pMesh->DrawSubset( 0 ) );

        V( m_pEffect->EndPass() );
    }
    V( m_pEffect->End() );

	return S_OK;
} 

HRESULT D3DShaderMeshNode9::VOnLostDevice(Scene *pScene)
{
	SAFE_RELEASE(m_pEffect);

	HRESULT hr;
	V_RETURN(D3DMeshNode9::VOnLostDevice(pScene));
	return S_OK;
}


//
// TeapotMeshNode::TeapotMeshNode					- Chapter X, page Y 
//
D3DTeapotMeshNode9::D3DTeapotMeshNode9(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, std::string fxFileName, RenderPass renderPass, const Mat4x4 *t)
: D3DShaderMeshNode9(actorId, renderComponent, NULL, fxFileName, renderPass, t) 
{ 
	// there's nothing else needed here...
}

//
// TeapotMeshNode::VOnRestore						- Chapter X, page Y
//
HRESULT D3DTeapotMeshNode9::VOnRestore(Scene *pScene)
{
	HRESULT hr;

	IDirect3DDevice9 * pDevice = DXUTGetD3D9Device();

	SAFE_RELEASE(m_pMesh);
	V( D3DXCreateTeapot( pDevice, &m_pMesh, NULL ) );

	//Rotate the teapot 90 degrees from default so that the spout faces forward
	Mat4x4 rotateY90;
	rotateY90.BuildRotationY(-GCC_PI / 2.0f);
	IDirect3DVertexBuffer9* pVB = NULL;
	m_pMesh->GetVertexBuffer(&pVB);
	Vec3* pVertices = NULL;
	pVB->Lock( 0, 0, (void**)&pVertices, 0 );
	for (unsigned int i=0; i<m_pMesh->GetNumVertices(); ++i)
	{
		*pVertices = rotateY90.Xform(*pVertices);
		++pVertices;
		//The structs depicted in this vertex buffer actually store
		//information for normals in addition to xyz, thereby
		//making the vertices in pVB twice the size of the one described
		//by *pVertices.  So we address that here.
		*pVertices = rotateY90.Xform(*pVertices);	//rotate the normals, too
		++pVertices;
	}
	pVB->Unlock();
	SAFE_RELEASE( pVB );
	//...end rotation



	// Note - the Mesh is needed BEFORE calling the base class VOnRestore.
	V ( D3DShaderMeshNode9::VOnRestore ( pScene ) );
	return S_OK;
}




//
// D3DShaderMeshNode11::D3DShaderMeshNode11					- Chapter 16, page 562 
//
D3DShaderMeshNode11::D3DShaderMeshNode11(const ActorId actorId, 
	WeakBaseRenderComponentPtr renderComponent,
	std::string sdkMeshFileName, 
	RenderPass renderPass, 
	const Mat4x4 *t)
: SceneNode(actorId, renderComponent, renderPass, t)
{
	m_sdkMeshFileName = sdkMeshFileName;
}


//
// D3DShaderMeshNode11::VOnRestore							- Chapter 16, page 563
//
HRESULT D3DShaderMeshNode11::VOnRestore(Scene *pScene)
{
	HRESULT hr;

	V_RETURN(SceneNode::VOnRestore(pScene) );

	V_RETURN (m_VertexShader.OnRestore(pScene) );
	V_RETURN (m_PixelShader.OnRestore(pScene) );

    // Force the Mesh to reload
	Resource resource(m_sdkMeshFileName);
    shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);  	
	shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	SetRadius(CalcBoundingSphere(&extra->m_Mesh11));

	return S_OK;
}

//
// D3DShaderMeshNode11::VRender								- Chapter 16, page 564
//
HRESULT D3DShaderMeshNode11::VRender(Scene *pScene)
{
	HRESULT hr;

	V_RETURN (m_VertexShader.SetupRender(pScene, this) );
	V_RETURN (m_PixelShader.SetupRender(pScene, this) );

    //Get the Mesh
	Resource resource(m_sdkMeshFileName);
    shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);  	
	shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	// FUTURE WORK - this code WON'T be able to find texture resources referred to by the sdkmesh file 
    // in the Resource cache.

    //IA setup
    UINT Strides[1];
    UINT Offsets[1];
    ID3D11Buffer* pVB[1];
    pVB[0] = extra->m_Mesh11.GetVB11( 0, 0 );
    Strides[0] = ( UINT )extra->m_Mesh11.GetVertexStride( 0, 0 );
    Offsets[0] = 0;
    DXUTGetD3D11DeviceContext()->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
    DXUTGetD3D11DeviceContext()->IASetIndexBuffer( extra->m_Mesh11.GetIB11( 0 ), extra->m_Mesh11.GetIBFormat11( 0 ), 0 );

    //Render
    D3D11_PRIMITIVE_TOPOLOGY PrimType;
    for( UINT subset = 0; subset < extra->m_Mesh11.GetNumSubsets( 0 ); ++subset )
    {
        // Get the subset
        SDKMESH_SUBSET *pSubset = extra->m_Mesh11.GetSubset( 0, subset );

        PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
        DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology( PrimType );
 
        ID3D11ShaderResourceView* pDiffuseRV = extra->m_Mesh11.GetMaterial( pSubset->MaterialID )->pDiffuseRV11;
        DXUTGetD3D11DeviceContext()->PSSetShaderResources( 0, 1, &pDiffuseRV );

        DXUTGetD3D11DeviceContext()->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
    }
	return S_OK;
}

HRESULT D3DShaderMeshNode11::VPick(Scene *pScene, RayCast *pRayCast)
{
	if (SceneNode::VPick(pScene, pRayCast)==E_FAIL)
		return E_FAIL;

	pScene->PushAndSetMatrix(m_Props.ToWorld());

    //Get the Mesh
	Resource resource(m_sdkMeshFileName);
    shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);  	
	shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	HRESULT hr = pRayCast->Pick(pScene, m_Props.ActorId(), &extra->m_Mesh11);
	pScene->PopMatrix();

	return hr;
}




float D3DShaderMeshNode11::CalcBoundingSphere(CDXUTSDKMesh *mesh11)
{
	float radius = 0.0f;
    for( UINT subset = 0; subset < mesh11->GetNumSubsets( 0 ); ++subset )
    {
		Vec3 extents = mesh11->GetMeshBBoxExtents(subset);
		extents.x = abs(extents.x);
		extents.y = abs(extents.y);
		extents.z = abs(extents.z);
		radius = (radius > extents.x) ? radius : extents.x;
		radius = (radius > extents.y) ? radius : extents.y;
		radius = (radius > extents.z) ? radius : extents.z;
	}
	return radius;
}
