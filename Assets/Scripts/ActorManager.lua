--========================================================================
-- ActorManager.lua : Defines the ActorManager class
--
-- Part of the GameCode4 Application
--
-- GameCode4 is the sample application that encapsulates much of the source code
-- discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
-- "Rez" Graham, published by Charles River Media. 
-- ISBN-10: 1133776574 | ISBN-13: 978-1133776574
--
-- If this source code has found it's way to you, and you think it has helped you
-- in any way, do the authors a favor and buy a new copy of the book - there are 
-- detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
-- by clicking here: 
--    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
--
-- There's a companion web site at http://www.mcshaffry.com/GameCode/
-- 
-- The source code is managed and maintained through Google Code: 
--    http://code.google.com/p/gamecode4/
--
-- (c) Copyright 2012 Michael L. McShaffry and David Graham
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU Lesser GPL v3
-- as published by the Free Software Foundation.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
-- http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
--
-- You should have received a copy of the GNU Lesser GPL v3
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
--
--========================================================================

require("scripts\\TeapotAi.lua");

-- brains
require("scripts\\HardCodedBrain.lua");
require("scripts\\DecisionTreeBrain.lua");
require("scripts\\DebugBrain.lua");

-- [rez] Set this constant to change brains for teapots.  Note that there's no visual difference between 
-- the hard-coded brain and the decision tree brain, they just have different implementations under the 
-- covers.  This is a good example of how to keep the AI decision making nice and decoupled from the rest 
-- of your engine.
TEAPOT_BRAIN = DecisionTreeBrain;
--TEAPOT_BRAIN = HardCodedBrain;

-----------------------------------------------------------------------------------------------------------------------
-- Internal Processes that operate on the ActorManager.
-----------------------------------------------------------------------------------------------------------------------
ActorManagerProcess = class(ScriptProcess,
{
	_enemies = nil,  -- this points to the same table as ActorManager._enemies
});

--------------------

EnemyHealer = class(ActorManagerProcess,
{
	--
});

function EnemyHealer:OnUpdate(deltaMs)
	print("Healing all enemies");
	for id, actor in pairs(self._enemies) do
		actor.hitPoints = actor.maxHitPoints;
	end
	g_actorMgr:UpdateUi();
end

--------------------
-- Chapter 21, page 744

EnemyThinker = class(ActorManagerProcess,
{
	--
});

function EnemyThinker:OnUpdate(deltaMs)
	print("Running AI update for enemies");
	for id, actor in pairs(self._enemies) do
		actor.stateMachine:ChooseBestState();
	end
end

--------------------

EnemyUpdater = class(ActorManagerProcess,
{
	--
});

function EnemyUpdater:OnUpdate(deltaMs)
	for id, actor in pairs(self._enemies) do
		actor.stateMachine:Update(deltaMs);
	end
end


-----------------------------------------------------------------------------------------------------------------------
-- ActorManager													- Chapter 21, page 739
-- This class manages the script representation of all actors in the game.
-----------------------------------------------------------------------------------------------------------------------
ActorManager = class(nil,
{
	_player = nil,  -- this will be filled automatically when player_teapot.xml is loaded
	_enemies = {},  -- a map of enemy teapots; key = actor id
	_spheres = {},  -- a map of spheres; key = actor id
	
	-- processes
	_enemyProcesses = nil;
	
	_enemyHealer = nil,  -- process that periodically heals all enemies
	_enemyThinker = nil,  -- process that causes enemies to make a new decision
	_enemyUpdater = nil,  -- process that updates all enemy states
});

function ActorManager:AddPlayer(scriptObject)
	if (self._player ~= nil) then
		print("Attempting to add player to ActorManager when one already exists; id = " .. self._player:GetActorId());
	end

	-- add the new player regardless
	self._player = scriptObject;
	
	-- tell the human view that this is the controlled actor
	QueueEvent(EventType.EvtData_Set_Controlled_Actor, self._player:GetActorId());
end

function ActorManager:RemovePlayer(scriptObject)
	self._player = nil;
end

function ActorManager:GetPlayer()
	return self._player;
end

function ActorManager:AddEnemy(scriptObject)
	print("Adding Enemy");
	
	-- add the enemy to the list of enemies
	local actorId = scriptObject:GetActorId();
	if (self._enemies[actorId] ~= nil) then
		print("Overwriting enemy actor; id = " .. actorId);
	end
	self._enemies[actorId] = scriptObject;
	
	-- set up some sample game data
	scriptObject.maxHitPoints = 3;
	scriptObject.hitPoints = scriptObject.maxHitPoints;

	-- create the teapot brain
	local brain = nil;
	if (TEAPOT_BRAIN) then
		brain = TEAPOT_BRAIN:Create({_teapot = scriptObject});
		if (not brain:Init()) then
			print("Failed to initialize brain");
			brain = nil;
		end
	end

	-- set up the state machine
	scriptObject.stateMachine = TeapotStateMachine:Create({_teapot = scriptObject, _brain = brain});
	
	-- set the initial state
	scriptObject.stateMachine:SetState(PatrolState);
	
	-- increment the enemy count and create the enemy processes if necessary
	if (self._enemyProcesses == nil) then
		self:_CreateEnemyProcesses();
	end
	
	-- make sure the UI is up to date
	self:UpdateUi();
end

function ActorManager:RemoveEnemy(scriptObject)
	-- destroy the state machine
	if (scriptObject.stateMachine) then
		scriptObject.stateMachine:Destroy();
		scriptObject.stateMachine = nil;
	end

	-- remove the teapot from the enemy list
	local actorId = scriptObject:GetActorId();
	self._enemies[actorId] = nil;
	
	-- update the UI
	self:UpdateUi();
end

function ActorManager:GetEnemy(actorId)
	return self._enemies[actorId];
end

function ActorManager:AddSphere(scriptObject)
	local actorId = scriptObject:GetActorId();
	if (self._spheres[actorId] ~= nil) then
		print("Overwriting sphere actor; id = " .. actorId);
	end
	self._spheres[actorId] = scriptObject;
end

function ActorManager:RemoveSphere(scriptObject)
	local actorId = scriptObject:GetActorId();
	self._spheres[actorId] = nil;
end

function ActorManager:GetSphere(actorId)
	return self._spheres[actorId];
end

function ActorManager:OnFireWeapon(eventData)
	local aggressor = self:GetActorById(eventData.id);
	
	if (aggressor == nil) then
		print("FireWeapon from noone?");
		return;
	end;
	
	print("FireWeapon!");
	local pos = Vec3:Create(aggressor:GetPos());
	local lookAt = Vec3:Create(aggressor:GetLookAt());
	lookAt.y = lookAt.y + 1;
	local dir = lookAt * 2;
	pos = pos + dir;
	local ypr = Vec3:Create({x=0, y=0, z=0});
	local ball = CreateActor("actors\\sphere.xml", pos, ypr);
	if (ball ~= -1) then
		dir:Normalize();
		ApplyForce(dir, .3, ball);
	end
end

function ActorManager:OnPhysicsCollision(eventData)
	local actorA = self:GetActorById(eventData.actorA);
	local actorB = self:GetActorById(eventData.actorB);
	
	-- one of the actors isn't in the script manager
	if (actorA == nil or actorB == nil) then
		return;
	end
	
	local teapot = nil;
	local sphere = nil;
	
	if (actorA.actorType == "Teapot" and actorB.actorType == "Sphere") then
		teapot = actorA;
		sphere = actorB;
	elseif (actorA.actorType == "Sphere" and actorB.actorType == "Teapot") then
		teapot = actorB;
		sphere = actorA;
	end
	
	-- needs to be a teapot and sphere collision for us to care
	if (teapot == nil or sphere == nil) then
		return;
	end
	
	-- If we get here, there was a collision between a teapot and a sphere.  Damage the teapot.
	self:_DamageTeapot(teapot);
	
	-- destroy the sphere
	self:RemoveSphere(sphere);
	QueueEvent(EventType.EvtData_Request_Destroy_Actor, sphere:GetActorId());
	
	-- play the hit sound
	QueueEvent(EventType.EvtData_PlaySound, "audio\\computerbeep3.wav");
end

function ActorManager:GetActorById(actorId)
	local actor = nil;

	if (self._player and self._player:GetActorId() == actorId) then
		actor = self._player;
	end
	
	if (not actor) then
		actor = self:GetEnemy(actorId);
	end
	
	if (not actor) then
		actor = self:GetSphere(actorId);
	end
	
	return actor;
end

function ActorManager:_CreateEnemyProcesses()
	self._enemyProcesses = {};

	-- create all enemy processes
	-- [rez] Note: The frequency values probably look a little weird.  I chose these number because they are prime 
	-- numbers, ensuring that these two processes will rarely ever update on the same frame.
	self._enemyProcesses[#self._enemyProcesses+1] = EnemyUpdater:Create({_enemies = self._enemies});
	self._enemyProcesses[#self._enemyProcesses+1] = EnemyHealer:Create({_enemies = self._enemies, frequency = 15013});  -- ~10 seconds
	self._enemyProcesses[#self._enemyProcesses+1] = EnemyThinker:Create({_enemies = self._enemies, frequency = 3499});  -- ~ 3.5 seconds

	-- attach all the processes
	for i, proc in ipairs(self._enemyProcesses) do
		AttachProcess(proc);
	end
end

function ActorManager:UpdateUi()
	-- Build up the UI text string for the human view
	local uiText = "";
	if (self._enemies ~= nil) then
		for id, teapot in pairs(self._enemies) do
			uiText = uiText .. "Teapot " .. id .. " HP: " .. teapot.hitPoints .. "\n";
		end
	end
	
	QueueEvent(EventType.EvtData_Gameplay_UI_Update, uiText);
end

function ActorManager:_DamageTeapot(teapot)
	-- [rez] DEBUG: player is immune for now....
	if (teapot == self._player) then
		return;
	end

	-- subtract a hitpoint
	teapot.hitPoints = teapot.hitPoints - 1;

	-- if the teapot is dead, destroy it, otherwise update the UI
	if (teapot.hitPoints <= 0) then
		self:RemoveEnemy(teapot);
		QueueEvent(EventType.EvtData_Request_Destroy_Actor, teapot:GetActorId());
	else
		self:UpdateUi();
	end
end

