//========================================================================
// TeapotWars.cpp : source file for the sample game
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

#include "TeapotWarsStd.h"

#include "../GCC4/GameCode4/GameCode.h"
#include "../GCC4/Physics/Physics.h"
#include "../GCC4/Physics/PhysicsEventListener.h"
#include "../GCC4/MainLoop/Initialization.h"
#include "../GCC4/AI/Pathing.h"
#include "../GCC4/EventManager/Events.h"
#include "../GCC4/EventManager/EventManagerImpl.h"
#include "../GCC4/Actors/Actor.h"
#include "../GCC4/Actors/PhysicsComponent.h"
#include "../GCC4/Actors/TransformComponent.h"
#include "../GCC4/Utilities/String.h"

#include "TeapotWars.h"
#include "TeapotWarsView.h"
#include "TeapotWarsNetwork.h"
#include "TeapotEvents.h"
#include "TeapotResources.h"


TeapotWarsApp g_TeapotWarsApp;


//========================================================================
// wWinMain - Defines the entry point to your application (Chapter 5 - page 121)
//
// In the book _tWinMain goes straight into the initialization sequence. Here,
// the GameCode4() free function handles it. This allows the GameCode4 function
// to live in a library, separating the game engine from game specific code,
// in this case TeapotWars.
//
//========================================================================


INT WINAPI wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR    lpCmdLine,
                     int       nCmdShow)
{
	return GameCode4(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

//========================================================================
//
// TeapotWarsApp Implementation     - Chapter 21, page 722
//
//========================================================================



//
// TeapotWarsApp::VCreateGameAndView
//
BaseGameLogic *TeapotWarsApp::VCreateGameAndView()
{
	m_pGame = GCC_NEW TeapotWarsLogic();
    m_pGame->Init();

	shared_ptr<IGameView> menuView(GCC_NEW MainMenuView());
	m_pGame->VAddView(menuView);

	return m_pGame;
}


HICON TeapotWarsApp::VGetIcon()
{
	return LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_ICON1));
}

void TeapotWarsApp::VRegisterGameEvents(void)
{
    REGISTER_EVENT(EvtData_StartThrust);
    REGISTER_EVENT(EvtData_EndThrust);
    REGISTER_EVENT(EvtData_StartSteer);
    REGISTER_EVENT(EvtData_EndSteer);
}



void TeapotWarsApp::VCreateNetworkEventForwarder(void)
{
	GameCodeApp::VCreateNetworkEventForwarder();
    if (m_pNetworkEventForwarder != NULL)
    {
	    IEventManager* pGlobalEventManager = IEventManager::Get();
		// FUTURE WORK - Events should have a "classification" that signals if they are sent from client to server, from server to client, or both.
		//               Then as events are created, they are automatically added to the right network forwarders.
		//               This could also detect a 
	    pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Fire_Weapon::sk_EventType);
        pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_StartThrust::sk_EventType);
        pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_EndThrust::sk_EventType);
        pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_StartSteer::sk_EventType);
        pGlobalEventManager->VAddListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_EndSteer::sk_EventType);

	}
}

void TeapotWarsApp::VDestroyNetworkEventForwarder(void)
{
	GameCodeApp::VDestroyNetworkEventForwarder();
    if (m_pNetworkEventForwarder)
    {
        IEventManager* pGlobalEventManager = IEventManager::Get();
        pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Fire_Weapon::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_StartThrust::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_EndThrust::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_StartSteer::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_EndSteer::sk_EventType);
		pGlobalEventManager->VRemoveListener(MakeDelegate(m_pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Environment_Loaded::sk_EventType);
        SAFE_DELETE(m_pNetworkEventForwarder);
    }
}


//========================================================================
//
// TeapotWarsLogic Implementation       - Chapter 21, page 725
//
//========================================================================

//
// TeapotWarsLogic::TeapotWarsLogic
//
TeapotWarsLogic::TeapotWarsLogic()
{
	m_pPhysics.reset(CreateGamePhysics());
    RegisterAllDelegates();
}


//
// TeapotWarsLogic::~TeapotWarsLogic
//
TeapotWarsLogic::~TeapotWarsLogic()
{
    RemoveAllDelegates();
    DestroyAllNetworkEventForwarders();
}


void TeapotWarsLogic::VSetProxy()
{
	// FUTURE WORK: This can go in the base game logic!!!!
	BaseGameLogic::VSetProxy();
}



class WatchMeProcess : public Process
{
	ActorId m_me;
	ActorId m_target;
public:
	WatchMeProcess(ActorId me, ActorId target) { m_me = me; m_target = target; }
	void VOnUpdate(unsigned long deltaMs);
};

void WatchMeProcess::VOnUpdate(unsigned long deltaMs)
{
    StrongActorPtr pTarget = MakeStrongPtr(g_pApp->m_pGame->VGetActor(m_target));
	StrongActorPtr pMe = MakeStrongPtr(g_pApp->m_pGame->VGetActor(m_me));

    shared_ptr<TransformComponent> pTargetTransform = MakeStrongPtr(pTarget->GetComponent<TransformComponent>(TransformComponent::g_Name));
    shared_ptr<TransformComponent> pMyTransform = MakeStrongPtr(pMe->GetComponent<TransformComponent>(TransformComponent::g_Name));

	if (!pTarget || !pMe || !pTargetTransform || !pMyTransform)
	{
		GCC_ERROR("This shouldn't happen");
		Fail();
	}

	Vec3 targetPos = pTargetTransform->GetPosition();
	Mat4x4 myTransform = pMyTransform->GetTransform();
	Vec3 myDir = myTransform.GetDirection();
	myDir = Vec3(0.0f, 0.0f, 1.0f);
	Vec3 myPos = pMyTransform->GetPosition();

	Vec3 toTarget = targetPos - myPos;
	toTarget.Normalize();

	float dotProduct = myDir.Dot(toTarget);
	Vec3 crossProduct = myDir.Cross(toTarget);

	float angleInRadians = acos(dotProduct);

	if (crossProduct.y < 0)
		angleInRadians = -angleInRadians;
	
	Mat4x4 rotation;
	rotation.BuildRotationY(angleInRadians);
	rotation.SetPosition(myPos);
	pMyTransform->SetTransform(rotation);
}



//
// TeapotWarsLogic::VChangeState
//
void TeapotWarsLogic::VChangeState(BaseGameState newState)
{
	BaseGameLogic::VChangeState(newState);

	switch(newState)
	{
		case BGS_WaitingForPlayers:
		{

			// spawn all local players (should only be one, though we might support more in the future)
			GCC_ASSERT(m_ExpectedPlayers == 1);
			for (int i = 0; i < m_ExpectedPlayers; ++i)
			{
				shared_ptr<IGameView> playersView(GCC_NEW TeapotWarsHumanView(g_pApp->m_Renderer));
				VAddView(playersView);

				if (m_bProxy)
				{
					// if we are a remote player, all we have to do is spawn our view - the server will do the rest.
					return;
				}
			}

			// spawn all remote player's views on the game
			for (int i = 0; i < m_ExpectedRemotePlayers; ++i)
			{
				shared_ptr<IGameView> remoteGameView(GCC_NEW NetworkGameView);
				VAddView(remoteGameView);
			}

			// spawn all AI's views on the game
			for (int i = 0; i < m_ExpectedAI; ++i)
			{
				shared_ptr<IGameView> aiView(GCC_NEW AITeapotView(m_pPathingGraph));
				VAddView(aiView);
			}
			break;
		}


		case BGS_SpawningPlayersActors:
		{
			if (m_bProxy)
			{
				// only the server needs to do this.
				return;
			}

			for (auto it = m_gameViews.begin(); it != m_gameViews.end(); ++it)
			{
				shared_ptr<IGameView> pView = *it;
				if (pView->VGetType() == GameView_Human)
				{
					StrongActorPtr pActor = VCreateActor("actors\\player_teapot.xml", NULL);
					if (pActor)
					{
						shared_ptr<EvtData_New_Actor> pNewActorEvent(GCC_NEW EvtData_New_Actor(pActor->GetId(), pView->VGetId()));
                        IEventManager::Get()->VTriggerEvent(pNewActorEvent);  // [rez] This needs to happen asap because the constructor function for Lua (which is called in through VCreateActor()) queues an event that expects this event to have been handled
					}
				}
				else if (pView->VGetType() == GameView_Remote)
				{
					shared_ptr<NetworkGameView> pNetworkGameView = static_pointer_cast<NetworkGameView, IGameView>(pView);
					StrongActorPtr pActor = VCreateActor("actors\\remote_teapot.xml", NULL);
					if (pActor)
					{
						shared_ptr<EvtData_New_Actor> pNewActorEvent(GCC_NEW EvtData_New_Actor(pActor->GetId(), pNetworkGameView->VGetId()));
						IEventManager::Get()->VQueueEvent(pNewActorEvent);
					}
				}
				else if (pView->VGetType() == GameView_AI)
				{
					shared_ptr<AITeapotView> pAiView = static_pointer_cast<AITeapotView, IGameView>(pView);
					StrongActorPtr pActor = VCreateActor("actors\\ai_teapot.xml", NULL);
					if (pActor)
					{
						shared_ptr<EvtData_New_Actor> pNewActorEvent(GCC_NEW EvtData_New_Actor(pActor->GetId(), pAiView->VGetId()));
						IEventManager::Get()->VQueueEvent(pNewActorEvent);
					}
				}
			}

			break;
		}
	}
}

//
// TeapotWarsLogic::VAddView
//
void TeapotWarsLogic::VAddView(shared_ptr<IGameView> pView, ActorId actor)
{
	BaseGameLogic::VAddView(pView, actor);
//  This is commented out because while the view is created and waiting, the player has NOT attached yet. 
//	if (dynamic_pointer_cast<NetworkGameView>(pView))
//	{
//		m_HumanPlayersAttached++;
//	}
	if (dynamic_pointer_cast<TeapotWarsHumanView>(pView))
	{
		m_HumanPlayersAttached++;
	}
	else if (dynamic_pointer_cast<AITeapotView>(pView))
	{
		m_AIPlayersAttached++;
	}
}


void TeapotWarsLogic::VMoveActor(const ActorId id, Mat4x4 const &mat)
{
    BaseGameLogic::VMoveActor(id, mat);

    // [rez] HACK: This will be removed whenever the gameplay update stuff is in.  This is meant to model the death
    // zone under the grid.

	// FUTURE WORK - This would make a great basis for a Trigger actor that ran a LUA script when other
	//               actors entered or left it!

    StrongActorPtr pActor = MakeStrongPtr(VGetActor(id));
    if (pActor)
    {
        shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(pActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
        if (pTransformComponent && pTransformComponent->GetPosition().y < -25)
        {
            shared_ptr<EvtData_Destroy_Actor> pDestroyActorEvent(GCC_NEW EvtData_Destroy_Actor(id));
            IEventManager::Get()->VQueueEvent(pDestroyActorEvent);
        }
    }
}

void TeapotWarsLogic::RequestStartGameDelegate(IEventDataPtr pEventData)
{
	VChangeState(BGS_WaitingForPlayers);
}

void TeapotWarsLogic::EnvironmentLoadedDelegate(IEventDataPtr pEventData)
{
    ++m_HumanGamesLoaded;
}


// FUTURE WORK - this isn't TeapotWars specific so it can go into the game agnostic base class
void TeapotWarsLogic::RemoteClientDelegate(IEventDataPtr pEventData)
{
	// This event is always sent from clients to the game server.

	shared_ptr<EvtData_Remote_Client> pCastEventData = static_pointer_cast<EvtData_Remote_Client>(pEventData);
	const int sockID = pCastEventData->GetSocketId();
	const int ipAddress = pCastEventData->GetIpAddress();

	// go find a NetworkGameView that doesn't have a socket ID, and attach this client to that view.
    for (auto it = m_gameViews.begin(); it != m_gameViews.end(); ++it)
    {
        shared_ptr<IGameView> pView = *it;
        if (pView->VGetType() == GameView_Remote)
        {
            shared_ptr<NetworkGameView> pNetworkGameView = static_pointer_cast<NetworkGameView, IGameView>(pView);
			if (!pNetworkGameView->HasRemotePlayerAttached())
			{				
				pNetworkGameView->AttachRemotePlayer(sockID);
				CreateNetworkEventForwarder(sockID);
				m_HumanPlayersAttached++;

				return;
			}
        }
    }

}

void TeapotWarsLogic::NetworkPlayerActorAssignmentDelegate(IEventDataPtr pEventData)
{
	if (!m_bProxy)
		return;

    // we're a remote client getting an actor assignment.
    // the server assigned us a playerId when we first attached (the server's socketId, actually)
    shared_ptr<EvtData_Network_Player_Actor_Assignment> pCastEventData = static_pointer_cast<EvtData_Network_Player_Actor_Assignment>(pEventData);

	if (pCastEventData->GetActorId()==INVALID_ACTOR_ID)
	{
		m_remotePlayerId = pCastEventData->GetSocketId();
		return;
	}

	for (auto it = m_gameViews.begin(); it != m_gameViews.end(); ++it)
    {
        shared_ptr<IGameView> pView = *it;
        if (pView->VGetType() == GameView_Human)
        {
            shared_ptr<TeapotWarsHumanView> pHumanView = static_pointer_cast<TeapotWarsHumanView, IGameView>(pView);
			if (m_remotePlayerId == pCastEventData->GetSocketId())
			{
				pHumanView->VSetControlledActor(pCastEventData->GetActorId());
			}
			return;
		}
	}

	GCC_ERROR("Could not find HumanView to attach actor to!");
}

void TeapotWarsLogic::StartThrustDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_StartThrust> pCastEventData = static_pointer_cast<EvtData_StartThrust>(pEventData);
    StrongActorPtr pActor = MakeStrongPtr(VGetActor(pCastEventData->GetActorId()));
    if (pActor)
    {
        shared_ptr<PhysicsComponent> pPhysicalComponent = MakeStrongPtr(pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
        if (pPhysicalComponent)
        {
            pPhysicalComponent->ApplyAcceleration(pCastEventData->GetAcceleration());
        }
    }
}

void TeapotWarsLogic::EndThrustDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_StartThrust> pCastEventData = static_pointer_cast<EvtData_StartThrust>(pEventData);
    StrongActorPtr pActor = MakeStrongPtr(VGetActor(pCastEventData->GetActorId()));
    if (pActor)
    {
        shared_ptr<PhysicsComponent> pPhysicalComponent = MakeStrongPtr(pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
        if (pPhysicalComponent)
        {
            pPhysicalComponent->RemoveAcceleration();
        }
    }
}

void TeapotWarsLogic::StartSteerDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_StartThrust> pCastEventData = static_pointer_cast<EvtData_StartThrust>(pEventData);
    StrongActorPtr pActor = MakeStrongPtr(VGetActor(pCastEventData->GetActorId()));
    if (pActor)
    {
        shared_ptr<PhysicsComponent> pPhysicalComponent = MakeStrongPtr(pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
        if (pPhysicalComponent)
        {
            pPhysicalComponent->ApplyAngularAcceleration(pCastEventData->GetAcceleration());
        }
    }
}

void TeapotWarsLogic::EndSteerDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_StartThrust> pCastEventData = static_pointer_cast<EvtData_StartThrust>(pEventData);
    StrongActorPtr pActor = MakeStrongPtr(VGetActor(pCastEventData->GetActorId()));
    if (pActor)
    {
        shared_ptr<PhysicsComponent> pPhysicalComponent = MakeStrongPtr(pActor->GetComponent<PhysicsComponent>(PhysicsComponent::g_Name));
        if (pPhysicalComponent)
        {
            pPhysicalComponent->RemoveAngularAcceleration();
        }
    }
}

void TeapotWarsLogic::TestScriptDelegate(IEventDataPtr pEventData)
{
    shared_ptr<EvtData_ScriptEventTest_FromLua> pCastEventData = static_pointer_cast<EvtData_ScriptEventTest_FromLua>(pEventData);
    GCC_LOG("Lua", "Event received in C++ from Lua: " + ToStr(pCastEventData->GetNum()));
}

void TeapotWarsLogic::RegisterAllDelegates(void)
{
	// FUTURE WORK: Lots of these functions are ok to go into the base game logic!
    IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::RemoteClientDelegate), EvtData_Remote_Client::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::RequestStartGameDelegate), EvtData_Request_Start_Game::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::NetworkPlayerActorAssignmentDelegate), EvtData_Network_Player_Actor_Assignment::sk_EventType);
	pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::EnvironmentLoadedDelegate), EvtData_Environment_Loaded::sk_EventType);
	pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::EnvironmentLoadedDelegate), EvtData_Remote_Environment_Loaded::sk_EventType);

	// FUTURE WORK: Only these belong in TeapotWars.
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::StartThrustDelegate), EvtData_StartThrust::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::EndThrustDelegate), EvtData_EndThrust::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::StartSteerDelegate), EvtData_StartSteer::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::EndSteerDelegate), EvtData_EndSteer::sk_EventType);

    pGlobalEventManager->VAddListener(MakeDelegate(this, &TeapotWarsLogic::TestScriptDelegate), EvtData_ScriptEventTest_FromLua::sk_EventType);
}

void TeapotWarsLogic::RemoveAllDelegates(void)
{
	// FUTURE WORK: See the note in RegisterDelegates above....
    IEventManager* pGlobalEventManager = IEventManager::Get();
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::RemoteClientDelegate), EvtData_Remote_Client::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::MoveActorDelegate), EvtData_Move_Actor::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::RequestStartGameDelegate), EvtData_Request_Start_Game::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::NetworkPlayerActorAssignmentDelegate), EvtData_Network_Player_Actor_Assignment::sk_EventType);
	pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::EnvironmentLoadedDelegate), EvtData_Environment_Loaded::sk_EventType);
	pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::EnvironmentLoadedDelegate), EvtData_Remote_Environment_Loaded::sk_EventType);
	if (m_bProxy)
	{
	    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::RequestNewActorDelegate), EvtData_Request_New_Actor::sk_EventType);
	}

	// FUTURE WORK: These belong in teapot wars!
	pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::StartThrustDelegate), EvtData_StartThrust::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::EndThrustDelegate), EvtData_EndThrust::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::StartSteerDelegate), EvtData_StartSteer::sk_EventType);
    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::EndSteerDelegate), EvtData_EndSteer::sk_EventType);

    pGlobalEventManager->VRemoveListener(MakeDelegate(this, &TeapotWarsLogic::TestScriptDelegate), EvtData_ScriptEventTest_FromLua::sk_EventType);
}

void TeapotWarsLogic::CreateNetworkEventForwarder(const int socketId)
{
    NetworkEventForwarder* pNetworkEventForwarder = GCC_NEW NetworkEventForwarder(socketId);

    IEventManager* pGlobalEventManager = IEventManager::Get();

	// then add those events that need to be sent along to amy attached clients
    pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_PhysCollision::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Destroy_Actor::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Fire_Weapon::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Environment_Loaded::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_New_Actor::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Move_Actor::sk_EventType);
	pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Request_New_Actor::sk_EventType);
    pGlobalEventManager->VAddListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Network_Player_Actor_Assignment::sk_EventType);

    m_networkEventForwarders.push_back(pNetworkEventForwarder);
}

void TeapotWarsLogic::DestroyAllNetworkEventForwarders(void)
{
    for (auto it = m_networkEventForwarders.begin(); it != m_networkEventForwarders.end(); ++it)
    {
        NetworkEventForwarder* pNetworkEventForwarder = (*it);

        IEventManager* pGlobalEventManager = IEventManager::Get();
        pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_PhysCollision::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Destroy_Actor::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Fire_Weapon::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Environment_Loaded::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_New_Actor::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Move_Actor::sk_EventType);
        pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Request_New_Actor::sk_EventType);
	    pGlobalEventManager->VRemoveListener(MakeDelegate(pNetworkEventForwarder, &NetworkEventForwarder::ForwardEvent), EvtData_Network_Player_Actor_Assignment::sk_EventType);

        delete pNetworkEventForwarder;
    }

    m_networkEventForwarders.clear();
}

bool TeapotWarsLogic::VLoadGameDelegate(TiXmlElement* pLevelData)
{
    RegisterTeapotScriptEvents();
    return true;
}


