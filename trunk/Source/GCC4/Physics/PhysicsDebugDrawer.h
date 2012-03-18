#pragma once
//========================================================================
// PhysicsDebugDrawer.h - implements a physics debug drawer in DX9 
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


#include "btBulletDynamicsCommon.h"

//////////////////////////////////////////////////////////////////////////////
// class BulletDebugDrawer						- Chapter 17, page 605
//
// Bullet uses this object to draw debug information.  This implementation
//   of btIDebugDraw uses direct3D lines to represent the current state
//   of the physics simulation 
//
class BulletDebugDrawer : public btIDebugDraw
{
	DebugDrawModes               m_DebugModes;
public:
	// btIDebugDraw interface
	virtual void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color) override;
	virtual void	reportErrorWarning(const char* warningString) override;
	virtual void	draw3dText(const btVector3& location,const char* textString) override;
	virtual void	setDebugMode(int debugMode) override;
	virtual int		getDebugMode() const override;
	virtual void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color) override;

	// [mrmike] Added post press to read PlayerOptions.xml to turn on physics debug options.
	void ReadOptions(void);
};


