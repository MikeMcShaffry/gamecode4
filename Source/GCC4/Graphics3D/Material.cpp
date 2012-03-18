//========================================================================
// File: Material.cpp - stores texture and material information for D3D9 and D3D11
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
#include "../ResourceCache/ResCache.h"
#include "SceneNodes.h"


////////////////////////////////////////////////////
// class Material						- Chapter 14, page 486
////////////////////////////////////////////////////

Material::Material()
{
	ZeroMemory( &m_D3DMaterial, sizeof( D3DMATERIAL9 ) );
	m_D3DMaterial.Diffuse = g_White;
	m_D3DMaterial.Ambient = Color(0.10f, 0.10f, 0.10f, 1.0f);		
	m_D3DMaterial.Specular = g_White;
	m_D3DMaterial.Emissive = g_Black;
}

void Material::SetAmbient(const Color &color)
{
	m_D3DMaterial.Ambient = color;
}

void Material::SetDiffuse(const Color &color)
{
	m_D3DMaterial.Diffuse = color;
}

void Material::SetSpecular(const Color &color, const float power)
{
	m_D3DMaterial.Specular = color;
	m_D3DMaterial.Power = power;
}

void Material::SetEmissive(const Color &color)
{
	m_D3DMaterial.Emissive = color;
}

void Material::SetAlpha(float alpha)
{
	m_D3DMaterial.Diffuse.a = alpha;
}

void Material::D3DUse9()
{
	DXUTGetD3D9Device()->SetMaterial( &m_D3DMaterial );	
}


//
// class DdsResourceLoader					- creates an interface with the Resource cache to load DDS files
//
class DdsResourceLoader : public TextureResourceLoader
{
public:
	virtual std::string VGetPattern() { return "*.dds"; }
};

shared_ptr<IResourceLoader> CreateDDSResourceLoader()
{
	return shared_ptr<IResourceLoader>(GCC_NEW DdsResourceLoader());
}

//
// class JpgResourceLoader					- creates an interface with the Resource cache to load JPG files
//
class JpgResourceLoader : public TextureResourceLoader
{
public:
	virtual std::string VGetPattern() { return "*.jpg"; }
};

shared_ptr<IResourceLoader> CreateJPGResourceLoader()
{
	return shared_ptr<IResourceLoader>(GCC_NEW JpgResourceLoader());
}

D3DTextureResourceExtraData9::D3DTextureResourceExtraData9()
: m_pTexture(NULL) 
{	
}

D3DTextureResourceExtraData11::D3DTextureResourceExtraData11()
: m_pTexture(NULL), m_pSamplerLinear(NULL)
{	
}


unsigned int TextureResourceLoader::VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize)
{
	// This will keep the resource cache from allocating memory for the texture, so DirectX can manage it on it's own.
	return 0;
}

//
// TextureResourceLoader::VLoadResource				- Chapter 14, page 492
//
bool TextureResourceLoader::VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle)
{
	GameCodeApp::Renderer renderer = GameCodeApp::GetRendererImpl();
	if (renderer == GameCodeApp::Renderer_D3D9)
	{
		shared_ptr<D3DTextureResourceExtraData9> extra = shared_ptr<D3DTextureResourceExtraData9>(GCC_NEW D3DTextureResourceExtraData9());

		if ( FAILED ( D3DXCreateTextureFromFileInMemory( DXUTGetD3D9Device(), rawBuffer, rawSize, &extra->m_pTexture ) ) )
			return false;
		else 
		{
			handle->SetExtra(shared_ptr<D3DTextureResourceExtraData9>(extra));
			return true;
		}
	}
	else if (renderer == GameCodeApp::Renderer_D3D11)
	{
		shared_ptr<D3DTextureResourceExtraData11> extra = shared_ptr<D3DTextureResourceExtraData11>(GCC_NEW D3DTextureResourceExtraData11());

		// Load the Texture
		if ( FAILED ( D3DX11CreateShaderResourceViewFromMemory( DXUTGetD3D11Device(), rawBuffer, rawSize, NULL, NULL, &extra->m_pTexture, NULL ) ) )
			return false;

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		if( FAILED( DXUTGetD3D11Device()->CreateSamplerState( &sampDesc, &extra->m_pSamplerLinear ) ) ) 
			return false;

		handle->SetExtra(shared_ptr<D3DTextureResourceExtraData11>(extra));
		return true;
	}

	GCC_ASSERT(0 && "Unsupported Renderer in TextureResourceLoader::VLoadResource");
	return false;
}

