#!/usr/bin/env lua51

-- Example of script that uses the cgi launcher to launch an Orbit application

local cgi = require"wsapi.cgi"

-- require'ing the application. It should be in your package.path
local app = require"app"

-- Calls the launcher, app.run is the WSAPI run method of the application
cgi.run(app.run)
