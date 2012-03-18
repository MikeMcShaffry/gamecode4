#pragma once
//========================================================================
// BaseScriptComponent.h - Component for exposing actors to Lua
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

#include "ScriptComponentInterface.h"
#include "LuaPlus.h"


//---------------------------------------------------------------------------------------------------------------------
// BaseScriptComponent class
// Chapter 12, page 337
//---------------------------------------------------------------------------------------------------------------------
class BaseScriptComponent : public ScriptComponentInterface
{
	std::string m_scriptObjectName;
    std::string m_constructorName;
    std::string m_destructorName;

	LuaPlus::LuaObject m_scriptObject;
    LuaPlus::LuaObject m_scriptConstructor;
	LuaPlus::LuaObject m_scriptDestructor;
	
public:
	static const char* g_Name;
	virtual const char* VGetName() const { return g_Name; }

	BaseScriptComponent(void);
	virtual ~BaseScriptComponent(void);
	virtual bool VInit(TiXmlElement* pData);
    virtual void VPostInit(void);
    virtual TiXmlElement* VGenerateXml(void);
	
	static void RegisterScriptFunctions(void);
	static void UnregisterScriptFunctions(void);
	
private:
	void CreateScriptObject(void);
	
	// component script functions
	LuaPlus::LuaObject GetActorId(void);
	
	// physics component script functions
	LuaPlus::LuaObject GetPos(void);
    void SetPos(LuaPlus::LuaObject newPos);
    LuaPlus::LuaObject GetLookAt(void) const;
    float GetYOrientationRadians(void) const;
	void RotateY(float angleRadians);
	void SetPosition(float x, float y, float z);
	void LookAt(Vec3 target);
    void Stop(void);
};

