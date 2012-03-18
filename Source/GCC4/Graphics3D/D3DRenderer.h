#pragma once
//========================================================================
// D3DRenderer.h : Defines the D3D renderer abstraction classes of the GameCode4 application
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


#include <DXUTGui.h>

//
// struct ConstantBuffer_Matrices				- Chapter X, page Y
//
struct ConstantBuffer_Matrices
{
    Mat4x4 m_WorldViewProj;
    Mat4x4 m_World;
};

//
// struct ConstantBuffer_Material				- Chapter X, page Y
//
struct ConstantBuffer_Material
{
    Vec4 m_vDiffuseObjectColor;
	Vec4 m_vAmbientObjectColor;
	BOOL m_bHasTexture;
	Vec3 m_vUnused;
};

#define MAXIMUM_LIGHTS_SUPPORTED (8)

//
// struct ConstantBuffer_Lighting				- Chapter X, page Y
//
struct ConstantBuffer_Lighting
{
    Vec4 m_vLightDiffuse[MAXIMUM_LIGHTS_SUPPORTED];
    Vec4 m_vLightDir[MAXIMUM_LIGHTS_SUPPORTED];
	Vec4 m_vLightAmbient;
	UINT m_nNumLights;
	Vec3 m_vUnused;
};

//
// class D3DRenderer11								- Chapter 10, page 270
//
//   The D3DRenderer and D3DRenderer9 classes are not discussed in the book. The D3DRenderer class is designed to 
//   implement the IRenderer interface, which abstracts the implentation of the renderer technology, which for this
//   engine can be either D3D9 or D3D11. It also encapsulates the usefulness of CDXUTDialogResourceManager 
//   and CDXUTTextHelper for user interface tasks whether D3D9 or D3D11 is being used.
//
class D3DRenderer : public IRenderer
{
public:
	// You should leave this global - it does wacky things otherwise.
	static CDXUTDialogResourceManager g_DialogResourceManager;
	static CDXUTTextHelper* g_pTextHelper;

	virtual HRESULT VOnRestore() { return S_OK; }
	virtual void VShutdown() { SAFE_DELETE(g_pTextHelper); }
};

class D3DRenderer9 : public D3DRenderer
{
public:
	D3DRenderer9() { m_pFont = NULL; m_pTextSprite = NULL; }

	virtual void VSetBackgroundColor(BYTE bgA, BYTE bgR, BYTE bgG, BYTE bgB) { m_backgroundColor = 	D3DCOLOR_ARGB(bgA, bgR, bgG, bgB); }
	virtual bool VPreRender();
	virtual bool VPostRender();
	virtual HRESULT VOnRestore();
	virtual void VCalcLighting(Lights *lights, int maximumLights);

	virtual void VShutdown() { D3DRenderer::VShutdown(); SAFE_RELEASE(m_pFont); SAFE_RELEASE(m_pTextSprite); }

	virtual void VSetWorldTransform(const Mat4x4 *m) { DXUTGetD3D9Device()->SetTransform(D3DTS_WORLD, m); }	
	virtual void VSetViewTransform(const Mat4x4 *m) { DXUTGetD3D9Device()->SetTransform( D3DTS_VIEW, m ); }
	virtual void VSetProjectionTransform(const Mat4x4 *m) { DXUTGetD3D9Device()->SetTransform( D3DTS_PROJECTION, m ); }

	virtual shared_ptr<IRenderState> VPrepareAlphaPass();
	virtual shared_ptr<IRenderState> VPrepareSkyBoxPass();

	virtual void VDrawLine(const Vec3& from,const Vec3& to,const Color& color);

protected:
	D3DCOLOR m_backgroundColor;		// the color that the view is cleared to each frame

	ID3DXFont*			        m_pFont;
	ID3DXSprite*				m_pTextSprite;
};





//
// class D3DLineDrawer11 - not described in the book, it is a helper class to draw lines in D3D11
//
class D3DLineDrawer11 
{
public:
	D3DLineDrawer11() { m_pVertexBuffer = NULL; }
	~D3DLineDrawer11() { SAFE_RELEASE(m_pVertexBuffer); }

	void DrawLine(const Vec3& from,const Vec3& to,const Color& color);
	HRESULT OnRestore();

protected:
	Vec3						m_Verts[2];
	LineDraw_Hlsl_Shader		m_LineDrawerShader;
	ID3D11Buffer*               m_pVertexBuffer;
};


class D3DRenderer11 : public D3DRenderer
{
public:
	D3DRenderer11() { m_pLineDrawer = NULL; }
	virtual void VShutdown()  { D3DRenderer::VShutdown(); SAFE_DELETE(m_pLineDrawer); }

	virtual void VSetBackgroundColor(BYTE bgA, BYTE bgR, BYTE bgG, BYTE bgB) 
	{ 
		m_backgroundColor[0] = float(bgA) / 255.0f;
		m_backgroundColor[1] = float(bgR) / 255.0f;
		m_backgroundColor[2] = float(bgG) / 255.0f;
		m_backgroundColor[3] = float(bgB) / 255.0f;
	}

	virtual bool VPreRender();
	virtual bool VPostRender();
	virtual HRESULT VOnRestore();
	virtual void VCalcLighting(Lights *lights, int maximumLights) { }

	// These three functions are done for each shader, not as a part of beginning the render - so they do nothing in D3D11.
	virtual void VSetWorldTransform(const Mat4x4 *m) {  }	
	virtual void VSetViewTransform(const Mat4x4 *m) {  }
	virtual void VSetProjectionTransform(const Mat4x4 *m) {  }

	virtual void VDrawLine(const Vec3& from,const Vec3& to,const Color& color);

	virtual shared_ptr<IRenderState> VPrepareAlphaPass();
	virtual shared_ptr<IRenderState> VPrepareSkyBoxPass();

	HRESULT CompileShader( LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

protected:
	float								m_backgroundColor[4];

	D3DLineDrawer11						*m_pLineDrawer;
};

