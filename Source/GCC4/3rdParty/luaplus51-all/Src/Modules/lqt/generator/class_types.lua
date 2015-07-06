#!/usr/bin/lua

lqt = lqt or {}
lqt.classes = lqt.classes or {}

-- Field explanation:
-- * push - push the instance of a class onto Lua stack
-- * get - retrieve the instance from Lua stack
-- * raw_test - true if the stack index is an instance
-- * test - true if the stack index is an instance, or if it is convertible to the type
-- * bound - true for generated classes (false for native types)
-- * foreign - comes from other module (like QtCore)

local pointer_t = function(fn, foreign)
	local cn = string.gsub(fn, '::', '.')
	return {
		-- the argument is a pointer to class
		push = function(n)
			return 'lqtL_pushudata(L, '..n..', "'..cn..'*")', 1
		end,
		get = function(n)
			return 'static_cast<'..fn..'*>'
			..'(lqtL_toudata(L, '..n..', "'..cn..'*"))', 1
		end,
		test = function(n)
			return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1
		end,
		raw_test = function(n) return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1 end,
		onstack = cn..'*,',
		bound = true,
		foreign = foreign,
	}
end
local pointer_const_t = function(fn, foreign)
	local cn = string.gsub(fn, '::', '.')
	return {
		-- the argument is a pointer to constant class instance
		push = function(n)
			return 'lqtL_pushudata(L, '..n..', "'..cn..'*")', 1
		end,
		get = function(n)
			local val
			if typesystem.can_convert[cn] then
				val = 'lqtL_convert(L, '..n..', "'..cn..'*")'
			else
				val = 'lqtL_toudata(L, '..n..', "'..cn..'*")'
			end
			return 'static_cast<'..fn..'*>('..val..')', 1
		end,
		test = function(n)
			if typesystem.can_convert[cn] then
				return 'lqtL_canconvert(L, '..n..', "'..cn..'*")', 1
			else
				return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1
			end
		end,
		raw_test = function(n) return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1 end,
		onstack = cn..'*,',
		bound = true,
		foreign = foreign,
	}
end
local ref_t = function(fn, foreign)
	local cn = string.gsub(fn, '::', '.')
	return {
		-- the argument is a reference to class
		push = function(n)
			return 'lqtL_pushudata(L, &'..n..', "'..cn..'*")', 1
		end,
		get = function(n)
			return '*static_cast<'..fn..'*>'
			..'(lqtL_toudata(L, '..n..', "'..cn..'*"))', 1
		end,
		test = function(n)
			return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1
		end,
		onstack = cn..'*,',
		bound = true,
		foreign = foreign,
	}
end
local instance_t = function(fn, foreign)
	local cn = string.gsub(fn, '::', '.')
	return {
		-- the argument is the class itself
		push = function(n)
			return 'lqtL_copyudata(L, &'..n..', "'..cn..'*")', 1
		end,
		get = function(n)
			return '*static_cast<'..fn..'*>'
			..'(lqtL_toudata(L, '..n..', "'..cn..'*"))', 1
		end,
		test = function(n)
			return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1
		end,
		onstack = cn..'*,',
		bound = true,
		foreign = foreign,
	}
end
local const_ref_t = function(fn, foreign)
	local cn = string.gsub(fn, '::', '.')
	return {
		-- the argument is a pointer to class
		push = function(n)
			return 'lqtL_copyudata(L, &'..n..', "'..cn..'*")', 1, string.gsub(fn, ' const&$', '')
		end,
		get = function(n)
			local val
			if typesystem.can_convert[cn] then
				val = 'lqtL_convert(L, '..n..', "'..cn..'*")'
			else
				val = 'lqtL_toudata(L, '..n..', "'..cn..'*")'
			end
			return '*static_cast<'..fn..'*>('..val..')', 1
		end,
		test = function(n)
			if typesystem.can_convert[cn] then
				return 'lqtL_canconvert(L, '..n..', "'..cn..'*")', 1
			else
				return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1
			end
		end,
		raw_test = function(n) return 'lqtL_isudata(L, '..n..', "'..cn..'*")', 1 end,
		onstack = cn..'*,',
		bound = true,
		foreign = foreign,
	}
end

local const_ptr_ref_t = pointer_const_t

lqt.classes.insert = function(cname, foreign)
	if typesystem[cname]==nil then
		typesystem[cname..'*'] = pointer_t(cname, foreign)
		typesystem[cname..' const*'] =  pointer_const_t(cname, foreign)
		typesystem[cname..'&'] = ref_t(cname, foreign)

		typesystem[cname] = instance_t(cname, foreign)
		typesystem[cname..' const'] = instance_t(cname, foreign)
		typesystem[cname..' const&'] = const_ref_t(cname, foreign)
		typesystem[cname..'* const&'] = const_ptr_ref_t(cname, foreign)

		return true
	else
		return nil
	end
end



