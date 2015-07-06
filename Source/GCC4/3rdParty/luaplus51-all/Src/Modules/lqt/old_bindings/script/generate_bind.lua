#!/usr/bin/lua
--[[

Copyright (c) 2007 Mauro Iazzi

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


require 'lxp'
require 'lxp.lom'

filename = 'main.xml'
my_typename = 'QObject'

hpp_text = ''
hpp_public = ''
hpp_private = ''
hpp_protected = ''
cpp_text = ''

hpp = function(fn) io.write(hpp_text) print() end
cpp = function(fn) io.write(cpp_text) print() end
wr = function(fn) local f = io.open('bind.hpp', 'w') f:write(hpp_text) f:write'\n' f:close() end

xmlf = io.open(filename, 'r')
xmls = xmls or xmlf:read('*a')
xmlf:close()
xmlt = xmlt or lxp.lom.parse(xmls)
xmlf, xmls = nil, nil


------------ FIND -------------
find = function (t, f)
  if type(t)~='table' then
    return nil
  end;
  if f(t) then
    return t
  end
  local ret = nil
  for k,v in pairs(t) do
    ret = ret or find(v, f)
    if ret then break end
  end;
  return ret;
end

name_search = function (n)
  return function(t)
      return (type(t)=='table') and (type(t.attr)=='table') and (t.attr.name==n)
    end
end

id_search = function (i)
  return function (t)
      return (type(t)=='table') and (type(t.attr)=='table') and (t.attr.id==i) -- or ((type(i)=='table') and i[t.attr.id])
    end
end

tag_search = function (n)
  return function (t)
      return (type(t)=='table') and (t.tag==n) -- or ((type(i)==table) and i[t.attr.id])
    end
end

find_name = function (n) return find(xmlt, name_search(n)) end
find_id = function (n) return find(xmlt, id_search(n)) end


------------ ---- -------------

--[[
type_name = function (t, el)
      if el.tag == 'FundamentalType' then
    return el.attr.name
  elseif (el.tag == 'Class') or (el.tag == 'Struct') then
    return el.attr.name
  elseif el.tag == 'Typedef' then
    return type_name_by_id(t, el.attr.type)
  elseif el.tag == 'PointerType' then
    return type_name_by_id(t, el.attr.type) .. ' *'
  elseif el.tag == 'ReferenceType' then
    return type_name_by_id(t, el.attr.type) .. '&'
  elseif el.tag == 'CvQualifiedType' then
    return ( (el.attr.volatile=='1') and 'volatile ' or '') .. ( (el.attr.const=='1') and 'const ' or '') .. type_name_by_id(t, el.attr.type)
  else
    print (el.attr.id)
    return '<>'
  end
end
--]]

type_name = {}
setmetatable(type_name, {
  __call = function (t, el)
    print('getting name of', el, el.tag)
    if t[el] then return t[el] end
        if el.tag == 'FundamentalType' then
      t[el] = el.attr.name
    elseif (el.tag == 'Class') or (el.tag == 'Struct') then
      t[el] = el.attr.name
    elseif el.tag == 'Typedef' then
      t[el] = t(find_id(el.attr.type))
    elseif el.tag == 'PointerType' then
      t[el] = t(find_id(el.attr.type)) .. ' *'
    elseif el.tag == 'ReferenceType' then
      t[el] = t(find_id(el.attr.type)) .. '&'
    elseif el.tag == 'CvQualifiedType' then
      t[el] = ( (el.attr.volatile=='1') and 'volatile ' or '') .. ( (el.attr.const=='1') and 'const ' or '') .. t(find_id(el.attr.type))
    elseif el.tag == 'Enumeration' then
      t[el] = ((find_id(el.attr.context).attr.name=='::') and '' or
               (find_id(el.attr.context).attr.name) .. '::') .. el.attr.name
    else
--       error'SHIT!'
--       table.foreach(el, print)
--       table.foreach(el.attr, print)
--       print (el.attr.id)
      t[el] = '<>'
    end
    return rawget(t,el)
  end
})



function get_base_pointer_to(n, i)
  return '*static_cast<' .. n .. '**>(luaL_checkudata(L, ' .. tostring(i) .. ', "' .. n .. '*"))'
end



type_from_stack = {}
do
local type_from_stack_mt = {
  ['const char *'] = function(i) return 'lua_tostring(L, ' .. tostring(i) .. ')' end,
  ['short int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['unsigned short int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['unsigned int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['long int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['unsigned long int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['long long int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['unsigned long long int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['float'] = function(i) return 'lua_tonumber(L, ' .. tostring(i) .. ')' end,
  ['double'] = function(i) return 'lua_tonumber(L, ' .. tostring(i) .. ')' end,
  ['bool'] = function(i) return '(bool)lua_toboolean(L, ' .. tostring(i) .. ')' end,
  ['void *'] = function(i) return 'lua_touserdata(L, ' .. tostring(i) .. ')' end,
  ['void * *'] = function(i) return 'static_cast<void **>(lua_touserdata(L, ' .. tostring(i) .. '))' end,
}
setmetatable(type_from_stack, type_from_stack_mt)
type_from_stack_mt.__call = function (t, el, i)
      local name = type_name(el)
--       if t[el] then return t[el] end
      if type(type_from_stack_mt[name])=='function' then
        t[el] = type_from_stack_mt[name](i)
      else
        if (el.tag=='Class') or (el.tag=='Struct') then
          t[el] = '*' .. get_base_pointer_to(name, i)
        elseif (el.tag=='CvQualifiedType') then
          t[el] = t(find_id(el.attr.type), i)
        elseif (el.tag=='ReferenceType') then
          t[el] = t(find_id(el.attr.type), i)
        elseif (el.tag=='Enumeration') then
          t[el] = 'static_cast<'..name..'>(luaL_toenum(L, '..tostring(i)..', "'..name..'"))'
        elseif (el.tag=='PointerType') then
          t[el] = 'static_cast<'..type_name(el).. '>(&(' .. t(find_id(el.attr.type), i) .. '))'
        end
      end
      print (el.tag, el, rawget(t,el) or '<>')
      return rawget(t,el) or '<>'
    end
end

type_on_stack_test = {}
do
local type_on_stack_test_mt = {
  ['const char *'] = function(i) return '(lua_type(L, ' .. tostring(i) .. ')==LUA_TSTRING)' end,
  ['short int'] = function(i) return 'lua_isinteger(L, ' .. tostring(i) .. ')' end,
  ['unsigned short int'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['int'] = function(i) return 'lua_tointeger(L, ' .. tostring(i) .. ')' end,
  ['unsigned int'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['long int'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['unsigned long int'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['long long int'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['unsigned long long int'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['float'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['double'] = function(i) return 'lua_isnumber(L, ' .. tostring(i) .. ')' end,
  ['bool'] = function(i) return 'lua_isboolean(L, ' .. tostring(i) .. ')' end,
  ['void *'] = function(i) return 'lua_isuserdata(L, ' .. tostring(i) .. ')' end,
  ['void * *'] = function(i) return 'lua_isuserdata(L, ' .. tostring(i) .. ')' end,
}

-- more precise integer test
-- function(i) return '(lua_isnumber(L, ' .. tostring(i) .. ') && (lua_tointeger(L, ' .. tostring(i) .. ') == lua_tonumber(L, ' .. tostring(i) .. ')))' end,

setmetatable(type_on_stack_test, type_on_stack_test_mt)
type_on_stack_test_mt.__call = function (t, el, i)
      print ('TESTING ARGUMENT', i)
      local name = type_name(el)
--       if t[el] then return t[el] end
      if type(type_on_stack_test_mt[name])=='function' then
        t[el] = type_on_stack_test_mt[name](i)
      else
        if (el.tag=='Class') then
          t[el] = 'luaL_testudata(L, ' .. tostring(i) .. ', "' .. name .. '*")'
        elseif (el.tag=='CvQualifiedType') then
          t[el] = t(find_id(el.attr.type), i)
        elseif (el.tag=='ReferenceType') then
          t[el] = t(find_id(el.attr.type), i)
        elseif (el.tag=='PointerType') then
          t[el] = t(find_id(el.attr.type), i)
        elseif (el.tag=='Enumeration') then
          t[el] = 'luaL_isenum(L, '..tostring(i)..', "'..name..'")'
        end
      end
      print (el.tag, el, rawget(t,el) or '<>')
      return rawget(t,el) or '<>'
    end
end


cpairs = function (t, c)
  return function (table, key, val)
    key, val = next(table, key, val)
    while not c(val) and key~=nil do key, val = next(table, key, val) end
    return key, val
  end, t
end



------------------ MAIN PROGRAM ----------------------

my_class = find_name(my_typename)

if not my_members then
  my_members = {}
  for s in string.gmatch(my_class.attr.members, '(_%d+) ') do
    print("member found:", s)
    table.insert(my_members, find(xmlt, id_search(s)))
  end
end

public_members = {}
for i, m in pairs(my_members) do
  if m.attr.access=='public' then
    table.insert( public_members, m )
  end
end


getarg = function (a, n, i, def)
  local ret = ''
  local a_t = a.attr.id
  local argtype = type_name(a)
  print ('getting arg type', argtype, a_t)
  ret = ret .. '  ' .. argtype .. ' ' .. n .. ' = '
            .. (def and (type_on_stack_test(a, i) .. '?') or '') .. type_from_stack(a, i)
  ret = ret .. (def and (':' .. tostring(def)) or '') .. '; // '..tostring(def)..'\n'
  return ret
end

function pointer_search(id)
  return function(t)
    return (type(t)=='table') and (t.tag=='PointerType') and (type(t.attr)=='table') and (t.attr.type==id)
  end
end

function function_body(p, n)
  local ret = ''
  local isstatic = 0
  
  ret = ret .. 'static int ' .. n .. '(lua_State *L) {\n'
  
  
  if p.attr.static~='1' then
--     ret = ret .. '  ' .. my_typename .. ' *__lua__obj = *static_cast<' .. my_typename
--                         .. '**>(luaL_checkudata(L, 1, "'.. my_typename .. '**"));\n'
    ret = ret .. getarg( find(xmlt, pointer_search(my_class.attr.id)) or error'FUCK!', '__lua__obj', 1)
--     print (ret)
    isstatic = 1
  end
  
  local args = {}
  for argi = 1, table.maxn(p) do
    if (type(p[argi])=='table') and (p[argi].tag=='Argument') then
      table.insert(args, p[argi])
    end
  end
  
  for argi = 1, table.maxn(args) do
    local arg = args[argi]
    local argname = 'arg' .. tostring(argi)
    if (type(arg)=='table') and (arg.tag=='Argument') then
      local def = arg.attr.default or nil
      local argt = find_id(arg.attr.type)
      if def and find_id(arg.attr.type).tag=='Enumeration' then
        local cont = find_id(argt.attr.context).attr.name
        def = ((cont=='::') and '' or (cont .. '::')) .. def
      end
      ret = ret .. getarg(argt, argname, argi+isstatic, def)
    end
  end
  
  if p.attr.static~='1' then
    ret = ret .. '  __lua__obj->' .. p.attr.name .. '('
    if table.maxn(args) > 0 then ret = ret .. 'arg1' end
    for argi = 2, table.maxn(args) do
      ret = ret .. ', arg' .. tostring(argi)
    end
    ret = ret .. ');\n'
  else
    ret = ret .. p.attr.name .. '('
    if table.maxn(args) > 0 then ret = ret .. 'arg1' end
    for argi = 2, table.maxn(args) do
      ret = ret .. ', arg' .. tostring(argi)
    end
    ret = ret .. ');\n'
  end
  
  ret = ret .. '  return 1;\n'
  ret = ret .. '}\n'
  
  return ret
end


function function_test(p, score)
  local ret = ''
  local isstatic = 0
  
  ret = ret .. '  ' .. score .. ' = 0;\n'
  
  if p.attr.static~='1' then
    ret = ret .. '  ' .. score .. ' += ' .. type_on_stack_test( find(xmlt, pointer_search(my_class.attr.id)), 1 )
              .. '?premium:-premium*premium;\n' 
    isstatic = 1
  end
  
  local args = {}
  for argi = 1, table.maxn(p) do
    if (type(p[argi])=='table') and (p[argi].tag=='Argument') then
      table.insert(args, p[argi])
    end
  end
  
  for argi = 1, table.maxn(args) do
    local arg = args[argi]
    print ( 'ARGUMENT TEST', argi)
    local argname = 'arg' .. tostring(argi)
    if (type(arg)=='table') and (arg.tag=='Argument') then
      ret = ret .. '  if (' .. type_on_stack_test( find_id(arg.attr.type) , argi+isstatic ) .. ') {\n'
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
  
--   print ('|||||||||||||||||||', ret)
  return ret
end






hpp_text = hpp_text .. [[

static int luaL_toenum(lua_State *L, int index, const char *name) {
  int ret;
  lua_getfield(L, LUA_REGISTRYINDEX, "Enumerations");
  
  if (!lua_istable(L, -1)) { lua_pop(L, 1); return -1; }
  
  lua_pushstring(L, name);
  lua_gettable(L, -2);
  lua_remove(L, -2);
  
  if (!lua_istable(L, -1)) { lua_pop(L, 1); return -1; }
  
  lua_pushvalue(L, index);
  lua_gettable(L, -2);
  lua_remove(L, -2);
  
  ret = lua_tointeger(L, -1);
  
  lua_pop(L, 1);
  
  return ret;
}

static bool luaL_isenum(lua_State *L, int index, const char *name) {
  bool ret;
  lua_getfield(L, LUA_REGISTRYINDEX, "Enumerations");
  
  if (!lua_istable(L, -1)) { lua_pop(L, 1); return false; }
  
  lua_pushstring(L, name);
  lua_gettable(L, -2);
  lua_remove(L, -2);
  
  if (!lua_istable(L, -1)) { lua_pop(L, 1); return false; }
  
  lua_pushvalue(L, index);
  lua_gettable(L, -2);
  lua_remove(L, -2);
  
  ret = (bool)lua_isnumber(L, -1);
  
  lua_pop(L, 1);
  
  return ret;
}

static bool luaL_testudata(lua_State *L, int index, const char *name) {
  bool ret = false;
  lua_getmetatable(L, index);
  lua_getfield(L, LUA_REGISTRYINDEX, name);
  ret = (bool)lua_equal(L, -1, -2);
  lua_pop(L, 2);
  return ret;
}

]]

hpp_text = hpp_text .. 'class LuaWrap' .. my_typename .. ': public ' .. my_typename .. ' {\n'

hpp_text = hpp_text .. 'public:\n'

overloaded_public = {}
for i, p in pairs(public_members) do
  print('=========EXAMINING MEMBER==========', i)
  if p.tag=='Method' then
    if overloaded_public[p.attr.name]==nil then
      overloaded_public[p.attr.name] = { p }
    else
      table.insert(overloaded_public[p.attr.name], p)
    end
  else
  end
end

for n, l in pairs(overloaded_public) do
  print('=========MAKING METHOD==========', n)
  if table.maxn(l) == 1 then
--     print(l, l[1])
    hpp_text = hpp_text .. function_body(l[1], 'LuaCallWrap__'..n)
  else
    local dispatcher = 'static int '..'LuaWrapCall__'..n..'(lua_State *L) {\n'
    dispatcher = dispatcher .. '  int scores['..tostring(table.maxn(l))..'];\n'
    dispatcher = dispatcher .. '  const int premium = 11+lua_gettop(L);\n'
    
    for j = 1, table.maxn(l) do
      hpp_text = hpp_text .. function_body(l[j], 'LuaCallWrap__'..n..'__LuaOverloadedVersion__'..j)
      local test = function_test(l[j], 'scores['..j..']', table.maxn(l)+11)
      dispatcher = dispatcher .. test
      print (test)
    end
    
    dispatcher = dispatcher .. '  int best = 1;\n'
    dispatcher = dispatcher .. '  for (int i=1;i<='.. table.maxn(l) ..';i++) {\n'
    dispatcher = dispatcher .. '    if (scores[best] < scores[i]) { best = i; }\n'
    dispatcher = dispatcher .. '  }\n'
    dispatcher = dispatcher .. '  switch (best) {\n'
    for j = 1, table.maxn(l) do
      dispatcher = dispatcher .. '    case ' .. tostring(j) .. ': return ' .. 'LuaCallWrap__'..n..'__LuaOverloadedVersion__'..j..'(L); break;\n'
    end
    dispatcher = dispatcher .. '  }\n'
    
    dispatcher = dispatcher .. '};\n'
    
    hpp_text = hpp_text .. dispatcher
  end
end

hpp_text = hpp_text .. 'private:\n'

hpp_text = hpp_text .. 'protected:\n'
hpp_text = hpp_text .. '  lua_State *L;\n'
hpp_text = hpp_text .. '};'

--[[
hpp_text = hpp_text .. '\n'
hpp_text = hpp_text .. 'class LuaWrap' .. my_typename .. ': public ' .. my_typename .. ' {\n'
hpp_text = hpp_text .. 'private:\n' .. hpp_public
hpp_text = hpp_text .. 'private:\n' .. hpp_private
hpp_text = hpp_text .. 'protected:\n' .. hpp_protected
hpp_text = hpp_text .. '};'
--]]

