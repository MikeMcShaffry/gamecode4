-----------------------------------------------------------------------------
-- CGI WSAPI handler
--
-- Author: Fabio Mascarenhas
-- Copyright (c) 2007 Kepler Project
--
-----------------------------------------------------------------------------

local os = require"os"
local io = require"io"
local common = require"wsapi.common"

common.setmode()

module(...)

-- Runs an WSAPI application for this CGI request
function run(app_run)
   common.run(app_run, { input = io.stdin, output = io.stdout,
     error = io.stderr, env = os.getenv })
end
