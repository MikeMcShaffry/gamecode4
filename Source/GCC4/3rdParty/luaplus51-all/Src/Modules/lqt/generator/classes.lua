require 'virtuals'
require 'templates'
require 'operators'
require 'signalslot'
require 'properties'

module('classes', package.seeall)

-- collection of all functions
local functions = {}
-- collection of bound classes
local classes = {}
-- list of files to be included
local cpp_files = {}
-- table of classes by their cname
local classlist = {}

--- Copies functions from the index.
function copy_functions(index)
	for e in pairs(index) do
		if e.label:match'^Function' then
			e.label = 'Function'
			functions[e] = true
		end
	end
end


function fix_arguments(index)
	for a in pairs(index) do
		if a.label=='Argument'
			and a.xarg.default=='1'
			and (not string.match(a.xarg.defaultvalue, '^[-+]?%d+%.?%d*[L]?$'))
			and (not string.match(a.xarg.defaultvalue, '^".*"$'))
			and a.xarg.defaultvalue~='true'
			and a.xarg.defaultvalue~='false'
			and (not string.match(a.xarg.defaultvalue, '^0[xX]%d+$')) then
			local dv, call = string.match(a.xarg.defaultvalue, '(.-)(%(%))')
			dv = dv or a.xarg.defaultvalue
			call = call or ''
			local context = a.xarg.context
			while not fullnames[context..'::'..dv] and context~='' do
				context = string.match(context, '^(.*)::') or ''
			end
			if fullnames[context..'::'..dv] then
				if fullnames[context..'::'..dv].xarg.name==fullnames[context..'::'..dv].xarg.member_of_class then
					context = string.match(context, '^(.*)::') or ''
				end
				a.xarg.defaultvalue = context..'::'..dv..call
			elseif fullnames[dv] then
				a.xarg.defaultvalue = dv..call
			else
				a.xarg.default = nil
				a.xarg.defaultvalue = nil
			end
		end
	end
end


--- Removes unneeded 'void' parameters and return values.
function fix_functions()
	for f in pairs(functions) do
		local args = {}
		for i, a in ipairs(f) do
			-- avoid bogus 'void' arguments
			if a.xarg.type_name=='void' and i==1 and f[2]==nil then break end
			if a.label=='Argument' then
				table.insert(args, a)
			end
		end
		f.arguments = args
		f.return_type = f.xarg.type_name
		if f.xarg.type_name=='void' then
			f.return_type = nil
		end
	end
end

--- Determines, if a class is public.
function class_is_public(c)
	repeat
		if c.xarg.access~='public' then return false end
		if c.xarg.member_of_class then
			local p = fullnames[c.xarg.member_of_class]
			assert(p, 'member_of_class should exist')
			assert(p.label=='Class', 'member_of_class should be a class')
			c = fullnames[c.xarg.member_of_class]
		else
			return true
		end
	until true
end

--- Selects public classes from the index, and creates templated instances
-- where appropriate.
function copy_classes(index)
	for e in pairs(index) do
		if e.label=='Class' then
			e.xarg.cname = string.gsub(e.xarg.fullname, '::', '_LQT_')
			if class_is_public(e)
				and not e.xarg.fullname:match'%b<>' then
				classes[e] = true
			elseif not e.xarg.fullname:match'%b<>' then
				ignore(e.xarg.fullname, 'not public')
			else
				if templates.should_copy(e) then
					templates.create(e, classes)
				end
			end
		end
	end
	templates.finish(index)
	for c in pairs(classes) do
		classlist[c.xarg.cname] = c
	end
end

function fix_methods_wrappers()
	for c in pairs(classes) do
		c.shell = c.public_destr
		c.shell = c.shell and (next(c.virtuals)~=nil)
		for _, constr in ipairs(c.constructors) do
			if c.shell then
				local shellname = 'lqt_shell_'..c.xarg.cname
				constr.calling_line = 'new '..shellname..'(L'
				if #(constr.arguments)>0 then constr.calling_line = constr.calling_line .. ', ' end
			else
				local shellname = c.xarg.fullname
				constr.calling_line = 'new '..shellname..'('
			end
			for i=1,#(constr.arguments) do
				constr.calling_line = constr.calling_line .. (i==1 and '' or ', ') .. 'arg' .. i
			end
			constr.calling_line = '*('..constr.calling_line .. '))'
			constr.xarg.static = '1'
			constr.return_type = constr.xarg.scope..'&'
		end
		if c.destructor then
			c.destructor.return_type = nil
		end
	end
end

--- Determines, whether classes are children of QObject.
-- Fills the 'qobject' field on class if it is child of QObject.
function get_qobjects()
	local function is_qobject(c)
		if c==nil then return false end
		if c.qobject then return true end
		if c.xarg.fullname=='QObject' then
			c.qobject = true
			return true
		end
		for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
			local base = fullnames[b]
			if is_qobject(base) then
				--debug(c.xarg.fullname, "is a QObject")
				c.qobject = true
				return true
			end
		end
		return false
	end
	for c in pairs(classes) do
		local qobj = is_qobject(c)
	end
end



local should_wrap = function(f)
	local name = f.xarg.name
	-- unfixed operator and friend, causes trouble with QDataStream
	-- if f.xarg.friend and #f.arguments ==2 then return false end
	-- not an operator - accept
	if not name:match('^operator') then return true end
	-- accept supported operators
	if operators.is_operator(name) then return true end
	return false
end



function distinguish_methods()
	for c in pairs(classes) do
		local construct, destruct, normal = {}, nil, {}
		local n = c.xarg.name:gsub('%b<>', '')

		local copy = nil
		for _, f in ipairs(c) do
			if n==f.xarg.name then
				table.insert(construct, f)
			elseif f.xarg.name:match'~' then
				destruct = f
			else
				if should_wrap(f)
					and (not f.xarg.member_template_parameters) then
					table.insert(normal, f)
				else
					ignore(f.xarg.fullname, 'operator/template/friend', c.xarg.name)
				end
			end
		end
		c.constructors = construct
		c.destructor = destruct
		c.methods = normal
	end
end

--- Determines, if a class has a public destructor. It also scans the superclasses.
-- Sets the 'public_destr' fiield on the class.
function fill_public_destr()
	local function destr_is_public(c)
		if c.destructor then
			return c.destructor.xarg.access=='public'
		else
			for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
				local base = fullnames[b]
				if base and not destr_is_public(base) then
					return false
				end
			end
			return true
		end
	end
	for c in pairs(classes) do
		c.public_destr = destr_is_public(c)
	end
end


function  generate_default_copy_constructor(c)
	if not c.xarg then return end
	
	local copy = {
		[1] = {
			label = "Argument";
			xarg = {
				context = c.xarg.name;
				id = next_id();
				name = "p";
				scope = "";
				type_base = c.xarg.name;
				type_constant = "1";
				type_name = c.xarg.name .. " const&";
				type_reference = "1";
			}
		};
		label = "Function";
		return_type = c.xarg.name;
		xarg = {
			access = "public";
			context = c.xarg.name;
			fullname = c.xarg.name.."::"..c.xarg.name;
			id = next_id();
			inline = "1";
			member_of = c.xarg.name;
			member_of_class = c.xarg.name;
			name = c.xarg.name;
			scope = c.xarg.name;
			type_base = c.xarg.name;
			type_name = c.xarg.name;
		};
	}
	copy.arguments = {copy[1]}
	
	table.insert(c, copy)
	table.insert(c.constructors, copy)
	functions[copy] = true
	
	return copy
end

-- HACK: do not create copy contructors for classes, that
-- contain variables of class '*Private' - they will not compile
-- in Qt 4.6
local function has_private_fields(c)
	for _,v in ipairs(c) do
		if v.label == 'Variable' then
			if v.xarg.type_base:match('Private') then
				ignore(c.xarg.fullname, 'cannot create copy constructor', v.xarg.fullname .. ' : ' ..v.xarg.type_base)
				return true
			end
		end
	end
	return false
end

function fill_copy_constructor()
	for c in pairs(classes) do
		local copy = nil
		for _, f in ipairs(c.constructors) do
			if #(f.arguments)==1
				and f.arguments[1].xarg.type_name==c.xarg.fullname..' const&' then
				copy = f
				break
			end
		end
		c.copy_constructor = copy
	end
	local function copy_constr_is_public(c)
		if c.copy_constructor then
			return (c.copy_constructor.xarg.access=='public')
			or (c.copy_constructor.xarg.access=='protected')
		else
			if has_private_fields(c) then return false end
			local ret = nil
			for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
				local base = fullnames[b]
				if base and not copy_constr_is_public(base) then
					return false
				end
			end
			return true
		end
	end
	for c in pairs(classes) do
		c.public_constr = copy_constr_is_public(c)
		if c.public_constr and not c.copy_constructor then
			c.copy_constructor = generate_default_copy_constructor(c)
		end
	end
end

function fill_implicit_constructor()
	typesystem.can_convert = {}
	for c in pairs(classes) do
		for _,f in ipairs(c) do
			if f.label:match("^Function") then
				-- find non-explicit constructor, which has 1 argument of type different
				-- from class, i.e. an implicit conversion constructor
				if f.xarg.name == c.xarg.name
					and #f == 1
					and (not f.xarg.access or f.xarg.access == "public")
					and f[1].xarg.type_base ~= c.xarg.name
					and not f[1].xarg.type_base:match('Private$')
					and not f.xarg.explicit
					and not c.abstract
				then
					local class_name = c.xarg.cname
					local from_type = f[1].xarg.type_base
					typesystem.can_convert[class_name] = typesystem.can_convert[class_name] or { from = {}, class = c }
					typesystem.can_convert[class_name].from[ from_type ] = f
					local safe_from = from_type:gsub('[<>*]', '_'):gsub('::', '_LQT_')
				end
			end
		end
	end
end

local function generate_implicit_code(class_name, t)
	local c = t.class
	local fullname = c.xarg.fullname
	local luaname = fullname:gsub('::', '.')

	local test_header = 'bool lqt_canconvert_' .. class_name .. '(lua_State *L, int n)'
	local convert_header = 'void* lqt_convert_' .. class_name .. '(lua_State *L, int n)' 

	local test_code = ""
	local convert_code = ""
	local tests = {}

	local order = {}
	for _, f in pairs(t.from) do
		local typ = f[1].xarg.type_name
		if not typesystem[typ] then
			ignore(typ, "implicit constructor - unknown type", _)
		else
			local test
			if typesystem[typ].raw_test then
				test = typesystem[typ].raw_test('n')
			else
				test = typesystem[typ].test('n')
			end
			if not tests[test] then
				tests[test] = true
				local test_code =
				  '  if ('..test..')\n'
				..'    return true;\n'
				
				local newtype = fullname .. '(arg)'
				if c.shell then newtype = 'lqt_shell_'..c.xarg.cname..'(L,arg)' end
				local convert_code = 
					  '  if ('..test..') {\n'
					..'    '..typ..' arg = '..typesystem[typ].get('n')..';\n'
					..'    '..fullname..' *ret = new '..newtype..';\n'
					..'    lqtL_passudata(L, ret, "'..luaname..'*");\n'
					..'    return ret;\n  }\n'

				local item = { type = fullname, test = test, defect = typesystem[typ].defect or 0,
					test_code = test_code, convert_code = convert_code }
				table.insert(order, item)
			end
		end
	end
	
	table.sort(order, function(a,b) return a.defect < b.defect end)
	for _,v in ipairs(order) do
		test_code = test_code .. v.test_code
		convert_code = convert_code .. v.convert_code
	end

	test_code = test_code .. '  return false;'
	convert_code = convert_code..'  return NULL;'

	c.implicit = {
		headers = { test = test_header, convert = convert_header },
		test = test_header .. '{\n' .. test_code .. '\n}',
		convert = convert_header .. '{\n' .. convert_code .. '\n}'
	}
end


function fill_implicit_wrappers()
	for class_name, t in pairs(typesystem.can_convert) do
		if not t.class.abstract then
			generate_implicit_code(class_name, t)
		end
	end
end


local put_class_in_filesystem = lqt.classes.insert

function fill_typesystem_with_classes()
	for c in pairs(classes) do
		classes[c] = put_class_in_filesystem(c.xarg.fullname)
	end
end


function fill_wrapper_code(f)
	if f.wrapper_code then return f end
	local stackn, argn = 1, 1
	local stack_args, defects = '', 0
	local has_args = true
	local wrap, line = '  int oldtop = lua_gettop(L);\n', ''
	if f.xarg.abstract then
		ignore(f.xarg.fullname, 'abstract method', f.xarg.member_of_class)
		return nil
	end
	if f.xarg.member_of_class and f.xarg.static~='1' then
		if not typesystem[f.xarg.member_of_class..'*'] then
			ignore(f.xarg.fullname, 'not a member of wrapped class', f.xarg.member_of_class)
			return nil
		end
		stack_args = stack_args .. typesystem[f.xarg.member_of_class..'*'].onstack
		defects = defects + 7 -- FIXME: arbitrary
		if f.xarg.constant=='1' then
			defects = defects + 8 -- FIXME: arbitrary
		end
		local sget, sn = typesystem[f.xarg.member_of_class..'*'].get(stackn)
		wrap = wrap .. '  ' .. f.xarg.member_of_class .. '* self = ' .. sget .. ';\n'
		stackn = stackn + sn
		wrap = wrap .. '  lqtL_selfcheck(L, self, "'..f.xarg.member_of_class..'");\n'
		--print(sget, sn)
		if operators.is_operator(f.xarg.name) then
			line, has_args = operators.call_line(f)
			if not line then return nil end
		else
			line = 'self->'..f.xarg.fullname..'('
		end
	else
		line = f.xarg.fullname..'('
	end
	for i, a in ipairs(f.arguments) do
		if not typesystem[a.xarg.type_name] then
			ignore(f.xarg.fullname, 'unkown argument type', a.xarg.type_name)
			return nil
		end
		local aget, an, arg_as = typesystem[a.xarg.type_name].get(stackn)
		stack_args = stack_args .. typesystem[a.xarg.type_name].onstack
		if typesystem[a.xarg.type_name].defect then defects = defects + typesystem[a.xarg.type_name].defect end
		wrap = wrap .. '  ' .. argument_name(arg_as or a.xarg.type_name, 'arg'..argn) .. ' = '
		if a.xarg.default=='1' and an>0 then
			wrap = wrap .. 'lua_isnoneornil(L, '..stackn..')'
			for j = stackn+1,stackn+an-1 do
				wrap = wrap .. ' && lua_isnoneornil(L, '..j..')'
			end
			local dv = a.xarg.defaultvalue
			wrap = wrap .. ' ? static_cast< ' .. a.xarg.type_name .. ' >(' .. dv .. ') : '
		end
		wrap = wrap .. aget .. ';\n'
		line = line .. (argn==1 and 'arg' or ', arg') .. argn
		stackn = stackn + an
		argn = argn + 1
	end
	if has_args then
		line = line .. ')'
	end
	-- FIXME: hack follows for constructors
	if f.calling_line then line = f.calling_line end
	if f.return_type then line = f.return_type .. ' ret = ' .. line end
	wrap = wrap .. '  ' .. line .. ';\n  lua_settop(L, oldtop);\n' -- lua_pop(L, '..stackn..');\n'
	if f.return_type then
		if not typesystem[f.return_type] then
			ignore(f.xarg.fullname, 'unknown return type', f.return_type)
			return nil
		end
		local rput, rn = typesystem[f.return_type].push'ret'
		wrap = wrap .. '  luaL_checkstack(L, '..rn..', "cannot grow stack for return value");\n'
		wrap = wrap .. '  '..rput..';\n  return '..rn..';\n'
	else
		wrap = wrap .. '  return 0;\n'
	end
	f.wrapper_code = wrap
	f.stack_arguments = stack_args
	f.defects = defects
	return f
end


function fill_test_code(f)
	local stackn = 1
	local test = ''
	if f.xarg.member_of_class and f.xarg.static~='1' then
		if not typesystem[f.xarg.member_of_class..'*'] then return nil end -- print(f.xarg.member_of_class) return nil end
		local stest, sn = typesystem[f.xarg.member_of_class..'*'].test(stackn)
		test = test .. ' && ' .. stest
		stackn = stackn + sn
	end
	for i, a in ipairs(f.arguments) do
		if not typesystem[a.xarg.type_name] then return nil end -- print(a.xarg.type_name) return nil end
		local atest, an = typesystem[a.xarg.type_name].test(stackn)
		if a.xarg.default=='1' and an>0 then
			test = test .. ' && (lqtL_missarg(L, ' .. stackn .. ', ' .. an .. ') || '
			test = test .. atest .. ')'
		else
			test = test .. ' && ' .. atest
		end
		stackn = stackn + an
	end
	-- can't make use of default values if I fix number of args
	test = '(lua_gettop(L)<' .. stackn .. ')' .. test
	f.test_code = test
	return f
end



function fill_wrappers()
	for f in pairs(functions) do
		local nf = fill_wrapper_code(f)
		if nf then
			nf = assert(fill_test_code(nf), nf.xarg.fullname) -- MUST pass
		else
			-- failed to generate wrapper
			functions[f] = nil
		end
	end
end

---- Output functions

function print_wrappers()
	for c in pairs(classes) do
		local meta = {}
		local wrappers = ''
		for _, f in ipairs(c.methods) do
			-- FIXME: should we really discard virtual functions?
			-- if the virtual overload in the shell uses rawget
			-- on the environment then we can leave these in the
			-- metatable
			if f.wrapper_code and not f.ignore then
				local out = 'static int lqt_bind'..f.xarg.id
				..' (lua_State *L) {\n'.. f.wrapper_code .. '}\n'
				if f.xarg.access=='public' then
					--print_meta(out)
					wrappers = wrappers .. out .. '\n'
					meta[f] = f.xarg.name
				end
			end
		end
		if not c.abstract then
			for _, f in ipairs(c.constructors) do
				if f.wrapper_code then
					local out = 'static int lqt_bind'..f.xarg.id
					    ..' (lua_State *L) {\n'.. f.wrapper_code .. '}\n'
					if f.xarg.access=='public' then
						--print_meta(out)
						wrappers = wrappers .. out .. '\n'
						meta[f] = 'new'
					end
				end
			end
		end
		--local shellname = 'lqt_shell_'..string.gsub(c.xarg.fullname, '::', '_LQT_')
		local lua_name = string.gsub(c.xarg.fullname, '::', '.')
		local out = 'static int lqt_delete'..c.xarg.id..' (lua_State *L) {\n'
		out = out ..'  '..c.xarg.fullname..' *p = static_cast<'
			..c.xarg.fullname..'*>(lqtL_toudata(L, 1, "'..lua_name..'*"));\n'
		if c.public_destr then
			out = out .. '  if (p) delete p;\n'
		end
		out = out .. '  lqtL_eraseudata(L, 1, "'..lua_name..'*");\n  return 0;\n}\n'
		--print_meta(out)
		wrappers = wrappers .. out .. '\n'
		c.meta = meta
		c.wrappers = wrappers
	end
end


local print_metatable = function(c)
	local methods = {}
	local wrappers = c.wrappers
	for m, n in pairs(c.meta) do
		methods[n] = methods[n] or {}
		table.insert(methods[n], m)
	end
	for n, l in pairs(methods) do
		local duplicates = {}
		for _, f in ipairs(l) do
			if not f.ignore then
				local itisnew = true
				for sa, g in pairs(duplicates) do
					if sa==f.stack_arguments then
					--debug("function equal: ", f.xarg.fullname, f.stack_arguments, sa, f.defects, g.defects)
						if f.defects<g.defects then
						else
							ignore(f.xarg.fullname, "duplicate function", f.stack_arguments)
							itisnew = false
						end
					elseif string.match(sa, "^"..f.stack_arguments) then -- there is already a version with more arguments
						--debug("function superseded: ", f.xarg.fullname, f.stack_arguments, sa, f.defects, g.defects)
					elseif string.match(f.stack_arguments, '^'..sa) then -- there is already a version with less arguments
						--debug("function superseding: ", f.xarg.fullname, f.stack_arguments, sa, f.defects, g.defects)
					end
				end
				if itisnew then
					duplicates[f.stack_arguments] = f
				end
			end
		end
		--[[
		local numinitial = 0
		local numfinal = 0
		for sa, f in pairs(l) do
			numinitial = numinitial + 1
		end
		for sa, f in pairs(duplicates) do
			numfinal = numfinal + 1
		end
		if numinitial-numfinal>0 then debug(c.xarg.fullname, "suppressed:", numinitial-numfinal) end
		--]]
		methods[n] = duplicates
	end
	for n, l in pairs(methods) do
		local name = operators.rename_operator(n)
		local disp = 'static int lqt_dispatcher_'..name..c.xarg.id..' (lua_State *L) {\n'
		local testcode = {}
		for tc, f in pairs(l) do
			disp = disp..'  if ('..f.test_code..') return lqt_bind'..f.xarg.id..'(L);\n'
			testcode[#testcode+1] = tc
		end
		-- disp = disp .. '  lua_settop(L, 0);\n'
		disp = disp .. '  const char * args = lqtL_getarglist(L);\n'
		disp = disp .. '  lua_pushfstring(L, "%s(%s): incorrect or extra arguments, expecting: %s.", "' ..
			c.xarg.fullname..'::'..n..'", args, '..string.format("%q", table.concat(testcode, ' or ')) .. ');\n'
		disp = disp .. '  return lua_error(L);\n}\n'
		--print_meta(disp)
		wrappers = wrappers .. disp .. '\n'
	end
	local metatable = 'static luaL_Reg lqt_metatable'..c.xarg.id..'[] = {\n'
	for n, l in pairs(methods) do
		local nn = operators.rename_operator(n)
		metatable = metatable .. '  { "'..nn..'", lqt_dispatcher_'..nn..c.xarg.id..' },\n'
	end
	metatable = metatable .. '  { "delete", lqt_delete'..c.xarg.id..' },\n'
	metatable = metatable .. '  { 0, 0 },\n};\n'
	--print_meta(metatable)
	wrappers = wrappers .. metatable .. '\n'
	local bases = ''
	for b in string.gmatch(c.xarg.bases_with_attributes or '', '([^;]*);') do
		if not string.match(b, '^virtual') then
			b = string.gsub(b, '^[^%s]* ', '')
			bases = bases .. '  {"'..string.gsub(b,'::','.')..'*", (char*)(void*)static_cast<'..b..'*>(('..c.xarg.fullname..'*)1)-(char*)1},\n'
		end
	end
	bases = 'static lqt_Base lqt_base'..c.xarg.id..'[] = {\n'..bases..'  {NULL, 0}\n};\n'
	--print_meta(bases)
	wrappers = wrappers .. bases .. '\n'
	c.wrappers = wrappers
	return c
end


function print_metatables()
	for c in pairs(classes) do
		print_metatable(c)
	end
end


function print_single_class(c)
	local n = c.xarg.cname
	local lua_name = string.gsub(c.xarg.fullname, '::', '.')
	local cppname = module_name..'_meta_'..n..'.cpp'
	table.insert(cpp_files, n) -- global cpp_files
	local fmeta = assert(io.open(output_path..cppname, 'w'))
	local print_meta = function(...)
		fmeta:write(...)
		fmeta:write'\n'
	end
	print_meta('#include "'..module_name..'_head_'..n..'.hpp'..'"\n\n')
	
	if c.implicit then
		print_meta(c.implicit.test)
		print_meta(c.implicit.convert)
	end
	
	print_meta(c.wrappers)
	if c.virtual_overloads then
		print_meta(c.virtual_overloads)
	end
	
	local getters_setters = 'NULL, NULL'
	if c.properties then
		print_meta(c.properties)
		getters_setters = 'lqt_getters'..c.xarg.id..', lqt_setters'..c.xarg.id
	end
	
	print_meta('extern "C" LQT_EXPORT int luaopen_'..n..' (lua_State *L) {')
	print_meta('\tlqtL_createclass(L, "'
		..lua_name..'*", lqt_metatable'
		..c.xarg.id..', '..getters_setters..', lqt_base'
		..c.xarg.id..');')
	
	if c.implicit then
		print_meta('\tluaL_getmetatable(L, "'..lua_name..'*");')
		print_meta('\tlua_pushliteral(L, "__test");')
		print_meta('\tlua_pushlightuserdata(L, (void*)&lqt_canconvert_'..c.xarg.cname..');')
		print_meta('\tlua_rawset(L, -3);')
		print_meta('\tlua_pushliteral(L, "__convert");')
		print_meta('\tlua_pushlightuserdata(L, (void*)&lqt_convert_'..c.xarg.cname..');')
		print_meta('\tlua_rawset(L, -3);')
		print_meta('\tlua_pop(L, 1);')
	end
	
	print_meta'\treturn 0;'
	print_meta'}'
	print_meta''
	if c.shell and c.qobject then
		print_meta([[
#include <QDebug>

QMetaObject lqt_shell_]]..n..[[::staticMetaObject;

const QMetaObject *lqt_shell_]]..n..[[::metaObject() const {
        //int oldtop = lua_gettop(L);
        lqtL_pushudata(L, this, "]]..c.xarg.fullname..[[*");
        lua_getfield(L, -1, LQT_OBJMETASTRING);
        if (lua_isnil(L, -1)) {
                lua_pop(L, 2);
                return &]]..c.xarg.fullname..[[::staticMetaObject;
        }
        lua_getfield(L, -2, LQT_OBJMETADATA);
        lqtL_touintarray(L);
        //qDebug() << "copying qmeta object for slots in ]]..c.xarg.fullname..[[";
        lqt_shell_]]..n..[[::staticMetaObject.d.superdata = &]]..c.xarg.fullname..[[::staticMetaObject;
        lqt_shell_]]..n..[[::staticMetaObject.d.stringdata = lua_tostring(L, -2);
        lqt_shell_]]..n..[[::staticMetaObject.d.data = (uint*)lua_touserdata(L, -1);
        lqt_shell_]]..n..[[::staticMetaObject.d.extradata = 0; // slot_metaobj->d.extradata;
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_OBJMETADATA);
        lua_setfield(L, LUA_REGISTRYINDEX, LQT_OBJMETASTRING);
        lua_pop(L, 1);
        //qDebug() << (lua_gettop(L) - oldtop);
        return &lqt_shell_]]..n..[[::staticMetaObject;
}

int lqt_shell_]]..n..[[::qt_metacall(QMetaObject::Call call, int index, void **args) {
        //qDebug() << "fake calling!";
        index = ]]..c.xarg.fullname..[[::qt_metacall(call, index, args);
        if (index < 0) return index;
        return lqtL_qt_metacall(L, this, lqtSlotAcceptor_]]..module_name..[[, call, "]]..c.xarg.fullname..[[*", index, args);
}
]])
	end
	fmeta:close()
end

function print_merged_build()
	local path = output_path
	local mergename = module_name..'_merged_build'
	local merged = assert(io.open(path..mergename..'.cpp', 'w'))
	for _, p in ipairs(cpp_files) do
		merged:write('#include "',module_name,'_head_',p,'.hpp"\n')
	end
	merged:write('\n')
	for _, p in ipairs(cpp_files) do
		merged:write('#include "',module_name,'_meta_',p,'.cpp"\n')
	end
	local pro_file = assert(io.open(path..mergename..'.pro', 'w'))

	local print_pro= function(...)
		pro_file:write(...)
		pro_file:write'\n'
	end
	print_pro('TEMPLATE = lib')
	print_pro('TARGET = '..module_name)
	print_pro('INCLUDEPATH += .')
	print_pro('HEADERS += '..module_name..'_slot.hpp')
	print_pro('SOURCES += ../common/lqt_common.cpp \\')
	print_pro('          ../common/lqt_qt.cpp \\')
	print_pro('          '..module_name..'_enum.cpp \\')
	print_pro('          '..module_name..'_meta.cpp \\')
	print_pro('          '..module_name..'_slot.cpp \\')
	print_pro('          '..mergename..'.cpp')
end


function print_class_list()
	local qobject_present = false
	local big_picture = {}
	local type_list_t = {}
	for c in pairs(classes) do
		local n = c.xarg.cname
		if n=='QObject' then qobject_present = true end
		print_single_class(c)
		table.insert(big_picture, n)
		table.insert(type_list_t, 'add_class \''..c.xarg.fullname..'\'\n')
	end

	local type_list_f = assert(io.open(output_path..module_name..'_types.lua', 'w'))
	type_list_f:write([[
#!/usr/bin/lua
local types = (...) or {}
assert(lqt.classes.insert, 'module lqt.classes not loaded')
local function add_class(class)
	lqt.classes.insert(class, true)
end
]])
	for k, v in ipairs(type_list_t) do
		type_list_f:write(v)
	end
	type_list_f:write('return types\n')
	type_list_f:close()

	print_merged_build()
	local fmeta = assert(io.open(output_path..module_name..'_meta.cpp', 'w'))
	local print_meta = function(...)
		fmeta:write(...)
		fmeta:write'\n'
	end
	print_meta()
	print_meta('#include "lqt_common.hpp"')
	print_meta('#include "'..module_name..'_slot.hpp'..'"\n\n')
	for _, p in ipairs(big_picture) do
		print_meta('extern "C" LQT_EXPORT int luaopen_'..p..' (lua_State *);')
	end
	print_meta('void lqt_create_enums_'..module_name..' (lua_State *);')
	print_meta('extern "C" LQT_EXPORT int luaopen_'..module_name..' (lua_State *L) {')
	for _, p in ipairs(big_picture) do
		print_meta('\tluaopen_'..p..'(L);')
	end
	print_meta('\tlqt_create_enums_'..module_name..'(L);')
	if qobject_present then
		print_meta('\tlqtL_qobject_custom(L);')
	end
	print_meta('\t//lua_pushlightuserdata(L, (void*)&LqtSlotAcceptor::staticMetaObject);')
	print_meta('\t//lua_setfield(L, LUA_REGISTRYINDEX, LQT_METAOBJECT);')
	print_meta('\t//lqtL_passudata(L, (void*)(new LqtSlotAcceptor(L)), "QObject*");')
	print_meta('\t//lua_setfield(L, LUA_REGISTRYINDEX, LQT_METACALLER);')
	print_meta('\tlqtL_register_super(L);')
	print_meta('\tlqtSlotAcceptor_'..module_name..' = new LqtSlotAcceptor(L);')
	print_meta('\treturn 0;\n}')
	if fmeta then fmeta:close() end
end

------------------------------------------------------------

function preprocess(index)
	copy_classes(index) -- picks classes if not private and not blacklisted
	copy_functions(index) -- picks functions and fixes label
	fix_arguments(index) -- fixes default arguments if they are context-relative
	fix_functions() -- fixes name and fullname and fills arguments
	operators.fix_operators(index)
end

function process(index, typesystem, filterfiles)
	for _, f in ipairs(filterfiles) do
		classes = loadfile(f)(classes)
	end

	virtuals.fill_virtuals(classes) -- does that, destructor ("~") excluded
	distinguish_methods() -- does that
	fill_public_destr() -- does that: checks if destructor is public
	fill_copy_constructor() -- does that: checks if copy contructor is public or protected
	fill_implicit_constructor()
	fix_methods_wrappers()
	get_qobjects()

	fill_typesystem_with_classes()
	fill_wrappers()
	virtuals.fill_virtual_overloads(classes) -- does that
	virtuals.fill_shell_classes(classes) -- does that
	properties.fill_properties(classes)
	fill_implicit_wrappers()

	signalslot.process(functions)
end

function output()
	virtuals.print_shell_classes(classes) -- does that, and outputs headers
	virtuals.print_virtual_overloads(classes) -- does that

	print_wrappers(classes) -- just compiles metatable list
	print_metatables(classes) -- just collects the wrappers + generates dispatchers
	print_class_list(classes) -- does that + prints everything related to class

	signalslot.output()
end
