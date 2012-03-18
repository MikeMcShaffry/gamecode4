//========================================================================
// Geometry.cpp : Collection of code for 3D math and 3D shapes
//
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
#include "Geometry.h"

//========================================================================
// 
// Note - FVFs are part of the fixed function pipeline, and were described in
//        Game Coding Complete, 3rd Edition. 
//
// Our custom FVF, which describes our custom vertex structure
// These were #define'd in the book - now they are static constants.
//
const DWORD D3D9Vertex_UnlitColored::FVF =
	(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_SPECULAR);		
const DWORD D3D9Vertex_ColoredTextured::FVF = (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1);
const DWORD D3D9Vertex_Colored::FVF = (D3DFVF_XYZ|D3DFVF_DIFFUSE);
const DWORD D3D9Vertex_UnlitTextured::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1;

const Mat4x4 Mat4x4::g_Identity(D3DXMATRIX(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1));
const Quaternion Quaternion::g_Identity(D3DXQUATERNION(0,0,0,1));

bool Plane::Inside(const Vec3 &point) const
{
	// Inside the plane is defined as the direction the normal is facing
	float result = D3DXPlaneDotCoord(this, &point);
	return (result >= 0.0f);
}

bool Plane::Inside(const Vec3 &point, const float radius) const
{
	float fDistance;	// calculate our distances to each of the planes

	// find the distance to this plane
	fDistance = D3DXPlaneDotCoord(this, &point);		

	// if this distance is < -radius, we are outside
	return (fDistance >= -radius);
}

//
// Frustum::Frustum					- Chapter 14, page 477
//
Frustum::Frustum()
{
	m_Fov = GCC_PI/4.0f;		// default field of view is 90 degrees
	m_Aspect = 1.0f;			// default aspect ratio is 1:1
	m_Near = 1.0f;				// default near clip plane is 1m away from the camera
	m_Far = 1000.0f;				// default near clip plane is 100m away from the camera
}


//
// Frustum::Inside					- Chapter 14, page 477
//
bool Frustum::Inside(const Vec3 &point) const
{
	for (int i=0; i<NumPlanes; ++i)
	{
		if (!m_Planes[i].Inside(point))
			return false;
	}

	return true;
}


//
// Frustum::Inside					- Chapter 14, page 477
//
bool Frustum::Inside(const Vec3 &point, const float radius) const
{
	for(int i = 0; i < NumPlanes; ++i) 
	{	
		if (!m_Planes[i].Inside(point, radius))
			return false;
	}	
	
	// otherwise we are fully in view
	return(true);
}

//
// Frustum::Init					- Chapter 14, page 477
//
void Frustum::Init(const float fov, const float aspect, const float nearClip, const float farClip)
{
	m_Fov = fov;
	m_Aspect = aspect;
	m_Near = nearClip;
	m_Far = farClip;

	float tanFovOver2 = (float)tan(m_Fov/2.0f);
	Vec3 nearRight = (m_Near * tanFovOver2) * m_Aspect * g_Right;
	Vec3 farRight = (m_Far * tanFovOver2) * m_Aspect * g_Right;
	Vec3 nearUp = (m_Near * tanFovOver2 ) * g_Up;
	Vec3 farUp = (m_Far * tanFovOver2)  * g_Up;

	// points start in the upper right and go around clockwise
	m_NearClip[0] = (m_Near * g_Forward) - nearRight + nearUp;
	m_NearClip[1] = (m_Near * g_Forward) + nearRight + nearUp;
	m_NearClip[2] = (m_Near * g_Forward) + nearRight - nearUp;
	m_NearClip[3] = (m_Near * g_Forward) - nearRight - nearUp;

	m_FarClip[0] = (m_Far * g_Forward) - farRight + farUp;
	m_FarClip[1] = (m_Far * g_Forward) + farRight + farUp;
	m_FarClip[2] = (m_Far * g_Forward) + farRight - farUp;
	m_FarClip[3] = (m_Far * g_Forward) - farRight - farUp;

	// now we have all eight points. Time to construct 6 planes.
	// the normals point away from you if you use counter clockwise verts.

	Vec3 origin(0.0f, 0.0f, 0.0f);
	m_Planes[Near].Init(m_NearClip[2], m_NearClip[1], m_NearClip[0]);
	m_Planes[Far].Init(m_FarClip[0], m_FarClip[1], m_FarClip[2]);
	m_Planes[Right].Init(m_FarClip[2], m_FarClip[1], origin);
	m_Planes[Top].Init(m_FarClip[1], m_FarClip[0], origin);
	m_Planes[Left].Init(m_FarClip[0], m_FarClip[3], origin);
	m_Planes[Bottom].Init(m_FarClip[3], m_FarClip[2], origin);
}



//
// Frustum::Render					- not described in the 4th edition
//
void Frustum::Render()
{
	D3D9Vertex_Colored verts[24];
	for (int i=0; i<8; ++i)
	{
		verts[i].color = g_White;
	}

	for (int i=0; i<8; ++i)
	{
		verts[i+8].color = g_Red;
	}

	for (int i=0; i<8; ++i)
	{
		verts[i+16].color = g_Blue;
	}


	// Draw the near clip plane
	verts[0].position = m_NearClip[0];	verts[1].position = m_NearClip[1];
	verts[2].position = m_NearClip[1];	verts[3].position = m_NearClip[2];
	verts[4].position = m_NearClip[2];	verts[5].position = m_NearClip[3];
	verts[6].position = m_NearClip[3];	verts[7].position = m_NearClip[0];

	// Draw the far clip plane
	verts[8].position = m_FarClip[0];	verts[9].position = m_FarClip[1];
	verts[10].position = m_FarClip[1];	verts[11].position = m_FarClip[2];
	verts[12].position = m_FarClip[2];	verts[13].position = m_FarClip[3];
	verts[14].position = m_FarClip[3];	verts[15].position = m_FarClip[0];

	// Draw the edges between the near and far clip plane
	verts[16].position = m_NearClip[0];	verts[17].position = m_FarClip[0];
	verts[18].position = m_NearClip[1];	verts[19].position = m_FarClip[1];
	verts[20].position = m_NearClip[2];	verts[21].position = m_FarClip[2];
	verts[22].position = m_NearClip[3];	verts[23].position = m_FarClip[3];

	DWORD oldLightMode;
	DXUTGetD3D9Device()->GetRenderState( D3DRS_LIGHTING, &oldLightMode );
	DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, FALSE );

    DXUTGetD3D9Device()->SetFVF( D3D9Vertex_Colored::FVF );
	DXUTGetD3D9Device()->DrawPrimitiveUP( D3DPT_LINELIST, 12, verts, sizeof(D3D9Vertex_Colored) );

	DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, oldLightMode );
}


Vec3 BarycentricToVec3(Vec3 v0, Vec3 v1, Vec3 v2, float u, float v)
{
	//V1 + U(V2 - V1) + V(V3 - V1).
	Vec3 result = v0 + u * (v1  - v0) + v * (v2 - v0);
	return result;
}


//--------------------------------------------------------------------------------------
// Given a ray origin (orig) and direction (dir), and three vertices of a triangle, this
// function returns TRUE and the interpolated texture coordinates if the ray intersects 
// the triangle
//--------------------------------------------------------------------------------------
bool IntersectTriangle( const Vec3& orig, const Vec3& dir,
                        Vec3& v0, Vec3& v1, Vec3& v2,
                        FLOAT* t, FLOAT* u, FLOAT* v )
{
    // Find vectors for two edges sharing vert0
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;

    // Begin calculating determinant - also used to calculate U parameter
    Vec3 pvec;
    D3DXVec3Cross( &pvec, &dir, &edge2 );

    // If determinant is near zero, ray lies in plane of triangle
    FLOAT det = D3DXVec3Dot( &edge1, &pvec );

    Vec3 tvec;
    if( det > 0 )
    {
        tvec = orig - v0;
    }
    else
    {
        tvec = v0 - orig;
        det = -det;
    }

    if( det < 0.0001f )
        return FALSE;

    // Calculate U parameter and test bounds
    *u = D3DXVec3Dot( &tvec, &pvec );
    if( *u < 0.0f || *u > det )
        return FALSE;

    // Prepare to test V parameter
    Vec3 qvec;
    D3DXVec3Cross( &qvec, &tvec, &edge1 );

    // Calculate V parameter and test bounds
    *v = D3DXVec3Dot( &dir, &qvec );
    if( *v < 0.0f || *u + *v > det )
        return FALSE;

    // Calculate t, scale parameters, ray intersects triangle
    *t = D3DXVec3Dot( &edge2, &qvec );
    FLOAT fInvDet = 1.0f / det;
    *t *= fInvDet;
    *u *= fInvDet;
    *v *= fInvDet;

    return TRUE;
}



bool TriangleIterator::InitializeStrippedMesh(LPDIRECT3DVERTEXBUFFER9 pVerts, int stride, int strips, int *triCountList )
{
	char *pVertices = NULL;
	if( FAILED( pVerts->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
        return false;

	for (int i=0; i<strips; ++i)
	{
		m_Size += triCountList[i];
	}

	m_Triangles = GCC_NEW Vec3[m_Size * 3];
	int src = 0;
	int dest = 0;

	for (int strip=0; strip<strips; ++strip )
	{
		int vertsInStrip = triCountList[strip]+2;
		GCC_ASSERT(vertsInStrip);

		m_Triangles[dest] = *((Vec3*)&pVertices[stride * src]);
		m_Triangles[dest+1] = *((Vec3*)&pVertices[stride * (src+1)]);
		m_Triangles[dest+2] = *((Vec3*)&pVertices[stride * (src+2)]);
		dest+=3;
		src+=3;
		for (int tri=1; tri<triCountList[strip]; ++tri)
		{
			// for every extra vertex in the triangle strip, you have to grab
			// the two previous verts in the dest list, reverse them, and copy them
			// forward. This will give you a triangle with the same winding
			m_Triangles[dest] = m_Triangles[dest-1];
			m_Triangles[dest+1] = m_Triangles[dest-2];
			m_Triangles[dest+2] = *((Vec3*)&pVertices[stride * (++src)]);
			dest+=3;
		}
	}
	GCC_ASSERT(dest==m_Size*3);
	pVerts->Unlock();

	return true;
}

void *TriangleIterator::VGet(unsigned int i) 
{	
	GCC_ASSERT(i<m_Size);
	return &m_Triangles[i*3]; 
}



