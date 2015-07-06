require "luasql.sqlite3"
require "orbit.model"

local env = luasql.sqlite3()
local conn = env:connect("blog.db")

local mapper = orbit.model.new("blog_", conn, "sqlite3")

local tables = { "post", "comment", "page" }

print [[

require "luasql.mysql"
require "orbit.model"

local env = luasql.mysql()
local conn = env:connect("blog", "root", "password")

local mapper = orbit.model.new("blog_", conn, "mysql")

]]

local function serialize_prim(v)
  local type = type(v)
  if type == "string" then
    return string.format("%q", v)
  else
    return tostring(v)
  end
end

local function serialize(t)
  local fields = {}
  for k, v in pairs(t) do
    table.insert(fields, " [" .. string.format("%q", k) .. "] = " ..
	       serialize_prim(v))
  end
  return "{\n" .. table.concat(fields, ",\n") .. "}"
end

for _, tn in ipairs(tables) do
  print("\n -- Table " .. tn .. "\n")
  local t = mapper:new(tn)
  print("local t = mapper:new('" .. tn .. "')")
  local recs = t:find_all()
  for i, rec in ipairs(recs) do
    print("\n-- Record " .. i .. "\n")
    print("local rec = " .. serialize(rec))
    print("rec = t:new(rec)")
    print("rec:save(true)")
  end
end
