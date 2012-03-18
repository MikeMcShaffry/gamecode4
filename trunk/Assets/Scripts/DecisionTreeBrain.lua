--========================================================================
-- DecisionTreeBrain.lua : A brain implemented using a simple decision tree
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

require("Scripts\\TeapotBrains.lua");
require("Scripts\\TeapotStates.lua");

-----------------------------------------------------------------------------------------------------------------------
-- DecisionTreeBrain													- Chapter 18, page 622
-- This brain uses a simple decision tree.  The nodes are all reusable, making it much more extendable than the 
-- hard-coded brain above.
-----------------------------------------------------------------------------------------------------------------------
DecisionTreeBrain = class(TeapotBrain,
{
	_root = nil,
});

function DecisionTreeBrain:Init()
	self:_BuildDecisionTree();
	return true;
end

function DecisionTreeBrain:Think()
	return self._root:Decide();
end


-- This function manually builds the decision tree.  You would never do this in a real game, you'd have trees 
-- defined in XML and build them from that definition instead.  For complex games, you would often have an 
-- editor that generated the XML for you.
function DecisionTreeBrain:_BuildDecisionTree()
	self._root = IsObjectCloseNode:Create({_teapot = self._teapot, _testObjId = g_actorMgr:GetPlayer():GetActorId()});
	local healthNode = IsHealthLowNode:Create({_teapot = self._teapot});
	
	healthNode:SetTrueNode(ActionNode:Create({_action = RunAwayState}));
	healthNode:SetFalseNode(ActionNode:Create({_action = AttackState}));

	self._root:SetTrueNode(healthNode);
	self._root:SetFalseNode(ActionNode:Create({_action = PatrolState}));
end


-----------------------------------------------------------------------------------------------------------------------
-- DecisionNode
-- This is a single decision node from which all other decision nodes derive.  Implementations of this class must 
-- define the Decide() funtion, which must return either the result of another Decide() function or the new state.
-----------------------------------------------------------------------------------------------------------------------
DecisionNode = class(nil,
{
	_brain = nil,
	_trueNode = nil,
	_falseNode = nil,
});

function DecisionNode:Decide()
	error("Calling unimplemented function DecisionNode.Decide()");
	return nil;
end

function DecisionNode:SetTrueNode(node)
	self._trueNode = node;
end

function DecisionNode:SetFalseNode(node)
	self._falseNode = node;
end


-----------------------------------------------------------------------------------------------------------------------
-- ActionNode
-- Action nodes represent the actual decision being made.  They are the leaf nodes of a decision tree.  Their Decide()
-- function simply returns the state they were assigned.
-----------------------------------------------------------------------------------------------------------------------
ActionNode = class(DecisionNode,
{
	_action = nil;
});

function ActionNode:Decide()
	return self._action;
end

function ActionNode:SetTrueNode(node)
	error("Action nodes cannot have children");
end

function ActionNode:SetFalseNode(node)
	error("Action nodes cannot have children");
end


-----------------------------------------------------------------------------------------------------------------------
-- IsObjectCloseNode
-- This decision node will branch true if the object is valid and "close".  Otherwise, it branches false.
-----------------------------------------------------------------------------------------------------------------------
IsObjectCloseNode = class(DecisionNode,
{
	_testObjId = nil,
	_closeDistance = 25,  -- default definition for what "close" means
});

function IsObjectCloseNode:Decide()
	if (self._testObjId) then
		local actor = g_actorMgr:GetActorById(self._testObjId);
		if (actor) then
			local actorPos = Vec3:Create(actor:GetPos());
			local teapotPos = Vec3:Create(self._teapot:GetPos());
			local diff = actorPos - teapotPos;
			if (diff:Length() <= self._closeDistance) then
				return self._trueNode:Decide();
			end
		end
	end
	
	return self._falseNode:Decide();
end


-----------------------------------------------------------------------------------------------------------------------
-- IsHealthLowNode
-- This decision node will branch true if the teapot's health is "low".  Otherwise, it branches false.
-----------------------------------------------------------------------------------------------------------------------
IsHealthLowNode = class(DecisionNode,
{
	_lowValuePercentage = 0.34,  -- default definition of what "low" means
});

function IsHealthLowNode:Decide()
	local hitPointPercentageLeft = self._teapot.hitPoints / self._teapot.maxHitPoints;
	if (hitPointPercentageLeft <= self._lowValuePercentage) then
		return self._trueNode:Decide();
	else
		return self._falseNode:Decide();
	end
end

