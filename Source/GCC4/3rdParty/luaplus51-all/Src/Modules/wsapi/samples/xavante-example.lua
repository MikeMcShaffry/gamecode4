-------------------------------------------------------------------------------
-- Sample Xavante configuration file for launching WSAPI applications.
------------------------------------------------------------------------------

require "xavante"
require "xavante.filehandler"
require "wsapi.xavante"

-- Define here where Xavante HTTP documents scripts are located
local webDir = "/var/www"

local simplerules = {

    { -- WSAPI application will be mounted under /app
      match = { "%.lua$", "%.lua/" },
      with = wsapi.xavante.makeGenericHandler(webDir)
    },
    
    { -- filehandler 
      match = ".",
      with = xavante.filehandler,
      params = {baseDir = webDir}
    },
} 

-- Displays a message in the console with the used ports
xavante.start_message(function (ports)
    local date = os.date("[%Y-%m-%d %H:%M:%S]")
    print(string.format("%s Xavante started on port(s) %s",
      date, table.concat(ports, ", ")))
  end)

xavante.HTTP{
    server = {host = "*", port = 8080},
    
    defaultHost = {
    	rules = simplerules
    },
}

xavante.start()

