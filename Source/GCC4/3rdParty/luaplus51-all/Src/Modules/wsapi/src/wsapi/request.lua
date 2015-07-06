local util = require"wsapi.util"

module("wsapi.request", package.seeall)

local function split_filename(path)
  local name_patt = "[/\\]?([^/\\]+)$"
  return (string.match(path, name_patt))
end

local function insert_field (tab, name, value, overwrite)
  if overwrite or not tab[name] then
    tab[name] = value
  else
    local t = type (tab[name])
    if t == "table" then
      table.insert (tab[name], value)
    else
      tab[name] = { tab[name], value }
    end
  end
end

local function parse_qs(qs, tab, overwrite)
  tab = tab or {}
  if type(qs) == "string" then
    local url_decode = util.url_decode
    for key, val in string.gmatch(qs, "([^&=]+)=([^&=]*)&?") do
      insert_field(tab, url_decode(key), url_decode(val), overwrite)
    end
  elseif qs then
    error("WSAPI Request error: invalid query string")
  end
  return tab
end

local function get_boundary(content_type)
  local boundary = string.match(content_type, "boundary%=(.-)$")
  return "--" .. tostring(boundary)
end

local function break_headers(header_data)
  local headers = {}
  for type, val in string.gmatch(header_data, '([^%c%s:]+):%s+([^\n]+)') do
    type = string.lower(type)
    headers[type] = val
  end
  return headers
end

local function read_field_headers(input, pos)
  local EOH = "\r\n\r\n"
  local s, e = string.find(input, EOH, pos, true)
  if s then
    return break_headers(string.sub(input, pos, s-1)), e+1
  else return nil, pos end
end

local function get_field_names(headers)
  local disp_header = headers["content-disposition"] or ""
  local attrs = {}
  for attr, val in string.gmatch(disp_header, ';%s*([^%s=]+)="(.-)"') do
    attrs[attr] = val
  end
  return attrs.name, attrs.filename and split_filename(attrs.filename)
end

local function read_field_contents(input, boundary, pos)
  local boundaryline = "\r\n" .. boundary
  local s, e = string.find(input, boundaryline, pos, true)
  if s then
    return string.sub(input, pos, s-1), s-pos, e+1
  else return nil, 0, pos end
end

local function file_value(file_contents, file_name, file_size, headers)
  local value = { contents = file_contents, name = file_name,
    size = file_size }
  for h, v in pairs(headers) do
    if h ~= "content-disposition" then
      value[h] = v
    end
  end
  return value
end

local function fields(input, boundary)
  local state, _ = { }
  _, state.pos = string.find(input, boundary, 1, true)
  state.pos = state.pos + 1
  return function (state, _)
     local headers, name, file_name, value, size
     headers, state.pos = read_field_headers(input, state.pos)
     if headers then
       name, file_name = get_field_names(headers)
       if file_name then
         value, size, state.pos = read_field_contents(input, boundary,
            state.pos)
         value = file_value(value, file_name, size, headers)
       else
         value, size, state.pos = read_field_contents(input, boundary,
            state.pos)
       end
     end
     return name, value
   end, state
end

local function parse_multipart_data(input, input_type, tab, overwrite)
  tab = tab or {}
  local boundary = get_boundary(input_type)
  for name, value in fields(input, boundary) do
    insert_field(tab, name, value, overwrite)
  end
  return tab
end

local function parse_post_data(wsapi_env, tab, overwrite)
  tab = tab or {}
  local input_type = wsapi_env.CONTENT_TYPE
  if string.find(input_type, "x-www-form-urlencoded", 1, true) then
    local length = tonumber(wsapi_env.CONTENT_LENGTH) or 0
    parse_qs(wsapi_env.input:read(length) or "", tab, overwrite)
  elseif string.find(input_type, "multipart/form-data", 1, true) then
    local length = tonumber(wsapi_env.CONTENT_LENGTH) or 0
    if length > 0 then
       parse_multipart_data(wsapi_env.input:read(length) or "", input_type, tab, overwrite)
    end
  else
    local length = tonumber(wsapi_env.CONTENT_LENGTH) or 0
    tab.post_data = wsapi_env.input:read(length) or ""
  end
  return tab
end

methods = {}

function methods.__index(tab, name)
  local func
  if methods[name] then
    func = methods[name]
  else
    local route_name = name:match("link_([%w_]+)")
    if route_name then
      func = function (self, query, ...)
         return tab:route_link(route_name, query, ...)
       end
    end
  end
  tab[name] = func
  return func
end

function methods:qs_encode(query)
  local parts = {}
  for k, v in pairs(query or {}) do
    parts[#parts+1] = k .. "=" .. wsapi.util.url_encode(v)
  end
  if #parts > 0 then
    return "?" .. table.concat(parts, "&")
  else
    return ""
  end
end

function methods:route_link(route, query, ...)
  local builder = self.mk_app["link_" .. route]
  if builder then
    local uri = builder(self.mk_app, self.env, ...)
    return uri .. self:qs_encode(query)
  else
    error("there is no route named " .. route)
  end
end

function methods:link(url, query)
  local prefix = (self.mk_app and self.mk_app.prefix) or self.script_name
  local uri = prefix .. url
  return prefix .. url .. self:qs_encode(query)
end

function methods:absolute_link(url, query)
  return url .. self:qs_encode(query)
end

function methods:static_link(url)
  local prefix = (self.mk_app and self.mk_app.prefix) or self.script_name
  local is_script = prefix:match("(%.%w+)$")
  if not is_script then return self:link(url) end
  local vpath = prefix:match("(.*)/") or ""
  return vpath .. url
end

function methods:empty(s)
  return not s or string.match(s, "^%s*$")
end

function methods:empty_param(param)
  return self:empty(self.params[param])
end

function new(wsapi_env, options)
  options = options or {}
  local req = {
    GET          = {},
    POST         = {},
    method       = wsapi_env.REQUEST_METHOD,
    path_info    = wsapi_env.PATH_INFO,
    query_string = wsapi_env.QUERY_STRING,
    script_name  = wsapi_env.SCRIPT_NAME,
    env          = wsapi_env,
    mk_app       = options.mk_app,
    doc_root     = wsapi_env.DOCUMENT_ROOT,
    app_path     = wsapi_env.APP_PATH
  }
  parse_qs(wsapi_env.QUERY_STRING, req.GET, options.overwrite)
  if options.delay_post then
    req.parse_post = function (self)
      parse_post_data(wsapi_env, self.POST, options.overwrite)
      self.parse_post = function () return nil, "postdata already parsed" end
      return self.POST
    end
  else
    parse_post_data(wsapi_env, req.POST, options.overwrite)
    req.parse_post = function () return nil, "postdata already parsed" end
  end
  req.params = {}
  setmetatable(req.params, { __index = function (tab, name)
    local var = req.GET[name] or req.POST[name]
    rawset(tab, name, var)
    return var
  end})
  req.cookies = {}
  local cookies = string.gsub(";" .. (wsapi_env.HTTP_COOKIE or "") .. ";",
            "%s*;%s*", ";")
  setmetatable(req.cookies, { __index = function (tab, name)
    name = name
    local pattern = ";" .. name ..
      "=(.-);"
    local cookie = string.match(cookies, pattern)
    cookie = util.url_decode(cookie)
    rawset(tab, name, cookie)
    return cookie
  end})
  return setmetatable(req, methods)
end