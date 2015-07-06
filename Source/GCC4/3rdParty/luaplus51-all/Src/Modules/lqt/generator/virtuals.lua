module('virtuals', package.seeall)

--- Retrieves the virtual method for each class. Also retrieves the virtual
-- methods for all superclasses.
function fill_virtuals(classes)
	local byname = {}
	for c in pairs(classes) do
		byname[c.xarg.fullname] = c
	end
	local function get_virtuals(c, includePrivate)
		local ret = {}
		for _, f in ipairs(c) do
			if f.label=='Function' and f.xarg.virtual=='1' then
				local n = string.match(f.xarg.name, '~') or f.xarg.name
				if n~='~' and n~='metaObject' then ret[n] = f end
			end
		end
		for b in string.gmatch(c.xarg.bases or '', '([^;]+);') do
			local base = byname[b]
			if type(base)=='table' then
				local bv = get_virtuals(base, true)
				for n, f in pairs(bv) do
					if not ret[n] then ret[n] = f end
				end
			end
		end
		for _, f in ipairs(c) do
			if f.label=='Function'
				and (includePrivate or f.xarg.access~='private')
				and (ret[string.match(f.xarg.name, '~') or f.xarg.name]) then
				f.xarg.virtual = '1'
				local n = string.match(f.xarg.name, '~')or f.xarg.name
				ret[n] = f
			end
		end
		return ret
	end
	for c in pairs(classes) do
		c.virtuals = get_virtuals(c)
	end
end


--- Generates a virtual overload for function 'v'.
-- Returns nil if a parameter or return type is of unknown/ignored type. Normal
-- virtual methods call original virtual method if no corresponding Lua function is
-- found, pure virtual (abstract) methods throw Lua error.
function virtual_overload(v)
	local ret = ''
	if v.virtual_overload then return v end
	-- make return type
	if v.return_type and not typesystem[v.return_type] then
		ignore(v.xarg.fullname, 'unknown return type', v.return_type)
		return nil, 'return: '..v.return_type
	end
	local rget, rn, ret_as = '', 0
	if v.return_type then rget, rn, ret_as = typesystem[v.return_type].get'oldtop+2' end
	local retget = ''
	if v.return_type then
		local atest, an = typesystem[v.return_type].test('oldtop+2')
		retget = [[if (!(]]..atest..[[)) {
        luaL_error(L, "Unexpected virtual method return type: %s; expecting %s\nin: %s",
          luaL_typename(L,oldtop+2), "]]..v.return_type..[[", lqtL_source(L,oldtop+1));
      }
      ]]
		retget = retget .. argument_name(ret_as or v.return_type, 'ret') .. ' = ' .. rget .. ';\n      '
	end
	retget = retget .. 'lua_settop(L, oldtop);\n      return' .. (v.return_type and ' ret' or '')
	-- make argument push
	local pushlines, stack = make_pushlines(v.arguments)
	if not pushlines then
		ignore(v.xarg.fullname, 'unknown argument type', stack)
		return nil, 'argument: '..stack
	end
	-- make lua call
	local luacall = 'lqtL_pcall(L, '..(stack+1)..', '..rn..', 0)'
	-- make prototype and fallback
	local proto = (v.return_type or 'void')..' ;;'..v.xarg.name..' ('
	local fallback = ''
	for i, a in ipairs(v.arguments) do
		proto = proto .. (i>1 and ', ' or '')
		.. argument_name(a.xarg.type_name, 'arg'..i)
		fallback = fallback .. (i>1 and ', arg' or 'arg') .. i
	end
	proto = proto .. ')' .. (v.xarg.constant=='1' and ' const' or '')
	fallback = (v.return_type and 'return this->' or 'this->') .. v.xarg.fullname .. '(' .. fallback .. ');'
	if v.xarg.abstract then
		fallback = 'luaL_error(L, "Abstract method %s not implemented! In %s", "' .. v.xarg.name .. '", lqtL_source(L,oldtop+1));'
	end
	ret = proto .. [[ {
  int oldtop = lua_gettop(L);
  lqtL_pushudata(L, this, "]]..string.gsub(v.xarg.member_of_class, '::', '.')..[[*");
  lqtL_getoverload(L, -1, "]]..v.xarg.name..[[");
  lua_pushvalue(L, -1); // copy of function
  if (lua_isfunction(L, -1)) {
    lua_insert(L, -3);
    lua_insert(L, -3);
]] .. pushlines .. [[
    if (!]]..luacall..[[) {
      ]]..retget..[[;
    } else {
      if (lqtL_is_super(L, lua_gettop(L))) {
        lua_settop(L, oldtop);
        ]]..fallback..[[ 
      } else
        lua_error(L);
    }
  }
  lua_settop(L, oldtop);
  ]] .. fallback .. '\n}\n'
	v.virtual_overload = ret
	v.virtual_proto = string.gsub(proto, ';;', '', 1)
	return v
end



function fill_virtual_overloads(classes)
	for c in pairs(classes) do
		if c.virtuals then
			for i, v in pairs(c.virtuals) do
				if v.xarg.access~='private' then
					local vret, err = virtual_overload(v)
					if not vret and v.xarg.abstract then
						-- cannot create instance without implementation of an abstract method
						c.abstract = true
					end
				end
			end
		end
	end
end



function fill_shell_class(c)
	local shellname = 'lqt_shell_'..c.xarg.cname
	local shell = 'class LQT_EXPORT ' .. shellname .. ' : public ' .. c.xarg.fullname .. ' {\npublic:\n'
	shell = shell .. '  lua_State *L;\n'
	for _, constr in ipairs(c.constructors) do
		if constr.xarg.access~='private' then
			local cline = '  '..shellname..' (lua_State *l'
			local argline = ''
			for i, a in ipairs(constr.arguments) do
				cline = cline .. ', ' .. argument_name(a.xarg.type_name, 'arg'..i)
				argline = argline .. (i>1 and ', arg' or 'arg') .. i
			end
			cline = cline .. ') : ' .. c.xarg.fullname
				.. '(' .. argline .. '), L(l) '
				.. '{\n    lqtL_register(L, this);\n'
			if c.protected_enums then
				cline = cline .. '    registerEnums();\n'
			end
			cline = cline .. '  }\n'
			shell = shell .. cline
		end
	end
	if c.copy_constructor==nil and c.public_constr then
		local cline = '  '..shellname..' (lua_State *l, '..c.xarg.fullname..' const& arg1)'
		cline = cline .. ' : ' .. c.xarg.fullname .. '(arg1), L(l) {}\n'
		shell = shell .. cline
	end
	for i, v in pairs(c.virtuals) do
		if v.xarg.access~='private' then
			if v.virtual_proto then shell = shell .. '  virtual ' .. v.virtual_proto .. ';\n' end
		end
	end
	shell = shell .. '  ~'..shellname..'() { lqtL_unregister(L, this); }\n'
	if c.shell and c.qobject then
		shell = shell .. '  static QMetaObject staticMetaObject;\n'
		shell = shell .. '  virtual const QMetaObject *metaObject() const;\n'
		shell = shell .. '  virtual int qt_metacall(QMetaObject::Call, int, void **);\n'
		shell = shell .. 'private:\n'
		shell = shell .. '      Q_DISABLE_COPY('..shellname..');\n'
	end
	if c.protected_enums then
		shell = shell .. '  void registerEnums() {\n'
		for _,e in ipairs(c.protected_enums) do
			shell = shell .. e.enum_table
			shell = shell .. '    lqtL_createenum(L, lqt_enum'..e.xarg.id..', "'..string.gsub(e.xarg.fullname, "::", ".")..'");\n'
		end
		shell = shell .. '  }\n'
	end
	shell = shell .. '};\n'
	c.shell_class = shell
	return c
end


function fill_shell_classes(classes)
	for c in pairs(classes) do
		if c.shell then
			local nc = fill_shell_class(c)
			if not nc then
				 -- FIXME: useless, but may change
				ignore(c.xarg.fullname, 'failed to generate shell class')
				classes[c] = nil
			end
		end
	end
end

----------------------------------------------------------------------

function print_shell_classes(classes)
	for c in pairs(classes) do
		local n = c.xarg.cname
		local fhead = assert(io.open(output_path..module_name..'_head_'..n..'.hpp', 'w'))
		local print_head = function(...)
			fhead:write(...)
			fhead:write'\n'
		end
		print_head('#ifndef LQT_HEAD_'..n)
		print_head('#define LQT_HEAD_'..n)
		print_head(output_includes)
		--print_head('#include <'..string.match(c.xarg.fullname, '^[^:]+')..'>')
		print_head''
		if c.shell then
			print_head('#include "'..module_name..'_slot.hpp'..'"\n\n')
			if c.shell_class then
				print_head(c.shell_class)
			else
				dump(c)
			end
		end
		
		print_head('extern "C" LQT_EXPORT int luaopen_'..n..' (lua_State *);')
		print_head('\n\n#endif // LQT_HEAD_'..n)
		fhead:close()
	end
	return classes
end

function print_virtual_overloads(classes)
	for c in pairs(classes) do
		if c.shell then
			local vo = ''
			local shellname = 'lqt_shell_'..c.xarg.cname
			for _,v in pairs(c.virtuals) do
				if v.virtual_overload then
					vo = vo .. string.gsub(v.virtual_overload, ';;', shellname..'::', 1)
				end
			end
			c.virtual_overloads = vo
		end
	end
	return classes
end


