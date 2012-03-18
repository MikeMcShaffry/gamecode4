#pragma once
//========================================================================
// RenderComponent.h : classes that define renderable components of actors like Meshes, Skyboxes, Lights, etc.
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

#include "RenderComponentInterface.h"
#include "../Graphics3D/SceneNodes.h"
#include "../Graphics3D/Lights.h"

//---------------------------------------------------------------------------------------------------------------------
// RenderComponent base class.  This class does most of the work except actually creating the scene, which is 
// delegated to the subclass through a factory method:
// http://en.wikipedia.org/wiki/Factory_method_pattern
//---------------------------------------------------------------------------------------------------------------------
class BaseRenderComponent : public RenderComponentInterface
{
protected:
    Color m_color;
    shared_ptr<SceneNode> m_pSceneNode;

public:
    virtual bool VInit(TiXmlElement* pData) override;
    virtual void VPostInit(void) override;
	virtual void VOnChanged(void) override;
    virtual TiXmlElement* VGenerateXml(void) override;
	const Color GetColor() const { return m_color; }

protected:
    // loads the SceneNode specific data (represented in the <SceneNode> tag)
    virtual bool VDelegateInit(TiXmlElement* pData) { return true; }
    virtual shared_ptr<SceneNode> VCreateSceneNode(void) = 0;  // factory method to create the appropriate scene node
    Color LoadColor(TiXmlElement* pData);

    // editor stuff
	virtual TiXmlElement* VCreateBaseElement(void) { return GCC_NEW TiXmlElement(VGetName()); }
    virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement) = 0;

private:
    virtual shared_ptr<SceneNode> VGetSceneNode(void) override;
};


//---------------------------------------------------------------------------------------------------------------------
// This class represents a render component built from a Mesh.  In a real game, this is the one you'll use 99% of the 
// time towards the end of the project.  The other classes are important for testing since programming tends to move 
// a lot faster than art in the early stages of development.
//---------------------------------------------------------------------------------------------------------------------
class MeshRenderComponent : public BaseRenderComponent
{
public:
	static const char *g_Name;
	virtual const char *VGetName() const { return g_Name; }

protected:
    virtual shared_ptr<SceneNode> VCreateSceneNode(void) override;  // factory method to create the appropriate scene node
    virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};



//---------------------------------------------------------------------------------------------------------------------
// Spheres, which Teapot Wars uses as projectiles.
//---------------------------------------------------------------------------------------------------------------------
class SphereRenderComponent : public BaseRenderComponent
{
    unsigned int m_segments;
	float m_radius;

public:
	static const char *g_Name;
	virtual const char *VGetName() const { return g_Name; }

    SphereRenderComponent(void);

protected:
    virtual bool VDelegateInit(TiXmlElement* pData) override;
    virtual shared_ptr<SceneNode> VCreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};


//---------------------------------------------------------------------------------------------------------------------
// Teapots, which are the main characters in the game.  DirectX actually has a function that generates a teapot Mesh
// so we might as well use it.  We're game programmers, not artists.  ;)
//---------------------------------------------------------------------------------------------------------------------
class TeapotRenderComponent : public BaseRenderComponent
{
public:
	static const char *g_Name;
	virtual const char *VGetName() const { return g_Name; }

protected:
    virtual shared_ptr<SceneNode> VCreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};


//---------------------------------------------------------------------------------------------------------------------
// Grids, which represent the world
//---------------------------------------------------------------------------------------------------------------------
class GridRenderComponent : public BaseRenderComponent
{
    std::string m_textureResource;
    int m_squares;

public:
	static const char *g_Name;
	virtual const char *VGetName() const { return g_Name; }

    GridRenderComponent(void);
	const char* GetTextureResource() { return m_textureResource.c_str(); }
	const int GetDivision() { return m_squares; }

protected:
    virtual bool VDelegateInit(TiXmlElement* pData) override;
    virtual shared_ptr<SceneNode> VCreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};


//---------------------------------------------------------------------------------------------------------------------
// Lights
//---------------------------------------------------------------------------------------------------------------------
class LightRenderComponent : public BaseRenderComponent
{
	LightProperties m_Props; 

public:
	static const char *g_Name;
	virtual const char *VGetName() const { return g_Name; }

    LightRenderComponent(void);

protected:
    virtual bool VDelegateInit(TiXmlElement* pData) override;
    virtual shared_ptr<SceneNode> VCreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};


class SkyRenderComponent : public BaseRenderComponent
{
	std::string m_textureResource;

public:
	static const char *g_Name;
	virtual const char *VGetName() const { return g_Name; }

    SkyRenderComponent(void);

protected:
    virtual bool VDelegateInit(TiXmlElement* pData) override;
    virtual shared_ptr<SceneNode> VCreateSceneNode(void) override;  // factory method to create the appropriate scene node

    // editor stuff
    virtual void VCreateInheritedXmlElements(TiXmlElement* pBaseElement);
};
