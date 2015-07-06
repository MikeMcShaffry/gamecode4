
local themes = {}

local _, template = pcall(require, "mk.template")
local util = require "mk.util"

local methods = {}
methods.__index = methods

local template_methods = {}
template_methods.__index = template_methods

function template_methods:render(req, res, env)
  local blocks = self.theme.blocks
  local areas = setmetatable({}, { __index = function (t, name)
                                               local area = function (args, has_block)
                                                              local out = {}
                                                              for _, block in ipairs(self.theme.areas[name]) do
                                                                out[#out+1] = blocks[block](req, res, env, block)
                                                              end
                                                              return table.concat(out)
                                                            end
                                               t[name] = area
                                               return area
                                             end })
  local env = setmetatable({ areas = areas }, { __index = env })
  return self.tmpl:render(req, res, env)
end

function methods:load(name, engine)
  engine = engine or self.engine
  local cached = self.cache[name]
  if cached then
    return cached
  end
  local tmpl, err = engine.load(self.path ..  "/" .. name)
  if tmpl then
    local template = setmetatable({ theme = self, tmpl = tmpl }, template_methods)
    self.cache[name] = template
    return template
  else
    return tmpl, err
  end
end

function themes.new(args)
  local blocks, name, path, theme, engine =
    args.blocks, args.name, args.path, args.config, args.engine
  local theme_path = (path or "") .. "/" .. name
  local theme, err = theme or util.loadin(theme_path .. "/config.lua")
  if theme then
    theme.blocks = blocks
    theme.name = name
    theme.path = theme_path
    theme.engine = engine or template
    theme.cache = setmetatable({}, { __mode = "v" })
    return setmetatable(theme, methods)
  else
    return nil, err
  end
end

return themes
