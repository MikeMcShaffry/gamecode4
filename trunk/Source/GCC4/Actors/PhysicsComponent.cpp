//========================================================================
// PhysicsComponent.cpp - Component for physical movement and collision detection
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
#include "PhysicsComponent.h"
#include "TransformComponent.h"
#include "../Physics/Physics.h"
#include "../Utilities/String.h"

// all times are units-per-second
const float DEFAULT_MAX_VELOCITY = 7.5f;
const float DEFAULT_MAX_ANGULAR_VELOCITY = 1.2f;


//---------------------------------------------------------------------------------------------------------------------
// PhysicsComponent implementation
//---------------------------------------------------------------------------------------------------------------------

const char *PhysicsComponent::g_Name = "PhysicsComponent";

PhysicsComponent::PhysicsComponent(void)
{
    m_RigidBodyLocation = Vec3(0.f,0.f,0.f);
	m_RigidBodyOrientation = Vec3(0.f,0.f,0.f);
	m_RigidBodyScale = Vec3(1.f,1.f,1.f);

    m_acceleration = 0;
    m_angularAcceleration = 0;
    m_maxVelocity = DEFAULT_MAX_VELOCITY;
    m_maxAngularVelocity = DEFAULT_MAX_ANGULAR_VELOCITY;
}

PhysicsComponent::~PhysicsComponent(void)
{
    m_pGamePhysics->VRemoveActor(m_pOwner->GetId());
}

bool PhysicsComponent::VInit(TiXmlElement* pData)
{
    // no point in having a physics component with no game physics
    m_pGamePhysics = g_pApp->m_pGame->VGetGamePhysics();
    if (!m_pGamePhysics)
        return false;

    // shape
    TiXmlElement* pShape = pData->FirstChildElement("Shape");
    if (pShape)
    {
		m_shape = pShape->FirstChild()->Value();
    }

    // density
    TiXmlElement* pDensity = pData->FirstChildElement("Density");
    if (pDensity)
        m_density = pDensity->FirstChild()->Value();

    // material
    TiXmlElement* pMaterial = pData->FirstChildElement("PhysicsMaterial");
    if (pMaterial)
        m_material = pMaterial->FirstChild()->Value();

    // initial transform
    TiXmlElement* pRigidBodyTransform = pData->FirstChildElement("RigidBodyTransform");
    if (pRigidBodyTransform)
        BuildRigidBodyTransform(pRigidBodyTransform);

    return true;
}

TiXmlElement* PhysicsComponent::VGenerateXml(void)
{
    TiXmlElement* pBaseElement = GCC_NEW TiXmlElement(VGetName());

    // shape
	TiXmlElement* pShape = GCC_NEW TiXmlElement("Shape");
    TiXmlText* pShapeText = GCC_NEW TiXmlText(m_shape.c_str());
    pShape->LinkEndChild(pShapeText);
	pBaseElement->LinkEndChild(pShape);

    // density
    TiXmlElement* pDensity = GCC_NEW TiXmlElement("Density");
    TiXmlText* pDensityText = GCC_NEW TiXmlText(m_density.c_str());
    pDensity->LinkEndChild(pDensityText);
    pBaseElement->LinkEndChild(pDensity);

    // material
    TiXmlElement* pMaterial = GCC_NEW TiXmlElement("Material");
    TiXmlText* pMaterialText = GCC_NEW TiXmlText(m_material.c_str());
    pMaterial->LinkEndChild(pMaterialText);
    pBaseElement->LinkEndChild(pMaterial);

    // rigid body transform
    TiXmlElement* pInitialTransform = GCC_NEW TiXmlElement("RigidBodyTransform");

    // initial transform -> position
    TiXmlElement* pPosition = GCC_NEW TiXmlElement("Position");
    pPosition->SetAttribute("x", ToStr(m_RigidBodyLocation.x).c_str());
    pPosition->SetAttribute("y", ToStr(m_RigidBodyLocation.y).c_str());
    pPosition->SetAttribute("z", ToStr(m_RigidBodyLocation.z).c_str());
    pInitialTransform->LinkEndChild(pPosition);

    // initial transform -> orientation
    TiXmlElement* pOrientation = GCC_NEW TiXmlElement("Orientation");
    pOrientation->SetAttribute("yaw", ToStr(m_RigidBodyOrientation.x).c_str());
    pOrientation->SetAttribute("pitch", ToStr(m_RigidBodyOrientation.y).c_str());
    pOrientation->SetAttribute("roll", ToStr(m_RigidBodyOrientation.z).c_str());
    pInitialTransform->LinkEndChild(pOrientation);

	// initial transform -> scale 
    TiXmlElement* pScale = GCC_NEW TiXmlElement("Scale");
    pScale->SetAttribute("x", ToStr(m_RigidBodyScale.x).c_str());
    pScale->SetAttribute("y", ToStr(m_RigidBodyScale.y).c_str());
    pScale->SetAttribute("z", ToStr(m_RigidBodyScale.z).c_str());
    pInitialTransform->LinkEndChild(pScale);

    pBaseElement->LinkEndChild(pInitialTransform);

    return pBaseElement;
}

void PhysicsComponent::VPostInit(void)
{
    if (m_pOwner)
    {
		if (m_shape == "Sphere")
		{
			m_pGamePhysics->VAddSphere((float)m_RigidBodyScale.x, m_pOwner, m_density, m_material);
		}
		else if (m_shape == "Box")
		{
			m_pGamePhysics->VAddBox(m_RigidBodyScale, m_pOwner, m_density, m_material);
		}
		else if (m_shape == "PointCloud")
		{
			GCC_ERROR("Not supported yet!");
		}
	}
}

void PhysicsComponent::VUpdate(int deltaMs)
{
    // get the transform component
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (!pTransformComponent)
    {
        GCC_ERROR("No transform component!");
        return;
    }

    // get the direction the object is facing
    Mat4x4 transform = pTransformComponent->GetTransform();

	if (m_acceleration != 0)
    {
        // calculate the acceleration this frame

        float accelerationToApplyThisFrame = m_acceleration / 1000.f * (float)deltaMs;

        // Get the current velocity vector and convert to a scalar.  The velocity vector is a combination of 
        // the direction this actor is going in and the speed of the actor.  The scalar is just the speed 
        // component.
        Vec3 velocity(m_pGamePhysics->VGetVelocity(m_pOwner->GetId()));
        float velocityScalar = velocity.Length();

		Vec3 direction(transform.GetDirection());
		m_pGamePhysics->VApplyForce(direction, accelerationToApplyThisFrame, m_pOwner->GetId());

        // logging
        // [rez] Comment this back in if you want to debug the physics thrust & rotation stuff.  It spams quite 
        // a bit of info the output window so I'm commenting it out for now.
/*
        GCC_LOG("Actor", "Acceleration: " + ToStr(accelerationToApplyThisFrame) + "; velocityScalar: " + ToStr(velocityScalar) + \
                "; direction: " + ToStr(direction) + "; direction.Length(): " + ToStr(direction.Length()) + \
                "; velocity: " + ToStr(velocity) + "; velocity.Length(): " + ToStr(velocity.Length()));
*/
    }

    if (m_angularAcceleration != 0)
    {
        // calculate the acceleration this frame
        float angularAccelerationToApplyThisFrame = m_angularAcceleration / 1000.f * (float)deltaMs;
		m_pGamePhysics->VApplyTorque(transform.GetUp(), angularAccelerationToApplyThisFrame, m_pOwner->GetId());

        // logging
        // [rez] Comment this back in if you want to debug the physics thrust & rotation stuff.  It spams quite 
        // a bit of info the output window so I'm commenting it out for now.
        //GCC_LOG("Actor", "Angular Acceleration: " + ToStr(angularAccelerationToApplyThisFrame) );
    }
}

void PhysicsComponent::BuildRigidBodyTransform(TiXmlElement* pTransformElement)
{
	// FUTURE WORK Mrmike - this should be exactly the same as the TransformComponent - maybe factor into a helper method?
    GCC_ASSERT(pTransformElement);

    TiXmlElement* pPositionElement = pTransformElement->FirstChildElement("Position");
    if (pPositionElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pPositionElement->Attribute("x", &x);
        pPositionElement->Attribute("y", &y);
        pPositionElement->Attribute("z", &z);
        m_RigidBodyLocation = Vec3(x, y, z);
    }

    TiXmlElement* pOrientationElement = pTransformElement->FirstChildElement("Orientation");
    if (pOrientationElement)
    {
        double yaw = 0;
        double pitch = 0;
        double roll = 0;
        pPositionElement->Attribute("yaw", &yaw);
        pPositionElement->Attribute("pitch", &pitch);
        pPositionElement->Attribute("roll", &roll);
        m_RigidBodyOrientation = Vec3((float)DEGREES_TO_RADIANS(yaw), (float)DEGREES_TO_RADIANS(pitch), (float)DEGREES_TO_RADIANS(roll));
    }

    TiXmlElement* pScaleElement = pTransformElement->FirstChildElement("Scale");
    if (pScaleElement)
    {
        double x = 0;
        double y = 0;
        double z = 0;
        pScaleElement->Attribute("x", &x);
        pScaleElement->Attribute("y", &y);
        pScaleElement->Attribute("z", &z);
        m_RigidBodyScale = Vec3((float)x, (float)y, (float)z);
    }
}

void PhysicsComponent::ApplyForce(const Vec3& direction, float forceNewtons)
{
    m_pGamePhysics->VApplyForce(direction, forceNewtons, m_pOwner->GetId());
}

void PhysicsComponent::ApplyTorque(const Vec3& direction, float forceNewtons)
{
    m_pGamePhysics->VApplyTorque(direction, forceNewtons, m_pOwner->GetId());
}

bool PhysicsComponent::KinematicMove(const Mat4x4 &transform)
{
	return m_pGamePhysics->VKinematicMove(transform, m_pOwner->GetId());
}

void PhysicsComponent::ApplyAcceleration(float acceleration)
{
    m_acceleration = acceleration;
}

void PhysicsComponent::RemoveAcceleration(void)
{
    m_acceleration = 0;
}

void PhysicsComponent::ApplyAngularAcceleration(float acceleration)
{
    m_angularAcceleration = acceleration;
}

void PhysicsComponent::RemoveAngularAcceleration(void)
{
    m_angularAcceleration = 0;
}

Vec3 PhysicsComponent::GetVelocity(void)
{
    return m_pGamePhysics->VGetVelocity(m_pOwner->GetId());
}

void PhysicsComponent::SetVelocity(const Vec3& velocity)
{
    m_pGamePhysics->VSetVelocity(m_pOwner->GetId(), velocity);
}

void PhysicsComponent::RotateY(float angleRadians)
{
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
        Mat4x4 transform = pTransformComponent->GetTransform();
        Vec3 position = transform.GetPosition();

        Mat4x4 rotateY;
        rotateY.BuildRotationY(angleRadians);
        rotateY.SetPosition(position);

        KinematicMove(rotateY);
    }
    else
        GCC_ERROR("Attempting to call RotateY() on actor with no transform component");
}

void PhysicsComponent::SetPosition(float x, float y, float z)
{
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
    if (pTransformComponent)
    {
        Mat4x4 transform = pTransformComponent->GetTransform();
        Vec3 position = Vec3(x, y, z);
        transform.SetPosition(position);

        KinematicMove(transform);
    }
    else
        GCC_ERROR("Attempting to call RotateY() on actor with no trnasform component");
}

void PhysicsComponent::Stop(void)
{
    return m_pGamePhysics->VStopActor(m_pOwner->GetId());
}

