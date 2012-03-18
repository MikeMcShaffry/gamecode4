//========================================================================
// Physics.cpp : Implements the IGamePhysics interface with Bullet
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
#include "../GameCode4/GameCode.h"
#include "Physics.h"
#include "../Actors/Actor.h"
#include "../Actors/TransformComponent.h"
#include "../ResourceCache/XmlResource.h"
#include "../EventManager/EventManager.h"


/////////////////////////////////////////////////////////////////////////////
// g_Materials Description						- Chapter 17, page 579
//
//   Predefines some useful physics materials. Define new ones here, and 
//   have similar objects use it, so if you ever need to change it you'll 
//   only have to change it here.
//
/////////////////////////////////////////////////////////////////////////////
struct MaterialData
{
	float m_restitution;
	float m_friction;

    MaterialData(float restitution, float friction)
    {
        m_restitution = restitution;
        m_friction = friction;
    }

    MaterialData(const MaterialData& other)
    {
        m_restitution = other.m_restitution;
        m_friction = other.m_friction;
    }
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// a physics implementation which does nothing.  used if physics is disabled.
//
class NullPhysics : public IGamePhysics
{
public:
	NullPhysics() { }
	virtual ~NullPhysics() { }

	// Initialization and Maintenance of the Physics World
	virtual bool VInitialize() { return true; }
	virtual void VSyncVisibleScene() { };
	virtual void VOnUpdate( float ) { }

	// Initialization of Physics Objects
	virtual void VAddSphere(float radius, WeakActorPtr gameActor, const std::string& densityStr, const std::string& physicsMaterial) { }
	virtual void VAddBox(const Vec3& dimensions, WeakActorPtr gameActor,const std::string& densityStr, const std::string& physicsMaterial) { }
	virtual void VAddPointCloud(Vec3 *verts, int numPoints, WeakActorPtr gameActor, const std::string& densityStr, const std::string& physicsMaterial) { }
	virtual void VRemoveActor(ActorId id) { }

	// Debugging
	virtual void VRenderDiagnostics() { }

	// Physics world modifiers
	virtual void VCreateTrigger(WeakActorPtr pGameActor, const Vec3 &pos, const float dim) { }
	virtual void VApplyForce(const Vec3 &dir, float newtons, ActorId aid) { }
	virtual void VApplyTorque(const Vec3 &dir, float newtons, ActorId aid) { }
	virtual bool VKinematicMove(const Mat4x4 &mat, ActorId aid) { return true; }

	// Physics actor states
	virtual void VRotateY(ActorId actorId, float angleRadians, float time) { }
	virtual float VGetOrientationY(ActorId actorId) { return 0.0f; }
	virtual void VStopActor(ActorId actorId) { }
    virtual Vec3 VGetVelocity(ActorId actorId) { return Vec3(); }
    virtual void VSetVelocity(ActorId actorId, const Vec3& vel) { }
    virtual Vec3 VGetAngularVelocity(ActorId actorId) { return Vec3(); }
    virtual void VSetAngularVelocity(ActorId actorId, const Vec3& vel) { }
	virtual void VTranslate(ActorId actorId, const Vec3& vec) { }
	virtual void VSetTransform(const ActorId id, const Mat4x4& mat) { }
    virtual Mat4x4 VGetTransform(const ActorId id) { return Mat4x4::g_Identity; }
};

#ifndef DISABLE_PHYSICS


#include "../Graphics3D/Geometry.h"
#include "../EventManager/Events.h"

#include "PhysicsDebugDrawer.h"
#include "PhysicsEventListener.h"

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include <set>
#include <iterator>
#include <map>

/////////////////////////////////////////////////////////////////////////////
// helpers for conversion to and from Bullet's data types
static btVector3 Vec3_to_btVector3( Vec3 const & vec3 )
{
	return btVector3( vec3.x, vec3.y, vec3.z );
}

static Vec3 btVector3_to_Vec3( btVector3 const & btvec )
{
	return Vec3( btvec.x(), btvec.y(), btvec.z() );
}

static btTransform Mat4x4_to_btTransform( Mat4x4 const & mat )
{
	// convert from Mat4x4 (GameCode) to btTransform (Bullet)
	btMatrix3x3 bulletRotation;
	btVector3 bulletPosition;
	
	// copy rotation matrix
	for ( int row=0; row<3; ++row )
		for ( int column=0; column<3; ++column )
			bulletRotation[row][column] = mat.m[column][row]; // note the reversed indexing (row/column vs. column/row)
			                                                  //  this is because Mat4x4s are row-major matrices and
			                                                  //  btMatrix3x3 are column-major.  This reversed indexing
			                                                  //  implicitly transposes (flips along the diagonal) 
			                                                  //  the matrix when it is copied.
	
	// copy position
	for ( int column=0; column<3; ++column )
		bulletPosition[column] = mat.m[3][column];
		
	return btTransform( bulletRotation, bulletPosition );
}

static Mat4x4 btTransform_to_Mat4x4( btTransform const & trans )
{
	Mat4x4 returnValue = Mat4x4::g_Identity;

	// convert from btTransform (Bullet) to Mat4x4 (GameCode)
	btMatrix3x3 const & bulletRotation = trans.getBasis();
	btVector3 const & bulletPosition = trans.getOrigin();
	
	// copy rotation matrix
	for ( int row=0; row<3; ++row )
		for ( int column=0; column<3; ++column )
			returnValue.m[row][column] = bulletRotation[column][row]; 
			          // note the reversed indexing (row/column vs. column/row)
			          //  this is because Mat4x4s are row-major matrices and
			          //  btMatrix3x3 are column-major.  This reversed indexing
			          //  implicitly transposes (flips along the diagonal) 
			          //  the matrix when it is copied.
	
	// copy position
	for ( int column=0; column<3; ++column )
		returnValue.m[3][column] = bulletPosition[column];
		
	return returnValue;
}

/////////////////////////////////////////////////////////////////////////////
// struct ActorMotionState						- Chapter 17, page 597
//
// Interface that Bullet uses to communicate position and orientation changes
//   back to the game.  note:  this assumes that the actor's center of mass
//   and world position are the same point.  If that was not the case,
//   an additional transformation would need to be stored here to represent
//   that difference.
//
struct ActorMotionState : public btMotionState
{
	Mat4x4 m_worldToPositionTransform;
	
	ActorMotionState( Mat4x4 const & startingTransform )
	  : m_worldToPositionTransform( startingTransform ) { }
	
	// btMotionState interface:  Bullet calls these
	virtual void getWorldTransform( btTransform& worldTrans ) const
	   { worldTrans = Mat4x4_to_btTransform( m_worldToPositionTransform ); }

	virtual void setWorldTransform( const btTransform& worldTrans )
	   { m_worldToPositionTransform = btTransform_to_Mat4x4( worldTrans ); }
};


/////////////////////////////////////////////////////////////////////////////
// BaseGamePhysics								- Chapter 17, page 590
//
//   The implementation of IGamePhysics interface using the Bullet SDK.
//
/////////////////////////////////////////////////////////////////////////////

class BulletPhysics : public IGamePhysics, GCC_noncopyable
{
	// use auto pointers to automatically call delete on these objects
	//   during ~BulletPhysics
	
	// these are all of the objects that Bullet uses to do its work.
	//   see BulletPhysics::VInitialize() for some more info.
	btDynamicsWorld*                 m_dynamicsWorld;
	btBroadphaseInterface*           m_broadphase;
	btCollisionDispatcher*           m_dispatcher;
	btConstraintSolver*              m_solver;
	btDefaultCollisionConfiguration* m_collisionConfiguration;
	BulletDebugDrawer*               m_debugDrawer;

    // tables read from the XML
    typedef std::map<std::string, float> DensityTable;
    typedef std::map<std::string, MaterialData> MaterialTable;
    DensityTable m_densityTable;
    MaterialTable m_materialTable;

	void LoadXml();
    float LookupSpecificGravity(const std::string& densityStr);
    MaterialData LookupMaterialData(const std::string& materialStr);

	// keep track of the existing rigid bodies:  To check them for updates
	//   to the actors' positions, and to remove them when their lives are over.
	typedef std::map<ActorId, btRigidBody*> ActorIDToBulletRigidBodyMap;
	ActorIDToBulletRigidBodyMap m_actorIdToRigidBody;
	btRigidBody * FindBulletRigidBody( ActorId id ) const;
	
	// also keep a map to get the actor id from the btRigidBody*
	typedef std::map<btRigidBody const *, ActorId> BulletRigidBodyToActorIDMap;
	BulletRigidBodyToActorIDMap m_rigidBodyToActorId;
	ActorId FindActorID( btRigidBody const * ) const;
	
	// data used to store which collision pair (bodies that are touching) need
	//   Collision events sent.  When a new pair of touching bodies are detected,
	//   they are added to m_previousTickCollisionPairs and an event is sent.
	//   When the pair is no longer detected, they are removed and another event
	//   is sent.
	typedef std::pair< btRigidBody const *, btRigidBody const * > CollisionPair;
	typedef std::set< CollisionPair > CollisionPairs;
	CollisionPairs m_previousTickCollisionPairs;
	
	// helpers for sending events relating to collision pairs
	void SendCollisionPairAddEvent( btPersistentManifold const * manifold, btRigidBody const * body0, btRigidBody const * body1 );
	void SendCollisionPairRemoveEvent( btRigidBody const * body0, btRigidBody const * body1 );
	
	// common functionality used by VAddSphere, VAddBox, etc
	void AddShape(StrongActorPtr pGameActor, btCollisionShape* shape, float mass, const std::string& physicsMaterial);
	
	// helper for cleaning up objects
	void RemoveCollisionObject( btCollisionObject * removeMe );

	// callback from bullet for each physics time step.  set in VInitialize
	static void BulletInternalTickCallback( btDynamicsWorld * const world, btScalar const timeStep );
	
public:
	BulletPhysics();				// [mrmike] This was changed post-press to add event registration!
	virtual ~BulletPhysics();

	// Initialiazation and Maintenance of the Physics World
	virtual bool VInitialize() override;
	virtual void VSyncVisibleScene() override; 
	virtual void VOnUpdate( float deltaSeconds ) override; 

	// Initialization of Physics Objects
	virtual void VAddSphere(float radius, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial) override;
	virtual void VAddBox(const Vec3& dimensions, WeakActorPtr pGameActor,  const std::string& densityStr, const std::string& physicsMaterial) override;
	virtual void VAddPointCloud(Vec3 *verts, int numPoints, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial) override;
	virtual void VRemoveActor(ActorId id) override;

	// Debugging
	virtual void VRenderDiagnostics() override;

	// Physics world modifiers
	virtual void VCreateTrigger(WeakActorPtr pGameActor, const Vec3 &pos, const float dim) override;
	virtual void VApplyForce(const Vec3 &dir, float newtons, ActorId aid) override;
	virtual void VApplyTorque(const Vec3 &dir, float newtons, ActorId aid) override;
	virtual bool VKinematicMove(const Mat4x4 &mat, ActorId aid) override;
	
	virtual void VRotateY(ActorId actorId, float angleRadians, float time);
	virtual float VGetOrientationY(ActorId actorId);
	virtual void VStopActor(ActorId actorId);
    virtual Vec3 VGetVelocity(ActorId actorId);
    virtual void VSetVelocity(ActorId actorId, const Vec3& vel);
    virtual Vec3 VGetAngularVelocity(ActorId actorId);
    virtual void VSetAngularVelocity(ActorId actorId, const Vec3& vel);
	virtual void VTranslate(ActorId actorId, const Vec3& vec);

    virtual void VSetTransform(const ActorId id, const Mat4x4& mat);

	virtual Mat4x4 VGetTransform(const ActorId id);
};


BulletPhysics::BulletPhysics()
{
	// [mrmike] This was changed post-press to add event registration!
	REGISTER_EVENT(EvtData_PhysTrigger_Enter);
	REGISTER_EVENT(EvtData_PhysTrigger_Leave);
	REGISTER_EVENT(EvtData_PhysCollision);
	REGISTER_EVENT(EvtData_PhysSeparation);
}


/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::~BulletPhysics				- Chapter 17, page 596
//
BulletPhysics::~BulletPhysics()
{
	// delete any physics objects which are still in the world
	
	// iterate backwards because removing the last object doesn't affect the
	//  other objects stored in a vector-type array
	for ( int ii=m_dynamicsWorld->getNumCollisionObjects()-1; ii>=0; --ii )
	{
		btCollisionObject * const obj = m_dynamicsWorld->getCollisionObjectArray()[ii];
		
		RemoveCollisionObject( obj );
	}
	
	m_rigidBodyToActorId.clear();

	SAFE_DELETE(m_debugDrawer);
	SAFE_DELETE(m_dynamicsWorld);
	SAFE_DELETE(m_solver);
	SAFE_DELETE(m_broadphase);
    SAFE_DELETE(m_dispatcher);
	SAFE_DELETE(m_collisionConfiguration);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::LoadXml						- not described in the book
//
//    Loads the physics materials from an XML file
//
void BulletPhysics::LoadXml()
{
    // Load the physics config file and grab the root XML node
    TiXmlElement* pRoot = XmlResourceLoader::LoadAndReturnRootXmlElement("config\\Physics.xml");
    GCC_ASSERT(pRoot);

    // load all materials
    TiXmlElement* pParentNode = pRoot->FirstChildElement("PhysicsMaterials");
    GCC_ASSERT(pParentNode);
    for (TiXmlElement* pNode = pParentNode->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
    {
        double restitution = 0;
        double friction = 0;
        pNode->Attribute("restitution", &restitution);
        pNode->Attribute("friction", &friction);
        m_materialTable.insert(std::make_pair(pNode->Value(), MaterialData((float)restitution, (float)friction)));
    }

    // load all densities
    pParentNode = pRoot->FirstChildElement("DensityTable");
    GCC_ASSERT(pParentNode);
    for (TiXmlElement* pNode = pParentNode->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement())
    {
        m_densityTable.insert(std::make_pair(pNode->Value(), (float)atof(pNode->FirstChild()->Value())));
    }
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VInitialize					- Chapter 17, page 594
//
bool BulletPhysics::VInitialize()
{
	LoadXml();

	// this controls how Bullet does internal memory management during the collision pass
	m_collisionConfiguration = GCC_NEW btDefaultCollisionConfiguration();

	// this manages how Bullet detects precise collisions between pairs of objects
	m_dispatcher = GCC_NEW btCollisionDispatcher( m_collisionConfiguration);

	// Bullet uses this to quickly (imprecisely) detect collisions between objects.
	//   Once a possible collision passes the broad phase, it will be passed to the
	//   slower but more precise narrow-phase collision detection (btCollisionDispatcher).
	m_broadphase = GCC_NEW btDbvtBroadphase();

	// Manages constraints which apply forces to the physics simulation.  Used
	//  for e.g. springs, motors.  We don't use any constraints right now.
	m_solver = GCC_NEW btSequentialImpulseConstraintSolver;

	// This is the main Bullet interface point.  Pass in all these components to customize its behavior.
	m_dynamicsWorld = GCC_NEW btDiscreteDynamicsWorld( m_dispatcher, 
	                                                        m_broadphase, 
	                                                        m_solver, 
	                                                        m_collisionConfiguration );

	m_debugDrawer = GCC_NEW BulletDebugDrawer;
	m_debugDrawer->ReadOptions();

	if(!m_collisionConfiguration || !m_dispatcher || !m_broadphase ||
			  !m_solver || !m_dynamicsWorld || !m_debugDrawer)
	{
		GCC_ERROR("BulletPhysics::VInitialize failed!");
		return false;
	}

	m_dynamicsWorld->setDebugDrawer( m_debugDrawer );

	
	// and set the internal tick callback to our own method "BulletInternalTickCallback"
	m_dynamicsWorld->setInternalTickCallback( BulletInternalTickCallback );
	m_dynamicsWorld->setWorldUserInfo( this );
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VOnUpdate						- Chapter 17, page 596
//
void BulletPhysics::VOnUpdate( float const deltaSeconds )
{
	// Bullet uses an internal fixed timestep (default 1/60th of a second)
	//   We pass in 4 as a max number of sub steps.  Bullet will run the simulation
	//   in increments of the fixed timestep until "deltaSeconds" amount of time has
	//   passed, but will only run a maximum of 4 steps this way.
	m_dynamicsWorld->stepSimulation( deltaSeconds, 4 );
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VSyncVisibleScene				- Chapter 17, page 598
//
void BulletPhysics::VSyncVisibleScene()
{
	// Keep physics & graphics in sync
	
	// check all the existing actor's bodies for changes. 
	//  If there is a change, send the appropriate event for the game system.
	for ( ActorIDToBulletRigidBodyMap::const_iterator it = m_actorIdToRigidBody.begin();
	      it != m_actorIdToRigidBody.end();
	      ++it )
	{ 
		ActorId const id = it->first;
		
		// get the MotionState.  this object is updated by Bullet.
		// it's safe to cast the btMotionState to ActorMotionState, because all the bodies in m_actorIdToRigidBody
		//   were created through AddShape()
		ActorMotionState const * const actorMotionState = static_cast<ActorMotionState*>(it->second->getMotionState());
		GCC_ASSERT( actorMotionState );
		
		StrongActorPtr pGameActor = MakeStrongPtr(g_pApp->m_pGame->VGetActor(id));
		if (pGameActor && actorMotionState)
		{
            shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(pGameActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
            if (pTransformComponent)
            {
			    if (pTransformComponent->GetTransform() != actorMotionState->m_worldToPositionTransform)
                {
                    // Bullet has moved the actor's physics object.  Sync the transform and inform the game an actor has moved
					pTransformComponent->SetTransform(actorMotionState->m_worldToPositionTransform);
                    shared_ptr<EvtData_Move_Actor> pEvent(GCC_NEW EvtData_Move_Actor(id, actorMotionState->m_worldToPositionTransform));
                    IEventManager::Get()->VQueueEvent(pEvent);
			    }
            }
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::AddShape						- Chapter 17, page 600
//
void BulletPhysics::AddShape(StrongActorPtr pGameActor, btCollisionShape* shape, float mass, const std::string& physicsMaterial)
{
    GCC_ASSERT(pGameActor);

    ActorId actorID = pGameActor->GetId();
	GCC_ASSERT(m_actorIdToRigidBody.find( actorID ) == m_actorIdToRigidBody.end() && "Actor with more than one physics body?");

    // lookup the material
    MaterialData material(LookupMaterialData(physicsMaterial));

	// localInertia defines how the object's mass is distributed
	btVector3 localInertia( 0.f, 0.f, 0.f );
	if ( mass > 0.f )
		shape->calculateLocalInertia( mass, localInertia );


	Mat4x4 transform = Mat4x4::g_Identity;
    shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(pGameActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
	GCC_ASSERT(pTransformComponent);
    if (pTransformComponent)
    {
		transform = pTransformComponent->GetTransform();
	}
	else
	{
		// Physics can't work on an actor that doesn't have a TransformComponent!
		return;
	}

	// set the initial transform of the body from the actor
	ActorMotionState * const myMotionState = GCC_NEW ActorMotionState(transform);
	
	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, shape, localInertia );
	
	// set up the materal properties
	rbInfo.m_restitution = material.m_restitution;
	rbInfo.m_friction    = material.m_friction;
	
	btRigidBody * const body = new btRigidBody(rbInfo);
	
	m_dynamicsWorld->addRigidBody( body );
	
	// add it to the collection to be checked for changes in VSyncVisibleScene
	m_actorIdToRigidBody[actorID] = body;
	m_rigidBodyToActorId[body] = actorID;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::RemoveCollisionObject			- not described in the book
//
//    Removes a collision object from the game world
//
void BulletPhysics::RemoveCollisionObject( btCollisionObject * const removeMe )
{
	// first remove the object from the physics sim
	m_dynamicsWorld->removeCollisionObject( removeMe );
	
	// then remove the pointer from the ongoing contacts list
	for ( CollisionPairs::iterator it = m_previousTickCollisionPairs.begin();
	      it != m_previousTickCollisionPairs.end(); )
    {
		CollisionPairs::iterator next = it;
		++next;
		
		if ( it->first == removeMe || it->second == removeMe )
		{
			SendCollisionPairRemoveEvent( it->first, it->second );
			m_previousTickCollisionPairs.erase( it );
		}
		
		it = next;
    }
	
	// if the object is a RigidBody (all of ours are RigidBodies, but it's good to be safe)
	if ( btRigidBody * const body = btRigidBody::upcast(removeMe) )
	{
		// delete the components of the object
		delete body->getMotionState();
		delete body->getCollisionShape();
		delete body->getUserPointer();
		delete body->getUserPointer();
		
		for ( int ii=body->getNumConstraintRefs()-1; ii >= 0; --ii )
		{
			btTypedConstraint * const constraint = body->getConstraintRef( ii );
			m_dynamicsWorld->removeConstraint( constraint );
			delete constraint;
		}
	}
	
	delete removeMe;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::FindBulletRigidBody			- not described in the book
//    Finds a Bullet rigid body given an actor ID
//
btRigidBody* BulletPhysics::FindBulletRigidBody( ActorId const id ) const
{
	ActorIDToBulletRigidBodyMap::const_iterator found = m_actorIdToRigidBody.find( id );
	if ( found != m_actorIdToRigidBody.end() )
		return found->second;

	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::FindActorID				- not described in the book
//    Finds an Actor ID given a Bullet rigid body 
//
ActorId BulletPhysics::FindActorID( btRigidBody const * const body ) const
{
	BulletRigidBodyToActorIDMap::const_iterator found = m_rigidBodyToActorId.find( body );
	if ( found != m_rigidBodyToActorId.end() )
		return found->second;
		
	return ActorId();
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VAddSphere					- Chapter 17, page 599
//
void BulletPhysics::VAddSphere(float const radius, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial)
{
    StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
    if (!pStrongActor)
        return;  // FUTURE WORK - Add a call to the error log here
	
	// create the collision body, which specifies the shape of the object
	btSphereShape * const collisionShape = new btSphereShape( radius );
	
	// calculate absolute mass from specificGravity
    float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = (4.f / 3.f) * GCC_PI * radius * radius * radius;
	btScalar const mass = volume * specificGravity;
	
	AddShape(pStrongActor, /*initialTransform,*/ collisionShape, mass, physicsMaterial);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VAddBox
//
void BulletPhysics::VAddBox(const Vec3& dimensions, WeakActorPtr pGameActor, const std::string& densityStr, const std::string& physicsMaterial)
{
    StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
    if (!pStrongActor)
        return;  // FUTURE WORK: Add a call to the error log here

	// create the collision body, which specifies the shape of the object
	btBoxShape * const boxShape = new btBoxShape( Vec3_to_btVector3( dimensions ) );
	
	// calculate absolute mass from specificGravity
    float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = dimensions.x * dimensions.y * dimensions.z;
	btScalar const mass = volume * specificGravity;
	
	AddShape(pStrongActor,/* initialTransform,*/ boxShape, mass, physicsMaterial);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VAddPointCloud				- Chapter 17, page 601
//
void BulletPhysics::VAddPointCloud(Vec3 *verts, int numPoints, WeakActorPtr pGameActor, /*const Mat4x4& initialTransform,*/ const std::string& densityStr, const std::string& physicsMaterial)
{
    StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
    if (!pStrongActor)
        return;  // FUTURE WORK: Add a call to the error log here
	
	btConvexHullShape * const shape = new btConvexHullShape();
	
	// add the points to the shape one at a time
	for ( int ii=0; ii<numPoints; ++ii )
		shape->addPoint(  Vec3_to_btVector3( verts[ii] ) );
	
	// approximate absolute mass using bounding box
	btVector3 aabbMin(0,0,0), aabbMax(0,0,0);
	shape->getAabb( btTransform::getIdentity(), aabbMin, aabbMax );
	
	btVector3 const aabbExtents = aabbMax - aabbMin;
	
    float specificGravity = LookupSpecificGravity(densityStr);
	float const volume = aabbExtents.x() * aabbExtents.y() * aabbExtents.z();
	btScalar const mass = volume * specificGravity;
	
	AddShape(pStrongActor, shape, mass, physicsMaterial);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VRemoveActor					- not described in the book
//
//    Implements the method to remove actors from the physics simulation
//
void BulletPhysics::VRemoveActor(ActorId id)
{
	if ( btRigidBody * const body = FindBulletRigidBody( id ) )
	{
		// destroy the body and all its components
		RemoveCollisionObject( body );
		m_actorIdToRigidBody.erase ( id );
		m_rigidBodyToActorId.erase( body );
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VRenderDiagnostics			- Chapter 17, page 604
//
void BulletPhysics::VRenderDiagnostics()
{
	m_dynamicsWorld->debugDrawWorld();
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VCreateTrigger				- Chapter 17, page 602
//
// FUTURE WORK: Mike create a trigger actor archetype that can be instantiated in the editor!!!!!
//
void BulletPhysics::VCreateTrigger(WeakActorPtr pGameActor, const Vec3 &pos, const float dim)
{
    StrongActorPtr pStrongActor = MakeStrongPtr(pGameActor);
    if (!pStrongActor)
        return;  // FUTURE WORK: Add a call to the error log here

	// create the collision body, which specifies the shape of the object
	btBoxShape * const boxShape = new btBoxShape( Vec3_to_btVector3( Vec3(dim,dim,dim) ) );
	
	// triggers are immoveable.  0 mass signals this to Bullet.
	btScalar const mass = 0;

	// set the initial position of the body from the actor
	Mat4x4 triggerTrans = Mat4x4::g_Identity;
	triggerTrans.SetPosition( pos );
	ActorMotionState * const myMotionState = GCC_NEW ActorMotionState( triggerTrans );
	
	btRigidBody::btRigidBodyConstructionInfo rbInfo( mass, myMotionState, boxShape, btVector3(0,0,0) );
	btRigidBody * const body = new btRigidBody(rbInfo);
	
	m_dynamicsWorld->addRigidBody( body );
	
	// a trigger is just a box that doesn't collide with anything.  That's what "CF_NO_CONTACT_RESPONSE" indicates.
	body->setCollisionFlags( body->getCollisionFlags() | btRigidBody::CF_NO_CONTACT_RESPONSE );

	m_actorIdToRigidBody[pStrongActor->GetId()] = body;
	m_rigidBodyToActorId[body] = pStrongActor->GetId();
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VApplyForce					- Chapter 17, page 603
//
void BulletPhysics::VApplyForce(const Vec3 &dir, float newtons, ActorId aid)
{
	if ( btRigidBody * const body = FindBulletRigidBody( aid ) )
	{
		body->setActivationState(DISABLE_DEACTIVATION);

		btVector3 const force( dir.x * newtons,
		                       dir.y * newtons,
		                       dir.z * newtons );

		body->applyCentralImpulse( force );
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VApplyTorque					- Chapter 17, page 603
//
void BulletPhysics::VApplyTorque(const Vec3 &dir, float magnitude, ActorId aid)
{
	if ( btRigidBody * const body = FindBulletRigidBody( aid ) )
	{
		body->setActivationState(DISABLE_DEACTIVATION);

		btVector3 const torque( dir.x * magnitude,
		                        dir.y * magnitude,
		                        dir.z * magnitude );

		body->applyTorqueImpulse( torque );
	}
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VKinematicMove				- not described in the book
//
//    Forces a phyics object to a new location/orientation
//
bool BulletPhysics::VKinematicMove(const Mat4x4 &mat, ActorId aid)
{
	if ( btRigidBody * const body = FindBulletRigidBody( aid ) )
	{
        body->setActivationState(DISABLE_DEACTIVATION);

		// warp the body to the new position
		body->setWorldTransform( Mat4x4_to_btTransform( mat ) );
		return true;
	}
	
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VGetTransform					- not described in the book
//
//   Returns the current transform of the phyics object
//
Mat4x4 BulletPhysics::VGetTransform(const ActorId id)
{
    btRigidBody * pRigidBody = FindBulletRigidBody(id);
    GCC_ASSERT(pRigidBody);

    const btTransform& actorTransform = pRigidBody->getCenterOfMassTransform();
    return btTransform_to_Mat4x4(actorTransform);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VSetTransform					- not described in the book
//
//   Sets the current transform of the phyics object
//
void BulletPhysics::VSetTransform(ActorId actorId, const Mat4x4& mat)
{
    VKinematicMove(mat, actorId);
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VRotateY						- not described in the book
//
//   A helper function used to turn objects to a new heading
//
void BulletPhysics::VRotateY( ActorId const actorId, float const deltaAngleRadians, float const time )
{
	btRigidBody * pRigidBody = FindBulletRigidBody(actorId);
	GCC_ASSERT(pRigidBody);

	// create a transform to represent the additional turning this frame
	btTransform angleTransform;
	angleTransform.setIdentity();
	angleTransform.getBasis().setEulerYPR( 0, deltaAngleRadians, 0 ); // rotation about body Y-axis
			
	// concatenate the transform onto the body's transform
	pRigidBody->setCenterOfMassTransform( pRigidBody->getCenterOfMassTransform() * angleTransform );
}



/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VGetOrientationY				- not described in the book
//
//   A helper functions use to access the current heading of a physics object
//
float BulletPhysics::VGetOrientationY(ActorId actorId)
{
	btRigidBody * pRigidBody = FindBulletRigidBody(actorId);
	GCC_ASSERT(pRigidBody);
	
	const btTransform& actorTransform = pRigidBody->getCenterOfMassTransform();
	btMatrix3x3 actorRotationMat(actorTransform.getBasis());  // should be just the rotation information

	btVector3 startingVec(0,0,1);
	btVector3 endingVec = actorRotationMat * startingVec; // transform the vector

	endingVec.setY(0);  // we only care about rotation on the XZ plane

	float const endingVecLength = endingVec.length();
	if (endingVecLength < 0.001)
	{
		// gimbal lock (orientation is straight up or down)
		return 0;
	}

	else
	{
		btVector3 cross = startingVec.cross(endingVec);
		float sign = cross.getY() > 0 ? 1.0f : -1.0f;
		return (acosf(startingVec.dot(endingVec) / endingVecLength) * sign);
	}

	return FLT_MAX;  // fail...
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VStopActor					- Chapter 17, page 604
//
void BulletPhysics::VStopActor(ActorId actorId)
{
   VSetVelocity(actorId, Vec3(0.f, 0.f, 0.f));
}

/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::VGetVelocity					- Chapter 17, page 604
//
Vec3 BulletPhysics::VGetVelocity(ActorId actorId)
{
    btRigidBody* pRigidBody = FindBulletRigidBody(actorId);
    GCC_ASSERT(pRigidBody);
    if (!pRigidBody)
        return Vec3();
    btVector3 btVel = pRigidBody->getLinearVelocity();
    return btVector3_to_Vec3(btVel);
}

/////////////////////////////////////////////////////////////////////////////
void BulletPhysics::VSetVelocity(ActorId actorId, const Vec3& vel)
{
	btRigidBody * pRigidBody = FindBulletRigidBody(actorId);
	GCC_ASSERT(pRigidBody);
	if (!pRigidBody)
		return;
	btVector3 btVel = Vec3_to_btVector3(vel);
	pRigidBody->setLinearVelocity(btVel);
}

/////////////////////////////////////////////////////////////////////////////
Vec3 BulletPhysics::VGetAngularVelocity(ActorId actorId)
{
    btRigidBody* pRigidBody = FindBulletRigidBody(actorId);
    GCC_ASSERT(pRigidBody);
    if (!pRigidBody)
        return Vec3();
    btVector3 btVel = pRigidBody->getAngularVelocity();
    return btVector3_to_Vec3(btVel);
}

/////////////////////////////////////////////////////////////////////////////
void BulletPhysics::VSetAngularVelocity(ActorId actorId, const Vec3& vel)
{
    btRigidBody * pRigidBody = FindBulletRigidBody(actorId);
    GCC_ASSERT(pRigidBody);
    if (!pRigidBody)
        return;
    btVector3 btVel = Vec3_to_btVector3(vel);
    pRigidBody->setAngularVelocity(btVel);
}

/////////////////////////////////////////////////////////////////////////////
void BulletPhysics::VTranslate(ActorId actorId, const Vec3& vec)
{
	btRigidBody * pRigidBody = FindBulletRigidBody(actorId);
	GCC_ASSERT(pRigidBody);
	btVector3 btVec = Vec3_to_btVector3(vec);
	pRigidBody->translate(btVec);
}


/////////////////////////////////////////////////////////////////////////////
// BulletPhysics::BulletInternalTickCallback		- Chapter 17, page 606
//
// This function is called after bullet performs its internal update.  We
//   use it to detect collisions between objects for Game code.
//
void BulletPhysics::BulletInternalTickCallback( btDynamicsWorld * const world, btScalar const timeStep )
{
	GCC_ASSERT( world );
	
	GCC_ASSERT( world->getWorldUserInfo() );
	BulletPhysics * const bulletPhysics = static_cast<BulletPhysics*>( world->getWorldUserInfo() );
	
	CollisionPairs currentTickCollisionPairs;
	
	// look at all existing contacts
	btDispatcher * const dispatcher = world->getDispatcher();
	for ( int manifoldIdx=0; manifoldIdx<dispatcher->getNumManifolds(); ++manifoldIdx )
	{
		// get the "manifold", which is the set of data corresponding to a contact point
		//   between two physics objects
		btPersistentManifold const * const manifold = dispatcher->getManifoldByIndexInternal( manifoldIdx );
		GCC_ASSERT( manifold );
		
		// get the two bodies used in the manifold.  Bullet stores them as void*, so we must cast
		//  them back to btRigidBody*s.  Manipulating void* pointers is usually a bad
		//  idea, but we have to work with the environment that we're given.  We know this
		//  is safe because we only ever add btRigidBodys to the simulation
		btRigidBody const * const body0 = static_cast<btRigidBody const *>(manifold->getBody0());
		btRigidBody const * const body1 = static_cast<btRigidBody const *>(manifold->getBody1());
		
		// always create the pair in a predictable order
		bool const swapped = body0 > body1;
		
		btRigidBody const * const sortedBodyA = swapped ? body1 : body0;
		btRigidBody const * const sortedBodyB = swapped ? body0 : body1;
		
		CollisionPair const thisPair = std::make_pair( sortedBodyA, sortedBodyB );
		currentTickCollisionPairs.insert( thisPair );
		
		if ( bulletPhysics->m_previousTickCollisionPairs.find( thisPair ) == bulletPhysics->m_previousTickCollisionPairs.end() )
		{
			// this is a new contact, which wasn't in our list before.  send an event to the game.
			bulletPhysics->SendCollisionPairAddEvent( manifold, body0, body1 );
		}
	}
	
	CollisionPairs removedCollisionPairs;
	
	// use the STL set difference function to find collision pairs that existed during the previous tick but not any more
	std::set_difference( bulletPhysics->m_previousTickCollisionPairs.begin(), bulletPhysics->m_previousTickCollisionPairs.end(),
	                     currentTickCollisionPairs.begin(), currentTickCollisionPairs.end(),
	                     std::inserter( removedCollisionPairs, removedCollisionPairs.begin() ) );
	
	for ( CollisionPairs::const_iterator it = removedCollisionPairs.begin(), 
         end = removedCollisionPairs.end(); it != end; ++it )
	{
		btRigidBody const * const body0 = it->first;
		btRigidBody const * const body1 = it->second;
		
		bulletPhysics->SendCollisionPairRemoveEvent( body0, body1 );
	}
	
	// the current tick becomes the previous tick.  this is the way of all things.
	bulletPhysics->m_previousTickCollisionPairs = currentTickCollisionPairs;
}

//////////////////////////////////////////////////////////////////////////////////////////
void BulletPhysics::SendCollisionPairAddEvent( btPersistentManifold const * manifold, btRigidBody const * const body0, btRigidBody const * const body1 )
{
	if ( body0->getUserPointer() || body1->getUserPointer() )
	{
		// only triggers have non-NULL userPointers
		
		// figure out which actor is the trigger
		btRigidBody const * triggerBody, * otherBody;
	
		if ( body0->getUserPointer() )
		{
			triggerBody = body0;
			otherBody = body1;
		}
		else
		{
			otherBody = body0;
			triggerBody = body1;
		}
		
		// send the trigger event.
		int const triggerId = *static_cast<int*>(triggerBody->getUserPointer());
        shared_ptr<EvtData_PhysTrigger_Enter> pEvent(GCC_NEW EvtData_PhysTrigger_Enter(triggerId, FindActorID(otherBody)));
        IEventManager::Get()->VQueueEvent(pEvent);
	}
	else
	{
		ActorId const id0 = FindActorID( body0 );
		ActorId const id1 = FindActorID( body1 );
		
		if (id0 == INVALID_ACTOR_ID || id1 == INVALID_ACTOR_ID)
		{
			// something is colliding with a non-actor.  we currently don't send events for that
			return;
		}
		
		// this pair of colliding objects is new.  send a collision-begun event
		Vec3List collisionPoints;
		Vec3 sumNormalForce;
		Vec3 sumFrictionForce;
		
		for ( int pointIdx = 0; pointIdx < manifold->getNumContacts(); ++pointIdx )
		{
			btManifoldPoint const & point = manifold->getContactPoint( pointIdx );
		
			collisionPoints.push_back( btVector3_to_Vec3( point.getPositionWorldOnB() ) );
			
			sumNormalForce += btVector3_to_Vec3( point.m_combinedRestitution * point.m_normalWorldOnB );
			sumFrictionForce += btVector3_to_Vec3( point.m_combinedFriction * point.m_lateralFrictionDir1 );
		}
		
		// send the event for the game
        shared_ptr<EvtData_PhysCollision> pEvent(GCC_NEW EvtData_PhysCollision(id0, id1, sumNormalForce, sumFrictionForce, collisionPoints));
        IEventManager::Get()->VQueueEvent(pEvent);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void BulletPhysics::SendCollisionPairRemoveEvent( btRigidBody const * const body0, btRigidBody const * const body1 )
{
	if ( body0->getUserPointer() || body1->getUserPointer() )
	{
		// figure out which actor is the trigger
		btRigidBody const * triggerBody, * otherBody;
	
		if ( body0->getUserPointer() )
		{
			triggerBody = body0;
			otherBody = body1;
		}
		else
		{
			otherBody = body0;
			triggerBody = body1;
		}
		
		// send the trigger event.
		int const triggerId = *static_cast<int*>(triggerBody->getUserPointer());
        shared_ptr<EvtData_PhysTrigger_Leave> pEvent(GCC_NEW EvtData_PhysTrigger_Leave(triggerId, FindActorID( otherBody)));
        IEventManager::Get()->VQueueEvent(pEvent);
	}
	else
	{
		ActorId const id0 = FindActorID( body0 );
		ActorId const id1 = FindActorID( body1 );
	
		if ( id0 == INVALID_ACTOR_ID || id1 == INVALID_ACTOR_ID )
		{
			// collision is ending between some object(s) that don't have actors.  we don't send events for that.
			return;
		}

        shared_ptr<EvtData_PhysSeparation> pEvent(GCC_NEW EvtData_PhysSeparation(id0, id1));
        IEventManager::Get()->VQueueEvent(pEvent);
	}
}

float BulletPhysics::LookupSpecificGravity(const std::string& densityStr)
{
    float density = 0;
    auto densityIt = m_densityTable.find(densityStr);
    if (densityIt != m_densityTable.end())
        density = densityIt->second;
    // else: dump error

    return density;
}

MaterialData BulletPhysics::LookupMaterialData(const std::string& materialStr)
{
    auto materialIt = m_materialTable.find(materialStr);
    if (materialIt != m_materialTable.end())
        return materialIt->second;
    else
        return MaterialData(0, 0);
}


#endif // #ifndef DISABLE_PHYSICS

/////////////////////////////////////////////////////////////////////////////
//
// CreateGamePhysics 
//   The free function that creates an object that implements the IGamePhysics interface.
//
IGamePhysics *CreateGamePhysics()
{
	std::auto_ptr<IGamePhysics> gamePhysics;
	gamePhysics.reset( GCC_NEW BulletPhysics );

	if (gamePhysics.get() && !gamePhysics->VInitialize())
	{
		// physics failed to initialize.  delete it.
		gamePhysics.reset();
	}

	return gamePhysics.release();
}

IGamePhysics *CreateNullPhysics()
{
	std::auto_ptr<IGamePhysics> gamePhysics;
	gamePhysics.reset( GCC_NEW NullPhysics );
	if (gamePhysics.get() && !gamePhysics->VInitialize())
	{
		// physics failed to initialize.  delete it.
		gamePhysics.reset();
	}

	return gamePhysics.release();
}
