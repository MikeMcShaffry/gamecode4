//========================================================================
// PhysicsDebugDrawer.cpp - implements a physics debug drawer in DX9 
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

#include "PhysicsDebugDrawer.h"
#include "../UserInterface/HumanView.h"

void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	// draw a line to represent the normal.  This only lasts one frame, and is hard to see.
	//   it might help to linger this drawn object onscreen for a while to make it more noticeable
	
	btVector3 const startPoint = PointOnB;
	btVector3 const endPoint = PointOnB + normalOnB * distance;
	
	drawLine( startPoint, endPoint, color );
}

void BulletDebugDrawer::reportErrorWarning(const char* warningString)
{
    GCC_WARNING(warningString);
}

void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	// FUTURE WORK - BulletDebugDrawer::draw3dText needs an implementation
}

void BulletDebugDrawer::setDebugMode(int debugMode)
{
    m_DebugModes = (DebugDrawModes) debugMode;
}

int BulletDebugDrawer::getDebugMode() const
{
    return m_DebugModes;
}

void BulletDebugDrawer::ReadOptions()
{
	TiXmlDocument *optionsDoc = g_pApp->m_Options.m_pDoc;
	TiXmlElement *pRoot = optionsDoc->RootElement();
    if (!pRoot)
		return;

	int debugModes = btIDebugDraw::DBG_NoDebug;
	TiXmlElement *pNode = pRoot->FirstChildElement("PhysicsDebug");
	if (pNode)
	{
		if (pNode->Attribute("DrawWireFrame"))
		{
	        std::string attribute(pNode->Attribute("DrawWireFrame"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DrawWireframe;
		}

		if (pNode->Attribute("DrawAabb"))
		{
	        std::string attribute(pNode->Attribute("DrawAabb"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DrawAabb;
		}

		if (pNode->Attribute("DrawFeaturesText"))
		{
	        std::string attribute(pNode->Attribute("DrawFeaturesText"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DrawFeaturesText;
		}

		if (pNode->Attribute("DrawContactPoints"))
		{
	        std::string attribute(pNode->Attribute("DrawContactPoints"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DrawContactPoints;
		}

		if (pNode->Attribute("NoDeactivation"))
		{
	        std::string attribute(pNode->Attribute("NoDeactivation"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_NoDeactivation;
		}

		if (pNode->Attribute("NoHelpText"))
		{
	        std::string attribute(pNode->Attribute("NoHelpText"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_NoHelpText;
		}

		if (pNode->Attribute("DrawText"))
		{
	        std::string attribute(pNode->Attribute("DrawText"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DrawText;
		}

		if (pNode->Attribute("ProfileTimings"))
		{
	        std::string attribute(pNode->Attribute("ProfileTimings"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_ProfileTimings;
		}

		if (pNode->Attribute("EnableSatComparison"))
		{
	        std::string attribute(pNode->Attribute("EnableSatComparison"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_EnableSatComparison;
		}

		if (pNode->Attribute("DisableBulletLCP"))
		{
	        std::string attribute(pNode->Attribute("DisableBulletLCP"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DisableBulletLCP;
		}

		if (pNode->Attribute("EnableCCD"))
		{
	        std::string attribute(pNode->Attribute("EnableCCD"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_EnableCCD;
		}

		if (pNode->Attribute("DrawConstraints"))
		{
	        std::string attribute(pNode->Attribute("DrawConstraints"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DrawConstraints;
		}

		if (pNode->Attribute("DrawConstraintLimits"))
		{
	        std::string attribute(pNode->Attribute("DrawConstraintLimits"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_DrawConstraintLimits;
		}

		if (pNode->Attribute("FastWireframe"))
		{
	        std::string attribute(pNode->Attribute("FastWireframe"));
			if (attribute == "yes") debugModes |= btIDebugDraw::DBG_FastWireframe;
		}

		if (debugModes != btIDebugDraw::DBG_NoDebug)
		{
			setDebugMode(debugModes);
		}
	}

}



void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& lineColor)
{
	shared_ptr<Scene> pScene = g_pApp->GetHumanView()->m_pScene;
	shared_ptr<IRenderer> pRenderer = pScene->GetRenderer();

	Vec3 vec3From, vec3To;
	vec3From.x = from.x();
	vec3From.y = from.y();
	vec3From.z = from.z();

	vec3To.x = to.x();
	vec3To.y = to.y();
	vec3To.z = to.z();

	Color color = D3DCOLOR_XRGB( BYTE(255*lineColor.x()), BYTE(255*lineColor.y()), BYTE(255*lineColor.z()) );

	pRenderer->VDrawLine(vec3From, vec3To, color);

}



