#pragma once
//========================================================================
// File: Material.h - stores texture and material information for D3D9 and D3D11
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

//========================================================================
//  Content References in Game Coding Complete 4th Edition
// 
//  class Material				- Chapter 14, page 485

#include "Geometry.h"
#include "../ResourceCache/ResCache.h"

class Material
{
	D3DMATERIAL9 m_D3DMaterial;				// This structure stores diffuse, ambient, specular, emissive, and power.
public:
	Material();
	void SetAmbient(const Color &color);
	const Color GetAmbient() { return m_D3DMaterial.Ambient; }

	void SetDiffuse(const Color &color);
	const Color GetDiffuse() { return m_D3DMaterial.Diffuse; }

	void SetSpecular(const Color &color, const float power);
	void GetSpecular(Color &_color, float &_power) 
		{ _color = m_D3DMaterial.Specular; _power = m_D3DMaterial.Power; }

	void SetEmissive(const Color &color);
	const Color GetEmissive() { return m_D3DMaterial.Emissive; }

	void SetAlpha(const float alpha);
	bool HasAlpha() const { return GetAlpha() != fOPAQUE; }
	float GetAlpha() const { return m_D3DMaterial.Diffuse.a; }

	void D3DUse9();
};

// 
//  class D3DTextureResourceExtraData9				- not described in the book, see D3DTextureResourceExtraData11
//
class D3DTextureResourceExtraData9 : public IResourceExtraData
{
	friend class TextureResourceLoader;

public:
	D3DTextureResourceExtraData9();
	virtual ~D3DTextureResourceExtraData9() { SAFE_RELEASE(m_pTexture); }
	virtual std::string VToString() { return "D3DTextureResourceExtraData9"; }

	LPDIRECT3DTEXTURE9 const GetTexture() { return m_pTexture; }

protected:
	LPDIRECT3DTEXTURE9		m_pTexture;			
};

// 
//  class D3DTextureResourceExtraData11				- Chapter 14, page 492
//
class D3DTextureResourceExtraData11 : public IResourceExtraData
{
	friend class TextureResourceLoader;

public:
	D3DTextureResourceExtraData11();
	virtual ~D3DTextureResourceExtraData11() { SAFE_RELEASE(m_pTexture); SAFE_RELEASE(m_pSamplerLinear); }
	virtual std::string VToString() { return "D3DTextureResourceExtraData11"; }

	ID3D11ShaderResourceView * const *GetTexture() { return &m_pTexture; }
	ID3D11SamplerState * const *GetSampler() { return &m_pSamplerLinear; }

protected:
	ID3D11ShaderResourceView *m_pTexture;			
	ID3D11SamplerState* m_pSamplerLinear;
};



// 
//  class TextureResourceLoader				- Chapter 14, page 492
//
class TextureResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle);
};
