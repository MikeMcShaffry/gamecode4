#!/usr/bin/env wsapi.cgi

local mk = require "mk"
local request = require "wsapi.request"
local response = require "wsapi.response"
local cosmo = require "cosmo"
local template = require "mk.template"
local themes = require "mk.themes"
local blocks = require "mk.blocks"

----------------------------------------------------------------
-- create new mk application (it is also a wsapi app)
--

local songs = mk.new()

----------------------------------------------------------------
-- engine that looks up templates in songs.templates
-- sputnik would use look up templates in SACI
--

local engine = setmetatable({}, { __index = template })

function engine.load(tmpl_name)
  return engine.compile(songs.templates[tmpl_name])
end

----------------------------------------------------------------
-- actions and routing
--

function songs:index(req, res)
   -- load and render the pages/main template of our theme
   res:write(songs.theme:load("pages/main"):render(req, res))
end

-- internal route
songs:dispatch_get("index", "/")
-- route to another application
songs:dispatch_get("hello", "/say/:name", "/samples/hello.lua")

----------------------------------------------------------------
-- intializing table of block instances
--

songs.blocks = {
  links = blocks.links(songs, { title = "Links",
                                links = { { "Kepler", "http://www.keplerproject.org" },
                                          { "Lua", "http://www.lua.org" } } })
}

----------------------------------------------------------------
-- block prototypes
--

local function block_song_list(app, args, tmpl)
  tmpl = tmpl or template.compile[=[<ul>$songs[[<li>$it</li>]]</ul>]=]
  return function (req, res, env, name)
           return tmpl:render(req, res, setmetatable({ songs = args.list }, { __index = env }))
         end
end

----------------------------------------------------------------
-- load the default theme with the area configuration passed
-- in the last parameter
--

-- hardcoded here, but should be at some stable storage
-- sputnik would store this information in some SACI node
local name = "default"
local prefix = nil -- set to have a prefix in all template names
local config = { areas = { main = { "song_list", "links" } } }

songs.theme = themes.new{ blocks = songs.blocks,
                          name = name,
                          config = config,
                          engine = engine }

----------------------------------------------------------------
-- our templates, this should also be at some stable storage
--

songs.templates = {}

-- main template, includes the only area in this theme
songs.templates["/default/pages/main"] = [[
      <html>
        <head><title>Song List</title></head>
        <body>$(areas['main'])</body>
      </html>
]]

-- our override for the song_list block template
songs.templates["/default/blocks/song_list"] = [[
    <h1>Songs</h1>
      <table>
        $songs[=[<tr><td>$it</td></tr>]=]
      </table>
      <p><a href="$(req:link_hello({ greeting = "Hi" }, { name = "World" }))">Say Hi!</a></p>
]]

----------------------------------------------------------------
-- finally, create block instance using one of our templates
--

songs.blocks.song_list = block_song_list(songs, {
                           list = {
                             "Sgt. Pepper's Lonely Hearts Club Band",
                             "With a Little Help from My Friends",
                             "Lucy in the Sky with Diamonds",
                             "Getting Better",
                             "Fixing a Hole",
                             "She's Leaving Home",
                             "Being for the Benefit of Mr. Kite!",
                             "Within You Without You",
                             "When I'm Sixty-Four",
                             "Lovely Rita",
                             "Good Morning Good Morning",
                             "Sgt. Pepper's Lonely Hearts Club Band (Reprise)",
                             "A Day in the Life"
                         } }, songs.theme:load("blocks/song_list"))
-- remove the last argument above to use the block's default template (inline
-- at the block prototype)

----------------------------------------------------------------
-- return wsapi application
--

return songs.run
