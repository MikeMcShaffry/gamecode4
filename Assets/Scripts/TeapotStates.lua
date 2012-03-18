--========================================================================
-- ActorManager.lua : Defines all the states for an AI controlled teapot
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

-----------------------------------------------------------------------------------------------------------------------
-- TeapotState										- Chapter 18, page 619
-- This is the base class for all Teapot states
-----------------------------------------------------------------------------------------------------------------------
TeapotState = class(nil,
{
	_teapot = nil,
	_teapotMoveSpeed = 5.25,
});

function TeapotState:Init()
	if (self._teapot == nil) then
		print("Invalid teapot in TeapotState");
		return false;
	end
	
	return true;
end

function TeapotState:Update(deltaMs)
	error("Calling unimplemented TeapotState.Update() fucntion");
end


-----------------------------------------------------------------------------------------------------------------------
-- PatrolState
-----------------------------------------------------------------------------------------------------------------------
PatrolState = class(TeapotState, 
{
	name = "PatrolState",

	_patrolPoints =
	{
		Vec3:Create({ x = 20, y = 1, z = 20 }),
		Vec3:Create({ x = -20, y = 1, z = 20 }),
	};
	
	_patrolPointIndex = 1;
	_patrolSpeedMultiplier = 0.75;
	
	_hasStartedRotation = false;
	_hasFinishedRotating = false;
	
	_angularAcceleration = 7;
	
	_waitTimer = 0;
});

function PatrolState:Update(deltaMs)
	if (self._waitTimer > 0) then
		self._waitTimer = self._waitTimer - deltaMs;
		return;
	end

	local teapotPos = Vec3:Create(self._teapot:GetPos());
	local diff = self._patrolPoints[self._patrolPointIndex] - teapotPos;

	if (not self._hasFinishedRotating) then
		-- get necessary rotation
		local targetOrientation = GccMath.GetYRotationFromVector(diff);  -- this is the orientation we need to be at to face the player
		local currOrientation = self._teapot:GetYOrientationRadians();
	
		-- if we're not facing the right direction, start rotating if we haven't already
		if (math.abs(targetOrientation - currOrientation) > 0.01) then
			if (not self._hasStartedRotation) then
				self._teapot:RotateY(targetOrientation);
				self._targetOrientation = targetOrientation;
				self._hasStartedRotation = true;
			end
			
		else
			self._hasFinishedRotating = true;
		end
	
	else  -- done rotating
		-- grab the distance to the patrol point and see if we're already there
		local distanceToPatrolPoint = diff:Length();
		if (distanceToPatrolPoint < 0.1) then
			self:GetNextPatrolPoint();
			return;
		end

		-- not there yet so calculate the velocity vector, which provides direction and speed
		local velocityVector = Vec3:Create(diff);  -- clone the diff vector
		velocityVector:Normalize();
		local speed = (self._teapotMoveSpeed * self._patrolSpeedMultiplier) * (deltaMs / 1000);
		velocityVector = velocityVector * speed;
		
		-- if we're about to overshoot our desitination, adjust the velocity so that we land on it
		if (velocityVector:Length() > distanceToPatrolPoint) then
			velocityVector = diff;
		end

		-- send the event
		teapotPos = teapotPos + velocityVector;
		self._teapot:SetPosition(teapotPos.x, teapotPos.y, teapotPos.z);
	end
end

function PatrolState:GetNextPatrolPoint()
	-- update the patrol point index and wrap if necessary
	self._patrolPointIndex = self._patrolPointIndex + 1;
	if (self._patrolPointIndex > #self._patrolPoints) then
		self._patrolPointIndex = 1;
	end
	
	-- reset rotation flags
	self._hasStartedRotation = false;
	self._hasFinishedRotating = false;
	
	-- wait here for a bit
	self._waitTimer = 1000;
end


-----------------------------------------------------------------------------------------------------------------------
-- AttackState
-----------------------------------------------------------------------------------------------------------------------
AttackState = class(TeapotState,
{
	name = "AttackState",

	_hasStartedRotating = false,
	_targetOrientation = nil,
	_weaponsRange = {15, 18},
	
	_fireInterval = 1500,
	_fireTimer = 0,
});

function AttackState:Init()
	self.base.Init(self);
	
	-- initialize target orientation to something reasonable
	self._targetOrientation = self._teapot:GetYOrientationRadians();
end

function AttackState:Update(deltaMs)
	-- update the fire timer
	self._fireTimer = self._fireTimer + deltaMs;

	-- get necessary rotation
	local playerPos = Vec3:Create(g_actorMgr:GetPlayer():GetPos());
	local teapotPos = Vec3:Create(self._teapot:GetPos());
	local diff = playerPos - teapotPos;
	local targetOrientation = GccMath.GetYRotationFromVector(diff);  -- this is the orientation we need to be at to face the player
	local currOrientation = self._teapot:GetYOrientationRadians();
	
	-- see if we need to recalculate the rotation
	if (math.abs(self._targetOrientation - targetOrientation) > 0.01) then
		self._hasStartedRotation = false;
	end

	-- if we're not facing the right direction, start rotating if we haven't already
	if (math.abs(targetOrientation - currOrientation) > 0.01) then
		if (not self._hasStartedRotation) then
			self._teapot:RotateY(targetOrientation);
			self._targetOrientation = targetOrientation;
			self._hasStartedRotation = true;
		end

	-- if we're already facing the right direction, pretend we rotated there
	elseif (not self._hasStartedRotation) then
		self._hasStartedRotation = true;

	-- if we get here, we're done rotating
	else
		self._hasStartedRotation = false;
	
		local distanceToPlayer = diff:Length();
		diff:Normalize();
		
		-- check to see if we're within firing range
		if (distanceToPlayer >= self._weaponsRange[1] and distanceToPlayer <= self._weaponsRange[2]) then
			self:_AttemptToFireWeapon();
			
		-- not in range, so towards (or away from) then player
		else
			-- teapot is too close, back up
			if (distanceToPlayer < self._weaponsRange[1]) then
				diff = diff * -1;
			end

			local speed = self._teapotMoveSpeed * (deltaMs / 1000);
			diff = diff * speed;
			-- QueueEvent(EventType.EvtData_AiTranslate, {id = self._teapot:GetActorId(), distanceAndDirection = diff});
			teapotPos = teapotPos + diff;
			self._teapot:SetPosition(teapotPos.x, teapotPos.y, teapotPos.z);
		end
	end
end

function AttackState:_AttemptToFireWeapon()
	if (self._fireTimer >= self._fireInterval) then
        QueueEvent(EventType.EvtData_Fire_Weapon, {id = self._teapot:GetActorId()});
		self._fireTimer = 0;
	end
end


-----------------------------------------------------------------------------------------------------------------------
-- RunAwayState
-----------------------------------------------------------------------------------------------------------------------
RunAwayState = class(TeapotState,
{
	name = "RunAwayState",

	_runPoints =
	{
		Vec3:Create({ x = 25, y = 1, z = 25 }),
		Vec3:Create({ x = -25, y = 1, z = 25 }),
		Vec3:Create({ x = 25, y = 1, z = -25 }),
		Vec3:Create({ x = -25, y = 1, z = -25 }),
	},
	
	_runPointIndex = 0;
	_hasRotated = false;
	
	_checkForBetterRunPointTimer = 0;
	_checkForBetterRunPointFrequency = 2000;  -- check for a better run point every 2 seconds
});

function RunAwayState:Init()
	self.base.Init(self);
	self:_FindRunPoint();
end

function RunAwayState:Update(deltaMs)
	-- update the timer and see if it's time to check for a better run point
	self._checkForBetterRunPointTimer = self._checkForBetterRunPointTimer + deltaMs;
	if (self._checkForBetterRunPointTimer >= self._checkForBetterRunPointFrequency) then
		self:_FindRunPoint();
		self._checkForBetterRunPointTimer = 0;
	end

	-- grab the teapot position and the delta to the run point
	local teapotPos = Vec3:Create(self._teapot:GetPos());
	local diff = self._runPoints[self._runPointIndex] - teapotPos;

	if (not self._hasRotated) then
		local teapotPos = Vec3:Create(self._teapot:GetPos());
		local diff = self._runPoints[self._runPointIndex] - teapotPos;
		local targetOrientation = GccMath.GetYRotationFromVector(diff);  -- this is the orientation we need to be at to face the player
		local currOrientation = self._teapot:GetYOrientationRadians();

		-- already facing the right way
		if (math.abs(currOrientation - targetOrientation) < 0.01) then
			self._hasRotated = true;
			
		-- start rotating
		else
			self._teapot:RotateY(targetOrientation);
			self._hasRotated = true;
		end

		return;
		
	-- teapot has already started rotation so start moving
	else
		-- grab the distance to the run point and see if we're already there
		local distanceToRunPoint = diff:Length();
		if (distanceToRunPoint < 0.1) then
			return;
		end

		-- not there yet so calculate the velocity vector, which provides direction and speed
		local velocityVector = Vec3:Create(diff);  -- clone the diff vector
		velocityVector:Normalize();
		local speed = self._teapotMoveSpeed * (deltaMs / 1000);
		velocityVector = velocityVector * speed;
		
		-- if we're about to overshoot our desitination, adjust the velocity so that we land on it
		if (velocityVector:Length() > distanceToRunPoint) then
			velocityVector = diff;
		end

		-- send the event
		-- QueueEvent(EventType.EvtData_AiTranslate, {id = self._teapot:GetActorId(), distanceAndDirection = velocityVector});
		teapotPos = teapotPos + velocityVector;
		self._teapot:SetPosition(teapotPos.x, teapotPos.y, teapotPos.z);		
	end
end

function RunAwayState:_FindRunPoint()
	local oldRunPoint = self._runPointIndex;
	local playerPos = Vec3:Create(g_actorMgr:GetPlayer():GetPos());
	
	-- find the furthest run point from the player
	local bestDistance2 = nil;
	for index, value in ipairs(self._runPoints) do
		local diff = value - playerPos;
		local distance2 = diff:Length2();
		if (bestDistance2 == nil or distance2 > bestDistance2) then
			self._runPointIndex = index;
			bestDistance2 = distance2;
		end
	end
	
	-- if we found a better run point, we need to rotate to the new point
	if (self._runPointIndex ~= oldRunPoint) then
		self._hasRotated = false;
	end
	
	return (self._runPointIndex ~= oldRunPoint);
end



-----------------------------------------------------------------------------------------------------------------------
-- WatchPlayerState (for debugging)
-----------------------------------------------------------------------------------------------------------------------
WatchPlayerState = class(TeapotState,
{
	name = "WatchPlayerState"
});

function WatchPlayerState:Init()
	self.base.Init(self);
end

function WatchPlayerState:Update(deltaMs)
	-- grab the teapot position and the delta to the run point
	local teapotPos = Vec3:Create(self._teapot:GetPos());
	local playerPos = Vec3:Create(g_actorMgr:GetPlayer():GetPos());
	local diff = playerPos - teapotPos;

	local targetOrientation = GccMath.GetYRotationFromVector(diff);  -- this is the orientation we need to be at to face the player

	self._teapot:RotateY(targetOrientation);
end


-----------------------------------------------------------------------------------------------------------------------
-- RotateState (for debugging)
-----------------------------------------------------------------------------------------------------------------------
RotateState = class(TeapotState,
{
	name = "RotateState"
});

function RotateState:Init()
	self.base.Init(self);
	
	self._teapot:RotateY(0);
	self._nextOrientation = 0.5;
	self._delayTime = 5000;
	self._timer = self._delayTime;
end

function RotateState:Update(deltaMs)
	-- update timer
	if (self._timer > 0) then
		self._timer = self._timer - deltaMs;
		return;
	end
	
	self._timer = self._delayTime;
	self._teapot:RotateY(self._nextOrientation);
	
	self._nextOrientation = self._nextOrientation + 0.5;
	GccMath.WrapPi(self._nextOrientation);
end



