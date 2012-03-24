#pragma once
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


#include "Geometry.h"

class Intersection
{
public:
    FLOAT m_fDist;                  // distance from ray origin to intersection
	DWORD m_dwFace;					// the face index of the intersection
	FLOAT m_fBary1, m_fBary2;		// Barycentric coordinates of the intersection
    FLOAT m_tu, m_tv;               // texture coords of intersection
	ActorId m_actorId;				// Which actor was intersected if there was one
	Vec3 m_worldLoc;				// world location of intersection
	Vec3 m_actorLoc;				// actor local coordinates of intersection
	Vec3 m_normal;					// normal of intersection

	bool const operator <(Intersection const &other) { return m_fDist < other.m_fDist; }
};

template <class T>
void InitIntersection(Intersection &intersection, DWORD faceIndex, FLOAT dist, FLOAT u, FLOAT v, ActorId actorId, WORD* pIndices, T* pVertices, const Mat4x4 &matWorld);


typedef std::vector<Intersection> IntersectionArray;

class CDXUTSDKMesh;

class RayCast
{
protected:
	LPDIRECT3DVERTEXBUFFER9     m_pVB; 

public:
	RayCast(Point point, DWORD maxIntersections = 16);

	DWORD m_MaxIntersections;
	DWORD m_NumIntersections;
	bool m_bUseD3DXIntersect;      // Whether to use D3DXIntersect
	bool m_bAllHits;			// Whether to just get the first "hit" or all "hits"
	Point m_Point;

	D3DXVECTOR3 m_vPickRayDir;
    D3DXVECTOR3 m_vPickRayOrig;

	IntersectionArray m_IntersectionArray;

	HRESULT Pick(Scene *pScene, ActorId actorId, ID3DXMesh *pMesh);
	HRESULT Pick(Scene *pScene, ActorId actorId, CDXUTSDKMesh *pMesh);

	HRESULT Pick(Scene *pScene, ActorId actorId, LPDIRECT3DVERTEXBUFFER9 pVerts, LPDIRECT3DINDEXBUFFER9 pIndices, DWORD numPolys);
	HRESULT Pick(Scene *pScene, ActorId actorId, LPDIRECT3DVERTEXBUFFER9 pVerts, DWORD numPolys);

	void Sort();
};


