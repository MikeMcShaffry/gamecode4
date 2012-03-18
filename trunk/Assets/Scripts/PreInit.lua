--========================================================================
-- PreInit.lua : Script that's run when the scripting system is initialized
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

-- This file is run immediately after Lua is created but before the rest of the game initializes.  This is the place
-- to put any language extentions or pure utility functions/classes you write.

-----------------------------------------------------------------------------------------------------------------------
-- Redifinition of require() and dofile().  This is needed to allow scripts to go through the resource system.
-----------------------------------------------------------------------------------------------------------------------
do
	local OldRequire = require;  -- save the old require() function
	local resourceIdMap = {};  -- map of resource id's we've already loaded
	
	require = function(script)
		-- If we haven't already loaded this file, try to load it now
		if (not resourceIdMap[script]) then
			if (LoadAndExecuteScriptResource(script)) then
				resourceIdMap[script] = true;
			else
				-- failed to load file through the resource system so fall back to the old method
				OldRequire(script);
			end
		end
	end
end

do
    local OldPrint = print;  -- save the old print() function
    
    print = function(text)
		if (text ~= nil) then
			Log(text);
		else
			Log("<nil>");
		end
	end
end


-----------------------------------------------------------------------------------------------------------------------
-- Utility functions
-----------------------------------------------------------------------------------------------------------------------
Utils = {};

-- Concatenate the contents of the parameter list, separated by the string delimiter (just like in perl)
-- example: Utils.Join(", ", {"Anna", "Bob", "Charlie", "Dolores"})
-- results: "Anna, Bob, Charlie, Dolores"
function Utils.Join(delimiter, list)
	local len = table.getn(list)
	if len == 0 then
		return "" 
	end
	local string = list[1]
	for i = 2, len do
		string = string .. delimiter .. list[i] 
	end
	return string
end

-- Split text into a list consisting of the strings in text, separated by strings matching delimiter (which may be a pattern). 
-- example: Utils.Split(",%s*", "Anna, Bob, Charlie,Dolores")
-- results: {"Anna", "Bob", "Charlie", "Delores"}
function Utils.Split(delimiter, text)
	local list = {}
	local pos = 1
	if string.find("", delimiter, 1) then -- this would result in endless loops
		--error("delimiter matches empty string!")
		print("delimiter matches empty string!")
	end
	
	while 1 do
		local first, last = string.find(text, delimiter, pos)
		if first then -- found?
			if first < pos then
				print("[WARNING] Lua Split() function failed; first < pos\nfirst = "..first.."; last = "..last.."; pos = "..pos);
				break
			end
			table.insert(list, string.sub(text, pos, first-1))
			pos = last + 1
		else
			table.insert(list, string.sub(text, pos))
			break
		end
	end
	return list
end

function Utils.ConvertBoolToString(bool)
	if (bool == true) then
		return "true";
	else
		return "false";
	end
end

function Utils.DumpTable(table)
	for key, val in pairs(table) do
		print(key, val);
	end
end

-- private function used by Utils.DumpObject()
function Utils._BuildMemberSet(obj, memberSet)
	if (obj.__index ~= nil and obj ~= obj.__index) then
		Utils._BuildMemberSet(obj.__index, memberSet);
	end
	
	for key, val in pairs(obj) do
		if (key ~= "__index" and memberSet[key] == nil) then
			memberSet[key] = true;
		end
	end
end

function Utils.DumpObject(obj)
	-- There might be copies of the same variable at various levels in the hierarchy so we need to build an
	-- exclusive set of names.
	local memberSet = {};
	Utils._BuildMemberSet(obj, memberSet);
	
	-- Loop through the set of member names and print them out, along with the value from the original object's
	-- perspective.
	for key, val in pairs(memberSet) do
		print(key, obj[key]);
	end
end

-----------------------------------------------------------------------------------------------------------------------
-- Math stuff
-- [rez] Note: These have been moved to C++.  I left the old implementations here for educational purposes.
-----------------------------------------------------------------------------------------------------------------------
GccMath = {};

--[[
-- not used....
function GccMath.ClampToUnsignedPI(val)
	local _2PI = math.pi * 2;
	if (val < 0) then
		return val + _2PI;
	elseif (val > _2PI) then
		return val - _2PI;
	else
		return val;
	end
end
--]]

--[[
function GccMath.GetVectorFromRotation(angleRadians)
	lookAt = Vec3:Create();
	
	local angle = angleRadians - (math.pi / 2);
	if (angle < 0) then
		angle = angle + math.pi * 2;
	end
	
	lookAt.x = math.cos(angle);
	lookAt.y = math.sin(angle);
	lookAt:Normalize();  -- just in case
	
	return lookAt;
end
--]]

--[[
function GccMath.WrapPi(wrapMe)
	local twoPi = 2 * math.pi;
	if (math.abs(wrapMe) > math.pi) then
		wrapMe = wrapMe + math.pi;
		wrapMe = wrapMe - (math.floor(wrapMe * (1 / twoPi)) * twoPi);
		wrapMe = wrapMe - math.pi;
	end

	return wrapMe;
end
--]]

--[[
function GccMath.GetYRotationFromVector(lookAtVec3)
	local zUnit = {x = 0, y = 0, z = 1};  -- 0 orientation means staring down the positive Z axis
	local angle = (math.atan2(lookAtVec3.z, lookAtVec3.x * -1) - math.atan2(zUnit.z, zUnit.x));
	return GccMath.WrapPi(angle);
end
--]]



-----------------------------------------------------------------------------------------------------------------------
-- Classes & Inheritance								- Chapter 12, page 349
-----------------------------------------------------------------------------------------------------------------------
function class(baseClass, body)
	local ret = body or {};
	
	-- if there's a base class, attach our new class to it
	if (baseClass ~= nil) then
		setmetatable(ret, ret);
		ret.__index = baseClass;
		ret.base = baseClass;
	end
	
	-- Add the Create() function.  This will end up being added to each subclass which isn't ideal.  This 
	-- function should probably be stored in a single place so we only have to create it once.
	-- 		-self: The self pointer that's calling this function.
	--		-constructionData: The table sent in as construction data.  Think of it like a constructor.
	--		-originalSubClass: Used for recursion.  Don't pass anything into this parameter, it's filled 
	--						   automatically.  It's used in case the C++ side needs access to the leaf 
	--						   subclass that is being instantiated.  For an example, see ScriptProcess 
	--						   in C++.
	ret.Create = function(self, constructionData, originalSubClass)
		local obj;
		if (self.__index ~= nil) then
			if (originalSubClass ~= nil) then
				obj = self.__index:Create(constructionData, originalSubClass);
			else
				obj = self.__index:Create(constructionData, self);
			end
		else
			obj = constructionData or {};
		end
		
		setmetatable(obj, obj);
		obj.__index = self;
		
		-- copy any operators over
		if (self.__operators ~= nil) then
			for key, val in pairs(self.__operators) do
				obj[key] = val;
			end
		end
		
		return obj;
	end
	
	-- Returns true if otherClass appears in this objects class hierarchy anywhere.
	ret.IsInstance = function(self, otherClass)
		local cls = self.__index;
		while cls do
			if cls == otherClass then 
				return true
			end
			cls = cls.base
		end
		return false
	end
	
	return ret;
end


-----------------------------------------------------------------------------------------------------------------------
-- Vec3 class
-----------------------------------------------------------------------------------------------------------------------
Vec3 = class(nil, 
{
	x = 0,
	y = 0,
	z = 0,
	__operators = {},
});

function Vec3:Length()
	return math.sqrt((self.x * self.x) + (self.y * self.y) + (self.z * self.z));
end

function Vec3:Length2()
	return ((self.x * self.x) + (self.y * self.y) + (self.z * self.z));
end

function Vec3:Normalize()
	local len = self:Length();
	self.x = self.x / len;
	self.y = self.y / len;
	self.z = self.z / len;
end

function Vec3.__operators.__add(left, right)
	local temp = Vec3:Create();
	if (type(right) == "number") then
		temp.x = left.x + right;
		temp.y = left.y + right;
		temp.z = left.z + right;
	else
		temp.x = left.x + right.x;
		temp.y = left.y + right.y;
		temp.z = left.z + right.z;
	end
	return temp;
end

function Vec3.__operators.__sub(left, right)
	local temp = Vec3:Create();
	if (type(right) == "number") then
		temp.x = left.x - right;
		temp.y = left.y - right;
		temp.z = left.z - right;
	else
		temp.x = left.x - right.x;
		temp.y = left.y - right.y;
		temp.z = left.z - right.z;
	end
	return temp;
end

function Vec3.__operators.__mul(left, right)
	local temp = Vec3:Create();
	if (type(right) == "number") then
		temp.x = left.x * right;
		temp.y = left.y * right;
		temp.z = left.z * right;
	else
		temp.x = left.x * right.x;
		temp.y = left.y * right.y;
		temp.z = left.z * right.z
	end
	return temp;
end

function Vec3.__operators.__div(left, right)
	local temp = Vec3:Create();
	if (type(right) == "number") then
		temp.x = left.x / right;
		temp.y = left.y / right;
		temp.z = left.z / right;
	else
		temp.x = left.x / right.x;
		temp.y = left.y / right.y;
		temp.z = left.z / right.z;
	end
	return temp;
end


-----------------------------------------------------------------------------------------------------------------------
-- Event system
-----------------------------------------------------------------------------------------------------------------------
Event = {};
EventType = {};

