-----------------------------------------------------------------------------
-- Fastcgi WSAPI handler
--
-- Author: Fabio Mascarenhas
-- Copyright (c) 2007 Kepler Project
--
-----------------------------------------------------------------------------

local lfcgi = require"lfcgi"
local os = require"os"
local io = require"io"
local common = require"wsapi.common"
local ipairs = ipairs

module(...)

io.stdout = lfcgi.stdout
io.stderr = lfcgi.stderr
io.stdin = lfcgi.stdin

-- Runs an WSAPI application for each FastCGI request that comes
-- from the FastCGI pipeline
function run(app_run)
   while lfcgi.accept() >= 0 do
     local headers
     local function getenv(n)
       if n == "headers" then
         if headers then return headers end
         local env_vars = lfcgi.environ()
         headers = {}
         for _, s in ipairs(env_vars) do
           local name, val = s:match("^([^=]+)=(.*)$")
           headers[name] = val
         end
         return headers
       else
         return lfcgi.getenv(n) or os.getenv(n)
       end
     end
     common.run(app_run, { input = lfcgi.stdin,
                           output = lfcgi.stdout,
                           error = lfcgi.stderr,
                           env = getenv })
   end
end