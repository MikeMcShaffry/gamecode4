//--------------------------------------------------------------------------------------
// GameCode4.fx - the shader file for GameCode4 (D3D9 renderer)
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
// This shader is not described in the 4th edition of Game Coding Complete. 
// It was described in the 3rd edition, Chapter 14, page 509-514
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
float4 g_MaterialAmbientColor;      // Material's ambient color
float4 g_MaterialDiffuseColor;      // Material's diffuse color
int g_nNumLights;

// This shader can handle eight directional lights with a diffuse component and one ambient light
float4 g_LightDir[8];               // Light's direction in world space
float4 g_LightDiffuse[8];           // Light's diffuse color
float4 g_LightAmbient;              // Light's ambient color
float g_fAlpha;						// Alpha value (0.0 totally transparent, 1.0 opaque)

texture g_MeshTexture;              // Color texture for mesh

float    g_fTime;                   // App's time in seconds
float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix



//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};




//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};


//--------------------------------------------------------------------------------------
// This shader computes standard transform and lighting
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0,
                         uniform bool bTexture )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
  
    // Transform the position from object space to homogeneous projection space
    Output.Position = mul(vPos, g_mWorldViewProjection);
    
    // Transform the normal from object space to world space    
    vNormalWorldSpace = normalize(mul(vNormal, (float3x3)g_mWorld)); // normal (world space)
    
    // Compute simple directional lighting equation
    float4 vTotalLightDiffuse = float4(0,0,0,0);
    for(int i=0; i<g_nNumLights; i++ )
        vTotalLightDiffuse += g_LightDiffuse[i] * max(0,dot(vNormalWorldSpace, g_LightDir[i]));
        
    Output.Diffuse.rgb = g_MaterialDiffuseColor * vTotalLightDiffuse + 
                         g_MaterialAmbientColor * g_LightAmbient;   

    Output.Diffuse.a = g_fAlpha; 
    
    // Just copy the texture coordinate through
    if( bTexture ) 
        Output.TextureUV = vTexCoord0; 
    else
        Output.TextureUV = 0; 
    
    return Output;    
}


//--------------------------------------------------------------------------------------
// Pixel shader output structure
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// This shader outputs the pixel's color by modulating the texture's
//       color with diffuse material color
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT In,
                         uniform bool bTexture ) 
{ 
    PS_OUTPUT Output;

    // Lookup mesh texture and modulate it with diffuse
    if( bTexture )
        Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;
    else
        Output.RGBColor = In.Diffuse;

    return Output;
}


//--------------------------------------------------------------------------------------
// Renders scene to render target
//--------------------------------------------------------------------------------------

technique RenderScene
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( false );
        PixelShader  = compile ps_2_0 RenderScenePS( false ); // trivial pixel shader (could use FF instead if desired)
    }
}


technique RenderSceneWithAlpha
{
    pass P0
    {          
		AlphaBlendEnable	= true;
		SrcBlend			= SrcAlpha;
		DestBlend			= InvSrcAlpha;
        VertexShader = compile vs_2_0 RenderSceneVS( false );
        PixelShader  = compile ps_2_0 RenderScenePS( false ); // trivial pixel shader (could use FF instead if desired)
    }
}

technique RenderSceneWithTexture
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( true );
        PixelShader  = compile ps_2_0 RenderScenePS( true ); // trivial pixel shader (could use FF instead if desired)
    }
}


