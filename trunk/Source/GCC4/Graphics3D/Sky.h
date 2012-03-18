#pragma once
//========================================================================
// Sky.h - implements a sky box in either D3D9 or D3D11
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

////////////////////////////////////////////////////
//
// class SkyNode					- Chapter 16, page 554
//
//    Implements a believable sky that follows
//	  the camera around - this is a base class that the D3D9 and D3D11 classes 
//    inherit from
//
////////////////////////////////////////////////////

class SkyNode : public SceneNode
{
protected:
	DWORD					m_numVerts;
	DWORD					m_sides;
	const char *			m_textureBaseName;
	shared_ptr<CameraNode>	m_camera;
	bool					m_bActive;

	std::string GetTextureName(const int side);

public:
	SkyNode(const char *textureFile);
	virtual ~SkyNode() { }
	HRESULT VPreRender(Scene *pScene);
	bool VIsVisible(Scene *pScene) const { return m_bActive; }
};


class D3DSkyNode9 : public SkyNode
{
protected:
	LPDIRECT3DTEXTURE9		m_pTexture[5];		// the sky textures
	LPDIRECT3DVERTEXBUFFER9 m_pVerts;			// the sky verts

public:
	D3DSkyNode9(const char *pTextureBaseName );
	virtual ~D3DSkyNode9();
	HRESULT VOnRestore(Scene *pScene);
	HRESULT VRender(Scene *pScene);
};


class D3DSkyNode11 : public SkyNode
{
public:
	D3DSkyNode11(const char *pTextureBaseName );
	virtual ~D3DSkyNode11();
	HRESULT VOnRestore(Scene *pScene);
	HRESULT VRender(Scene *pScene);

protected:
	ID3D11Buffer*               m_pIndexBuffer;
	ID3D11Buffer*               m_pVertexBuffer;

	GameCode4_Hlsl_VertexShader		m_VertexShader;
	GameCode4_Hlsl_PixelShader		m_PixelShader;
};

