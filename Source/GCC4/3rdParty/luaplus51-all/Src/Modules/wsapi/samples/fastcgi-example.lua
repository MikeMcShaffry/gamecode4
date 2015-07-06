#!/usr/bin/env lua51

-- Example of script that uses the fastcgi launcher to launch an Orbit application

local fastcgi = require"wsapi.fastcgi"

-- require'ing the application. It should be in your package.path
local app = require"app"

-- Calls the launcher, app.run is the WSAPI run method of the application
fastcgi.run(app.run)
