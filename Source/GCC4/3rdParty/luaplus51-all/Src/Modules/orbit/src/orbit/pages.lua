
local orbit = require "orbit"
local model = require "orbit.model"
local cosmo = require "cosmo"

local _M = orbit.new()

local template_cache = {}

local BOM = string.char(239) .. string.char(187) .. string.char(191)

local function remove_shebang(s)
  return s:gsub("^#![^\n]+", "")
end

local function splitpath(filename)
  local path, file = string.match(filename, "^(.*)[/\\]([^/\\]*)$")
  return path, file
end

function _M.load(filename, contents)
  filename = filename or contents
  local template = template_cache[filename]
  if not template then
     if not contents then
       local file = io.open(filename)
       if not file then
	 return nil
       end
       contents = file:read("*a")
       file:close()
       if contents:sub(1,3) == BOM then contents = contens:sub(4) end
     end
     template = cosmo.compile(remove_shebang(contents))
     template_cache[filename] = template
  end
  return template
end

local function env_index(env, key)
  local val = _G[key]
  if not val and type(key) == "string" then
    local template = 
      _M.load(env.req.app_path .. "/" .. key .. ".op")
    if not template then return nil end
    return function (arg)
	     arg = arg or {}
	     if arg[1] then arg.it = arg[1] end
	     local subt_env = setmetatable(arg, { __index = env })
	     return template(subt_env)
	   end
  end
  return val
end

local function abort(res)
  error{ abort, res or "abort" }
end

local function make_env(req, res, initial)
  local env = setmetatable(initial or {}, { __index = env_index })
  env._G = env
  env.app = _G
  env.req = req
  env.res = res
  env.finish = abort
  function env.lua(arg)
    local f, err = loadstring(arg[1])
    if not f then error(err .. " in \n" .. arg[1]) end
    setfenv(f, env)
    local ok, res = pcall(f)
    if not ok and (type(res)~= "table" or res[1] ~= abort) then 
      error(res .. " in \n" .. arg[1]) 
    elseif ok then
      return res or ""
    else
      abort(res[2])
    end
  end
  env["if"] = cosmo.cif
  function env.redirect(target)
    if type(target) == "table" then target = target[1] end
    res:redirect(target)
    abort()
  end
  function env.link(arg)
    local url = arg[1]
    arg[1] = nil
    return req:link(url, arg)
  end
  function env.static_link(arg)
    return req:static_link(arg[1])
  end
  function env.absolute_link(arg)
    return req:absolute_link(arg[1])
  end
  function env.include(name, subt_env)
    local filename
    if type(name) == "table" then 
      name = name[1] 
      subt_env = name[2]
    end
    if name:sub(1, 1) == "/" then
      filename = req.doc_root .. name
    else
      filename = req.app_path .. "/" .. name
    end
    local template = _M.load(filename)
    if not template then return "" end
    if subt_env then
      if type(subt_env) ~= "table" then subt_env = { it = subt_env } end
      subt_env = setmetatable(subt_env, { __index = env })
    else
      subt_env = env
    end
    return template(subt_env)
  end
  function env.forward(...)
    abort(env.include(...))
  end
  env.mapper = model.new()
  function env.model(name, dao)
    if type(name) == "table" then
      name, dao = name[1], name[2]
    end
    return env.mapper:new(name, dao)
  end
  env.recycle = model.recycle
  return env
end

function _M.fill(req, res, template, env)
  if template then
    local ok, res = xpcall(function () return template(make_env(req, res, env)) end,
			   function (msg) 
			     if type(msg) == "table" and msg[1] == abort then 
			       return msg
			     else 
			       return traceback(msg) 
			     end
			   end)
    if not ok and (type(res) ~= "table" or res[1] ~= abort) then
      error(res)
    elseif ok then
      return res
    else
      return res[2]
    end
  end
end

_M.handler = _M:wrap(function (req, res)
		       local write = res.write
		       res.write = nil
		       local filename = req.env.PATH_TRANSLATED
		       local response = fill(req, res, _M.load(filename))
		       if response then
			 write(res, response)
			 return res:finish()
		       else
			 return _M.not_found(req.env)
		       end
		     end)

for _, method in ipairs{ "get", "post", "put", "delete" } do
  _M["dispatch_" .. method](_M, method, "/*", _M.handler)
end

return _M
