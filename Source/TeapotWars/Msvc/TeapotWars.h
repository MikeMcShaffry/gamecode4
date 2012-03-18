#pragma once
//========================================================================
// TeapotWars.h : source file for the sample game
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


#pragma warning( disable : 4996 ) // 'function' declared deprecated - gets rid of all those 2005 warnings....

class PathingGraph;
class IEventManager;


//---------------------------------------------------------------------------------------------------------------------
// TeapotWarsApp class                          - Chapter 21, page 722
// Note: Don't put anything in this class that needs to be destructed
//---------------------------------------------------------------------------------------------------------------------
class TeapotWarsApp : public GameCodeApp
{
protected:
	virtual BaseGameLogic *VCreateGameAndView();

public:
	virtual TCHAR *VGetGameTitle() { return _T("Teapot Wars"); }
	virtual TCHAR *VGetGameAppDirectory() { return _T("Game Coding Complete 4\\Teapot Wars\\4.0"); }
	virtual HICON VGetIcon();

protected:
    virtual void VRegisterGameEvents(void);
    virtual void VCreateNetworkEventForwarder(void);
    virtual void VDestroyNetworkEventForwarder(void);
};


//---------------------------------------------------------------------------------------------------------------------
// TeapotWarsLogic class                        - Chapter 21, page 723
//---------------------------------------------------------------------------------------------------------------------
class TeapotWarsLogic : public BaseGameLogic
{
protected:
    std::list<NetworkEventForwarder*> m_networkEventForwarders;

public:
	TeapotWarsLogic();
	virtual ~TeapotWarsLogic();

	// TeapotWars Methods

	// Update
	virtual void VSetProxy();
    virtual void VMoveActor(const ActorId id, Mat4x4 const &mat);

	// Overloads
	virtual void VChangeState(BaseGameState newState);
	virtual void VAddView(shared_ptr<IGameView> pView, ActorId actorId=INVALID_ACTOR_ID);
	virtual shared_ptr<IGamePhysics> VGetGamePhysics(void) { return m_pPhysics; }

    // event delegates
    void RequestStartGameDelegate(IEventDataPtr pEventData);
    void RemoteClientDelegate(IEventDataPtr pEventData);
    void NetworkPlayerActorAssignmentDelegate(IEventDataPtr pEventData);
	void EnvironmentLoadedDelegate(IEventDataPtr pEventData);
    void ThrustDelegate(IEventDataPtr pEventData);
    void SteerDelegate(IEventDataPtr pEventData);
    void StartThrustDelegate(IEventDataPtr pEventData);
    void EndThrustDelegate(IEventDataPtr pEventData);
    void StartSteerDelegate(IEventDataPtr pEventData);
    void EndSteerDelegate(IEventDataPtr pEventData);

    void TestScriptDelegate(IEventDataPtr pEventData);

protected:
    virtual bool VLoadGameDelegate(TiXmlElement* pLevelData);

private:
    void RegisterAllDelegates(void);
    void RemoveAllDelegates(void);
    void CreateNetworkEventForwarder(const int socketId);
    void DestroyAllNetworkEventForwarders(void);
};
