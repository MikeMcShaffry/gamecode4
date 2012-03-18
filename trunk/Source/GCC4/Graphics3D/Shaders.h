#pragma once
//========================================================================
// Shaders.h - helper classes to call GameCode4_VS.hlsl and GameCode4_PS.hlsl
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
// Content References in Game Coding Complete 4th Edition
// 
// class GameCode4_Hlsl_VertexShader			- Chapter 15, page 508
// class GameCode4_Hlsl_PixelShader				- Chapter 15, page 515
//

#include <xnamath.h>

#include "Geometry.h"
#include "Material.h"

// Forward declarations
class SceneNode;
class Scene;

class GameCode4_Hlsl_VertexShader
{
public:
	GameCode4_Hlsl_VertexShader();
	~GameCode4_Hlsl_VertexShader();

	HRESULT OnRestore(Scene *pScene);
	HRESULT SetupRender(Scene *pScene, SceneNode *pNode);
	void EnableLights(bool enableLights) { m_enableLights = enableLights; }

protected:
	ID3D11InputLayout*          m_pVertexLayout11;
	ID3D11VertexShader*         m_pVertexShader;
	ID3D11Buffer*               m_pcbVSMatrices;
	ID3D11Buffer*               m_pcbVSLighting;
	ID3D11Buffer*               m_pcbVSMaterial;
	bool						m_enableLights;
};


class GameCode4_Hlsl_PixelShader
{
public:
	GameCode4_Hlsl_PixelShader();
	~GameCode4_Hlsl_PixelShader();

	HRESULT OnRestore(Scene *pScene);
	HRESULT SetupRender(Scene *pScene, SceneNode *pNode);
	HRESULT SetTexture(const std::string& textureName);
	HRESULT SetTexture(ID3D11ShaderResourceView* const *pDiffuseRV, ID3D11SamplerState * const *ppSamplers);

protected:
	ID3D11PixelShader*          m_pPixelShader;
	ID3D11Buffer*               m_pcbPSMaterial;
	std::string					m_textureResource;
};

struct ID3DX11Effect;
struct ID3DX11EffectTechnique;
struct ID3DX11EffectPass;

class LineDraw_Hlsl_Shader
{
public:
	LineDraw_Hlsl_Shader();
	~LineDraw_Hlsl_Shader();

	HRESULT OnRestore(Scene *pScene);
	HRESULT SetupRender(Scene *pScene);
	HRESULT SetDiffuse(const std::string& textureName, const Color &color);
	HRESULT SetTexture(ID3D11ShaderResourceView* const *pDiffuseRV, ID3D11SamplerState * const *ppSamplers);

protected:
	ID3D11InputLayout*          m_pVertexLayout11;
	ID3D11Buffer*               m_pcbRenderTargetSize;
	ID3DX11Effect*				m_pEffect;

	ID3DX11EffectTechnique*		m_EffectTechnique; // No need to be Release()-d.
	ID3DX11EffectPass*			m_EffectPass; // No need to be Release()-d.

	ID3D11Buffer*               m_pcbChangePerFrame;

	ID3D11Buffer*               m_pcbDiffuseColor;

	std::string					m_textureResource;

};

