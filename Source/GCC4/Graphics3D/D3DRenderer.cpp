//========================================================================
// D3DRenderer.cpp - implements both D3D9 and D3D11 renderer implementations
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
#include "D3DRenderer.h"
#include "Lights.h"
#include "../UserInterface/HumanView.h"

// You should leave this global - it does wacky things otherwise.
CDXUTDialogResourceManager D3DRenderer::g_DialogResourceManager;
CDXUTTextHelper *D3DRenderer::g_pTextHelper = NULL;

//
// class D3DRendererAlphaPass9				- not described in the book, see class D3DRendererAlphaPass11 
//
class D3DRendererAlphaPass9 : public IRenderState
{
protected:
	Mat4x4 m_oldWorld;
	DWORD m_oldZWriteEnable;

public:
	D3DRendererAlphaPass9();
	~D3DRendererAlphaPass9();
	std::string VToString() { return "D3DRenderAlphaPass9"; }
};

D3DRendererAlphaPass9::D3DRendererAlphaPass9()
{
	DXUTGetD3D9Device()->GetTransform(D3DTS_WORLD, &m_oldWorld);
	DXUTGetD3D9Device()->GetRenderState(D3DRS_ZWRITEENABLE, &m_oldZWriteEnable);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ZWRITEENABLE, false);

	DXUTGetD3D9Device()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    DXUTGetD3D9Device()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

D3DRendererAlphaPass9::~D3DRendererAlphaPass9()
{
	DXUTGetD3D9Device()->SetRenderState(D3DRS_COLORVERTEX, false);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ZWRITEENABLE, m_oldZWriteEnable);
	DXUTGetD3D9Device()->SetTransform(D3DTS_WORLD, &m_oldWorld);
}


//
// class D3DRendererAlphaPass911			- Chapter 16, page 543
//
class D3DRendererAlphaPass11 : public IRenderState
{
protected:
	ID3D11BlendState* m_pOldBlendState;
	FLOAT m_OldBlendFactor[ 4 ];
    UINT m_OldSampleMask;
        
	ID3D11BlendState* m_pCurrentBlendState;

public:
	D3DRendererAlphaPass11();
	~D3DRendererAlphaPass11();
	std::string VToString() { return "D3DRendererAlphaPass11"; }
};

//
// D3DRendererAlphaPass11::D3DRendererAlphaPass11			- Chapter 16, page 544
//
D3DRendererAlphaPass11::D3DRendererAlphaPass11()
{
	DXUTGetD3D11DeviceContext()->OMGetBlendState(&m_pOldBlendState, m_OldBlendFactor, &m_OldSampleMask);
	m_pCurrentBlendState = NULL;

	D3D11_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));

	BlendState.AlphaToCoverageEnable = false;
    BlendState.IndependentBlendEnable = false;
	BlendState.RenderTarget[0].BlendEnable = TRUE;
	BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	DXUTGetD3D11Device()->CreateBlendState(&BlendState, &m_pCurrentBlendState);
	DXUTGetD3D11DeviceContext()->OMSetBlendState(m_pCurrentBlendState, 0, 0xffffffff);
}

//
// D3DRendererAlphaPass11::~D3DRendererAlphaPass11			- Chapter 16, page 544
//
D3DRendererAlphaPass11::~D3DRendererAlphaPass11()
{
	DXUTGetD3D11DeviceContext()->OMSetBlendState(m_pOldBlendState, m_OldBlendFactor, m_OldSampleMask);
	SAFE_RELEASE(m_pCurrentBlendState);
	SAFE_RELEASE(m_pOldBlendState);
}

 
//
// class D3DRendererSkyBoxPass9					- not described in the book - see D3DRendererSkyBoxPass11 
//
class D3DRendererSkyBoxPass9 : public IRenderState
{
protected:
	DWORD m_oldZWriteEnable;
	DWORD m_oldLightMode;
	DWORD m_oldCullMode;

public:
	D3DRendererSkyBoxPass9();
	~D3DRendererSkyBoxPass9();
	std::string VToString() { return "D3DRendererSkyboxPass9"; }
};

D3DRendererSkyBoxPass9::D3DRendererSkyBoxPass9()
{
	DXUTGetD3D9Device()->GetRenderState(D3DRS_ZWRITEENABLE, &m_oldZWriteEnable);
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ZWRITEENABLE, false);
	DXUTGetD3D9Device()->GetRenderState( D3DRS_LIGHTING, &m_oldLightMode );
	DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, FALSE );
	DXUTGetD3D9Device()->GetRenderState( D3DRS_CULLMODE, &m_oldCullMode );
	DXUTGetD3D9Device()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
}


D3DRendererSkyBoxPass9::~D3DRendererSkyBoxPass9()
{
	DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, m_oldLightMode );
	DXUTGetD3D9Device()->SetRenderState( D3DRS_CULLMODE, m_oldCullMode );
	DXUTGetD3D9Device()->SetRenderState(D3DRS_ZWRITEENABLE, m_oldZWriteEnable);
}





//
// class D3DRendererSkyBoxPass11							- Chapter 16, page 548
//
class D3DRendererSkyBoxPass11 : public IRenderState
{
protected:
	ID3D11DepthStencilState* m_pOldDepthStencilState;
	ID3D11DepthStencilState* m_pSkyboxDepthStencilState;

public:
	D3DRendererSkyBoxPass11();
	~D3DRendererSkyBoxPass11();
	std::string VToString() { return "D3DRendererSkyBoxPass11"; }
};

//
// D3DRendererSkyBoxPass11::D3DRendererSkyBoxPass11()		- Chapter 16, page 548
//
D3DRendererSkyBoxPass11::D3DRendererSkyBoxPass11()
{
    // Depth stencil state
	D3D11_DEPTH_STENCIL_DESC DSDesc;
    ZeroMemory( &DSDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
    DSDesc.DepthEnable = TRUE;
    DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
    DSDesc.StencilEnable = FALSE;
    DXUTGetD3D11Device()->CreateDepthStencilState( &DSDesc, &m_pSkyboxDepthStencilState );
    DXUT_SetDebugName( m_pSkyboxDepthStencilState, "SkyboxDepthStencil" );

    UINT StencilRef;
    DXUTGetD3D11DeviceContext()->OMGetDepthStencilState( &m_pOldDepthStencilState, &StencilRef );
    DXUTGetD3D11DeviceContext()->OMSetDepthStencilState( m_pSkyboxDepthStencilState, 0 );
}

//
// D3DRendererSkyBoxPass11::~D3DRendererSkyBoxPass11()		- Chapter 16, page 548
//
D3DRendererSkyBoxPass11::~D3DRendererSkyBoxPass11()
{
    DXUTGetD3D11DeviceContext()->OMSetDepthStencilState( m_pOldDepthStencilState, 0 );
	SAFE_RELEASE(m_pOldDepthStencilState);
	SAFE_RELEASE(m_pSkyboxDepthStencilState);
}



////////////////////////////////////////////////////
// D3DRenderer9 Implementation
//
//    Not described in the book - but it abstracts 
//    some of the calls to get the game engine to run
//    under DX9.
////////////////////////////////////////////////////

bool D3DRenderer9::VPreRender()
{
	// Clear the render target and the zbuffer 
	if ( SUCCEEDED ( DXUTGetD3D9Device()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_backgroundColor, 1.0f, 0) ) )
	{
		// Render the scene
		return ( SUCCEEDED( DXUTGetD3D9Device()->BeginScene() ) );
	}
	
	return false;
}

HRESULT D3DRenderer9::VOnRestore()
{
	HRESULT hr;
	V_RETURN ( D3DRenderer::VOnRestore() );
	V_RETURN ( D3DRenderer::g_DialogResourceManager.OnD3D9ResetDevice() ); 

	SAFE_DELETE(D3DRenderer::g_pTextHelper);

	SAFE_RELEASE(m_pFont);
    V_RETURN(D3DXCreateFont( DXUTGetD3D9Device(), 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
                    OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
                    L"Arial", &m_pFont ) );

	SAFE_RELEASE(m_pTextSprite);
	V_RETURN( D3DXCreateSprite( DXUTGetD3D9Device(), &m_pTextSprite ) );
	
	D3DRenderer::g_pTextHelper = GCC_NEW CDXUTTextHelper( m_pFont, m_pTextSprite, 15 );			
	return S_OK;
}



bool D3DRenderer9::VPostRender(void)
{
	return SUCCEEDED (DXUTGetD3D9Device()->EndScene() );
}


shared_ptr<IRenderState> D3DRenderer9::VPrepareAlphaPass()
{
	return shared_ptr<IRenderState>(GCC_NEW D3DRendererAlphaPass9());
}

shared_ptr<IRenderState> D3DRenderer9::VPrepareSkyBoxPass()
{
	return shared_ptr<IRenderState>(GCC_NEW D3DRendererSkyBoxPass9());
}


void D3DRenderer9::VCalcLighting(Lights *lights, int maximumLights)
{
	int count = 1;
	if (lights && lights->size() > 0)
	{
		for(Lights::iterator i=lights->begin(); i!=lights->end(); ++i, ++count)
		{
			shared_ptr<LightNode> light = *i;
			shared_ptr<D3DLightNode9> light9 = static_pointer_cast<D3DLightNode9>(light);
			D3DLIGHT9 *m_pLight = &(light9->m_d3dLight9);
			DXUTGetD3D9Device()->SetLight( count, m_pLight );
			DXUTGetD3D9Device()->LightEnable( count, TRUE );
		}
		DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, TRUE );
	}
	else
	{
		DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, FALSE );
	}

	// turn off the other lights if they were on
	for (;count<maximumLights; ++count)
	{
		DXUTGetD3D9Device()->LightEnable( count, FALSE );
	}
}


void D3DRenderer9::VDrawLine(const Vec3& from,const Vec3& to,const Color& color)
{
	DWORD oldLightingState;
	DXUTGetD3D9Device()->GetRenderState( D3DRS_LIGHTING, &oldLightingState );
	
	// disable lighting for the lines
	DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, FALSE );
		
	D3D9Vertex_Colored verts[2];
	
	verts[0].position = from;
	verts[0].color = color;
	verts[1].position = to;
	verts[1].color = verts[0].color;
		
	DXUTGetD3D9Device()->SetFVF( D3D9Vertex_Colored::FVF );
	DXUTGetD3D9Device()->DrawPrimitiveUP( D3DPT_LINELIST, 1, verts, sizeof(D3D9Vertex_Colored) );
		
	// restore original lighting state
	DXUTGetD3D9Device()->SetRenderState( D3DRS_LIGHTING, oldLightingState );
}



HRESULT D3DLineDrawer11::OnRestore()
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof( D3D11Vertex_PositionColored ) * 2;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = &m_Verts;
	HRESULT hr;

	V_RETURN ( DXUTGetD3D11Device()->CreateBuffer( &bd, &InitData, &m_pVertexBuffer ) );

	shared_ptr<Scene> pScene = g_pApp->GetHumanView()->m_pScene;

	V_RETURN (m_LineDrawerShader.OnRestore(&(*pScene)) );

	return S_OK;
}



void D3DLineDrawer11::DrawLine(const Vec3& from,const Vec3& to,const Color& color)
{
	HRESULT hr;

	shared_ptr<Scene> pScene = g_pApp->GetHumanView()->m_pScene;
	shared_ptr<IRenderer> pRenderer = pScene->GetRenderer();

	if ( FAILED(m_LineDrawerShader.SetupRender(&(*pScene)) ) )
		return;

	m_LineDrawerShader.SetDiffuse("art\\grid.dds", color);	

    // Set vertex buffer
    UINT stride = sizeof( Vec3 );
    UINT offset = 0;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
    V( DXUTGetD3D11DeviceContext()->Map( m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );

	Vec3 * pVerts = ( Vec3* )MappedResource.pData;
	pVerts[0] = from;
	pVerts[1] = to;
	DXUTGetD3D11DeviceContext()->Unmap( m_pVertexBuffer, 0 );

    DXUTGetD3D11DeviceContext()->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );

    // Set primitive topology
    DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

	DXUTGetD3D11DeviceContext()->Draw( 2, 0 );
}



////////////////////////////////////////////////////
// D3DRenderer11 Implementation
//
//    Not described in the book - but it abstracts 
//    some of the calls to get the game engine to run
//    under DX11.
////////////////////////////////////////////////////

HRESULT D3DRenderer11::VOnRestore()
{
	HRESULT hr;
	V_RETURN ( D3DRenderer::VOnRestore() );
	SAFE_DELETE(D3DRenderer::g_pTextHelper);
	D3DRenderer::g_pTextHelper = GCC_NEW CDXUTTextHelper( DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext(), &g_DialogResourceManager, 15 );

	return S_OK;
}


bool D3DRenderer11::VPreRender()
{
	if (DXUTGetD3D11DeviceContext() && DXUTGetD3D11RenderTargetView())
	{
		DXUTGetD3D11DeviceContext()->ClearRenderTargetView( DXUTGetD3D11RenderTargetView(), m_backgroundColor );

		//
		// Clear the depth buffer to 1.0 (max depth)
		//
		DXUTGetD3D11DeviceContext()->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0 );
	}
	return true;
}

bool D3DRenderer11::VPostRender(void)
{
	return true;
}



//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT D3DRenderer11::CompileShader( LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromMemory( pSrcData, SrcDataLen, pFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


HRESULT D3DRenderer11::CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


void D3DRenderer11::VDrawLine(const Vec3& from,const Vec3& to,const Color& color) 
{
	if (!m_pLineDrawer)
	{
		m_pLineDrawer = GCC_NEW D3DLineDrawer11();
		m_pLineDrawer->OnRestore();
		

	}
	m_pLineDrawer->DrawLine(from, to, color); 
}




shared_ptr<IRenderState> D3DRenderer11::VPrepareAlphaPass()
{
	return shared_ptr<IRenderState>(GCC_NEW D3DRendererAlphaPass11());
}


shared_ptr<IRenderState> D3DRenderer11::VPrepareSkyBoxPass()
{
	return shared_ptr<IRenderState>(GCC_NEW D3DRendererSkyBoxPass11());
}


