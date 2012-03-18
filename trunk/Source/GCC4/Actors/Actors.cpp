
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

#include "GameCodeStd.h"

#include <strstream>
#include "../GameCode4/GameCode.h"
//#include "LuaPlus/LuaObject.h"
#include "../Actors/Actors.h"
#include "../Physics/Physics.h"
#include "../Graphics3D/SceneNodes.h"


ActorParams::ActorParams() 
{
	m_Pos=Vec3(0,0,0); 
	m_Type=AT_Unknown; 
	m_Color = g_White;
	//strcpy( m_OnCreateLuaFunctionName, "" );
	//strcpy( m_OnDestroyLuaFunctionName, "" );
	m_Size=sizeof(ActorParams); 
}

//
// ActorParams::VInit					- Chapter 19, page 684
//
bool ActorParams::VInit(std::istrstream &in)
{ 
	int hasActorId = 0;

	in >> m_Size;
	in >> hasActorId;
	if (hasActorId)
	{
		in >> hasActorId;
		m_Id = hasActorId;
	}
	in >> m_Pos.x >> m_Pos.y >> m_Pos.z;
	in >> m_Color.r >> m_Color.g >> m_Color.b >> m_Color.a;
	in >> m_OnCreateLuaFunctionName;
	in >> m_OnDestroyLuaFunctionName;
	return true;
}

/************
//
// ActorParams::VInit						- Chpater 19, page 685
//
bool ActorParams::VInit( LuaObject srcData, TErrorMessageList & errorMessages )
{
	LuaObject actorIDObj = srcData[ "ActorID" ];
	if ( actorIDObj.IsInteger() )
	{
		m_Id = actorIDObj.GetInteger();
	}


	LuaObject posObj = srcData[ "Pos" ];
	if ( posObj.IsTable() )
	{
		const int tableCount = posObj.GetTableCount();
		if ( 3 != tableCount )
		{
			const std::string err( "Incorrect number of parameters in the 'Pos' member." );
			errorMessages.push_back( err );
			return false;
		}
		else
		{
			//Get the three values.
			m_Pos.x = posObj[ 1 ].GetFloat();
			m_Pos.y = posObj[ 2 ].GetFloat();
			m_Pos.z = posObj[ 3 ].GetFloat();
		}
	}

	LuaObject colorObj = srcData[ "Color" ];
	if ( colorObj.IsTable() )
	{
		//Get the RGBA off of it.
		LuaObject r = colorObj[ "R" ];
		if ( r.IsNumber() )
		{
			m_Color.r = r.GetFloat();
		}
		LuaObject g = colorObj[ "G" ];
		if ( g.IsNumber() )
		{
			m_Color.g = g.GetFloat();
		}
		LuaObject b = colorObj[ "B" ];
		if ( b.IsNumber() )
		{
			m_Color.b = b.GetFloat();
		}
		LuaObject a = colorObj[ "A" ];
		if ( a.IsNumber() )
		{
			m_Color.a = a.GetFloat();
		}
	}

	//See if we have any on create/destroy handlers.
	LuaObject onCreateObj = srcData[ "OnCreateFunc" ];
	if ( onCreateObj.IsString() )
	{
		const char * pString = onCreateObj.GetString();
		strcpy_s( m_OnCreateLuaFunctionName, sk_MaxFuncName, onCreateObj.GetString() );
	}
	LuaObject onDestroyObj = srcData[ "OnDestroyFunc" ];
	if ( onDestroyObj.IsString() )
	{
		strcpy_s( m_OnDestroyLuaFunctionName, sk_MaxFuncName, onDestroyObj.GetString() );
	}

	return true;
}
***************************/


//
// ActorParams::VSerialize					- Chapter 19, 685
//
void ActorParams::VSerialize(std::ostrstream &out) const
{
	out << m_Type << " ";
	out << m_Size << " ";
	out << static_cast<int>(m_Id.valid()) << " ";
	if (m_Id.valid())
	{
		out << *m_Id << " ";
	}
	out << m_Pos.x << " " << m_Pos.y << " " << m_Pos.z << " ";
	out << m_Color.r << " " << m_Color.g << " " << m_Color.b << " " << m_Color.a << " ";
	out << m_OnCreateLuaFunctionName << " ";
	out << m_OnDestroyLuaFunctionName << " ";
}





SphereParams::SphereParams() 
: ActorParams()
{
	m_Type=AT_Sphere; 
	m_Radius=1.0f; 
	m_Segments=16; 
	m_Size=sizeof(SphereParams);
	m_NormalDir= Vec3( 0.0f, 0.0f, 0.0f );
	m_Force=0.0f;
}

bool SphereParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Sphere;
		in >> m_Radius;
		in >> m_Segments;
		in >> m_NormalDir.x >> m_NormalDir.y >> m_NormalDir.y;
		in >> m_Force;
		return true;
	}
	return false;
}

/*********
bool SphereParams::VInit( LuaObject srcData, TErrorMessageList & errorMessages )
{
	if ( false == ActorParams::VInit( srcData, errorMessages ) )
	{
		return false;
	}

	m_Type = AT_Sphere;

	LuaObject radiusObj = srcData[ "Radius" ];
	if ( radiusObj.IsNumber() )
	{
		m_Radius = radiusObj.GetFloat();
	}
	LuaObject segmentObj = srcData[ "Segments" ];
	if ( segmentObj.IsInteger() )
	{
		m_Segments = segmentObj.GetInteger();
	}
	LuaObject normalObj = srcData[ "NormalDir" ];
	if ( normalObj.IsTable() )
	{
		const int tableCount = normalObj.GetTableCount();
		if ( 3 != tableCount )
		{
			const std::string err( "Incorrect number of parameters in the 'NormalDir' member." );
			errorMessages.push_back( err );
			return false;
		}
		else
		{
			//Get the three values.
			m_NormalDir.x = normalObj[ 1 ].GetFloat();
			m_NormalDir.y = normalObj[ 2 ].GetFloat();
			m_NormalDir.z = normalObj[ 3 ].GetFloat();
		}
	}
	LuaObject forceObj = srcData[ "Force" ];
	if ( forceObj.IsNumber() )
	{
		m_Force = forceObj.GetFloat();
	}

	return true;
}
**************************/


void SphereParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Radius << " ";
	out << m_Segments << " ";
	out << m_Color.r << " " << m_Color.g << " " << m_Color.b << " " << m_Color.a << " ";
	out << m_NormalDir.x << " " << m_NormalDir.y << " " << m_NormalDir.z << " ";
	out << m_Force << " ";
}

shared_ptr<IActor> SphereParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat.BuildTranslation(m_Pos);
	shared_ptr<IActor> pSphere(new BaseActor(mat, AT_Sphere, shared_ptr<SphereParams>(GCC_NEW SphereParams(*this))));
	logic->VAddActor(pSphere, this);
	logic->VGetGamePhysics()->VAddSphere(m_Radius, &*pSphere, SpecificGravity(PhysDens_Pine), PhysMat_Bouncy);
	logic->VGetGamePhysics()->VApplyForce(m_NormalDir, m_Force, *( m_Id ));
	return pSphere;
}




TeapotParams::TeapotParams() 
  : ActorParams()
{ 
	m_Type=AT_Teapot; 
	m_Length=1.0f; 
	m_ViewId = VIEWID_NO_VIEW_ATTACHED;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(TeapotParams); 
}

bool TeapotParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Teapot; 
		in >> m_Length;
		in >> m_ViewId;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];

		return true;
	}
	return false;
}

//
// TeapotParams::VInit					- Chapter 19, page 691
//

/*********************
bool TeapotParams::VInit( LuaObject srcData, TErrorMessageList & errorMessages )
{
	if ( false == ActorParams::VInit( srcData, errorMessages ) )
	{
		return false;
	}

	m_Type = AT_Teapot;

	LuaObject lengthObj = srcData[ "Length" ];
	if ( lengthObj.IsNumber() )
	{
		m_Length = lengthObj.GetFloat();
	}

	m_Mat = Mat4x4::g_Identity;

	LuaObject matObj = srcData[ "Mat" ];
	if ( matObj.IsTable() )
	{
		const int tableCount = matObj.GetTableCount();
		if ( 16 != tableCount )
		{
			const std::string err( "Incorrect number of parameters in the 'Mat' member." );
			errorMessages.push_back( err );
			return false;
		}
		else
		{
			LuaObject m11 = matObj[ "_11" ];
			if ( m11.IsNumber() )
			{
				m_Mat._11 = m11.GetFloat();
			}
			LuaObject m12 = matObj[ "_12" ];
			if ( m12.IsNumber() )
			{
				m_Mat._12 = m12.GetFloat();
			}
			LuaObject m13 = matObj[ "_13" ];
			if ( m13.IsNumber() )
			{
				m_Mat._13 = m13.GetFloat();
			}
			LuaObject m14 = matObj[ "_14" ];
			if ( m14.IsNumber() )
			{
				m_Mat._14 = m14.GetFloat();
			}
			LuaObject m21 = matObj[ "_21" ];
			if ( m21.IsNumber() )
			{
				m_Mat._21 = m21.GetFloat();
			}
			LuaObject m22 = matObj[ "_22" ];
			if ( m22.IsNumber() )
			{
				m_Mat._22 = m22.GetFloat();
			}
			LuaObject m23 = matObj[ "_23" ];
			if ( m23.IsNumber() )
			{
				m_Mat._23 = m23.GetFloat();
			}
			LuaObject m24 = matObj[ "_24" ];
			if ( m24.IsNumber() )
			{
				m_Mat._24 = m24.GetFloat();
			}
			LuaObject m31 = matObj[ "_31" ];
			if ( m31.IsNumber() )
			{
				m_Mat._31 = m31.GetFloat();
			}
			LuaObject m32 = matObj[ "_32" ];
			if ( m32.IsNumber() )
			{
				m_Mat._32 = m32.GetFloat();
			}
			LuaObject m33 = matObj[ "_33" ];
			if ( m33.IsNumber() )
			{
				m_Mat._33 = m33.GetFloat();
			}
			LuaObject m34 = matObj[ "_34" ];
			if ( m34.IsNumber() )
			{
				m_Mat._34 = m34.GetFloat();
			}
			LuaObject m41 = matObj[ "_41" ];
			if ( m41.IsNumber() )
			{
				m_Mat._41 = m41.GetFloat();
			}
			LuaObject m42 = matObj[ "_42" ];
			if ( m42.IsNumber() )
			{
				m_Mat._42 = m42.GetFloat();
			}
			LuaObject m43 = matObj[ "_43" ];
			if ( m43.IsNumber() )
			{
				m_Mat._43 = m43.GetFloat();
			}
			LuaObject m44 = matObj[ "_44" ];
			if ( m44.IsNumber() )
			{
				m_Mat._44 = m44.GetFloat();
			}
		}
	}


	LuaObject viewIDObj = srcData[ "GameViewID" ];
	if ( viewIDObj.IsInteger() )
	{
		m_ViewId = viewIDObj.GetInteger();
	}

	return true;
}
****************************/



void TeapotParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Length << " ";
	out << m_ViewId;
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";

}

//
// TeapotParams::VCreate				- Chapter 19, page 693
//
shared_ptr<IActor> TeapotParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pTeapot(new BaseActor(mat, AT_Teapot, shared_ptr<TeapotParams>(GCC_NEW TeapotParams(*this))));
	logic->VAddActor(pTeapot, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Length, m_Length/3, m_Length), &*pTeapot, SpecificGravity(PhysDens_Water)*.8f, PhysMat_Normal);
	return pTeapot;
}




TestObjectParams::TestObjectParams() 
{ 
	m_Type=AT_TestObject; 
	m_Color=g_White; 
	m_Squashed=true;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(TestObjectParams); 
}

bool TestObjectParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_TestObject; 
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];

		return true;
	}
	return false;
}


/*******************
bool TestObjectParams::VInit( LuaObject srcData, TErrorMessageList & errorMessages )
{
	if ( false == ActorParams::VInit( srcData, errorMessages ) )
	{
		return false;
	}

	m_Type = AT_TestObject;

	m_Mat = Mat4x4::g_Identity;

	LuaObject matObj = srcData[ "Mat" ];
	if ( matObj.IsTable() )
	{
		const int tableCount = matObj.GetTableCount();
		if ( 16 != tableCount )
		{
			const std::string err( "Incorrect number of parameters in the 'Mat' member." );
			errorMessages.push_back( err );
			return false;
		}
		else
		{
			LuaObject m11 = matObj[ "_11" ];
			if ( m11.IsNumber() )
			{
				m_Mat._11 = m11.GetFloat();
			}
			LuaObject m12 = matObj[ "_12" ];
			if ( m12.IsNumber() )
			{
				m_Mat._12 = m12.GetFloat();
			}
			LuaObject m13 = matObj[ "_13" ];
			if ( m13.IsNumber() )
			{
				m_Mat._13 = m13.GetFloat();
			}
			LuaObject m14 = matObj[ "_14" ];
			if ( m14.IsNumber() )
			{
				m_Mat._14 = m14.GetFloat();
			}
			LuaObject m21 = matObj[ "_21" ];
			if ( m21.IsNumber() )
			{
				m_Mat._21 = m21.GetFloat();
			}
			LuaObject m22 = matObj[ "_22" ];
			if ( m22.IsNumber() )
			{
				m_Mat._22 = m22.GetFloat();
			}
			LuaObject m23 = matObj[ "_23" ];
			if ( m23.IsNumber() )
			{
				m_Mat._23 = m23.GetFloat();
			}
			LuaObject m24 = matObj[ "_24" ];
			if ( m24.IsNumber() )
			{
				m_Mat._24 = m24.GetFloat();
			}
			LuaObject m31 = matObj[ "_31" ];
			if ( m31.IsNumber() )
			{
				m_Mat._31 = m31.GetFloat();
			}
			LuaObject m32 = matObj[ "_32" ];
			if ( m32.IsNumber() )
			{
				m_Mat._32 = m32.GetFloat();
			}
			LuaObject m33 = matObj[ "_33" ];
			if ( m33.IsNumber() )
			{
				m_Mat._33 = m33.GetFloat();
			}
			LuaObject m34 = matObj[ "_34" ];
			if ( m34.IsNumber() )
			{
				m_Mat._34 = m34.GetFloat();
			}
			LuaObject m41 = matObj[ "_41" ];
			if ( m41.IsNumber() )
			{
				m_Mat._41 = m41.GetFloat();
			}
			LuaObject m42 = matObj[ "_42" ];
			if ( m42.IsNumber() )
			{
				m_Mat._42 = m42.GetFloat();
			}
			LuaObject m43 = matObj[ "_43" ];
			if ( m43.IsNumber() )
			{
				m_Mat._43 = m43.GetFloat();
			}
			LuaObject m44 = matObj[ "_44" ];
			if ( m44.IsNumber() )
			{
				m_Mat._44 = m44.GetFloat();
			}
		}
	}

	return true;
}
********************/

void TestObjectParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";

}

shared_ptr<IActor> TestObjectParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pTestObject(new BaseActor(mat, AT_TestObject, shared_ptr<TestObjectParams>(GCC_NEW TestObjectParams(*this))));
	logic->VAddActor(pTestObject, this);
	//m_pPhysics->VAddPointCloud(TestObject::g_TestObjectVerts, 8, &*pTestObject, SpecificGravity(PhysDens_Pine), PhysMat_Normal);
	return pTestObject;
}




GridParams::GridParams() : ActorParams()
{ 
	m_Type=AT_Grid; 
	m_Color=g_Gray40; 
	strcpy_s( m_Texture, sk_MaxTextureNameLen, "grid.dds" );
	m_Squares=10;
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(GridParams); 
}

 bool GridParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_Grid; 
		in >> m_Texture;
		in >> m_Squares;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];
		return true;
	}
	return false;
}

/*********************
bool GridParams::VInit( LuaObject srcData, TErrorMessageList & errorMessages )
{
	if ( false == ActorParams::VInit( srcData, errorMessages ) )
	{
		return false;
	}

	m_Type = AT_Grid;

	m_Mat = Mat4x4::g_Identity;

	LuaObject matObj = srcData[ "Mat" ];
	if ( matObj.IsTable() )
	{
		const int tableCount = matObj.GetTableCount();
		if ( 16 != tableCount )
		{
			const std::string err( "Incorrect number of parameters in the 'Mat' member." );
			errorMessages.push_back( err );
			return false;
		}
		else
		{
			LuaObject m11 = matObj[ "_11" ];
			if ( m11.IsNumber() )
			{
				m_Mat._11 = m11.GetFloat();
			}
			LuaObject m12 = matObj[ "_12" ];
			if ( m12.IsNumber() )
			{
				m_Mat._12 = m12.GetFloat();
			}
			LuaObject m13 = matObj[ "_13" ];
			if ( m13.IsNumber() )
			{
				m_Mat._13 = m13.GetFloat();
			}
			LuaObject m14 = matObj[ "_14" ];
			if ( m14.IsNumber() )
			{
				m_Mat._14 = m14.GetFloat();
			}
			LuaObject m21 = matObj[ "_21" ];
			if ( m21.IsNumber() )
			{
				m_Mat._21 = m21.GetFloat();
			}
			LuaObject m22 = matObj[ "_22" ];
			if ( m22.IsNumber() )
			{
				m_Mat._22 = m22.GetFloat();
			}
			LuaObject m23 = matObj[ "_23" ];
			if ( m23.IsNumber() )
			{
				m_Mat._23 = m23.GetFloat();
			}
			LuaObject m24 = matObj[ "_24" ];
			if ( m24.IsNumber() )
			{
				m_Mat._24 = m24.GetFloat();
			}
			LuaObject m31 = matObj[ "_31" ];
			if ( m31.IsNumber() )
			{
				m_Mat._31 = m31.GetFloat();
			}
			LuaObject m32 = matObj[ "_32" ];
			if ( m32.IsNumber() )
			{
				m_Mat._32 = m32.GetFloat();
			}
			LuaObject m33 = matObj[ "_33" ];
			if ( m33.IsNumber() )
			{
				m_Mat._33 = m33.GetFloat();
			}
			LuaObject m34 = matObj[ "_34" ];
			if ( m34.IsNumber() )
			{
				m_Mat._34 = m34.GetFloat();
			}
			LuaObject m41 = matObj[ "_41" ];
			if ( m41.IsNumber() )
			{
				m_Mat._41 = m41.GetFloat();
			}
			LuaObject m42 = matObj[ "_42" ];
			if ( m42.IsNumber() )
			{
				m_Mat._42 = m42.GetFloat();
			}
			LuaObject m43 = matObj[ "_43" ];
			if ( m43.IsNumber() )
			{
				m_Mat._43 = m43.GetFloat();
			}
			LuaObject m44 = matObj[ "_44" ];
			if ( m44.IsNumber() )
			{
				m_Mat._44 = m44.GetFloat();
			}
		}
	}

	LuaObject textureObj = srcData[ "Texture" ];
	if ( textureObj.IsString() )
	{
		const char * pTexture = textureObj.GetString();
		strcpy( m_Texture, pTexture );
	}
	LuaObject squaresObj = srcData[ "Squares" ];
	if ( squaresObj.IsInteger() )
	{
		const int tempVal = squaresObj.GetInteger();
		if ( tempVal < 0 )
		{
			errorMessages.push_back( "Attempted to build a Grid actor with 'Squares' member < 0!" );
			return false;
		}
		m_Squares = (unsigned int)tempVal;
	}

	return true;
}
************************************/


void GridParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_Color.r << " " << m_Color.g << " " << m_Color.b << " " << m_Color.a << " ";
	out << m_Texture;
	out << m_Squares;
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";
}

shared_ptr<IActor> GridParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pGrid(new BaseActor(mat, AT_Grid, shared_ptr<GridParams>(GCC_NEW GridParams(*this))));
	logic->VAddActor(pGrid, this);
	logic->VGetGamePhysics()->VAddBox(Vec3(m_Squares/2.0f, 0.01f, m_Squares/2.0f), &*pGrid, SpecificGravity(PhysDens_Infinite), PhysMat_Slippery);
	return pGrid;
}


GenericMeshObjectParams::GenericMeshObjectParams() : ActorParams()
{
	m_Type=AT_GenericMeshObject; 
	m_Color=g_Gray40; 
	strcpy_s( m_XFileName, sk_MaxFileNameLen, "" );
	strcpy_s( m_FXFileName, sk_MaxFileNameLen, "" );
	m_Mat=Mat4x4::g_Identity;
	m_Size=sizeof(GenericMeshObjectParams); 
}

bool GenericMeshObjectParams::VInit(std::istrstream &in)
{
	if (ActorParams::VInit(in))
	{
		m_Type=AT_GenericMeshObject; 
		in >> m_XFileName;
		in >> m_FXFileName;
		for (int i=0; i<4; ++i)
			for (int j=0; j<4; ++j)
				in >> m_Mat.m[i][j];
		return true;
	}
	return false;
}


/**************************
bool GenericMeshObjectParams::VInit( LuaObject srcData, TErrorMessageList & errorMessages )
{
	if ( false == ActorParams::VInit( srcData, errorMessages ) )
	{
		return false;
	}

	m_Type = AT_GenericMeshObject;

	m_Mat = Mat4x4::g_Identity;

	LuaObject matObj = srcData[ "Mat" ];
	if ( matObj.IsTable() )
	{
		const int tableCount = matObj.GetTableCount();
		if ( 16 != tableCount )
		{
			const std::string err( "Incorrect number of parameters in the 'Mat' member." );
			errorMessages.push_back( err );
			return false;
		}
		else
		{
			char name[4] = "_00";

			for( int i = 1; i <= 4; ++i ) 
			{
				name[1] = '0' + i;

				for( int j = 1; j <= 4; ++j ) 
				{
					name[2] = '0' + j;

					LuaObject entry = matObj[ name ];
					if( entry.IsNumber() ) 
					{
						m_Mat.m[i - 1][j - 1] = entry.GetFloat();
					}
				}
			}
		}
	}

	LuaObject XFileObj = srcData[ "XFile" ];
	if ( XFileObj.IsString() )
	{
		const char * pXFile = XFileObj.GetString();
		strcpy( m_XFileName, pXFile );
	}

	LuaObject FXFileObj = srcData[ "FXFile" ];
	if ( FXFileObj.IsString() )
	{
		const char * pFXFile = FXFileObj.GetString();
		strcpy( m_FXFileName, pFXFile );
	}

	return true;
}
*****************************************/


void GenericMeshObjectParams::VSerialize(std::ostrstream &out) const
{
	ActorParams::VSerialize(out);
	out << m_XFileName;
	out << m_FXFileName;
	for (int i=0; i<4; ++i)
		for (int j=0; j<4; ++j)
			out << m_Mat.m[i][j] << " ";
}

shared_ptr<IActor> GenericMeshObjectParams::VCreate(BaseGameLogic *logic)
{
	Mat4x4 mat;
	mat = m_Mat;
	shared_ptr<IActor> pGenericMeshObject(new BaseActor(mat, AT_GenericMeshObject, shared_ptr<GenericMeshObjectParams>(GCC_NEW GenericMeshObjectParams(*this))));
	logic->VAddActor(pGenericMeshObject, this);
	//m_pPhysics->VAddBox(Vec3(p.m_Squares/2.0f, 0.01f, p.m_Squares/2.0f), &*pGrid, SpecificGravity(PhysDens_Infinite), PhysMat_Slippery);
	return pGenericMeshObject;
}




shared_ptr<SceneNode> GridParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> grid(GCC_NEW Grid(m_Id, "Grid", *this));
	//m_pScene->VAddChild(p.m_Id, grid);
	//grid->VOnRestore(&*m_pScene);
	return grid;
}


//
// TeapotWarsGameView::CreateSphere				- Chapter 19, page 759
//
shared_ptr<SceneNode> SphereParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	ID3DXMesh *sphereMesh;
	if ( SUCCEEDED( 
		D3DXCreateSphere( 
		DXUTGetD3D9Device(), m_Radius, m_Segments, m_Segments, &sphereMesh, NULL) ) )
	{
		Mat4x4 trans;
		trans.BuildTranslation(m_Pos);
		shared_ptr<SceneNode> sphere(GCC_NEW ShaderMeshNode(m_Id, "Sphere", sphereMesh, L"GameCode4.fx", RenderPass_Actor, &trans, m_Color));
		//m_pScene->VAddChild(p.m_Id, sphere);

		SAFE_RELEASE(sphereMesh);
		return sphere;
	}

	shared_ptr<SceneNode> nul;
	return nul;
}


//
// TeapotParams::VCreateSceneNode				- Chapter 19, page 693
//
shared_ptr<SceneNode> TeapotParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> teapot(GCC_NEW TeapotMeshNode(m_Id, "Teapot", L"GameCode4.fx", RenderPass_Actor, &m_Mat, m_Color));
	return teapot;
}

//
// TeapotWarsGameView::CreateAiTeapot				- Chapter 19
//
shared_ptr<SceneNode> AiTeapotParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> teapot(GCC_NEW TeapotMeshNode(m_Id, "AiTeapot", L"GameCode4.fx", RenderPass_Actor, &m_Mat, m_Color));
	return teapot;
}


//
// TeapotWarsGameView::CreateTestObject	
//
shared_ptr<SceneNode> TestObjectParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	shared_ptr<SceneNode> testObject(GCC_NEW TestObject(*this, "TestObject"));
	//m_pScene->VAddChild(p.m_Id, testObject);
	//testObject->VOnRestore(&*m_pScene);
	return testObject;
}



shared_ptr<SceneNode> GenericMeshObjectParams::VCreateSceneNode(shared_ptr<Scene> pScene)
{
	size_t origsize = strlen(m_XFileName) + 1;
	const size_t newsize = GenericMeshObjectParams::sk_MaxFileNameLen ;
    size_t convertedChars = 0;
    wchar_t meshFileName[newsize];
    mbstowcs_s(&convertedChars, meshFileName, origsize, m_XFileName, _TRUNCATE);
	
	origsize = strlen(m_FXFileName) + 1;
	wchar_t effectFileName[newsize];
	mbstowcs_s(&convertedChars, effectFileName, origsize, m_FXFileName, _TRUNCATE);

	shared_ptr<SceneNode> meshFile(GCC_NEW ShaderMeshNode(m_Id, "MeshFile", meshFileName, effectFileName, RenderPass_Actor, &m_Mat, m_Color ));
	return meshFile;
}


