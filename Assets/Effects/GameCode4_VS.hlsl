//--------------------------------------------------------------------------------------
// GameCode4_VSMain_VS.hlsl - The vertex shader file for GameCode4 (D3D11 renderer)
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
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------
cbuffer cbMatrices : register( b0 )
{
	matrix		g_mWorldViewProjection	: packoffset( c0 );
	matrix		g_mWorld				: packoffset( c4 );
};

cbuffer cbLights : register( b1 )
{
	float4		g_LightDiffuse[8];           // Light's diffuse color - we can support up to eight lights with this shader.
	float4		g_LightDir[8];               // Light's direction in world space
	float4		g_fAmbient;
	int			g_nNumLights;
};

cbuffer cbObjectColors : register( b2 )
{
	float4		g_vDiffuseObjectColor	: packoffset( c0 );
	float4		g_vAmbientObjectColor	: packoffset( c1 );
	bool		g_bHasTexture			: packoffset( c2.x );
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 vDiffuse		: COLOR0;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT GameCode4_VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
    float3 vNormalWorldSpace;
	float dotProduct;
	float4 dottedLightColor;

	Output.vPosition = mul( Input.vPosition, g_mWorldViewProjection );
	vNormalWorldSpace = mul( Input.vNormal, (float3x3)g_mWorld );
	Output.vTexcoord = Input.vTexcoord;

    // Compute simple directional lighting equation
    float4 vTotalLightDiffuse = float4(0,0,0,0);
    for(int i=0; i<g_nNumLights; i++ )
	{
		dotProduct = dot(vNormalWorldSpace, g_LightDir[i]);
		dotProduct = max(0, dotProduct);
		dottedLightColor = g_LightDiffuse[i] * dotProduct;
        vTotalLightDiffuse += dottedLightColor;
	}

    Output.vDiffuse.rgb = g_vDiffuseObjectColor * vTotalLightDiffuse + 
                         g_vAmbientObjectColor * g_fAmbient;   

    Output.vDiffuse.a = g_vDiffuseObjectColor.a; 
	
	return Output;
}

