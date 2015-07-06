
local mk = require "mk"
local util = require "mk.util"
local md5 = require "md5"
local lfs

local _M = {}

_M.methods = {}
_M.methods.__index = _M.methods

local function pathinfo_to_file(cache, path_info)
  path_info = path_info:match("^/?(.-)/?$")
  local pi, ext = path_info:match("^(.+)%.([^%./]+)$")
  if not ext then
    pi, ext = path_info:match("^(.+)/([^/]+)$")
  end
  if ext and mk.mime_types[ext] then
    path_info = pi
  else
    ext = "html"
  end
  path_info = path_info:gsub("/", "_")
  if path_info == "" then path_info = "index" end
  if cache.base_path:sub(1,1) == "/" then
    return cache.base_path .. "/" .. path_info .. "." .. ext
  else
    return cache.mk_app.app_path .. "/" .. cache.base_path ..
      "/" .. path_info .. "." .. ext
  end 
end

function _M.methods:get(key)
  if not self.base_path then
    return unpack(self.values[key] or {})
  else
    local filename = pathinfo_to_file(self, key)
    local mtime = lfs.attributes(filename, "mtime")
    if mtime and lfs.attributes(filename .. ".delete_guard", "mtime") then
      return filename, mtime
    end
  end
end

function _M.methods:set(key, value, mtime, ext)
  mtime = mtime or os.time()
  ext = ext or "html"
  if not self.base_path then
    self.values[key] = { value, mtime, ext }
  else
    local tmpname = os.tmpname()
    if lfs.attributes(tmpname, "mtime") then
      os.remove(tmpname)
    end
    local tmpfile = pathinfo_to_file(self, tmpname)
    local filename = pathinfo_to_file(self, key)
    if util.writefile(tmpfile, value) then
      util.writefile(filename .. ".delete_guard", "1")
      if os.rename(tmpfile, filename) then
	lfs.touch(filename, mtime)
      end
    end
  end
end

function _M.methods:etag(key, mtime)
  if not self.base_path then
    return string.format('"%s"', md5.sumhexa(mtime .. key))
  else
    return string.format('"%s"', md5.sumhexa(mtime .. pathinfo_to_file(self, key)))
  end
end

function _M.methods:wrap(f)
  return function (wsapi_env, ...)
	   local key = wsapi_env.PATH_INFO
	   local value, mtime, ext = self:get(key)
	   if value then
	     if ext then
	       local etag = self:etag(key, mtime)
	       if wsapi_env.HTTP_IF_NONE_MATCH == etag then
		 return 304, { ["Content-Length"] = 0 }, function () end
	       end
	       local sent
	       return 200, { ["Content-Type"] = mk.mime_types[ext],
			     ["Content-Length"] = #value,
			     ["ETag"] = etag }, function ()
						  if not sent then
						    sent = true
						    return value
						  end
						end
	     else
	       return self.mk_app:serve_file(wsapi_env, value)
	     end
	   else
	     local status, headers, res = f(wsapi_env, ...)
	     if status == 200 or tostring(status):match("^200") then
	       local mtime = os.time()
	       local etag = self:etag(key, mtime)
	       headers["ETag"] = etag
	       local ext = mk.mime_extensions[headers["Content-Type"] or "text/html"]
	       local body = {}
	       return status, headers, function ()
					 local s = res()
					 if s then
					   body[#body+1] = s
					   return s
					 else
					   self:set(key, table.concat(body), mtime, ext)
					 end
				       end
	     else
	       return status, headers, res
	     end
	   end
	 end
end

function _M.methods:invalidate(...)
  for _, key in ipairs{...} do
    if not self.base_path then
      self.values[key] = nil
    else
      local filename = pathinfo_to_file(self, key)
      if lfs.attributes(filename .. ".delete_guard", "mtime") then
	os.remove(filename)
	os.remove(filename .. ".delete_guard")
      end
    end
  end
end

function _M.methods:nuke()
  if not self.base_path then 
    self.values = {}
  else
    local path
    if self.base_path:sub(1, 1) == "/" then
      path = self.base_path
    else
      path = self.mk_app.app_path .. "/" .. self.base_path
    end
    for file in lfs.dir(path) do
      if file ~= "." and file ~= ".." then 
	local filename = path .. "/" .. file
	if lfs.attributes(filename .. ".delete_guard", "mtime") then
	  os.remove(filename)
	  os.remove(filename .. ".delete_guard")
	end 
      end
    end
  end
end

_M.methods.__call = _M.methods.wrap

function _M.new(mk_app, base_path)
  local values
  if not base_path then
    values = {}
  else
    lfs = require "lfs"
  end
  local cache = { mk_app = mk_app, values = values, 
		  base_path = base_path }
  setmetatable(cache, _M.methods)
  return cache
end

return _M