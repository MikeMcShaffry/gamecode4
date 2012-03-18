-- PreInit.lua
-- Note: This file only defines the class() function from assets\scripts\PreInit.lua.  The other stuff isn't needed
-- for this simple demo.

-----------------------------------------------------------------------------------------------------------------------
-- Classes & Inheritance
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


