//========================================================================
// TransformComponent.cpp - Component for managing transforms on actors
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
#include "TransformComponent.h"
#include "Math.h"
#include "../Utilities/String.h"

const char* TransformComponent::g_Name = "TransformComponent";

bool TransformComponent::VInit(TiXmlElement* pData)
{
    GCC_ASSERT(pData);

	// [mrmike] - this was changed post-press - because changes to the TransformComponents can come in partial definitions,
	//            such as from the editor, its better to grab the current values rather than clear them out.
    
	Vec3 yawPitchRoll = m_transform.GetYawPitchRoll();
	yawPitchRoll.x = RADIANS_TO_DEGREES(yawPitchRoll.x);
	yawPitchRoll.y = RADIANS_TO_DEGREES(yawPitchRoll.y);
	yawPitchRoll.z = RADIANS_TO_DEGREES(yawPitchRoll.z);

	Vec3 position = m_transform.GetPosition();	

    TiXmlElement* pPositionElement = pData->FirstChildElement("Position");
    if (pPositionElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pPositionElement->Attribute("x", &x);
        pPositionElement->Attribute("y", &y);
        pPositionElement->Attribute("z", &z);
        position = Vec3(x, y, z);
    }

    TiXmlElement* pOrientationElement = pData->FirstChildElement("YawPitchRoll");
    if (pOrientationElement)
    {
        double yaw = 0;
        double pitch = 0;
        double roll = 0;
        pOrientationElement->Attribute("x", &yaw);
        pOrientationElement->Attribute("y", &pitch);
        pOrientationElement->Attribute("z", &roll);
		yawPitchRoll = Vec3(yaw, pitch, roll);
	}

	Mat4x4 translation;
	translation.BuildTranslation(position);

	Mat4x4 rotation;
	rotation.BuildYawPitchRoll((float)DEGREES_TO_RADIANS(yawPitchRoll.x), (float)DEGREES_TO_RADIANS(yawPitchRoll.y), (float)DEGREES_TO_RADIANS(yawPitchRoll.z));

	/**
	// This is not supported yet.
    TiXmlElement* pLookAtElement = pData->FirstChildElement("LookAt");
    if (pLookAtElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pLookAtElement->Attribute("x", &x);
        pLookAtElement->Attribute("y", &y);
        pLookAtElement->Attribute("z", &z);

		Vec3 lookAt((float)x, (float)y, (float)z);
		rotation.BuildRotationLookAt(translation.GetPosition(), lookAt, g_Up);
    }

    TiXmlElement* pScaleElement = pData->FirstChildElement("Scale");
    if (pScaleElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pScaleElement->Attribute("x", &x);
        pScaleElement->Attribute("y", &y);
        pScaleElement->Attribute("z", &z);
        m_scale = Vec3((float)x, (float)y, (float)z);
    }
	**/

    m_transform = rotation * translation;
    
    return true;
}

TiXmlElement* TransformComponent::VGenerateXml(void)
{
    TiXmlElement* pBaseElement = GCC_NEW TiXmlElement(VGetName());

    // initial transform -> position
    TiXmlElement* pPosition = GCC_NEW TiXmlElement("Position");
    Vec3 pos(m_transform.GetPosition());
    pPosition->SetAttribute("x", ToStr(pos.x).c_str());
    pPosition->SetAttribute("y", ToStr(pos.y).c_str());
    pPosition->SetAttribute("z", ToStr(pos.z).c_str());
    pBaseElement->LinkEndChild(pPosition);

    // initial transform -> LookAt
    TiXmlElement* pDirection = GCC_NEW TiXmlElement("YawPitchRoll");
	Vec3 orient(m_transform.GetYawPitchRoll());
	orient.x = RADIANS_TO_DEGREES(orient.x);
	orient.y = RADIANS_TO_DEGREES(orient.y);
	orient.z = RADIANS_TO_DEGREES(orient.z);
    pDirection->SetAttribute("x", ToStr(orient.x).c_str());
    pDirection->SetAttribute("y", ToStr(orient.y).c_str());
    pDirection->SetAttribute("z", ToStr(orient.z).c_str());
    pBaseElement->LinkEndChild(pDirection);

	/***
	// This is not supported yet
    // initial transform -> position
    TiXmlElement* pScale = GCC_NEW TiXmlElement("Scale");
    pPosition->SetAttribute("x", ToStr(m_scale.x).c_str());
    pPosition->SetAttribute("y", ToStr(m_scale.y).c_str());
    pPosition->SetAttribute("z", ToStr(m_scale.z).c_str());
    pBaseElement->LinkEndChild(pScale);
	**/

    return pBaseElement;
}



