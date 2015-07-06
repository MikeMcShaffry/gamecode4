module('operators', package.seeall)

local operatorTrans = {
	['<<'] = 'IN',
	['>>'] = 'OUT',
	['+='] = 'ADD',
	['-='] = 'SUB',
	['*='] = 'MUL',
	['/='] = 'DIV',
	['++'] = 'INC',
	['--'] = 'DEC',
	['+'] = '__add',
	['-'] = '__sub',
	['_'] = '__unm',
	['*'] = '__mul',
	['/'] = '__div',
	['=='] = '__eq',
}

local function is_helper_function(name)
	local s1 = name:sub(1,1)
	local s2 = name:sub(2,2)
	return s1 == 'q' and s2 == s2:upper()
end

function fix_operators(index)
	for f in pairs(index) do
		if f.label == "Function" then
			if f.xarg.name:match("^operator") and f.xarg.friend then
				if f[1].xarg.type_base == f.xarg.member_of then
					-- overloaded friend operator - its first argument is 'this', 
					-- needs to be removed
					table.remove(f, 1)
					table.remove(f.arguments, 1)
					if f.xarg.name == 'operator-' and #f.arguments == 0 then
						-- use '_' as a marker for unary minus
						f.xarg.name = 'operator_'
					end
				else
					-- operator in form: number OP class - do not bind these
					f.ignore = true
				end
			elseif is_helper_function(f.xarg.name) then
				f.ignore = true
			end
		end
	end
end

function call_line(f)
	local op = operators.get_operator(f.xarg.name)
	if op == "*" and #f.arguments == 0 then
		ignore(f.xarg.fullname, "pointer dereference operator", f.xarg.member_of_class)
		return nil
	elseif op == '_' then
		-- unary minus
		return '- *self', false
	elseif op == '++' or op == '--' then
		-- the ++ and -- operators don't line () at the end, like: *self ++()
		if f.arguments[1] then
			f.arguments[1] = nil
			return op..' *self', false
		else
			return '*self '..op, false
		end
	else
		return '*self '..op..'(', true
	end
end

function get_operator(name)
	return name:match('^operator(.+)$')
end

function is_operator(name)
	return name:match('^operator.') and operatorTrans[get_operator(name)]
end

function rename_operator(name)
	local trans = operatorTrans[get_operator(name)]
	if is_operator(name) and trans then
		return trans
	end
	return name
end
