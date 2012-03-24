//========================================================================
// Raycast.cpp - implements a raycast into the rendered scene
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

#include "Geometry.h"
#include "Raycast.h"
#include "SceneNodes.h"

template <class T>
void InitIntersection(Intersection &intersection, DWORD faceIndex, FLOAT dist, FLOAT u, FLOAT v, ActorId actorId, WORD* pIndices, T* pVertices, const Mat4x4 &matWorld)
{
	intersection.m_dwFace = faceIndex;
	intersection.m_fDist = dist;
	intersection.m_fBary1 = u;
    intersection.m_fBary2 = v;

	T *v0 = &pVertices[pIndices[3 * faceIndex + 0]];
	T *v1 = &pVertices[pIndices[3 * faceIndex + 1]];
	T *v2 = &pVertices[pIndices[3 * faceIndex + 2]];

    // If all you want is the vertices hit, then you are done.  In this sample, we
    // want to show how to infer texture coordinates as well, using the BaryCentric
    // coordinates supplied by D3DXIntersect
    FLOAT dtu1 = v1->tu - v0->tu;
    FLOAT dtu2 = v2->tu - v0->tu;
    FLOAT dtv1 = v1->tv - v0->tv;
    FLOAT dtv2 = v2->tv - v0->tv;
    intersection.m_tu = v0->tu + intersection.m_fBary1 * dtu1 + intersection.m_fBary2 * dtu2;
    intersection.m_tv = v0->tv + intersection.m_fBary1 * dtv1 + intersection.m_fBary2 * dtv2;

	Vec3 a = v0->position - v1->position;
	Vec3 b = v2->position - v1->position;

	Vec3 cross = a.Cross(b);
	cross /= cross.Length();

	Vec3 actorLoc = BarycentricToVec3(v0->position, v1->position, v2->position, intersection.m_fBary1, intersection.m_fBary2);
	intersection.m_actorLoc = actorLoc;
	intersection.m_worldLoc = matWorld.Xform(actorLoc);
	intersection.m_actorId = actorId;
	intersection.m_normal = cross;
}



RayCast::RayCast(Point point, DWORD maxIntersections)
{
	m_MaxIntersections = maxIntersections;
	m_IntersectionArray.reserve(m_MaxIntersections);
	m_bUseD3DXIntersect = true;
	m_bAllHits = true;
	m_NumIntersections = 0;
	m_Point = point;
}

HRESULT RayCast::Pick(Scene *pScene, ActorId actorId, ID3DXMesh *pMesh)
{
	if (!m_bAllHits && m_NumIntersections > 0)
		return S_OK;
	
	HRESULT hr;

    IDirect3DDevice9* pD3Device = DXUTGetD3D9Device();

	// Get the inverse view matrix
    const Mat4x4 matView = pScene->GetCamera()->GetView();
    const Mat4x4 matWorld = pScene->GetTopMatrix();
	const Mat4x4 proj = pScene->GetCamera()->GetProjection();

    // Compute the vector of the Pick ray in screen space
    D3DXVECTOR3 v;
    v.x = ( ( ( 2.0f * m_Point.x ) / g_pApp->GetScreenSize().x ) - 1 ) / proj._11;
    v.y = -( ( ( 2.0f * m_Point.y ) / g_pApp->GetScreenSize().y ) - 1 ) / proj._22;
    v.z = 1.0f;

	
    D3DXMATRIX mWorldView = matWorld * matView;
    D3DXMATRIX m;
    D3DXMatrixInverse( &m, NULL, &mWorldView );

    // Transform the screen space Pick ray into 3D space
    m_vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
    m_vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
    m_vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
    m_vPickRayOrig.x = m._41;
    m_vPickRayOrig.y = m._42;
    m_vPickRayOrig.z = m._43;

	ID3DXMesh* pTempMesh;
    V( pMesh->CloneMeshFVF( pMesh->GetOptions(), D3D9Vertex_UnlitTextured::FVF,
                                  DXUTGetD3D9Device(), &pTempMesh ) );

    LPDIRECT3DVERTEXBUFFER9 pVB;
    LPDIRECT3DINDEXBUFFER9 pIB;

    pTempMesh->GetVertexBuffer( &pVB );
    pTempMesh->GetIndexBuffer( &pIB );

	WORD* pIndices;
	D3D9Vertex_UnlitTextured* pVertices;

	pIB->Lock( 0, 0, ( void** )&pIndices, 0 );
	pVB->Lock( 0, 0, ( void** )&pVertices, 0 );
	
	DWORD intersections = 0;

    // When calling D3DXIntersect, one can get just the closest intersection and not
    // need to work with a D3DXBUFFER.  Or, to get all intersections between the ray and 
    // the Mesh, one can use a D3DXBUFFER to receive all intersections.  We show both
    // methods.
    if( !m_bAllHits )
    {
        // Collect only the closest intersection
        BOOL bHit;
        DWORD dwFace;
        FLOAT fBary1, fBary2, fDist;
        D3DXIntersect( pTempMesh, &m_vPickRayOrig, &m_vPickRayDir, &bHit, &dwFace, &fBary1, &fBary2, &fDist,
                       NULL, NULL );
        if( bHit )
        {
            m_NumIntersections = 1;
			m_IntersectionArray.resize(1);
			InitIntersection(m_IntersectionArray[0], dwFace, fDist, fBary1, fBary2, actorId, pIndices, pVertices, matWorld);
		}
        else
        {
            m_NumIntersections = 0;
        }
    }
    else
    {
        // Collect all intersections
        BOOL bHit;
        LPD3DXBUFFER pBuffer = NULL;
        D3DXINTERSECTINFO* pIntersectInfoArray;
        if( FAILED( hr = D3DXIntersect( pTempMesh, &m_vPickRayOrig, &m_vPickRayDir, &bHit, NULL, NULL, NULL, NULL,
                                        &pBuffer, &intersections ) ) )
        {
            SAFE_RELEASE( pTempMesh );
            SAFE_RELEASE( pVB );
            SAFE_RELEASE( pIB );

            return hr;
        }
        if( intersections > 0 )
        {
			m_IntersectionArray.resize(m_NumIntersections + intersections);

            pIntersectInfoArray = ( D3DXINTERSECTINFO* )pBuffer->GetBufferPointer();
            if( m_NumIntersections > m_MaxIntersections )
                m_NumIntersections = m_MaxIntersections;

            for( DWORD i = 0; i < intersections; i++ )
            {
				Intersection* pIntersection;
				pIntersection = &m_IntersectionArray[i + m_NumIntersections];

				InitIntersection(*pIntersection, pIntersectInfoArray[i].FaceIndex,
					pIntersectInfoArray[i].Dist,
					pIntersectInfoArray[i].U,
					pIntersectInfoArray[i].V,
					actorId, pIndices, pVertices, matWorld);
            }
        }

        SAFE_RELEASE( pBuffer );
    }

	m_NumIntersections += intersections;

    pVB->Unlock();
    pIB->Unlock();

    SAFE_RELEASE( pTempMesh );
    SAFE_RELEASE( pVB );
    SAFE_RELEASE( pIB );

	return S_OK;
}


HRESULT RayCast::Pick(Scene *pScene, ActorId actorId, CDXUTSDKMesh *pMesh)
{
	if (!m_bAllHits && m_NumIntersections > 0)
		return S_OK;
	
    IDirect3DDevice9* pD3Device = DXUTGetD3D9Device();

	// Get the inverse view matrix
    const Mat4x4 matView = pScene->GetCamera()->GetView();
    const Mat4x4 matWorld = pScene->GetTopMatrix();
	const Mat4x4 proj = pScene->GetCamera()->GetProjection();

    // Compute the vector of the Pick ray in screen space
    D3DXVECTOR3 v;
    v.x = ( ( ( 2.0f * m_Point.x ) / g_pApp->GetScreenSize().x ) - 1 ) / proj._11;
    v.y = -( ( ( 2.0f * m_Point.y ) / g_pApp->GetScreenSize().y ) - 1 ) / proj._22;
    v.z = 1.0f;

	
    D3DXMATRIX mWorldView = matWorld * matView;
    D3DXMATRIX m;
    D3DXMatrixInverse( &m, NULL, &mWorldView );

    // Transform the screen space Pick ray into 3D space
    m_vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
    m_vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
    m_vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
    m_vPickRayOrig.x = m._41;
    m_vPickRayOrig.y = m._42;
    m_vPickRayOrig.z = m._43;

	return E_FAIL;
}



HRESULT RayCast::Pick(Scene *pScene, ActorId actorId, LPDIRECT3DVERTEXBUFFER9 pVB, LPDIRECT3DINDEXBUFFER9 pIB, DWORD numPolys)
{
	if (!m_bAllHits && m_NumIntersections > 0)
		return S_OK;

	WORD* pIndices;
	D3D9Vertex_ColoredTextured* pVertices;

	pIB->Lock( 0, 0, ( void** )&pIndices, 0 );
	pVB->Lock( 0, 0, ( void** )&pVertices, 0 );

    IDirect3DDevice9* pD3Device = DXUTGetD3D9Device();

	// Get the inverse view matrix
    const Mat4x4 matView = pScene->GetCamera()->GetView();
    const Mat4x4 matWorld = pScene->GetTopMatrix();
	const Mat4x4 proj = pScene->GetCamera()->GetProjection();

    // Compute the vector of the Pick ray in screen space
    D3DXVECTOR3 v;
    v.x = ( ( ( 2.0f * m_Point.x ) / g_pApp->GetScreenSize().x ) - 1 ) / proj._11;
    v.y = -( ( ( 2.0f * m_Point.y ) / g_pApp->GetScreenSize().y ) - 1 ) / proj._22;
    v.z = 1.0f;

	
    D3DXMATRIX mWorldView = matWorld * matView;
    D3DXMATRIX m;
    D3DXMatrixInverse( &m, NULL, &mWorldView );

    // Transform the screen space Pick ray into 3D space
    m_vPickRayDir.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
    m_vPickRayDir.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
    m_vPickRayDir.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
    m_vPickRayOrig.x = m._41;
    m_vPickRayOrig.y = m._42;
    m_vPickRayOrig.z = m._43;

    FLOAT fBary1, fBary2;
    FLOAT fDist;
    for( DWORD i = 0; i < numPolys; i++ )
    {
        Vec3 v0 = pVertices[pIndices[3 * i + 0]].position;
        Vec3 v1 = pVertices[pIndices[3 * i + 1]].position;
        Vec3 v2 = pVertices[pIndices[3 * i + 2]].position;

        // Check if the Pick ray passes through this point
        if( IntersectTriangle( m_vPickRayOrig, m_vPickRayDir, v0, v1, v2,
                               &fDist, &fBary1, &fBary2 ) )
        {
            if( m_bAllHits || m_NumIntersections == 0 || fDist < m_IntersectionArray[0].m_fDist )
            {

                if( !m_bAllHits )
                    m_NumIntersections = 0;

				++m_NumIntersections;

				m_IntersectionArray.resize(m_NumIntersections);

				Intersection* pIntersection;
				pIntersection = &m_IntersectionArray[m_NumIntersections-1];

				InitIntersection(*pIntersection, i, fDist, fBary1, fBary2, actorId, pIndices, pVertices, matWorld);

                if( m_NumIntersections == m_MaxIntersections )
                    break;
            }
        }
    }

	pVB->Unlock();
    pIB->Unlock();

	return S_OK;
}




void RayCast::Sort() 
{ 
	std::sort(m_IntersectionArray.begin(), m_IntersectionArray.end()) ; 
}



