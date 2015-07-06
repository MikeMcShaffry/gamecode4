------------------------------------------------------------------------------
-- Expand.lua
-- http://lua-users.org/wiki/TextTemplate
------------------------------------------------------------------------------

local strfind = string.find
local strsub  = string.sub
local push    = table.insert
local pop     = table.remove
local concat  = table.concat

local statements = {}
for w in string.gfind('do if for while repeat', '%a+') do
  statements[w] = true
end

local function expand(str, ...)
  assert(type(str)=='string', 'expecting string')
  local searchlist = arg
  if not arg[1] then
    searchlist = { _G }
  end
  local estring,evar

  function estring(str)
    local b,e,i
    b,i = strfind(str, '%$.')
    if not b then return str end

    local R, pos = {}, 1
    repeat
      b,e = strfind(str, '^%$', i)
      if b then
        push(R, strsub(str, pos, b-2))
        push(R, '$')
        i = e+1
        pos = i
      else
      b,e = strfind(str, '^%b{}', i)
      if b then
        push(R, strsub(str, pos, b-2))
        push(R, evar(expand(strsub(str, b+1, e-1), unpack(arg))))
        i = e+1
        pos = i
      else
        b,e = strfind(str, '^%b()', i)
        if b then
          push(R, strsub(str, pos, b-2))
          push(R, evar(expand(strsub(str, b+1, e-1), unpack(arg))))
          i = e+1
          pos = i
        elseif strfind(str, '^%a', i) then
          push(R, strsub(str, pos, i-2))
          push(R, evar(expand(strsub(str, i, i), unpack(arg))))
          i = i+1
          pos = i
        elseif strfind(str, '^%$', i) then
          push(R, strsub(str, pos, i))
          i = i+1
          pos = i
        end
      end
      end
      b,i = strfind(str, '%$.', i)
    until not b

    push(R, strsub(str, pos))
    return concat(R)
  end

  local function search(index)
    for _,symt in ipairs(searchlist) do
      local ts = type(symt)
      local value
      if     ts == 'function' then value = symt(index)
      elseif ts == 'table'
          or ts == 'userdata' then value = symt[index]
          if type(value)=='function' then value = value(symt) end
      else error'search item must be a function, table or userdata' end
      if value ~= nil then return value end
    end
    error('unknown variable: '.. index)
  end

  local function elist(var, v, str, sep)
    local tab = search(v)
    if tab then
      assert(type(tab)=='table', 'expecting table from: '.. var)
      local R = {}
      push(searchlist, 1, tab)
      push(searchlist, 1, false)
      for _,elem in ipairs(tab) do
        searchlist[1] = elem
        push(R, estring(str))
      end
      pop(searchlist, 1)
      pop(searchlist, 1)
      return concat(R, sep)
    else
      return ''
    end
  end

  local function get(tab,index)
    for _,symt in ipairs(searchlist) do
      local ts = type(symt)
      local value
      if     ts == 'function' then value = symt(index)
      elseif ts == 'table'
          or ts == 'userdata' then value = symt[index]
      else error'search item must be a function, table or userdata' end
      if value ~= nil then
        tab[index] = value  -- caches value and prevents changing elements
        return value
      end
    end
  end

  function evar(var)
    if strfind(var, '^[_%a][_%w]*$') then -- ${vn}
      return estring(tostring(search(var)))
    end
    local b,e,cmd = strfind(var, '^(%a+)%s.')
    if cmd == 'foreach' then -- ${foreach vn xxx} or ${foreach vn/sep/xxx}
      local vn,s
      b,e,vn,s = strfind(var, '^([_%a][_%w]*)([%s%p]).', e)
      if vn then
        if strfind(s, '%s') then
          return elist(var, vn, strsub(var, e), '')
        end
        b = strfind(var, s, e, true)
        if b then
          return elist(var, vn, strsub(var, b+1), strsub(var,e,b-1))
        end
      end
      error('syntax error in: '.. var, 2)
    elseif cmd == 'when' then -- $(when vn xxx)
      local vn
      b,e,vn = strfind(var, '^([_%a][_%w]*)%s.', e)
      if vn then
        local t = search(vn)
        if not t then
          return ''
        end
        local s = strsub(var,e)
        if type(t)=='table' or type(t)=='userdata' then
          push(searchlist, 1, t)
          s = estring(s)
          pop(searchlist, 1)
          return s
        else
          return estring(s)
        end
      end
      error('syntax error in: '.. var, 2)
    else
      if statements[cmd] then -- do if for while repeat
        var = 'local OUT="" '.. var ..' return OUT'
      else  -- expression
        var = 'return '.. var
      end
      local f = assert(loadstring(var))
      local t = searchlist[1]
      assert(type(t)=='table' or type(t)=='userdata', 'expecting table')
      setfenv(f, setmetatable({}, {__index=get, __newindex=t}))
      return estring(tostring(f()))
    end
  end

  return estring(str)
end

--[=====[

template = [[
you can access variables: $v
or environment variables: ${HOME}

you can call functions: ${table.concat(list, ', ')}
this list has ${list.n} elements
   ${string.rep('=', list.n)}
   ${table.concat(list)}
   ${string.rep('=', list.n)}

or evaluate code inline
${for i=1,list.n do
    OUT = table.concat{ OUT, ' list[', i, '] = ', list[i], '\n'}
  end}
you can access global variables:
This example is from ${mjd} at $(mjdweb)

The Lord High Chamberlain has gotten ${L.n}
things for me this year.
${do diff = L.n - 5
    more = 'more'
    if diff == 0 then
      diff = 'no'
    elseif diff < 0 then
      diff = -diff
      more = 'fewer'
    end
  end}
That is $(diff) $(more) than he gave me last year.

values can have other variables: $(ref)
]]

mjd = "Mark J. Dominus"
mjdweb = 'http://perl.plover.com/'
L = {n=0}
for i = 1,4 do table.insert(L, string.char(64+i)) end
local x = {
  v = 'this is v',
  list = L,
  ref = "$(mjd) made Text::Template.pm"
}
setmetatable(x, {__index=_G})
-- fill in the template with values in table x
io.write(expand(template, x, os.getenv))

------------------------------------------------------------------------------

fun_temp = [[
==============================================================================
$(foreach funcs

  ${type} x = ${name}( ${table.concat(args, ', ')} ) {
    $(code)
$(when stuff
    x = $x;
    y = $y;
)    reutrn $(exit);
  }
)
==============================================================================
]]

fun_list = {
  exit = 1;
  stuff = false;

  funcs = {
    { type = 'int';
      name = 'bill';
      args = { 'a', 'b', 'c' };
      code = 'something';
      stuff = { x=99, y=34 };
    };
    { type = 'char *';
      name = 'bert';
      args = { 'one', 'two', 'three' };
      code = 'something else';
      exit = 2
    };
  };
}

io.write(expand(fun_temp, fun_list, _G))

--]=====]

return expand
