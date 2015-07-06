-- Orbit pages launcher, extracts script to launch

local common = require "wsapi.common"

local function op_loader(wsapi_env)
  common.normalize_paths(wsapi_env, nil, "op.ws")
  local app = wsapi.common.load_isolated_launcher(wsapi_env.PATH_TRANSLATED, "orbit.pages")
  return app(wsapi_env)
end 

return op_loader
