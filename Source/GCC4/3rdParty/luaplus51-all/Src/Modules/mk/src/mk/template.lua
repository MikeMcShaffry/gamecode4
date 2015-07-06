
local template = {}

local cosmo = require "cosmo"
local util = require "mk.util"

local methods = {}
methods.__index = methods

function methods:render(req, res, env)
  env = setmetatable({ 
		       ["if"] = cosmo.cif,
		       req = req, res = res,
		       format = function (args)
				  return string.format(args[1], select(2, unpack(args)))
				end
		     }, { __index = env})
  return self.template(env)
end

function template.load(filename)
  local tmpl, err = util.readfile(filename)
  if not tmpl then
    return nil, err
  end
  return template.compile(tmpl, filename)
end

function template.compile(str, filename)
  return setmetatable({ template = cosmo.compile(str, filename and "@" .. filename) }, methods)
end

return template
