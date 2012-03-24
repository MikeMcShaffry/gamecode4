#pragma once
//========================================================================
// File: MovementController.h
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

#include "../GameCode4/Interfaces.h"
#include "Geometry.h"


// Forward declarations
class SceneNode;

////////////////////////////////////////////////////
//
// MovementController Description
//
//    Implements a WASD style movement controller
//
//  class MovementController		- Chapter 10, page 281
//
////////////////////////////////////////////////////

class MovementController : public IPointerHandler, public IKeyboardHandler
{
protected:
	Mat4x4  m_matFromWorld;
	Mat4x4	m_matToWorld;
    Mat4x4  m_matPosition;

	Point					m_lastMousePos;
	bool					m_bKey[256];			// Which keys are up and down

	// Orientation Controls
	float		m_fTargetYaw;
	float		m_fTargetPitch;
	float		m_fYaw;
	float		m_fPitch;
	float		m_fPitchOnDown;
	float		m_fYawOnDown;
	float		m_maxSpeed;
	float		m_currentSpeed;

	// Added for Ch19/20 refactor
	bool		m_mouseLButtonDown;
	bool		m_bRotateWhenLButtonDown;

	shared_ptr<SceneNode> m_object;

public:
	MovementController(shared_ptr<SceneNode> object, float initialYaw, float initialPitch, bool rotateWhenLButtonDown);
	void SetObject(shared_ptr<SceneNode> newObject);

	void OnUpdate(DWORD const elapsedMs);

public:
	bool VOnPointerMove(const Point &mousePos, const int radius);
	bool VOnPointerButtonDown(const Point &mousePos, const int radius, const std::string &buttonName);
	bool VOnPointerButtonUp(const Point &mousePos, const int radius, const std::string &buttonName);

	bool VOnKeyDown(const BYTE c) { m_bKey[c] = true; return true; }
	bool VOnKeyUp(const BYTE c) { m_bKey[c] = false; return true; }

	const Mat4x4 *GetToWorld() { return &m_matToWorld; }
	const Mat4x4 *GetFromWorld() { return &m_matFromWorld; }	
};

