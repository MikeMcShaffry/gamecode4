#!/usr/bin/lua

--[[

Copyright (c) 2007-2008 Mauro Iazzi

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--]]

local binder = {}
local B = nil

-- a string devised to not compile. should not make into code. never.
binder.ERRORSTRING = '<<>>'
binder.WRAPCALL = '__LuaWrapCall__'
binder.debug_type = function(el) return (type(el)=='table'and (el.tag .. ' ' .. el.attr.id) or el) end

function binder:init(filename)
  --require 'lxp'
  --require 'lxp.lom'
  
  if not self.tree then
    local xmlf = io.open(filename, 'r')
    local xmls = xmlf:read('*a')
    xmlf:close()
    --self.tree = lxp.lom.parse(xmls)
		xml.id = {}
		self.tree = xml:collect(xmls)
  end
  
	self.ids = xml.id
  self.type_names = {}
  self.types_to_stack = {
    ['char * *'] = function(i) return 'lqtL_pusharguments(L, ' .. tostring(i) .. ')' end,
    ['const char * *'] = function(i) return 'lqtL_pusharguments(L, ' .. tostring(i) .. ')' end,
    ['const char *'] = function(i) return 'lua_pushstring(L, ' .. tostring(i) .. ')' end,
    ['short int'] =              function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['unsigned short int'] =     function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['short unsigned int'] =     function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['int'] =                    function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['unsigned int'] =           function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['long int'] =               function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['unsigned long int'] =      function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['long unsigned int'] =      function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['long long int'] =          function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['unsigned long long int'] = function(i) return 'lua_pushinteger(L, ' .. tostring(i) .. ')' end,
    ['float'] =  function(i) return 'lua_pushnumber(L, ' .. tostring(i) .. ')' end,
    ['double'] = function(i) return 'lua_pushnumber(L, ' .. tostring(i) .. ')' end,
    ['bool'] = function(i) return 'lua_pushboolean(L, ' .. tostring(i) .. ')' end,
    ['void *'] =   function(i) return 'lua_pushlightuserdata(L, ' .. tostring(i) .. ')' end,
    ['void * *'] = function(i) return 'lua_pushlightuserdata(L, ' .. tostring(i) .. ')' end,
  }
  self.types_from_stack = {
    ['int&'] = function(i) return 'lqtL_tointref(L, ' .. tostring(i) .. ')' end,
    ['char * *'] = function(i) return 'lqtL_toarguments(L, ' .. tostring(i) .. ')' end,
    ['const char * *'] = function(i) return 'lqtL_toarguments(L, ' .. tostring(i) .. ')' end,
    ['const char *'] = function(i) return 'lua_tostring(L, ' .. tostring(i) .. ')' end,
    ['short int'] =              function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['unsigned short int'] =     function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['short unsigned int'] =     function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['int'] =                    function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['unsigned int'] =           function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['long int'] =               function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['unsigned long int'] =      function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['long unsigned int'] =      function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['long long int'] =          function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['unsigned long long int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
    ['float'] =  function(i) return 'lua_tonumber(L, ' .. tostring(i) .. ')' end,
    ['double'] = function(i) return 'lua_tonumber(L, ' .. tostring(i) .. ')' end,
    ['bool'] = function(i) return '(bool)lua_toboolean(L, ' .. tostring(i) .. ')' end,
    ['void *'] =   function(i) return 'lua_touserdata(L, ' .. tostring(i) .. ')' end,
    ['void * *'] = function(i) return 'static_cast<void **>(lua_touserdata(L, ' .. tostring(i) .. '))' end,
  }
  self.types_test = {
    ['char * *'] = function(i) return 'lqtL_testarguments(L, ' .. tostring(i) .. ')' end,
    ['const char * *'] = function(i) return 'lqtL_testarguments(L, ' .. tostring(i) .. ')' end,
    ['const char *'] = function(i) return '(lua_type(L, ' .. tostring(i) .. ')==LUA_TSTRING)' end,
    ['short int'] =              function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['unsigned short int'] =     function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['short unsigned int'] =     function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['int'] =                    function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['unsigned int'] =           function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['long int'] =               function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['unsigned long int'] =      function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['long unsigned int'] =      function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['long long int'] =          function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['unsigned long long int'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['float'] =  function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['double'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
    ['bool'] = function(i) return 'lua_isboolean(L, ' .. tostring(i) .. ')' end,
    ['void *'] =   function(i) return 'lua_isuserdata(L, ' .. tostring(i) .. ')' end,
    ['void * *'] = function(i) return 'lua_isuserdata(L, ' .. tostring(i) .. ')' end,
  }
--   self.conditions = {}
  
  return self.tree
end

function binder.wrapclass(n)
  return 'LuaBinder< '..n..' >'
end

function binder.lua_proto(s)
  return 'int '..s..' (lua_State *L)'
end

function binder.fake_pointer (id)
  return { tag='PointerType', attr={ type=id } }
end

function binder:find(f, t)
  t = t or self.tree
  if type(t)~='table' then return nil end;
  if f(t) then return t end
  local ret = nil
  for k,v in pairs(t) do
    ret = ret or self:find(f, v)
    if ret then break end
  end;
  return ret;
end

function binder.name_search (n)
  return function (t)
      return (type(t)=='table') and (type(t.attr)=='table') and (t.attr.name==n)
    end
end

function binder.id_search (i)
  return function (t)
      return (type(t)=='table') and (type(t.attr)=='table') and (t.attr.id==i) -- or ((type(i)=='table') and i[t.attr.id])
    end
end

function binder.tag_search (n)
  return function (t)
      return (type(t)=='table') and (t.tag==n) -- or ((type(i)==table) and i[t.attr.id])
    end
end

function binder.pointer_search(id)
  return function (t)
    return (type(t)=='table') and (t.tag=='PointerType') and (type(t.attr)=='table') and (t.attr.type==id)
  end
end

function binder:find_name (n)
  if not self.names then self.names = {} end
  if not self.names[n] then
    self.names[n] = self:find(self.name_search(n))
  end
  return self.names[n]
end

function binder:find_id (n)
  if not self.ids then self.ids = {} end
  if not self.ids[n] then
    self.ids[n] = self:find(self.id_search(n))
  end
  return self.ids[n]
end

function binder:find_pointer (n)
  if not self.pointers then self.pointers = {} end
  if not self.pointers[n] then
    self.pointers[n] = self:find(self.pointer_search(n.attr.id))
  end
  return self.pointers[n]
end

function binder:context_name(el)
    if type(el.attr)~='table' then return '' end
    if type(el.attr.context)~='string' then return '' end
    
    local context_el = self:find_id(el.attr.context)
    
    if not context_el then return '' end
    
    local context = (context_el.attr.name=='::') and '' or (context_el.attr.name..'::')
    return context
end

function binder:type_name(el)
--     print('getting name of', el, el.tag)
    
  self.type_names = self.type_names or {}
  local t = self.type_names
  
  if t[el] then return t[el] end
  
      if el.tag == 'FundamentalType' then
    t[el] = el.attr.name
  elseif (el.tag == 'Class') or (el.tag == 'Struct') or (el.tag=='Union') then
    t[el] = self:context_name(el) .. el.attr.name
  elseif el.tag == 'Typedef' then
    t[el] = self:type_name(self:find_id(el.attr.type))
  elseif el.tag == 'PointerType' then
    t[el] = self:type_name(self:find_id(el.attr.type)) .. ' *'
  elseif el.tag == 'ReferenceType' then
    t[el] = self:type_name(self:find_id(el.attr.type)) .. '&'
  elseif el.tag == 'CvQualifiedType' then
    t[el] = ( (el.attr.volatile=='1') and 'volatile ' or '' )
         .. ( (el.attr.const=='1') and 'const ' or '' )
         .. self:type_name(self:find_id(el.attr.type))
  elseif el.tag == 'Enumeration' then
    t[el] = self:context_name(el) .. el.attr.name
  else
    error('cannot determine type name: ' .. self.debug_type(el))
  end
  return t[el]
end

function binder.wrapcall(m, overload, n)
  if m.tag=='Method' then
    return binder.WRAPCALL..m.attr.name..(overload and '__OverloadedVersion'..tostring(n) or '')
  elseif m.tag=='Constructor' then
    return binder.WRAPCALL..m.attr.name..'__new'..(overload and '__OverloadedVersion'..tostring(n) or '')
  elseif m.tag=='Destructor' then
    -- cannot be overloaded, true?
    return binder.WRAPCALL..m.attr.name..'__delete'..(overload and '__OverloadedVersion'..tostring(n) or '')
  end
  return false
end

function binder.arguments_of(f)
  local ret = {}
  for argi = 1, table.maxn(f) do
    if (type(f[argi])=='table') and (f[argi].tag=='Argument') then
      table.insert(ret, f[argi])
    end
  end
  return ret
end


function binder:base_type(el)
  local ret = self:find_id(el.attr.type)
  while (ret.tag=='Typedef') or (ret.tag=='CvQualifiedType') do
    ret = self:find_id(ret.attr.type)
  end
  return ret
end

function binder:type_from_stack(el)
  local t = self.types_from_stack
  if t[el] then return t[el] end
  
  local name = self:type_name(el)
  if t[name] then
    t[el] = t[name]
    return t[el]
  end

  if (el.tag=='Class') or (el.tag=='Struct') or (el.tag=='Union') then
    t[el] = function(i) return '**static_cast<'..name..'**>(lqtL_checkudata(L, '..tostring(i)..', "' ..name.. '*"))' end
  elseif (el.tag=='CvQualifiedType') then
    t[el] = self:type_from_stack(self:find_id(el.attr.type))
  elseif (el.tag=='ReferenceType') then
    t[el] = self:type_from_stack(self:find_id(el.attr.type))
  elseif (el.tag=='Typedef') then
    t[el] = self:type_from_stack(self:find_id(el.attr.type))
  elseif (el.tag=='Enumeration') then
    t[el] = function (i) return 'static_cast<'..name..'>(lqtL_toenum(L, '..tostring(i)..', "'..name..'"))' end
  elseif (el.tag=='PointerType') then
    local b = self:base_type(el)
    local base_t = self:type_from_stack(b)
    t[el] = (type(base_t)=='function') and function (i)
              local base = base_t(i)
              local c = string.sub(base, 1, 1)
              if (c=='*') then
                return string.sub(base, 2)
              else
                return 'static_cast<'..name..'>(lua_touserdata(L, '..tostring(i)..'))'
              end
            end or function (i) return '0' end
  elseif (el.tag=='FunctionType') then -- FIXME
  end
  
  if t[el]==nil then
    error('cannot deternime how to retrieve type: '.. ((type(el)=='table') and (el.tag..' '..el.attr.id) or el))
  end
  return t[el]
end


function binder:type_to_stack(el)
  local t = self.types_to_stack
  
  if t[el] then return t[el] end
  
  local name = self:type_name(el)
--   print (el.tag, '|'..name..'|', rawget(t,el) or '<>')
  
  if t[name] then
    t[el] = t[name]
    return t[el]
  end

  if (el.tag=='Class') or (el.tag=='Struct') or (el.tag=='Union') then
    -- FIXME: force deep copy if possible
    t[el] = function(i) return 'lqtL_passudata(L, new '..name..'('..tostring(i)..'), "'..name..'*")' end
  elseif (el.tag=='CvQualifiedType') then -- FIXED? FIXME: this is just a mess
    local base_t = self:base_type(el)
    local non_cv = self:type_to_stack(base_t)
    --if (base_t.tag=='Class') or (base_t.tag=='Struct') or (base_t.tag=='Union') then else end
    t[el] = non_cv
  elseif (el.tag=='ReferenceType') then
    local base_t = self:base_type(el)
		if (base_t.tag=='Class') or (base_t.tag=='Struct') or (base_t.tag=='Union') then
			t[el] = function(i) return 'lqtL_pushudata(L, &('..tostring(i)..'), "'..self:type_name(base_t)..'*")' end
		else
			t[el] = self:type_to_stack(self:find_id(el.attr.type))
		end
  elseif (el.tag=='Typedef') then
    t[el] = self:type_to_stack(self:find_id(el.attr.type))
  elseif (el.tag=='Enumeration') then
    t[el] = function (i) return 'lqtL_pushenum(L, '..tostring(i)..', "'..name..'")' end
  elseif (el.tag=='PointerType') then
    local base_t = self:base_type(el)
    t[el] = function(i) return 'lqtL_pushudata(L, '..tostring(i)..', "'..self:type_name(base_t)..'*")' end
  end
  
--   print (el.tag, el, rawget(t,el) or '<>')
  if t[el]==nil then
    error('cannot deternime how to push on stack type: '.. self.debug_type(el))
  end
  return t[el]
end



function binder:type_test(el)
  local t = self.types_test
  
  if t[el] then return t[el] end
  
  local name = self:type_name(el)
  
  if t[name] then
    t[el] = t[name]
    return t[el]
  end

  if (el.tag=='Class') or (el.tag=='Struct') or (el.tag=='Union') then
    t[el] = function(i) return 'lqtL_testudata(L, ' .. tostring(i) .. ', "' .. name .. '*")' end
  elseif (el.tag=='CvQualifiedType') then
    t[el] = self:type_test(self:find_id(el.attr.type))
  elseif (el.tag=='ReferenceType') then
    t[el] = self:type_test(self:find_id(el.attr.type))
  elseif (el.tag=='Typedef') then
    t[el] = self:type_test(self:find_id(el.attr.type))
  elseif (el.tag=='Enumeration') then
    t[el] = function (i) return 'lqtL_isenum(L, '..tostring(i)..', "'..name..'")' end
  elseif (el.tag=='PointerType') then
    t[el] = self:type_test(self:find_id(el.attr.type)) or function() return '(true)' end
  elseif (el.tag=='FunctionType') then -- FIXME
  end
  
--   print (el.tag, el, rawget(t,el) or '<>')
  if t[el]==nil then
    error('cannot deternime how to test type: '.. self.debug_type(el))
  end
  return t[el]
end


function binder:function_body(f)
  if f.attr.pure_virtual=='1' then error'cannot call pure vitual functions' end
	-- FIXME could be more precise... problem is copy constructor of QObject if private
  if f.tag=='Constructor' and f.attr.artificial=='1' then
		error'refuse to bind implicitly defined constructors'
	end

  local body = '{\n'
  local has_this = 0
  --local base_class = nil
  local args = self.arguments_of(f)
  local funcname = self:context_name(f) .. f.attr.name
  local ret_type = nil
  local pointer_to_class = self.fake_pointer(f.attr.context)
  
  --if f.attr.context then base_class = self:find_id(f.attr.context) end
  --if base_class and ((base_class.tag=='Class') or (base_class.tag=='Struct')) then
    --pointer_base = self:find(self.pointer_search(f.attr.context)) 
  --end
  
  -- NEEDS THIS POINTER?
  if ( (f.tag=='Method') and (f.attr.static~='1') ) or (f.tag == 'Destructor') then
    local pointer_base = pointer_to_class
    body = body .. '  ' .. self:type_name(pointer_base) .. '& __lua__obj = '
                .. self:type_from_stack(pointer_base)(1) .. ';\n';
---[==[
    body = body .. [[
	if (__lua__obj==0) {
		lua_pushstring(L, "trying to reference deleted pointer");
		lua_error(L);
		return 0;
	}
]]
--]==]
    has_this = 1
  end
  
  -- GETTING ARGUMENTS
  for argi = 1, table.maxn(args) do
    local arg = args[argi]
    local argname = 'arg' .. tostring(argi)
    
    local argt = self:find_id(arg.attr.type)
    local argtype = self:type_name(argt)
    local def = arg.attr.default
    
    if def and string.match(string.sub(def, 1, 1), '[%l%u]') then
      def = self:context_name(argt)..def
    elseif def then
      def = 'static_cast< ' .. argtype .. ' >(' .. def .. ')'
    end
    
    --print ('getting arg type', argtype, arg.attr.type)
    
    body = body .. '  ' .. argtype .. ' ' .. argname .. ' = '
                .. (def and (self:type_test(argt)(argi+has_this) .. '?') or '')
                .. self:type_from_stack(argt)(argi+has_this)
    body = body .. (def and (':' .. tostring(def)) or '') .. ';\n' --  '// default = '..tostring(def)..'\n'
  end
  body = body .. '  '
  
  if f.tag=='Constructor' then
		--[[
		local my_class = self:find_id(f.attr.context)
		if my_class.attr.abstract='1' then error'cannot construct abstract class' end
		--]]
    ret_type = pointer_to_class
    funcname = 'new ' .. self.wrapclass(f.attr.name)
    
--     ret_type = self:find_id(f.attr.context) -- wrong?
--     body = body .. self:type_name(ret_type) .. ' * ret = new '
--     print (self:type_name(ret_type))
  elseif f.tag=='Destructor' then
    -- TREATED AS SPECIAL CASE
    body = body .. 'delete __lua__obj;\n'
    body = body .. '  __lua__obj = 0;\n';
    body = body .. '  return 0;\n}\n'
    return body
  else
    ret_type = self:find_id(f.attr.returns)
  end

  -- GET RETURN TYPE
  if ret_type.attr.name=='void' then
    ret_type = nil
  else
    body = body .. self:type_name(ret_type) .. ' ret = '
  end

  -- CALL FUNCTION    
  if has_this==1 then
    body = body .. '__lua__obj->' .. funcname .. '('
  else
    body = body .. funcname .. '('
  end
  
  -- IF OVERRIDING CONSTRUCTOR ADD STATE POINTER
  if f.tag=='Constructor' then
    body = body .. 'L' .. ((table.maxn(args) > 0) and ', ' or '')
  end
  
  -- ADD ARGS TO FUNCTION CALL
  if table.maxn(args) > 0 then body = body .. 'arg1' end
  for argi = 2, table.maxn(args) do
    body = body .. ', arg' .. tostring(argi)
  end
  
  body = body .. ');\n'

  -- HANDLE RETURN VALUE
  if f.tag=='Constructor' then
		body = body .. '  lqtL_passudata(L, ret, "'..f.attr.name..'*");\n'
    body = body .. '  return 1;\n}\n'
	elseif ret_type then
    -- print('pushing', binder:type_name(ret_type))
    local ret_to_stack = self:type_to_stack(ret_type)'ret'
    body = body .. '  ' .. ret_to_stack .. ';\n'
    body = body .. '  return 1;\n}\n'
  else
    body = body .. '  return 0;\n}\n'
  end
  
  return body
end

function binder:function_test(p, score)
  local ret = ''
  local isstatic = 0
  
  ret = ret .. '  ' .. score .. ' = 0;\n'
  
  if p.attr.static~='1' and p.tag=='Method' then
    ret = ret .. '  ' .. score .. ' += ' .. self:type_test( self.fake_pointer(p.attr.context) )(1)
              .. '?premium:-premium*premium;\n' 
    isstatic = 1
  end
  
  local args = self.arguments_of(p)
  
  for argi = 1, table.maxn(args) do
    local arg = args[argi]
    --print ( 'ARGUMENT TEST', argi)
    local argname = 'arg' .. tostring(argi)
    if (type(arg)=='table') and (arg.tag=='Argument') then
      ret = ret .. '  if (' .. self:type_test( self:find_id(arg.attr.type) )(argi+isstatic)  .. ') {\n'
      ret = ret .. '    ' .. score .. ' += premium;\n'
      ret = ret .. '  } else if (' .. tostring(arg.attr.default and true or false) .. ') {\n'
      ret = ret .. '    ' .. score .. ' += premium-1; // '..tostring(arg, arg.attr.default)..';\n'
      ret = ret .. '  } else {\n'
      ret = ret .. '    ' .. score .. ' -= premium*premium;\n'
      ret = ret .. '  }\n'
      
--       ret = ret .. '  ' .. score .. ' += ' .. type_on_stack_test( find_id(arg.attr.type) , argi+isstatic )
--                 .. '?' .. tostring(premium) .. ':-' .. tostring(premium) .. '*' .. tostring(premium) .. ';\n' 
    end
  end
  
  return ret
end

function binder:get_members (c)
  if not self.members then self.members = {} end
  if not self.members[c] then
    local ret = { functions={}, enumerations={}, classes={}, methods={}, constructors={}, virtuals={} }
    for s in string.gmatch(c.attr.members, '(_%d+) ') do
      local m = self:find_id(s)
      local n = m.attr.name
      print("member of", c.attr.name , "found:", s, "name:", n)

			local filtered, motive = false, ''
			if self.filter then
				filtered, motive = self.filter(m)
			end

			if filtered then
				print('Filtered member: '..n..' for '..(motive or 'no apparent reason.'))
			elseif m.tag=='Enumeration' then
        table.insert(ret.enumerations, m)
      elseif m.tag=='Function' then
        ret.functions[n] = ret.functions[n] or {}
        table.insert(ret.functions[n], m)
      elseif m.tag=='Method' and (m.attr.access=='public') then
        ret.methods[n] = ret.methods[n] or {}
        table.insert(ret.methods[n], m)
      elseif m.tag=='Constructor' and (m.attr.access=='public') then
        table.insert(ret.constructors, m)
      elseif m.tag=='Destructor' then
        ret.destructor = m
      elseif m.tag=='Class' or m.tag=='Struct' then
        table.insert(ret.classes, m)
      end

      if (m.attr.virtual=='1') and (m.tag~='Constructor') and (m.tag~='Destructor') and not filtered then
        table.insert(ret.virtuals, m)
      end
      --[[
      local n = n..' < < of type > > '.. m.tag ..' < < with access > > ' .. m.attr.access
      ret.cache[n] = ret.cache[n] or {}
      table.insert(ret.cache[n], m)
      ]]
    end
    self.members[c] = ret
  end
  return self.members[c]
end

function binder:code_function (f)
  local body, test = {}, {}

  for i, m in ipairs(f) do
    local fname = self.wrapcall(m, overloaded, j)
    
    if not fname then error'this shout *NOT* happen!' end
    local st, err
    
    st, err = pcall(self.function_body, self, m)
    if st then
      body[i] = err
    else
      print(err)
      body[i] = nil
    end
    
    st, err = pcall(self.function_test, self, m, 'score['..i..']')
    if st then
      test[i] = err
    else
      print(err)
      test[i] = nil
    end

    --body[i] = self:function_body(m)
    --test[i] = self:function_test(m, 'score['..i..']')
  end
  
  return body, test
end

function binder:begin_dispatch(n, m)
    return self.lua_proto(n) .. ' {\n  int score[' .. (m+1)
                     ..'];\n  const int premium = 11+lua_gettop(L);\n'
end

function binder:choose_dispatch(m)
  return [[
  int best = 1;
  for (int i=1;i<=]]..m..[[;i++) {
    if (score[best] < score[i]) { best = i; }
  }
  switch (best) {
]]
end

function binder:solve_overload (f, n, c)
  local proto, def = '', ''
  local body, test = self:code_function(f)

  local number = 0
  for i = 1,table.maxn(f) do if (type(body[i])=='string') and (type(test[i])=='string') then number = number + 1 end end
  
  if number>1 then
    local fulltest = self:begin_dispatch(c..n, table.maxn(f))

    for i = 1,table.maxn(f) do
      local fullname = n..'__OverloadedVersion__'..i
      if (type(body[i])=='string') and (type(test[i])=='string') then
        proto = proto .. '  static '..self.lua_proto(fullname)..';\n'
        def = def .. self.lua_proto(c..fullname)..' '..body[i]
        fulltest = fulltest .. test[i]
      end
    end

    fulltest = fulltest .. self:choose_dispatch(table.maxn(f))

    for i = 1,table.maxn(f) do
      if (type(body[i])=='string') and (type(test[i])=='string') then
        local fullname = n..'__OverloadedVersion__'..i
        fulltest = fulltest .. '    case ' .. i .. ': return ' .. fullname ..'(L); break;\n'
      end
    end

    -- TODO: move to a function?
		-- TODO: trigger an error
    fulltest = fulltest .. [[
	}
	lua_pushstring(L, "no overload of function ]].. n ..[[ matching arguments");
	lua_error(L);
	return 0;
}
]]

    proto = proto .. '  static '..self.lua_proto(n)..';\n'
    def = def .. fulltest
  elseif number==1 then
    proto, def = nil, nil
    for i, v in ipairs(body) do
      proto = '  static '..self.lua_proto(n)..';\n'
      def = self.lua_proto(c..n)..' '..v
    end
  else
    proto, def = nil, nil
  end
  
  return proto, def
end

function binder:enum_push_body(id, c)
	local enum = (type(id)=='string') and self:find_id(id) or id
	local e_static = (self:find_id(enum.attr.context).tag == 'Class') and 'static ' or ''
	local e_context = self:context_name(enum)
	local e_name = 'lqt_pushenum_' .. enum.attr.name
	local e_proto, e_def = '', ''

	e_proto = e_proto .. '  ' .. e_static .. self.lua_proto(e_name) .. ';\n'
	e_def = e_def .. self.lua_proto(c .. e_name) .. ' '
	e_def = e_def .. '{\n'
	e_def = e_def .. '  int enum_table = 0;\n'
	e_def = e_def .. '  lua_getfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);\n'
	e_def = e_def .. '  if (!lua_istable(L, -1)) {\n'
	e_def = e_def .. '    lua_pop(L, 1);\n'
	e_def = e_def .. '    lua_newtable(L);\n'
	e_def = e_def .. '    lua_pushvalue(L, -1);\n'
	e_def = e_def .. '    lua_setfield(L, LUA_REGISTRYINDEX, LQT_ENUMS);\n'
	e_def = e_def .. '  }\n'

	e_def = e_def .. '  lua_newtable(L);\n'
	e_def = e_def .. '  enum_table = lua_gettop(L);\n'
	for i, e in ipairs(enum) do
		if (type(e)=='table') and (e.tag=='EnumValue') then
			e_def = e_def .. '  lua_pushstring(L, "' .. e.attr.name .. '");\n'
			e_def = e_def .. '  lua_rawseti(L, enum_table, ' .. e.attr.init .. ');\n'
			e_def = e_def .. '  lua_pushinteger(L, ' .. e.attr.init .. ');\n'
			e_def = e_def .. '  lua_setfield(L, enum_table, "' .. e.attr.name .. '");\n'
		end
	end
	e_def = e_def .. '  lua_pushvalue(L, -1);\n'
	e_def = e_def .. '  lua_setfield(L, -3, "' .. e_context .. enum.attr.name .. '");\n'
	e_def = e_def .. '  lua_remove(L, -2);\n'
	e_def = e_def .. '  return 1;\n'
	e_def = e_def .. '}\n'
	--print (e_def)
	return e_proto, e_def, e_name
end

function binder:mangled (f)
  local args = self:arguments_of(f)
  local k = f.attr.name..'('
  for i = 1, table.maxn(args) do
    k = k..', '..self:type_name(args.attr.type)
  end
  k = k..')'
  return k
end

function binder:get_virtuals (c)
  local c_v = self:get_members(c).virtuals
  local mang_virtuals = {}

  for n, f in pairs(c_v) do
    if f.attr.virtual=='1' then
      local k = self:mangled(f)
      mang_virtuals[k] = mang_virtuals[k] or f
    end
  end


  for s in string.gmatch(c.attr.bases or '', '(_%d+) ') do
    local my_base = self:find_id(s)
    local my_virtual = self:get_virtuals(my_base)
    for k, f in pairs(my_virtual) do
      mang_virtuals[k] = mang_virtuals[k] or f
    end
  end

  return mang_virtuals
end

function binder:proto_preamble (n, ...)
  -- FIXME: this is only Qt (the inclusion by name of class)
  -- FIXED?
  i = i or n
  local ret = [[
#include "lqt_common.hpp"
]]
  for n = 1,select('#', ...) do
		local i = select(n, ...)
		ret = ret .. '#include <'..i..'>\n'
	end

	-- FIXME handle namespaces
  ret = ret .. [[

template <> class ]] .. self.wrapclass(n) .. [[ : public ]]  .. n .. [[ {
  private:
  lua_State *L;
  public:
]]
  return ret
end

function binder:proto_ending (n)
 return [[
};

]]
end

function binder:copy_constructor(c)
  if c.tag=='Constructor' and c.attr.artificial=='1' then
		return ''
	end
	local constr = '  '
	local args = self.arguments_of(c)
	constr = constr .. self.wrapclass(c.attr.name) .. ' (lua_State *l'
	for argi = 1, table.maxn(args) do
		local argt = self:find_id(args[argi].attr.type)
		local argtype = self:type_name(argt)
		constr = constr .. ', ' .. argtype .. ' arg'..tostring(argi)
	end
	constr = constr .. '):'..c.attr.name..'('
	for argi = 1, table.maxn(args) do
		constr = constr .. ((argi>1) and ', ' or '') .. 'arg'..tostring(argi)
	end
	constr = constr .. '), L(l) {}\n'
	return constr, nil
end

function binder.meta_constr_proto (n) return 'extern "C" int luaopen_'..n..' (lua_State *L);\n' end
function binder.meta_constr_preamble (n)
  return [[
int luaopen_]]..n..[[ (lua_State *L) {
  if (luaL_newmetatable(L, "]]..n..[[*")) {
]]
end
function binder.meta_constr_method (n, c)
  c = c or ''
  return '    lua_pushcfunction(L, '..c..n..');\n    lua_setfield(L, -2, "'..n..'");\n'
end
function binder.meta_constr_ending (n)
  return [[
    lua_pushcfunction(L, lqtL_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, lqtL_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lqtL_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushstring(L, "]]..n..[[");
    lua_setfield(L, -2, "__qtype");
		lua_setglobal(L, "]]..n..[[");
  } else {
		lua_pop(L, 1);
	}
  return 0;
}
]]
end

function binder:virtual_overload (f, c, id)
	--if f.attr.access~='public' then error'only public virtual function are exported' end
	if f.attr.access=='private' then error'private virtual function are not exported' end

  c = c or ''
  local args = self.arguments_of(f)
  local ret_t = f.attr.returns and self:find_id(f.attr.returns)
  local ret_n = ret_t and self:type_name(ret_t) or 'void'
  local fh, fb = '  ', ''
  fh = fh .. ret_n .. ' ' .. f.attr.name .. ' ('
  fb = fb .. ret_n .. ' ' .. c .. f.attr.name .. ' ('
  
  -- GETTING ARGUMENTS
  for argi = 1, table.maxn(args) do
    local arg = args[argi]
    local argname = 'arg' .. tostring(argi)
    
    local argt = self:find_id(arg.attr.type)
    local argtype = self:type_name(argt)
    local def = arg.attr.default or nil
    
    def = def and (self:context_name(argt)..def)
    
    --print ('signing arg type', argtype)
    
    if argi>1 then fh = fh .. ', ' fb = fb .. ', ' end
    fh = fh .. argtype .. ' ' .. argname .. (def and (' = '..def) or '')
    fb = fb .. argtype .. ' ' .. argname
  end
  
  fh = fh .. ')' .. (f.attr.const and ' const' or '') .. ';\n'

  if f.attr.access~='public' then
    fh = f.attr.access .. ':\n' .. fh .. 'public:\n'
  end

  fb = fb .. ')' .. (f.attr.const and ' const' or '') .. ' {\n'
  fb = fb .. '  bool absorbed = false;\n  int oldtop = lua_gettop(L);\n'
  
  local context = self:context_name(f)
  local pointer_to_class = self.fake_pointer ( id or f.attr.context )
  local push_this = self:type_to_stack(pointer_to_class)'this'
  --fb = fb .. '  ' .. push_this .. ';\n'
  --fb = fb .. '  lua_getfield(L, -1, "'..(f.attr.name)..'");\n  lua_insert(L, -2);\n' 
---[=[
  fb = fb .. '  ' .. push_this .. [[;
	if (lua_getmetatable(L, -1)) {
		lua_getfield(L, -1, "]]..(f.attr.name)..[[");
		lua_remove(L, -2);
	} else {
		lua_pushnil(L);
	}
	lua_insert(L, -2);
]]
--]=]


  for argi = 1, table.maxn(args) do
    local arg = args[argi]
    local argname = 'arg' .. tostring(argi)
    
    local argt = self:find_id(arg.attr.type)
    local argtype = self:type_name(argt)
    local def = arg.attr.default
    
    def = def and (self:context_name(argt)..def)
    
    local to_stack = self:type_to_stack(argt)(argname)
    --to_stack = (type(to_stack)=='string') and to_stack or table.concat(to_stack, '\n  ')
    fb = fb .. '  ' .. to_stack .. ';\n'
  end
  
  local sig = '(' .. (args[1] and 'arg1' or '')
  for argi = 2, table.maxn(args) do
    sig = sig .. ', arg' .. argi
  end
  sig = sig .. ')'

  fb = fb .. [[
  if (lua_isfunction(L, -]]..table.maxn(args)..[[-2)) {
    lua_pcall(L, ]] .. table.maxn(args) .. [[+1, 2, 0);
		absorbed = (bool)lua_toboolean(L, -1) || (bool)lua_toboolean(L, -2);
		lua_pop(L, 1);
  }
  if (!absorbed) {
    lua_settop(L, oldtop);
    ]] .. (f.attr.pure_virtual~='1' and (((ret_n~='void') and 'return ' or '')..'this->'..context..f.attr.name..sig..';\n') or '') .. [[
  }
]]
--   fb = fb .. '  if (!lua_isnil)' -- TODO: check?
  if ret_n~='void' then
    fb = fb .. '  ' .. ret_n .. ' ret = ' .. self:type_from_stack(ret_t)(-1) .. ';\n'
    fb = fb .. '  lua_settop(L, oldtop);\n'
    fb = fb .. '  return ret;\n'
  else
    fb = fb .. '  lua_settop(L, oldtop);\n'
  end
  fb = fb .. '}\n'
  
  return fh, fb
end

function binder:virtual_destructor (f, c)
  c = c or ''
  local lname = self.wrapclass(f.attr.name)
  local pclass = self.fake_pointer(f.attr.context)
  local push_this = self:type_to_stack(pclass)'this'
  return [[
  ~]]..lname..[[ ();
]], 
c .. [[
  ~]]..lname..[[ () {
  int oldtop = lua_gettop(L);
  ]] .. push_this .. [[;
  lua_getfield(L, -1, "~]]..f.attr.name..[[");

  if (lua_isfunction(L, -1)) {
    lua_insert(L, -2);
    lua_pcall(L, 1, 1, 0);
  } else {
  }
  lua_settop(L, oldtop);
}
]]

end

function binder:make_namespace(tname, include_file, ...)
  local bind_file = 'lqt_bind_'..include_file..'.hpp'
  if string.match(include_file, '(%.[hH]([pP]?)%2)$') then
    bind_file = 'lqt_bind_'..include_file
  end

  local my_class = self:find_id(tname) or self:find_name(tname)
	---- FIXME handle namespaces
  local my_context = self.wrapclass(tname)..'::'

  local my = self:get_members(my_class)

  local my_enums = nil
  my.virtuals = self:get_virtuals(my_class)

  print 'writing preambles'

  local fullproto = self:proto_preamble(tname, include_file, ...)
  local fulldef = '#include "'..bind_file..'"\n\n'
  local metatable_constructor = self.meta_constr_preamble(tname)

  print 'binding each member'

  local my_members = {}
  table.foreach(my.methods, function(k, v) my_members[k] = v end)
  my_members.new = my.constructors
  my_members.delete = { my.destructor }
  for n, l in pairs(my_members) do
    local fname = self.WRAPCALL..n
    local proto, def = self:solve_overload(l, fname, my_context)
    if (proto and def) then
      fullproto = fullproto .. proto
      fulldef = fulldef .. def
      metatable_constructor = metatable_constructor .. self.meta_constr_method (n, my_context..self.WRAPCALL)
    end
  end

  print'binding virtual methods'

  for s, f in pairs(my.virtuals) do
    print ('virtual', s)
    local ret, h, c = pcall(self.virtual_overload, self, f, my_context, my_class.attr.id)
		if ret then
			fullproto, fulldef = fullproto..h, fulldef..c
		else
			print(h)
		end
  end

  print'overriding virtual destructor'
  if my.destructor and my.destructor.attr.virtual == '1' then
    local h, c = self:virtual_destructor(my.destructor, my_context)
    fullproto, fulldef = fullproto..h, fulldef..c
  end

	print'creating enum translation tables'
	for k, e in pairs(my.enumerations) do
		local e_p, e_d, e_n = self:enum_push_body(e, my_context)
		fulldef = fulldef .. e_d
		fullproto = fullproto .. e_p
		metatable_constructor = metatable_constructor .. '    ' .. my_context .. e_n .. '(L);\n    lua_setfield(L, -2, "'..e.attr.name..'");\n'
	end

  print'copying constructors'
  for i, v in ipairs(my.constructors) do
    fullproto = fullproto..self:copy_constructor(v)
  end
  fullproto = fullproto .. self:proto_ending(tname) .. self.meta_constr_proto (tname)

  print'specifying bases'
  metatable_constructor = metatable_constructor .. '    lua_newtable(L);\n'
	local deep_bases = {}
  for s in string.gmatch(my_class.attr.bases or '', '(_%d+) ') do
    local base = self:find_id(s)
    local bname = self:type_name(base)
    metatable_constructor = metatable_constructor .. [[
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "]]..bname..[[*");
]]
		deep_bases = self.set_union(deep_bases, self:tree_of_bases(base))
  end
  for n in pairs(deep_bases) do
    metatable_constructor = metatable_constructor .. [[
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "]]..n..[[*");
]]
  end
  metatable_constructor = metatable_constructor .. '    lua_setfield(L, -2, "__base");\n'


  print'finalizing code'
  metatable_constructor = metatable_constructor .. self.meta_constr_ending (tname)
  fulldef = fulldef .. metatable_constructor

  return fullproto, fulldef
end

function binder.set_union(...)
  local ret = {}
  for _, s in ipairs{...} do
    for v, t in pairs(s) do
      if t==true then ret[v] = true end
    end
  end
  return ret
end

function binder:tree_of_bases(c)
  local ret = {}
  for s in string.gmatch(c.attr.bases or '', '(_%d+) ') do
    local b = self:find_id(s)
    ret[b.attr.name] = true
    local bb = self:tree_of_bases(b)
    ret = self.set_union(ret, bb)
  end
  return ret
end


return binder

