//========================================================================
// File: Sky.cpp - implements a sky box in D3D9 or D3D11
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

#include "D3DRenderer.h"
#include "Geometry.h"
#include "SceneNodes.h"
#include "Shaders.h"
#include "Sky.h"

////////////////////////////////////////////////////
// SkyNode Implementation
////////////////////////////////////////////////////

//
// SkyNode::SkyNode						- Chapter 16, page 554
//
SkyNode::SkyNode(const char *pTextureBaseName)
: SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Sky, &Mat4x4::g_Identity)
, m_bActive(true)
{
	m_textureBaseName = pTextureBaseName;
}

//
// SkyNode::VPreRender				- Chapter 14, page 502
//
HRESULT SkyNode::VPreRender(Scene *pScene)
{
	Vec3 cameraPos = m_camera->VGet()->ToWorld().GetPosition();
	Mat4x4 mat = m_Props.ToWorld();
	mat.SetPosition(cameraPos);
	VSetTransform(&mat);

	return SceneNode::VPreRender(pScene);
}



//
// D3DSkyNode9::D3DSkyNode9				- 3rd Edition, Chapter 14, page 499
//
D3DSkyNode9::D3DSkyNode9(const char *pTextureBaseName)
: SkyNode(pTextureBaseName)
{
	for (int i=0; i<5; ++i)
	{
		m_pTexture[i] = NULL;
	}
	m_pVerts = NULL;
}


//
// D3DSkyNode9::~D3DSkyNode9			- 3rd Edition, Chapter 14, page 499
//
D3DSkyNode9::~D3DSkyNode9()
{
	for (int i=0; i<5; ++i)
	{
		SAFE_RELEASE(m_pTexture[i]);
	}
	SAFE_RELEASE(m_pVerts);
}

//
// D3DSkyNode9::VOnRestore				- 3rd Edition, Chapter 14, page 500
//
HRESULT D3DSkyNode9::VOnRestore(Scene *pScene)
{

	// Call the base class's restore
	SceneNode::VOnRestore(pScene);

	m_camera = pScene->GetCamera();					// added post press!

	m_numVerts = 20;

	SAFE_RELEASE(m_pVerts);
    if( FAILED( DXUTGetD3D9Device()->CreateVertexBuffer( 
		m_numVerts*sizeof(D3D9Vertex_ColoredTextured),
		D3DUSAGE_WRITEONLY, D3D9Vertex_ColoredTextured::FVF,
        D3DPOOL_MANAGED, &m_pVerts, NULL ) ) )
    {
        return E_FAIL;
    }

    // Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    D3D9Vertex_ColoredTextured* pVertices;
    if( FAILED( m_pVerts->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
        return E_FAIL;

	// Loop through the grid squares and calc the values
	// of each index. Each grid square has two triangles:
	//
	//		A - B
	//		| / |
	//		C - D

	D3D9Vertex_ColoredTextured skyVerts[4];
	D3DCOLOR skyVertColor = 0xffffffff;
	float dim = 50.0f;

	skyVerts[0].position = Vec3( dim, dim, dim ); skyVerts[0].color=skyVertColor; skyVerts[0].tu=1; skyVerts[0].tv=0; 
	skyVerts[1].position = Vec3(-dim, dim, dim ); skyVerts[1].color=skyVertColor; skyVerts[1].tu=0; skyVerts[1].tv=0; 
	skyVerts[2].position = Vec3( dim,-dim, dim ); skyVerts[2].color=skyVertColor; skyVerts[2].tu=1; skyVerts[2].tv=1; 
	skyVerts[3].position = Vec3(-dim,-dim, dim ); skyVerts[3].color=skyVertColor; skyVerts[3].tu=0; skyVerts[3].tv=1; 

	Vec3 triangle[3];
	triangle[0] = Vec3(0.f,0.f,0.f);
	triangle[1] = Vec3(5.f,0.f,0.f);
	triangle[2] = Vec3(5.f,5.f,0.f);

	Vec3 edge1 = triangle[1]-triangle[0];
	Vec3 edge2 = triangle[2]-triangle[0];

	Vec3 normal;
	normal = edge1.Cross(edge2);
	normal.Normalize();

	Mat4x4 rotY;
	rotY.BuildRotationY(GCC_PI/2.0f);
	Mat4x4 rotX;
	rotX.BuildRotationX(-GCC_PI/2.0f);
	
	m_sides = 5;

	for (DWORD side = 0; side < m_sides; side++)
	{
		for (DWORD v = 0; v < 4; v++)
		{
			Vec4 temp;
			if (side < m_sides-1)
			{
				temp = rotY.Xform(Vec3(skyVerts[v].position));
			}
			else
			{
				skyVerts[0].tu=1; skyVerts[0].tv=1; 
				skyVerts[1].tu=1; skyVerts[1].tv=0; 
				skyVerts[2].tu=0; skyVerts[2].tv=1; 
				skyVerts[3].tu=0; skyVerts[3].tv=0; 

				temp = rotX.Xform(Vec3(skyVerts[v].position));
			}
			skyVerts[v].position = Vec3(temp.x, temp.y, temp.z);
		}
		memcpy(&pVertices[side*4], skyVerts, sizeof(skyVerts));
	}

	m_pVerts->Unlock();
	return S_OK;
}


//
// D3DSkyNode9::VRender				- Chapter 14, page 502
//
HRESULT D3DSkyNode9::VRender(Scene *pScene)
{

	// Setup our texture. Using textures introduces the texture stage states,
    // which govern how textures get blended together (in the case of multiple
    // textures) and lighting information. In this case, we are modulating
    // (blending) our texture with the diffuse color of the vertices.

    DXUTGetD3D9Device()->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    DXUTGetD3D9Device()->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    DXUTGetD3D9Device()->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	DXUTGetD3D9Device()->SetStreamSource( 0, m_pVerts, 0, sizeof(D3D9Vertex_ColoredTextured) );
	DXUTGetD3D9Device()->SetFVF( D3D9Vertex_ColoredTextured::FVF );

	for (DWORD side = 0; side < m_sides; side++)
	{
		// FUTURE WORK: A good optimization would be to transform the camera's
		// world look vector into local space, and do a dot product. If the
		// result is positive, we shouldn't draw the side since it has to be
		// behind the camera!

		// Sky boxes aren't culled by the normal mechanism

		std::string name = GetTextureName(side);

		/***
		// [mrmike] - This was slightly changed post press, look at the lines below this commented out code
		const char *suffix[] = { "_n.jpg", "_e.jpg",  "_s.jpg",  "_w.jpg",  "_u.jpg" };
		name += suffix[side];
		****/
		
		Resource resource(name);
		shared_ptr<ResHandle> texture = g_pApp->m_ResCache->GetHandle(&resource);
		shared_ptr<D3DTextureResourceExtraData9> extra = static_pointer_cast<D3DTextureResourceExtraData9>(texture->GetExtra());

		DXUTGetD3D9Device()->SetTexture( 0, extra->GetTexture() );
		DXUTGetD3D9Device()->DrawPrimitive( D3DPT_TRIANGLESTRIP , 4 * side, 2);
	}

	DXUTGetD3D9Device()->SetTexture (0, NULL);
	return S_OK;
}


//
// D3DSkyNode11::D3DSkyNode11					- Chapter 16, page 555
//
D3DSkyNode11::D3DSkyNode11(const char *pTextureBaseName) 
	: SkyNode(pTextureBaseName)
{
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_VertexShader.EnableLights(false);
}

//
// D3DSkyNode11::~D3DSkyNode11					- Chapter 16, page 555
//
D3DSkyNode11::~D3DSkyNode11()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}


//
// D3DSkyNode11::VOnRestore						- Chapter 16, page 556
//
HRESULT D3DSkyNode11::VOnRestore(Scene *pScene)
{
	HRESULT hr;

	V_RETURN(SceneNode::VOnRestore(pScene) );

	m_camera = pScene->GetCamera();

	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);

	V_RETURN (m_VertexShader.OnRestore(pScene) );
	V_RETURN (m_PixelShader.OnRestore(pScene) );

	m_numVerts = 20;

    // Fill the vertex buffer. We are setting the tu and tv texture
    // coordinates, which range from 0.0 to 1.0
    D3D11Vertex_UnlitTextured *pVertices = GCC_NEW D3D11Vertex_UnlitTextured[m_numVerts];
	GCC_ASSERT(pVertices && "Out of memory in D3DSkyNode11::VOnRestore()");
	if (!pVertices)
		return E_FAIL;

	// Loop through the grid squares and calc the values
	// of each index. Each grid square has two triangles:
	//
	//		A - B
	//		| / |
	//		C - D

	D3D11Vertex_UnlitTextured skyVerts[4];
	D3DCOLOR skyVertColor = 0xffffffff;
	float dim = 50.0f;

	skyVerts[0].Pos = Vec3( dim, dim, dim ); skyVerts[0].Uv = Vec2(1.0f, 0.0f); 
	skyVerts[1].Pos = Vec3(-dim, dim, dim ); skyVerts[1].Uv = Vec2(0.0f, 0.0f);
	skyVerts[2].Pos = Vec3( dim,-dim, dim ); skyVerts[2].Uv = Vec2(1.0f, 1.0f);  
	skyVerts[3].Pos = Vec3(-dim,-dim, dim ); skyVerts[3].Uv = Vec2(0.0f, 1.0f);

	Vec3 triangle[3];
	triangle[0] = Vec3(0.f,0.f,0.f);
	triangle[1] = Vec3(5.f,0.f,0.f);
	triangle[2] = Vec3(5.f,5.f,0.f);

	Vec3 edge1 = triangle[1]-triangle[0];
	Vec3 edge2 = triangle[2]-triangle[0];

	Vec3 normal;
	normal = edge1.Cross(edge2);
	normal.Normalize();

	Mat4x4 rotY;
	rotY.BuildRotationY(GCC_PI/2.0f);
	Mat4x4 rotX;
	rotX.BuildRotationX(-GCC_PI/2.0f);
	
	m_sides = 5;

	for (DWORD side = 0; side < m_sides; side++)
	{
		for (DWORD v = 0; v < 4; v++)
		{
			Vec4 temp;
			if (side < m_sides-1)
			{
				temp = rotY.Xform(Vec3(skyVerts[v].Pos));
			}
			else
			{
				skyVerts[0].Uv = Vec2(1.0f, 1.0f); 
				skyVerts[1].Uv = Vec2(1.0f, 1.0f);
				skyVerts[2].Uv = Vec2(1.0f, 1.0f); 
				skyVerts[3].Uv = Vec2(1.0f, 1.0f);

				temp = rotX.Xform(Vec3(skyVerts[v].Pos));
			}
			skyVerts[v].Pos = Vec3(temp.x, temp.y, temp.z);
		}
		memcpy(&pVertices[side*4], skyVerts, sizeof(skyVerts));
	}

    D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( D3D11Vertex_UnlitTextured ) * m_numVerts;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = pVertices;
    hr = DXUTGetD3D11Device()->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
	SAFE_DELETE(pVertices);
    if( FAILED( hr ) )
        return hr;


	// Loop through the grid squares and calc the values
	// of each index. Each grid square has two triangles:
	//
	//		A - B
	//		| / |
	//		C - D

	WORD *pIndices = GCC_NEW WORD[m_sides * 2 * 3];

	WORD *current = pIndices;
	for (DWORD i=0; i<m_sides; i++)
	{
		// Triangle #1  ACB
		*(current) = WORD(i*4);
		*(current+1) = WORD(i*4 + 2);
		*(current+2) = WORD(i*4 + 1);

		// Triangle #2  BCD
		*(current+3) = WORD(i*4 + 1);
		*(current+4) = WORD(i*4 + 2);
		*(current+5) = WORD(i*4 + 3);
		current+=6;
	}
	
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * m_sides * 2 * 3;        // each side has 2 triangles
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
    InitData.pSysMem = pIndices;
    hr = DXUTGetD3D11Device()->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );
	SAFE_DELETE_ARRAY(pIndices);
    if( FAILED( hr ) )
        return hr;


	return S_OK;
}

//
// D3DSkyNode11::VRender						- Chapter 16, page 559
//
HRESULT D3DSkyNode11::VRender(Scene *pScene)
{
	HRESULT hr;

	V_RETURN (m_VertexShader.SetupRender(pScene, this) );
	V_RETURN (m_PixelShader.SetupRender(pScene, this) );

    // Set vertex buffer
    UINT stride = sizeof( D3D11Vertex_UnlitTextured );
    UINT offset = 0;
    DXUTGetD3D11DeviceContext()->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );

	// Set index buffer
    DXUTGetD3D11DeviceContext()->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

    // Set primitive topology
    DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	for (DWORD side = 0; side < m_sides; side++)
	{
		// FUTURTE WORK: A good optimization would be to transform the camera's
		// world look vector into local space, and do a dot product. If the
		// result is positive, we shouldn't draw the side since it has to be
		// behind the camera!

		// Sky boxes aren't culled by the normal mechanism

		/***
		// [mrmike] - This was slightly changed post press, look at the lines below this commented out code
		const char *suffix[] = { "_n.jpg", "_e.jpg",  "_s.jpg",  "_w.jpg",  "_u.jpg" };
		name += suffix[side];
		****/

		std::string name = GetTextureName(side);
		m_PixelShader.SetTexture(name.c_str());

		DXUTGetD3D11DeviceContext()->DrawIndexed( 6, side * 6, 0 );
	}
	return S_OK;
}


std::string SkyNode::GetTextureName(const int side)
{
	std::string name = m_textureBaseName;
	char *letters[] = { "n", "e", "s", "w", "u" };
	unsigned int index = name.find_first_of("_");
	GCC_ASSERT(index >= 0 && index < name.length()-1);
	if (index >= 0 && index < name.length()-1)
	{
		name[index+1] = *letters[side];
	}
	return name;
}



