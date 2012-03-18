#pragma once
//========================================================================
// RenderComponent.cpp : classes that define renderable components of actors like Meshes, Skyboxes, Lights, etc.
// Author: David "Rez" Graham
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

#include "..\Graphics3D\Mesh.h"
#include "..\Graphics3D\Sky.h"

#include "../Utilities/String.h"

#include "../EventManager/Events.h"

#include "RenderComponent.h"
#include "TransformComponent.h"

const char* MeshRenderComponent::g_Name = "MeshRenderComponent";
const char* SphereRenderComponent::g_Name = "SphereRenderComponent";
const char* TeapotRenderComponent::g_Name = "TeapotRenderComponent";
const char* GridRenderComponent::g_Name = "GridRenderComponent";
const char* LightRenderComponent::g_Name = "LightRenderComponent";
const char* SkyRenderComponent::g_Name = "SkyRenderComponent";

//---------------------------------------------------------------------------------------------------------------------
// RenderComponent
//---------------------------------------------------------------------------------------------------------------------
bool BaseRenderComponent::VInit(TiXmlElement* pData)
{
    // color
    TiXmlElement* pColorNode = pData->FirstChildElement("Color");
    if (pColorNode)
        m_color = LoadColor(pColorNode);

    return VDelegateInit(pData);
}

void BaseRenderComponent::VPostInit(void)
{
    shared_ptr<SceneNode> pSceneNode(VGetSceneNode());
    shared_ptr<EvtData_New_Render_Component> pEvent(GCC_NEW EvtData_New_Render_Component(m_pOwner->GetId(), pSceneNode));
    IEventManager::Get()->VTriggerEvent(pEvent);
}


void BaseRenderComponent::VOnChanged(void)
{
    shared_ptr<EvtData_Modified_Render_Component> pEvent(GCC_NEW EvtData_Modified_Render_Component(m_pOwner->GetId()));
    IEventManager::Get()->VTriggerEvent(pEvent);
}


TiXmlElement* BaseRenderComponent::VGenerateXml(void)
{
    TiXmlElement* pBaseElement = VCreateBaseElement();

    // color
    TiXmlElement* pColor = GCC_NEW TiXmlElement("Color");
    pColor->SetAttribute("r", ToStr(m_color.r).c_str());
    pColor->SetAttribute("g", ToStr(m_color.g).c_str());
    pColor->SetAttribute("b", ToStr(m_color.b).c_str());
    pColor->SetAttribute("a", ToStr(m_color.a).c_str());
    pBaseElement->LinkEndChild(pColor);

    // create XML for inherited classes
    VCreateInheritedXmlElements(pBaseElement);

    return pBaseElement;
}

shared_ptr<SceneNode> BaseRenderComponent::VGetSceneNode(void)
{
    if (!m_pSceneNode)
        m_pSceneNode = VCreateSceneNode();
    return m_pSceneNode;
}

Color BaseRenderComponent::LoadColor(TiXmlElement* pData)
{
	Color color;

    double r = 1.0;
    double g = 1.0;
    double b = 1.0;
    double a = 1.0;

    pData->Attribute("r", &r);
    pData->Attribute("g", &g);
    pData->Attribute("b", &b);
    pData->Attribute("a", &a);

    color.r = (float)r;
    color.g = (float)g;
    color.b = (float)b;
    color.a = (float)a;

	return color;
}


//---------------------------------------------------------------------------------------------------------------------
// MeshRenderComponent
//---------------------------------------------------------------------------------------------------------------------
shared_ptr<SceneNode> MeshRenderComponent::VCreateSceneNode(void)
{
    return shared_ptr<SceneNode>();
}

void MeshRenderComponent::VCreateInheritedXmlElements(TiXmlElement *)
{
    GCC_ERROR("MeshRenderComponent::VGenerateSubclassXml() not implemented");
}



//---------------------------------------------------------------------------------------------------------------------
// SphereRenderComponent
//---------------------------------------------------------------------------------------------------------------------
SphereRenderComponent::SphereRenderComponent(void)
{
    m_segments = 50;
}

bool SphereRenderComponent::VDelegateInit(TiXmlElement* pData)
{
    TiXmlElement* pMesh = pData->FirstChildElement("Sphere");
    int segments = 50;
	double radius = 1.0;
	pMesh->Attribute("radius", &radius);
    pMesh->Attribute("segments", &segments);
	m_radius = (float)radius;
    m_segments = (unsigned int)segments;

    return true;
}

shared_ptr<SceneNode> SphereRenderComponent::VCreateSceneNode(void)
{
    // get the transform component
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (!pTransformComponent)
    {
        // can't render without a transform
        return shared_ptr<SceneNode>();
    }

	WeakBaseRenderComponentPtr wbrcp(this);
	if (GameCodeApp::GetRendererImpl()==GameCodeApp::Renderer_D3D9)
	{
		// create the sphere Mesh
		ID3DXMesh* pSphereMesh;

		D3DXCreateSphere(DXUTGetD3D9Device(), m_radius, m_segments, m_segments, &pSphereMesh, NULL);

		shared_ptr<SceneNode> sphere(GCC_NEW D3DShaderMeshNode9(m_pOwner->GetId(), wbrcp, pSphereMesh, "Effects\\GameCode4.fx", RenderPass_Actor, &pTransformComponent->GetTransform()));
    
		SAFE_RELEASE(pSphereMesh);
	    return sphere;
	}
	else if (GameCodeApp::GetRendererImpl()==GameCodeApp::Renderer_D3D11)
	{
		shared_ptr<SceneNode> sphere(GCC_NEW D3DShaderMeshNode11(m_pOwner->GetId(), wbrcp, "art\\sphere.sdkmesh", RenderPass_Actor, &pTransformComponent->GetTransform()));
		return sphere;
	}
	else
	{
		GCC_ASSERT(0 && "Unknown Renderer Implementation in SphereRenderComponent::VCreateSceneNode");
		return shared_ptr<SceneNode>(NULL);
	}
}

void SphereRenderComponent::VCreateInheritedXmlElements(TiXmlElement* pBaseElement)
{
    TiXmlElement* pMesh = GCC_NEW TiXmlElement("Sphere");
	pMesh->SetAttribute("radius", ToStr(m_radius).c_str());
    pMesh->SetAttribute("segments", ToStr(m_segments).c_str());
    pBaseElement->LinkEndChild(pBaseElement);
}


//---------------------------------------------------------------------------------------------------------------------
// TeapotRenderComponent
//---------------------------------------------------------------------------------------------------------------------
shared_ptr<SceneNode> TeapotRenderComponent::VCreateSceneNode(void)
{
    // get the transform component
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		WeakBaseRenderComponentPtr weakThis(this);

		switch (GameCodeApp::GetRendererImpl())
		{
			case GameCodeApp::Renderer_D3D9: 
				return shared_ptr<SceneNode>(GCC_NEW D3DTeapotMeshNode9(m_pOwner->GetId(), weakThis, "Effects\\GameCode4.fx", RenderPass_Actor, &pTransformComponent->GetTransform()));

			case GameCodeApp::Renderer_D3D11: 
			{
				Mat4x4 rot90;
				rot90.BuildRotationY(-GCC_PI/2.0f);
				shared_ptr<SceneNode> parent(GCC_NEW SceneNode(m_pOwner->GetId(), weakThis, RenderPass_Actor, &pTransformComponent->GetTransform()));
				shared_ptr<SceneNode> teapot(GCC_NEW D3DTeapotMeshNode11(INVALID_ACTOR_ID, weakThis, RenderPass_Actor, &rot90));
				parent->VAddChild(teapot);
				return parent;
			}
								 
			default:
				GCC_ERROR("Unknown Renderer Implementation in TeapotRenderComponent");
		}
    }

    return shared_ptr<SceneNode>();
}

void TeapotRenderComponent::VCreateInheritedXmlElements(TiXmlElement *)
{
}


//---------------------------------------------------------------------------------------------------------------------
// GridRenderComponent
//---------------------------------------------------------------------------------------------------------------------
GridRenderComponent::GridRenderComponent(void)
{
    m_textureResource = "";
    m_squares = 0;
}

bool GridRenderComponent::VDelegateInit(TiXmlElement* pData)
{
    TiXmlElement* pTexture = pData->FirstChildElement("Texture");
    if (pTexture)
	{
		m_textureResource = pTexture->FirstChild()->Value();
	}

    TiXmlElement* pDivision = pData->FirstChildElement("Division");
    if (pDivision)
	{
		m_squares = atoi(pDivision->FirstChild()->Value());
	}

    return true;
}

shared_ptr<SceneNode> GridRenderComponent::VCreateSceneNode(void)
{
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		WeakBaseRenderComponentPtr weakThis(this);

		switch (GameCodeApp::GetRendererImpl())
		{
			case GameCodeApp::Renderer_D3D9: 
				return shared_ptr<SceneNode>(GCC_NEW D3DGrid9(m_pOwner->GetId(), weakThis, &(pTransformComponent->GetTransform())));  

			case GameCodeApp::Renderer_D3D11: 
				return shared_ptr<SceneNode>(GCC_NEW D3DGrid11(m_pOwner->GetId(), weakThis, &(pTransformComponent->GetTransform())));  
								 
			default:
				GCC_ERROR("Unknown Renderer Implementation in GridRenderComponent");
		}
    }

    return shared_ptr<SceneNode>();
}

void GridRenderComponent::VCreateInheritedXmlElements(TiXmlElement *pBaseElement)
{
    TiXmlElement* pTextureNode = GCC_NEW TiXmlElement("Texture");
    TiXmlText* pTextureText = GCC_NEW TiXmlText(m_textureResource.c_str());
    pTextureNode->LinkEndChild(pTextureText);
    pBaseElement->LinkEndChild(pTextureNode);

    TiXmlElement* pDivisionNode = GCC_NEW TiXmlElement("Division");
    TiXmlText* pDivisionText = GCC_NEW TiXmlText(ToStr(m_squares).c_str());
    pDivisionNode->LinkEndChild(pDivisionText);
    pBaseElement->LinkEndChild(pDivisionNode);
}


//---------------------------------------------------------------------------------------------------------------------
// LightRenderComponent
//---------------------------------------------------------------------------------------------------------------------
LightRenderComponent::LightRenderComponent(void)
{
}

bool LightRenderComponent::VDelegateInit(TiXmlElement* pData)
{
    TiXmlElement* pLight = pData->FirstChildElement("Light");

	double temp;
    TiXmlElement* pAttenuationNode = NULL;
	pAttenuationNode = pLight->FirstChildElement("Attenuation");
    if (pAttenuationNode)
	{
		double temp;
		pAttenuationNode->Attribute("const", &temp);
		m_Props.m_Attenuation[0] = (float) temp;

		pAttenuationNode->Attribute("linear", &temp);
		m_Props.m_Attenuation[1] = (float) temp;

		pAttenuationNode->Attribute("exp", &temp);
		m_Props.m_Attenuation[2] = (float) temp;
	}

    TiXmlElement* pShapeNode = NULL;
	pShapeNode = pLight->FirstChildElement("Shape");
    if (pShapeNode)
	{
		pShapeNode->Attribute("range", &temp);
		m_Props.m_Range = (float) temp;
		pShapeNode->Attribute("falloff", &temp);
		m_Props.m_Falloff = (float) temp;
		pShapeNode->Attribute("theta", &temp);		
		m_Props.m_Theta = (float) temp;
		pShapeNode->Attribute("phi", &temp);
		m_Props.m_Phi = (float) temp;	
	}
    return true;
}

shared_ptr<SceneNode> LightRenderComponent::VCreateSceneNode(void)
{
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
		WeakBaseRenderComponentPtr weakThis(this);

		switch (GameCodeApp::GetRendererImpl())
		{
			case GameCodeApp::Renderer_D3D9: 
				return shared_ptr<SceneNode>(GCC_NEW D3DLightNode9(m_pOwner->GetId(), weakThis, m_Props, &(pTransformComponent->GetTransform())));  

			case GameCodeApp::Renderer_D3D11: 
				return shared_ptr<SceneNode>(GCC_NEW D3DLightNode11(m_pOwner->GetId(), weakThis, m_Props, &(pTransformComponent->GetTransform())));  
								 
			default:
				GCC_ASSERT(0 && "Unknown Renderer Implementation in GridRenderComponent");
		}
	}
    return shared_ptr<SceneNode>();
}

void LightRenderComponent::VCreateInheritedXmlElements(TiXmlElement *pBaseElement)
{
    TiXmlElement* pSceneNode = GCC_NEW TiXmlElement("Light");

    // attenuation
    TiXmlElement* pAttenuation = GCC_NEW TiXmlElement("Attenuation");
    pAttenuation->SetAttribute("const", ToStr(m_Props.m_Attenuation[0]).c_str());
    pAttenuation->SetAttribute("linear", ToStr(m_Props.m_Attenuation[1]).c_str());
    pAttenuation->SetAttribute("exp", ToStr(m_Props.m_Attenuation[2]).c_str());
    pSceneNode->LinkEndChild(pAttenuation);

    // shape
    TiXmlElement* pShape = GCC_NEW TiXmlElement("Shape");
    pShape->SetAttribute("range", ToStr(m_Props.m_Range).c_str());
    pShape->SetAttribute("falloff", ToStr(m_Props.m_Falloff).c_str());
    pShape->SetAttribute("theta", ToStr(m_Props.m_Theta).c_str());
    pShape->SetAttribute("phi", ToStr(m_Props.m_Phi).c_str());
    pSceneNode->LinkEndChild(pShape);

    pBaseElement->LinkEndChild(pSceneNode);
}




SkyRenderComponent::SkyRenderComponent(void)
{
}

bool SkyRenderComponent::VDelegateInit(TiXmlElement* pData)
{
    TiXmlElement* pTexture = pData->FirstChildElement("Texture");
    if (pTexture)
	{
		m_textureResource = pTexture->FirstChild()->Value();
	}
	return true;
}

shared_ptr<SceneNode> SkyRenderComponent::VCreateSceneNode(void)
{
    shared_ptr<SkyNode> sky;
	if (GameCodeApp::GetRendererImpl()==GameCodeApp::Renderer_D3D9)
	{
		sky = shared_ptr<SkyNode>(GCC_NEW D3DSkyNode9(m_textureResource.c_str() ));
	}
	else if (GameCodeApp::GetRendererImpl()==GameCodeApp::Renderer_D3D11)
	{
		sky = shared_ptr<SkyNode>(GCC_NEW D3DSkyNode11(m_textureResource.c_str() ));
	}
	else
	{
		GCC_ERROR("Unknown Renderer Implementation in VLoadGameDelegate");
	}
	return sky;
}

void SkyRenderComponent::VCreateInheritedXmlElements(TiXmlElement *pBaseElement)
{
    TiXmlElement* pTextureNode = GCC_NEW TiXmlElement("Texture");
    TiXmlText* pTextureText = GCC_NEW TiXmlText(m_textureResource.c_str());
    pTextureNode->LinkEndChild(pTextureText);
    pBaseElement->LinkEndChild(pTextureNode);
}



