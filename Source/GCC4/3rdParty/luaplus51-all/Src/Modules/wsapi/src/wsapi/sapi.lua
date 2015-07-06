#!/usr/bin/env wsapi.cgi

require "wsapi.response"

module(..., package.seeall)

function run(wsapi_env)
  _G.CGILUA_APPS = _G.CGILUA_APPS or wsapi_env.DOCUMENT_ROOT .. "/cgilua"
  _G.CGILUA_CONF = _G.CGILUA_CONF or wsapi_env.DOCUMENT_ROOT .. "/cgilua"
  _G.CGILUA_TMP = _G.CGILUA_TMP or os.getenv("TMP") or os.getenv("TEMP") or "/tmp"
  _G.CGILUA_ISDIRECT = true

  local res = wsapi.response.new()

  _G.SAPI = {
    Info =  {
      _COPYRIGHT = "Copyright (C) 2007 Kepler Project",
      _DESCRIPTION = "WSAPI SAPI implementation",
      _VERSION = "WSAPI SAPI 1.0",
      ispersistent = false,
    },
    Request = {
      servervariable = function (name) return wsapi_env[name] end,
      getpostdata = function (n) return wsapi_env.input:read(n) end
    },
    Response = {
      contenttype = function (header)
        res["Content-Type"] = header
      end,
      errorlog = function (msg, errlevel)
        wsapi_env.error:write (msg)
      end,
      header = function (header, value)
        if res[header] then
          if type(res[header]) == "table" then
            table.insert(res[header], value)
          else
            res[header] = { res[header], value }
          end
        else
          res[header] = value
        end
      end,
      redirect = function (url)
        res.status = 302
        res.headers["Location"] = url
      end,
      write = function (...)
        res:write({...})
      end,
    },
  }
  require"cgilua"
  cgilua.main()
  return res:finish()
end