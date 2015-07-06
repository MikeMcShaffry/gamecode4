
local grammar = require "cosmo.grammar"

module(..., package.seeall)

local function is_callable(f)
  if type(f) == "function" then return true end
  local meta = getmetatable(f)
  if meta and meta.__call then return true end
  return false
end

local insert = table.insert
local concat = table.concat

local function prepare_env(env, parent)
  local __index = function (t, k)
		    local v = env[k]
		    if not v then
		      v = parent[k]
		    end
		    return v
		  end
  local __newindex = function (t, k, v)
		       env[k] = v
		     end
  return setmetatable({ self = env }, { __index = __index, __newindex = __newindex })
end

local interpreter = {}

function interpreter.text(state, text)
  assert(text.tag == "text")
  insert(state.out, text.text)
end

local function check_selector(name, selector)
  if not is_callable(selector) then
    error("selector " .. name .. " is not callable but is " .. type(selector))
  end
end

function interpreter.appl(state, appl)
  assert(appl.tag == "appl")
  local selector, args, subtemplates = appl.selector, appl.args, appl.subtemplates
  local env, out = state.env, state.out
  local selector_name = selector
  selector = loadstring("local env = (...); return " .. selector)(env) or function () return '' end
  if #subtemplates == 0 then
    if args and args ~= "" and args ~= "{}" then
      check_selector(selector_name, selector)
      selector = selector(loadstring("local env = (...); return " .. args)(env), false)
      insert(out, tostring(selector))
    else
      if is_callable(selector) then
	insert(out, tostring(selector()))
      else
	insert(out, tostring(selector or ""))
      end
    end
  else
    if args and args ~= "" and args ~= "{}" then
      check_selector(selector_name, selector)
      args = loadstring("local env = (...); return " .. args)(env)
      for e, literal in coroutine.wrap(selector), args, true do
	if literal then
	  insert(out, tostring(e))
	else
	  if type(e) ~= "table" then
	    e = prepare_env({ it = tostring(e) }, env)
	  else
	    e = prepare_env(e, env)
	  end
	  interpreter.template({ env = e, out = out }, subtemplates[e.self._template or 1])
	end
      end
    else
      if type(selector) == 'table' then
	for _, e in ipairs(selector) do
	  if type(e) ~= "table" then
	    e = prepare_env({ it = tostring(e) }, env)
	  else
	    e = prepare_env(e, env) 
	  end
	  interpreter.template({ env = e, out = out }, subtemplates[e.self._template or 1])
	end
      else
	check_selector(selector_name, selector)
	for e, literal in coroutine.wrap(selector), nil, true do
	  if literal then
	    insert(out, tostring(e))
	  else
	    if type(e) ~= "table" then
	      e = prepare_env({ it = tostring(e) }, env)
	    else
	      e = prepare_env(e, env)
	    end
	    interpreter.template({ env = e, out = out }, subtemplates[e.self._template or 1])
	  end
	end
      end
    end
  end
end

function interpreter.template(state, template)
  if template then
    assert(template.tag == "template")
    for _, part in ipairs(template.parts) do
      interpreter[part.tag](state, part)
    end
  end
end

function fill(template, env, opts)
   opts = opts or {}
   local out = opts.out or {}
   if type(env) == "string" then env = { it = env } end
   interpreter.template({ env = env, out = out }, grammar.ast:match(template))
   return concat(out, opts.delim)
end
