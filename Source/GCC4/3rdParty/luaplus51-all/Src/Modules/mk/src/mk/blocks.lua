
local template = require "mk.template"
local util = require "mk.util"

local blocks = {}

local title_tmpl = [[<title>$title</title>]]

function blocks.title(app, args, tmpl)
  tmpl = tmpl or template.compile(title_tmpl)
  return function (req, res)
	   return tmpl:render(req, res, { title = args[1] })
	 end
end

local js_tmpl = [=[
$js[[
  <script type = "text/javascript" src = "$it"></script>
]]
]=]

function blocks.javascript(app, args, tmpl)
  tmpl = tmpl or template.compile(js_tmpl)
  return function (req, res)
	   local js = util.concat(args, app.js)
	   for i, uri in ipairs(js) do
	     if not uri:match("^[%a+]:") and uri:sub(1, 1) ~= "/" then
	       js[i] = req:static_link("/" .. uri)
	     end
	   end
	   return tmpl:render(req, res, { js = js })
	 end
end

local css_tmpl = [=[
$css[[
  <link rel = "stylesheet" type = "text/css" href = "$it"/>
]]
]=]

function blocks.css(app, args, tmpl)
  tmpl = tmpl or template.compile(css_tmpl)
  return function (req, res)
	   local css = util.concat(args, app.css)
	   for i, uri in ipairs(css) do
	     if not uri:match("^[%a+]:") and uri:sub(1, 1) ~= "/" then
	       css[i] = req:static_link("/" .. uri)
	     end
	   end
	   return tmpl:render(req, res, { css = css })
	 end
end

local banner_tmpl = [[
  <div id = "$id">
    <h1><a href = "$(req:link_index())">$title</a></h1>
    <h3>$tagline</h3>
  </div>
]]

function blocks.banner(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(banner_tmpl)
  return function (req, res, env, name)
	   args.id = args.id or name
	   return tmpl:render(req, res, args)
	 end
end

local copyright_tmpl = [[
  <div id = "$id">
    <p>Copyright $year</p>
  </div>
]]

function blocks.copyright(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(copyright_tmpl)
  return function (req, res, env, name)
	   return tmpl:render(req, res, { year = args[1], id = args.id or name })
	 end
end

local generic_tmpl = [[
  <div id = "$id">
    <h3>$title</h3>
    <p>$text</p>
  </div>
]]

function blocks.generic(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(generic_tmpl)
  return function (req, res, env, name)
	   args.id = args.id or name
	   return tmpl:render(req, res, args)
	 end
end

local links_tmpl = [=[
  <div id = "$id">
    <h3>$title</h3>
    <ul>
      $links[[<li><a href = "$2">$1</a></li>]]
    </ul>
  </div>
]=]

function blocks.links(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(links_tmpl)
  return function (req, res, env, name)
	   args.id = args.id or name
	   return tmpl:render(req, res, args)
	 end
end

return blocks
