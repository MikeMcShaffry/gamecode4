
pcall(require, "luarocks.require")
require "wsapi.request"

local function make_env_get(qs)
   return {
      REQUEST_METHOD = "GET",
      QUERY_STRING = qs or "",
      CONTENT_LENGTH = 0,
      PATH_INFO = "/",
      SCRIPT_NAME = "",
      CONTENT_TYPE = "x-www-form-urlencoded",
      input = {
	 read = function () return nil end
      }
   }
end

local function make_env_post(pd, type, qs)
   pd = pd or ""
   return {
      REQUEST_METHOD = "POST",
      QUERY_STRING = qs or "",
      CONTENT_LENGTH = #pd,
      PATH_INFO = "/",
      CONTENT_TYPE = type or "x-www-form-urlencoded",
      SCRIPT_NAME = "",
      input = {
	 post_data = pd,
	 current = 1,
	 read = function (self, len)
		   if self.current > #self.post_data then return nil end
		   local s = self.post_data:sub(self.current, len)
		   self.current = self.current + len
		   return s
		end
      }
   }
end

local function encode_multipart(boundary, fields)
  local parts = { "--" .. boundary }
  for _, t in ipairs(fields) do
     parts[#parts+1] = '\r\nContent-Disposition: form-data; name="' .. t[1] .. '"\r\n\r\n' .. t[2] .. '\r\n--' .. boundary
  end
  return table.concat(parts)
end

local function is_empty_table(t)
   for k, v in pairs(t) do return false, k, v end
   return true
end

print("Test empty GET")
local env = make_env_get()
local req = wsapi.request.new(env)
assert(req.path_info == env.PATH_INFO)
assert(req.method == env.REQUEST_METHOD)
assert(req.script_name == env.SCRIPT_NAME)
assert(req.query_string == env.QUERY_STRING)
assert(is_empty_table(req.GET))
assert(is_empty_table(req.POST))

print("Test one-parameter GET")
local env = make_env_get("foo=bar")
local req = wsapi.request.new(env)
assert(req.GET["foo"] == "bar")
assert(req.params["foo"] == "bar")
assert(is_empty_table(req.POST))

print("Test one-parameter POST")
local env = make_env_post("foo=bar")
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(req.POST["foo"] == "bar")
assert(req.params["foo"] == "bar")

print("Test empty POST that is not form-encoded")
local env = make_env_post(nil, "application/json")
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(req.POST["post_data"] == "")
assert(req.params["post_data"] == "")

print("Test POST with content that is not form-encoded")
local env = make_env_post("{ foo: bar }", "application/json")
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(req.POST["post_data"] == "{ foo: bar }")
assert(req.params["post_data"] == "{ foo: bar }")

print("Test two-parameter GET")
local env = make_env_get("foo=bar&baz=boo")
local req = wsapi.request.new(env)
assert(req.GET["foo"] == "bar")
assert(req.GET["baz"] == "boo")
assert(req.params["foo"] == "bar")
assert(req.params["baz"] == "boo")
assert(is_empty_table(req.POST))

print("Test two-parameter POST")
local env = make_env_post("foo=bar&baz=boo")
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(req.POST["foo"] == "bar")
assert(req.POST["baz"] == "boo")
assert(req.params["foo"] == "bar")
assert(req.params["baz"] == "boo")
assert(not req:parse_post())

print("Test POST with GET")
local env = make_env_post("baz=boo", nil, "foo=bar")
local req = wsapi.request.new(env)
assert(req.GET["foo"] == "bar")
assert(req.POST["baz"] == "boo")
assert(req.params["foo"] == "bar")
assert(req.params["baz"] == "boo")

print("Test one-parameter POST")
local env = make_env_post("foo=bar")
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(req.POST["foo"] == "bar")
assert(req.params["foo"] == "bar")

print("Test multipart/form-data")
local boundary = "hello"
local env = make_env_post(encode_multipart(boundary, { { "foo", "bar\nbar" }, { "baz", "boo" } }),
					   "multipart/form-data; boundary=" .. boundary)
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(req.POST["foo"] == "bar\nbar")
assert(req.POST["baz"] == "boo")
assert(req.params["foo"] == "bar\nbar")
assert(req.params["baz"] == "boo")

print("Test POST with repeat parameters")
local env = make_env_post("foo=bar&foo=boo")
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(#req.POST["foo"] == 2)
assert(req.POST["foo"][1] == "bar")
assert(req.POST["foo"][2] == "boo")

print("Test GET with repeat parameters")
local env = make_env_get("foo=bar&foo=boo")
local req = wsapi.request.new(env)
assert(is_empty_table(req.POST))
assert(#req.GET["foo"] == 2)
assert(req.GET["foo"][1] == "bar")
assert(req.GET["foo"][2] == "boo")

print("Test POST with repeat parameters and overwrite enabled")
local env = make_env_post("foo=bar&foo=boo")
local req = wsapi.request.new(env, { overwrite = true })
assert(is_empty_table(req.GET))
assert(req.POST["foo"] == "boo")

print("Test GET with repeat parameters and overwrite enabled")
local env = make_env_get("foo=bar&foo=boo")
local req = wsapi.request.new(env, { overwrite = true })
assert(is_empty_table(req.POST))
assert(req.GET["foo"] == "boo")

print("Test multipart/form-data with repeat parameters")
local boundary = "hello"
local env = make_env_post(encode_multipart(boundary, { { "foo", "bar\nbar" }, { "foo", "boo" } }),
					   "multipart/form-data; boundary=" .. boundary)
local req = wsapi.request.new(env)
assert(is_empty_table(req.GET))
assert(#req.POST["foo"] == 2)
assert(req.POST["foo"][1] == "bar\nbar")
assert(req.POST["foo"][2] == "boo")

print("Test multipart/form-data with repeat parameters and overwrite")
local boundary = "hello"
local env = make_env_post(encode_multipart(boundary, { { "foo", "bar\nbar" }, { "foo", "boo" } }),
					   "multipart/form-data; boundary=" .. boundary)
local req = wsapi.request.new(env, { overwrite = true })
assert(is_empty_table(req.GET))
assert(req.POST["foo"] == "boo")

print("Test delayed POST")
local env = make_env_post("foo=bar&baz=boo")
local req = wsapi.request.new(env, { delay_post = true })
assert(is_empty_table(req.GET))
assert(is_empty_table(req.POST))
assert(req:parse_post())
assert(req.POST["foo"] == "bar")
assert(req.POST["baz"] == "boo")
assert(req.params["foo"] == "bar")
assert(req.params["baz"] == "boo")
assert(not req:parse_post())
