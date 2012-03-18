
#pragma once
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
//  Content References in Game Coding Complete 3rd Edition
// 
//  struct ActorParams				- Chapter 19, page 683
//  struct SphereParams				- Chapter 19, page 738
//  struct TeapotParams				- Chapter 19, page 690




/****************************************

IMPORTANT GROUND RULES FOR DEFINING ACTOR PARAMETERS!!!!

1. No destructors.
2. No pointers.
3. Never break rules one, two, or three.

Why? These parameters are marshalled over god knows what. 
Could be a function call - or it could be the internet.

****************************************/

#include <strstream>

// TODO Rez - Need to bring LUA back
//#include "LuaPlus/LuaObject.h"

class BaseActor : public IActor
{
	friend class BaseGameLogic;
protected:
	ActorId m_id;
	Mat4x4 m_Mat;
	int m_Type;
	shared_ptr<ActorParams> m_Params;


	virtual void VSetID(ActorId id) { m_id = id; } 
	virtual void VSetMat(const Mat4x4 &newMat) { m_Mat = newMat; }

public:
	BaseActor(Mat4x4 mat, int type, shared_ptr<ActorParams> params)
		{ m_Mat=mat; m_Type=type; m_Params=params; }

	virtual Mat4x4 const &VGetMat() { return m_Mat; }			// virtual dumb here?
	virtual const int VGetType() { return m_Type; }				// virtual dumb here?
	virtual ActorId VGetID() { return m_id; }			// virtual dumb here?
	shared_ptr<ActorParams> VGetParams() { return m_Params; }
	virtual bool VIsPhysical() { return true; }
	virtual bool VIsGeometrical() { return true; }
	virtual void VOnUpdate(int deltaMilliseconds) { }
	virtual void VRotateY(float angleRadians);
};



#define VIEWID_NO_VIEW_ATTACHED	(0)

//TODO Boy I hope we can get rid of ActorTypes!
enum ActorType
{
	AT_Unknown,
	AT_Sphere,
	AT_Teapot,
	AT_TestObject,
	AT_Grid,
	AT_GenericMeshObject
};


class BaseGameLogic;
class SceneNode;


struct ActorParams
{
	int m_Size;
	optional<ActorId> m_Id;
	Vec3 m_Pos;
	ActorType m_Type;
	Color m_Color;
	static const int sk_MaxFuncName = 64;
	char m_OnCreateLuaFunctionName[ sk_MaxFuncName ];	//Lua function name to call for this actor, upon creation.
	char m_OnDestroyLuaFunctionName[ sk_MaxFuncName ];	//As above, but on destruction.

	ActorParams(); 

	virtual ~ActorParams() { }

	virtual bool VInit(std::istrstream &in);
	virtual void VSerialize(std::ostrstream &out) const;

	typedef std::deque< std::string > TErrorMessageList;

	//TODO LUA
	//virtual bool VInit( LuaObject srcData, TErrorMessageList & errorMessages );

	static ActorParams *CreateFromStream(std::istrstream &in);
	//static ActorParams *CreateFromLuaObj( LuaObject srcData );

	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic) { shared_ptr<IActor> p; return p; }
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene) { shared_ptr<SceneNode> p; return p; }
};


struct SphereParams : public ActorParams
{
	float m_Radius;
	int m_Segments;
	Vec3 m_NormalDir;
	float m_Force;

	SphereParams();

	virtual bool VInit(std::istrstream &in);
	//TODO LUA
	//virtual bool VInit( LuaObject srcData, TErrorMessageList & errorMessages );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct TeapotParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	Mat4x4 m_Mat;

	TeapotParams();

	virtual bool VInit(std::istrstream &in);
	//TODO LUA
	//virtual bool VInit( LuaObject srcData, TErrorMessageList & errorMessages );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct AiTeapotParams : public ActorParams
{
	float m_Length;
	GameViewId m_ViewId;
	Mat4x4 m_Mat;

	AiTeapotParams(); 

	virtual bool VInit(std::istrstream &in);
	//TODO LUA
	//virtual bool VInit( LuaObject srcData, TErrorMessageList & errorMessages );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct TestObjectParams : public ActorParams
{
	bool m_Squashed;
	Mat4x4 m_Mat;

	TestObjectParams();

	virtual bool VInit(std::istrstream &in);
	//TODO LUA
	//virtual bool VInit( LuaObject srcData, TErrorMessageList & errorMessages );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};


struct GridParams : public ActorParams
{
	static const int sk_MaxTextureNameLen = _MAX_PATH;
	char m_Texture[sk_MaxTextureNameLen];
	unsigned int m_Squares;
	Mat4x4 m_Mat;

	GridParams();

	virtual bool VInit(std::istrstream &in);
	//TODO LUA
	//virtual bool VInit( LuaObject srcData, TErrorMessageList & errorMessages );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};

struct GenericMeshObjectParams : public ActorParams
{
	static const int sk_MaxFileNameLen = _MAX_PATH;
	char m_XFileName[sk_MaxFileNameLen];
	char m_FXFileName[sk_MaxFileNameLen];
	Mat4x4 m_Mat;

	GenericMeshObjectParams();

	virtual bool VInit(std::istrstream &in);
	//TODO LUA
	//virtual bool VInit( LuaObject srcData, TErrorMessageList & errorMessages );
	virtual void VSerialize(std::ostrstream &out) const;
	virtual shared_ptr<IActor> VCreate(BaseGameLogic *logic);
	virtual shared_ptr<SceneNode> VCreateSceneNode(shared_ptr<Scene> pScene);
};
