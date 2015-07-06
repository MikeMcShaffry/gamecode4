--
-- Compat-5.1 release 2
-- Copyright Kepler Project 2004-2005 (http://www.keplerproject.org/compat)
--

--
-- avoid overwriting the package table if it's already there
--
package = package or {}

package.path = LUA_PATH or os.getenv("LUA_PATH") or
             ("./?.lua;" ..
              "/usr/local/share/lua/5.0/?.lua;" ..
              "/usr/local/share/lua/5.0/?/init.lua" )
 
package.cpath = os.getenv("LUA_CPATH") or
             "./?.so;" ..
             "/usr/local/lib/lua/5.0/?.so;" ..
             "/usr/local/lib/lua/5.0/lib?.so"

--
-- make sure require works with standard libraries
--
package.loaded = package.loaded or {}
package.loaded.string = string
package.loaded.math = math
package.loaded.io = io
package.loaded.os = os
package.loaded.table = table 
package.loaded.base = _G
package.loaded.coroutine = coroutine

--
-- avoid overwriting the package.preload table if it's already there
--
package.preload = package.preload or {}

--
-- looks for a file `name' in given path
--
local function search (path, name)
  for c in string.gfind(path, "[^;]+") do
    c = string.gsub(c, "%?", name)
    local f = io.open(c)
    if f then   -- file exist?
      f:close()
      return c
    end
  end
  return nil    -- file not found
end


--
-- new require
--
function _G.require (name)
  if not package.loaded[name] then
    package.loaded[name] = true
    local f = package.preload[name]
    if not f then
      local filename = string.gsub(name, "%.", "/")
      local fullname = search(package.cpath, filename)
      if fullname then
        local openfunc = "luaopen_" .. string.gsub(name, "%.", "")
        f = assert(loadlib(fullname, openfunc))
      else
        fullname = search(package.path, filename)
        if not fullname then
          error("cannot find "..name.." in path "..package.path, 2)
        end
        f = assert(loadfile(fullname))
      end
    end
    local old_arg = arg
    arg = { name }
    local res = f(name)
	arg = old_arg
    if res then package.loaded[name] = res end
  end
  return package.loaded[name]
end


--
-- auxiliar function to read "nested globals"
--
local function getfield (t, f)
  for w in string.gfind(f, "[%w_]+") do
    if not t then return nil end
    t = t[w]
  end
  return t
end


--
-- auxiliar function to write "nested globals"
--
local function setfield (t, f, v)
  for w in string.gfind(f, "([%w_]+)%.") do
    t[w] = t[w] or {} -- create table if absent
    t = t[w]            -- get the table
  end
  local w = string.gsub(f, "[%w_]+%.", "")   -- get last field name
  t[w] = v            -- do the assignment
end


--
-- new module function
--
function _G.module (name)
  local _G = getfenv(0)       -- simulate C function environment
  local ns = getfield(_G, name)         -- search for namespace
  if not ns then
    ns = {}                             -- create new namespace
    setfield(_G, name, ns)
  elseif type(ns) ~= "table" then
    error("name conflict for module `"..name.."'")
  end
  if not ns._NAME then
    ns._NAME = name
    ns._PACKAGE = string.gsub(name, "[^.]*$", "")
  end
  setmetatable(ns, {__index = _G})
  _G.package.loaded[name] = ns
  setfenv(2, ns)
  return ns
end
