
local request = require "wsapi.request"
local response = require "wsapi.response"
local util = require "wsapi.util"
local _, lfs = pcall(require, "lfs")
local _, md5 = pcall(require, "md5")

local R = require "mk.routes"

local mk = { _NAME = "mk" }

mk._NAME = "mk"
mk._VERSION = "0.1"
mk._COPYRIGHT = "Copyright (C) 2010 Kepler Project"
mk._DESCRIPTION = "Mini-framework for Kepler applications"

mk.mime_types = {
  ez = "application/andrew-inset",
  atom = "application/atom+xml",
  hqx = "application/mac-binhex40",
  cpt = "application/mac-compactpro",
  mathml = "application/mathml+xml",
  doc = "application/msword",
  bin = "application/octet-stream",
  dms = "application/octet-stream",
  lha = "application/octet-stream",
  lzh = "application/octet-stream",
  exe = "application/octet-stream",
  class = "application/octet-stream",
  so = "application/octet-stream",
  dll = "application/octet-stream",
  dmg = "application/octet-stream",
  oda = "application/oda",
  ogg = "application/ogg",
  pdf = "application/pdf",
  ai = "application/postscript",
  eps = "application/postscript",
  ps = "application/postscript",
  rdf = "application/rdf+xml",
  smi = "application/smil",
  smil = "application/smil",
  gram = "application/srgs",
  grxml = "application/srgs+xml",
  mif = "application/vnd.mif",
  xul = "application/vnd.mozilla.xul+xml",
  xls = "application/vnd.ms-excel",
  ppt = "application/vnd.ms-powerpoint",
  rm = "application/vnd.rn-realmedia",
  wbxml = "application/vnd.wap.wbxml",
  wmlc = "application/vnd.wap.wmlc",
  wmlsc = "application/vnd.wap.wmlscriptc",
  vxml = "application/voicexml+xml",
  bcpio = "application/x-bcpio",
  vcd = "application/x-cdlink",
  pgn = "application/x-chess-pgn",
  cpio = "application/x-cpio",
  csh = "application/x-csh",
  dcr = "application/x-director",
  dir = "application/x-director",
  dxr = "application/x-director",
  dvi = "application/x-dvi",
  spl = "application/x-futuresplash",
  gtar = "application/x-gtar",
  hdf = "application/x-hdf",
  xhtml = "application/xhtml+xml",
  xht = "application/xhtml+xml",
  js = "application/x-javascript",
  skp = "application/x-koan",
  skd = "application/x-koan",
  skt = "application/x-koan",
  skm = "application/x-koan",
  latex = "application/x-latex",
  xml = "application/xml",
  xsl = "application/xml",
  dtd = "application/xml-dtd",
  nc = "application/x-netcdf",
  cdf = "application/x-netcdf",
  sh = "application/x-sh",
  shar = "application/x-shar",
  swf = "application/x-shockwave-flash",
  xslt = "application/xslt+xml",
  sit = "application/x-stuffit",
  sv4cpio = "application/x-sv4cpio",
  sv4crc = "application/x-sv4crc",
  tar = "application/x-tar",
  tcl = "application/x-tcl",
  tex = "application/x-tex",
  texinfo = "application/x-texinfo",
  texi = "application/x-texinfo",
  t = "application/x-troff",
  tr = "application/x-troff",
  roff = "application/x-troff",
  man = "application/x-troff-man",
  me = "application/x-troff-me",
  ms = "application/x-troff-ms",
  ustar = "application/x-ustar",
  src = "application/x-wais-source",
  zip = "application/zip",
  au = "audio/basic",
  snd = "audio/basic",
  mid = "audio/midi",
  midi = "audio/midi",
  kar = "audio/midi",
  mpga = "audio/mpeg",
  mp2 = "audio/mpeg",
  mp3 = "audio/mpeg",
  aif = "audio/x-aiff",
  aiff = "audio/x-aiff",
  aifc = "audio/x-aiff",
  m3u = "audio/x-mpegurl",
  ram = "audio/x-pn-realaudio",
  ra = "audio/x-pn-realaudio",
  wav = "audio/x-wav",
  pdb = "chemical/x-pdb",
  xyz = "chemical/x-xyz",
  bmp = "image/bmp",
  cgm = "image/cgm",
  gif = "image/gif",
  ief = "image/ief",
  jpeg = "image/jpeg",
  jpg = "image/jpeg",
  jpe = "image/jpeg",
  png = "image/png",
  svg = "image/svg+xml",
  svgz = "image/svg+xml",
  tiff = "image/tiff",
  tif = "image/tiff",
  djvu = "image/vnd.djvu",
  djv = "image/vnd.djvu",
  wbmp = "image/vnd.wap.wbmp",
  ras = "image/x-cmu-raster",
  ico = "image/x-icon",
  pnm = "image/x-portable-anymap",
  pbm = "image/x-portable-bitmap",
  pgm = "image/x-portable-graymap",
  ppm = "image/x-portable-pixmap",
  rgb = "image/x-rgb",
  xbm = "image/x-xbitmap",
  xpm = "image/x-xpixmap",
  xwd = "image/x-xwindowdump",
  igs = "model/iges",
  iges = "model/iges",
  msh = "model/mesh",
  mesh = "model/mesh",
  silo = "model/mesh",
  wrl = "model/vrml",
  vrml = "model/vrml",
  ics = "text/calendar",
  ifb = "text/calendar",
  css = "text/css",
  html = "text/html",
  htm = "text/html",
  asc = "text/plain",
  txt = "text/plain",
  rtx = "text/richtext",
  rtf = "text/rtf",
  sgml = "text/sgml",
  sgm = "text/sgml",
  tsv = "text/tab-separated-values",
  wml = "text/vnd.wap.wml",
  wmls = "text/vnd.wap.wmlscript",
  etx = "text/x-setext",
  mpeg = "video/mpeg",
  mpg = "video/mpeg",
  mpe = "video/mpeg",
  qt = "video/quicktime",
  mov = "video/quicktime",
  mxu = "video/vnd.mpegurl",
  avi = "video/x-msvideo",
  movie = "video/x-sgi-movie",
  ice = "x-conference/x-cooltalk",
  rss = "application/rss+xml",
  atom = "application/atom+xml"
}

mk.mime_extensions = {}

for ext, type in pairs(mk.mime_types) do
  mk.mime_extensions[type] = ext
end

mk.methods = {}

function mk.new(app)
  if type(app) == "string" then
    app = { _NAME = app }
  else
    app = app or {}
  end
  for k, v in pairs(mk.methods) do
    app[k] = v
  end
  app.app_path = app.app_path or wsapi.app_path or "."
  app.run = function (wsapi_env)
              return mk.run(app, wsapi_env)
            end
  app.not_found = function (wsapi_env)
                    local res = response.new()
                    res.status = 400
                    res:write[[<html>
                          <head><title>Not Found</title></head>
                          <body><p>Not found!</p></body></html>]]
                    return res:finish()
                  end
  app.server_error = function (wsapi_env, msg)
                       local res = response.new()
                       res.status = 500
                       res:write([[<html>
                             <head><title>Server Error</title></head>
                             <body><pre>]] .. msg .. [[</pre></body></html>]])
                       return res:finish()
                     end
  app.dispatch_table = { get = {}, post = {}, put = {}, delete = {} }
  return app
end

mk.pattern_route_methods = {}
mk.pattern_route_methods.__index = mk.pattern_route_methods

function mk.pattern_route_methods:match(s)
  return self.pattern:match(s)
end

function mk.pattern_route(route)
  return setmetatable({ pattern = "^" .. route .. "$" }, mk.pattern_route_methods)
end

for _, method in ipairs{ "get", "post", "put", "delete" } do
  mk.methods["dispatch_" .. method] = function (self, name, route, handler)
                                        handler = handler or self:wrap(name)
                                        if type(route) == "string" then
                                          route = R(route)
                                        end
                                        local build
                                        if type(handler) ~= "string" then
                                          table.insert(self.dispatch_table[method], { name = name,
                                                                                      route = route,
                                                                                      handler = handler })
                                          if route.build then
                                            build = function (self, wsapi_env, ...)
                                                      local prefix = self.prefix or wsapi_env.SCRIPT_NAME
                                                      return prefix .. route:build(...)
                                                    end
                                          end
                                        elseif route.build then
                                          build = function (self, wsapi_env, ...)
                                                    return handler .. route:build(...)
                                                  end
                                        end
                                        if route.build then
                                          self["route_" .. name] = function (self, ...)
                                                                     return route:build(...)
                                                                   end
                                        end
                                        self["link_" .. name] = build
                                      end
end

function mk.methods:dispatch_static(name, route)
  self:dispatch_get(name, route, self:serve_static())
end

function mk.methods:serve_static()
   return function (wsapi_env)
            local filename = self.app_path .. wsapi_env.PATH_INFO
            return self:serve_file(wsapi_env, filename)
          end
end

function mk.methods:serve_file(wsapi_env, filename)
  local res = response.new()
  local ext = string.match(filename, "%.([^%.]+)$")
  if self.use_xsendfile then
    res.headers["Content-Type"] = mk.mime_types[ext] or
      "application/octet-stream"
    res.headers["X-Sendfile"] = filename
    return res:finish()
  else
    local etag
    if lfs and md5 then
      local mtime = lfs.attributes(filename, "mtime")
      if not mtime then return self.not_found(wsapi_env) end
      etag = string.format('"%s"', md5.sumhexa(tostring(mtime) .. filename))
      if wsapi_env.HTTP_IF_NONE_MATCH == etag then
        res.status = 304
        res.headers["Content-Length"] = 0
        return res:finish()
      end
    end
    local file = io.open(filename, "rb")
    if not file then
      return self.not_found(wsapi_env)
    else
      res.headers["Content-Type"] = mk.mime_types[ext] or
        "application/octet-stream"
      res.headers["Content-Length"] = file:seek("end")
      if etag then res.headers["ETag"] = etag end
      file:seek("set")
      local block_size = self.block_size or 4096
      return res.status, res.headers, coroutine.wrap(function ()
                                                       local s = file:read(block_size)
                                                       while s do
                                                         coroutine.yield(s)
                                                         s = file:read(block_size)
                                                       end
                                                       file:close()
                                                     end)
    end
  end
end

function mk.methods:match(method, path, index)
  index = index or 0
  if #self.dispatch_table[method] == 0 then
    return self["handle_" .. method], {}
  else
    for index = index+1, #self.dispatch_table[method] do
      local entry = self.dispatch_table[method][index]
      local captures = { entry.route:match(path) }
      if #captures > 0 then
        return entry.handler, captures, index
      end
    end
  end
end

function mk.methods:wrap(action)
  local handler
  if type(action) == "string" then
    handler = function (req, res, ...)
                return self[action](self, req, res, ...)
              end
  else
    handler = action
  end
  return function (wsapi_env, ...)
           local req = request.new(wsapi_env, { mk_app = self })
           local res = response.new()
           local ans = { handler(req, res, ...) }
           if #ans == 0 then
             return res:finish()
           else
             return unpack(ans)
           end
         end
end

function mk.run(self, wsapi_env)
  local path = util.url_decode(wsapi_env.PATH_INFO)
  local method = string.lower(wsapi_env.REQUEST_METHOD)
  local handler, captures, index = self:match(method, path)
  if util.not_empty(wsapi_env.APP_PATH) then
    self.app_path = wsapi_env.APP_PATH
  else
    self.app_path = "."
  end
  handler = handler or self.not_found
  captures = captures or {}
  repeat
    local ok, status, headers, res = xpcall(function ()
                                              return handler(wsapi_env, unpack(captures))
                                            end, debug.traceback)
    if ok then
      if status == "MK_FORWARD" then
        local path = util.url_decode(wsapi_env.PATH_INFO)
        local method = string.lower(wsapi_env.REQUEST_METHOD)
        handler, captures, index = self:match(method, path, index)
        handler, captures = handler or self.not_found, captures or {}
        ok = false
      else
        return status, headers, res
      end
    else
      handler, captures = self.server_error, { status }
    end
  until ok
end

return mk
