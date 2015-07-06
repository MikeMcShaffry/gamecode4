-----------------------------------------------------------------------------
-- Mock WSAPI handler for Unit testing
--
-- Author: Norman Clarke
-- Copyright (c) 2010 Kepler Project
--
-----------------------------------------------------------------------------

module(..., package.seeall)

local common  = require "wsapi.common"
local request = require "wsapi.request"

-- Build a request that looks like something that would come from a real web
-- browser.
local function build_request(method, path, headers)
  local req = {
    GATEWAY_INTERFACE    = "CGI/1.1",
    HTTP_ACCEPT = "application/xml,application/xhtml+xml,text/html;q=0.9," ..
        "text/plain;q=0.8,image/png,*/*;q=0.5",
    HTTP_ACCEPT_CHARSET  = "ISO-8859-1,utf-8;q=0.7,*;q=0.3",
    HTTP_ACCEPT_ENCODING = "gzip,deflate,sdch",
    HTTP_ACCEPT_LANGUAGE = "en-US,en;q=0.8",
    HTTP_CACHE_CONTROL   = "max-age=0",
    HTTP_CONNECTION      = "keep-alive",
    HTTP_HOST            = "127.0.0.1:80",
    HTTP_USER_AGENT      = "Mozilla/5.0 (Macintosh; U; Intel Mac OS X " ..
        "10_6_4; en-US) AppleWebKit/534.3 (KHTML, like Gecko) " ..
        "Chrome/6.0.472.55",
    HTTP_VERSION         = "HTTP/1.1",
    REMOTE_ADDR          = "127.0.0.1",
    REMOTE_HOST          = "localhost",
    SCRIPT_NAME          = "wsapi_test",
    SERVER_NAME          = "localhost",
    SERVER_PORT          = "80",
    SERVER_PROTOCOL      = "HTTP/1.1"
  }

  req.PATH_INFO      = path
  req.REQUEST_METHOD = method:upper()
  req.METHOD         = req.REQUEST_METHOD
  req.REQUEST_PATH   = "/"

  if req.PATH_INFO == "" then req.PATH_INFO = "/" end

  for k, v in pairs(headers or {}) do req[k] = v end

  -- allow case-insensitive table key access
  setmetatable(req, {__index = function(t, k)
    return rawget(t, string.upper(k))
  end})
  return req
end

-- Override common's output handler to avoid writing headers
-- in the reponse body.
function common.send_output(out, status, headers, res_iter, write_method,res_line)
   common.send_content(out, res_iter, out:write())
end

-- Mock IO objects
local function make_io_object(content)
  local receiver = { buffer = content or "", bytes_read = 0 }

  function receiver:write(content)
    self.buffer = self.buffer .. content
  end

  function receiver:read(len)
    len = len or (#self.buffer - self.bytes_read)
    if self.bytes_read >= #self.buffer then return nil end
    local s = self.buffer:sub(self.bytes_read + 1, len)
    self.bytes_read = self.bytes_read + len
    if self.bytes_read > #self.buffer then self.bytes_read = #self.buffer end
    return s
  end

  function receiver:clear()
    self.buffer = ""
    self.bytes_read = 0
  end

  function receiver:reset()
    self.bytes_read = 0
  end

  return receiver
end

-- Build a GET request
local function build_get(path, params, headers)
  local req = build_request("GET", path, headers)
  req.QUERY_STRING = request.methods.qs_encode(nil, params)
  req.REQUEST_URI  = "http://" ..
      req.HTTP_HOST ..
      req.PATH_INFO ..
      req.QUERY_STRING

  return {
    env    = req,
    input  = make_io_object(),
    output = make_io_object(),
    error  = io.stderr
  }
end

local function build_post(path, params, headers)
  local req = build_request("POST", path, headers)
  local body = request.methods.qs_encode(nil, params):gsub("^?", "")
  req.REQUEST_URI  = "http://" .. req.HTTP_HOST .. req.PATH_INFO
  req.CONTENT_TYPE = "x-www-form-urlencoded"
  req.CONTENT_LENGTH = #body

  return {
    env    = req,
    input  = make_io_object(body),
    output = make_io_object(),
    error  = make_io_object()
  }
end

local function make_request(request_builder, app, path, params, headers)
  local wsapi_env = request_builder(path, params, headers)
  local response = {}
  response.code, response.headers = wsapi.common.run(app, wsapi_env)
  response.body = wsapi_env.output:read()
  return response, wsapi_env.env
end

local function get(self, path, params, headers)
  return make_request(build_get, self.app, path, params, headers)
end

local function post(self, path, params, headers)
  return make_request(build_post, self.app, path, params, headers)
end

--- Creates a WSAPI handler for testing.
-- @param app The WSAPI application you want to test.
function make_handler(app)
  return {
    app  = app,
    get  = get,
    post = post
  }
end
