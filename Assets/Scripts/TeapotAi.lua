--========================================================================
-- TeapotAi.lua : Defines the state machine for AI teapots
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

require("Scripts\\TeapotStates.lua");
require("Scripts\\TeapotBrains.lua");

-----------------------------------------------------------------------------------------------------------------------
-- State machine								- Chapter 18, page 616
-----------------------------------------------------------------------------------------------------------------------
TeapotStateMachine = class(nil,
{
	_teapot = nil,
	_currentState = nil,
	_brain = nil,
});

function TeapotStateMachine:Destroy()
	self._currentState = nil;
	self._brain = nil;
end

function TeapotStateMachine:SetState(newState)
	if (self._currentState == nil or not self._currentState:IsInstance(newState)) then
		self:_InternalSetState(newState);
	end
end

function TeapotStateMachine:ChooseBestState()
	if (self._brain) then
		local newState = self._brain:Think();
		
		if (newState ~= nil) then
			print("Teapot chose " .. newState.name);
		else
			print("Teapot chose nil");
		end
		
		self:SetState(newState);
	end
end

function TeapotStateMachine:Update(deltaMs)
	if (self._currentState) then
		self._currentState:Update(deltaMs);
	end
end

function TeapotStateMachine:_InternalSetState(newState)
	self._currentState = newState:Create({_teapot = self._teapot});
	self._currentState:Init();
end

