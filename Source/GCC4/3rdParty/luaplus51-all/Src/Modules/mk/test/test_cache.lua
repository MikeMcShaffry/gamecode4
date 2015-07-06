
local mk = require "mk"
local cache = require "mk.cache"
local util = require "wsapi.util"
local mkutil = require "mk.util"
local response = require "wsapi.response"
local socket = require "socket"

do
  local c = cache.new()
  assert(c.values)
  c:set("foo", "bar")
  local value, mtime, ext = c:get("foo")
  assert(value == "bar")
  assert(ext == "html")
  assert(mtime <= os.time())
  c:set("foo", "baz", mtime - 3600, "xml")
  local value, mtime2, ext = c:get("foo")
  assert(value == "baz")
  assert(ext == "xml")
  assert(mtime2 == mtime - 3600)
  c:invalidate("foo")
  assert(not c:get("foo"))
end

do
  local path = os.tmpname()
  os.remove(path)
  lfs.mkdir(path)
  local c = cache.new(nil, path)
  assert(not c.values)
  c:set("foo", "bar")
  local value, mtime, ext = c:get("foo")
  assert(value == path .. "/foo.html")
  assert(mkutil.readfile(value) == "bar")
  assert(not ext)
  assert(mtime <= os.time())
  c:set("foo", "baz", mtime - 3600, "xml")
  local value, mtime2, ext = c:get("foo")
  assert(value == path .. "/foo.html")
  assert(mkutil.readfile(value) == "baz")
  assert(not ext)
  assert(mtime2 == mtime - 3600)
  c:invalidate("foo")
  assert(not c:get("foo"))
  c:set("foo", "bar")
  assert(c:get("foo") == path .. "/foo.html")
  c:nuke()
  assert(not c:get("foo"))
  mkutil.writefile(path .. "/foo.html", "foo")
  assert(not c:get("foo"))
end

do
  local c = cache.new()
  local wsapi_app = c:wrap(function (wsapi_env)
			     local res = response.new()
			     res.headers["X-Not-Cached"] = true
			     res:write("foo")
			     return res:finish()
			   end)
  local env = util.make_env_get()
  env.PATH_INFO = "/bar"
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  local etag = headers["ETag"]
  assert(etag)
  assert(res() == "foo" and not res())
  local value, mtime, ext = c:get("/bar")
  assert(value == "foo")
  assert(ext == "html")
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(not headers["X-Not-Cached"])
  assert(etag == headers["ETag"])
  assert(res() == "foo" and not res())
  c:set("/bar", "baz")
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(not headers["X-Not-Cached"])
  assert(etag == headers["ETag"])
  assert(res() == "baz" and not res())
  c:invalidate("/bar")
  socket.sleep(1)
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  assert(headers["ETag"] and etag ~= headers["ETag"])
  assert(res() == "foo" and not res())
end

do
  local c = cache.new()
  local wsapi_app = c:wrap(function (wsapi_env)
			     local res = response.new()
			     res.headers["X-Not-Cached"] = true
			     res:write("foo")
			     return res:finish()
			   end)
  local env = util.make_env_get()
  env.PATH_INFO = "/bar"
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  local etag = headers["ETag"]
  assert(etag)
  assert(res() == "foo" and not res())
  env.HTTP_IF_NONE_MATCH = etag
  local status, headers, res = wsapi_app(env)
  assert(status == 304)
end

do
  local dirname = os.tmpname()
  os.remove(dirname)
  lfs.mkdir(dirname)
  local c = cache.new(mk.new(), dirname)
  assert(not c.values)
  local wsapi_app = c:wrap(function (wsapi_env)
			     local res = response.new()
			     res.headers["X-Not-Cached"] = true
			     res:write("foo")
			     return res:finish()
			   end)
  local env = util.make_env_get()
  env.PATH_INFO = "/bar"
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  local etag = headers["ETag"]
  assert(etag)
  assert(res() == "foo" and not res())
  local value, mtime, ext = c:get("/bar")
  assert(value == dirname .. "/bar.html")
  assert(not ext)
  assert(lfs.attributes(dirname .. "/bar.html", "mtime") == mtime)  
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(not headers["X-Not-Cached"])
  assert(etag == headers["ETag"])
  assert(res() == "foo" and not res())
  env.HTTP_IF_NONE_MATCH = etag
  local status, headers, res = wsapi_app(env)
  assert(status == 304)
  c:invalidate("/bar")
  socket.sleep(1)
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  assert(headers["ETag"] and etag ~= headers["ETag"])
  assert(res() == "foo" and not res())
end

do
  local dirname = os.tmpname()
  local c = cache.new(mk.new(), dirname)
  assert(not c.values)
  local wsapi_app = c:wrap(function (wsapi_env)
			     local res = response.new()
			     res.headers["X-Not-Cached"] = true
			     res:write("foo")
			     return res:finish()
			   end)
  local env = util.make_env_get()
  env.PATH_INFO = "/bar"
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  local etag = headers["ETag"]
  assert(etag)
  assert(res() == "foo" and not res())
  assert(not c:get("/bar"))
  socket.sleep(1)
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  assert(etag ~= headers["ETag"])
  assert(res() == "foo" and not res())
end

do
  local base = "mk_cache_" .. math.random(999999)
  lfs.mkdir("/tmp/" .. base)
  local app = mk.new()
  local c = cache.new(app, base)
  assert(not c.values)
  app:dispatch_get("all", "/*", c:wrap(function (wsapi_env)
					 local res = response.new()
					 res.headers["X-Not-Cached"] = true
					 res:write("foo")
					 return res:finish()
				       end))
  local wsapi_app = app.run
  local env = util.make_env_get()
  env.PATH_INFO = "/bar"
  env.APP_PATH = "/tmp"
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  local etag = headers["ETag"]
  assert(etag)
  assert(res() == "foo" and not res())
  local value, mtime, ext = c:get("/bar")
  assert(value == "/tmp/" .. base .. "/bar.html")
  assert(not ext)
  assert(lfs.attributes("/tmp/" .. base .. "/bar.html", "mtime") == mtime)  
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(not headers["X-Not-Cached"])
  assert(etag == headers["ETag"])
  assert(res() == "foo" and not res())
  env.HTTP_IF_NONE_MATCH = etag
  local status, headers, res = wsapi_app(env)
  assert(status == 304)
  c:invalidate("/bar")
  socket.sleep(1)
  local status, headers, res = wsapi_app(env)
  assert(status == 200)
  assert(headers["X-Not-Cached"])
  assert(headers["ETag"] and etag ~= headers["ETag"])
  assert(res() == "foo" and not res())
end
