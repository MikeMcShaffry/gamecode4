#!/usr/bin/env wsapi.cgi

local orbit = require "orbit"

local hello = orbit.new()

hello.config = {
  theme = {
    name = "default", config = {}
  }
}

hello.templates = {
  ["/default/layout.html"] = [=[
    <html>
      <head><title>Hello</title>
      <body>
        $inner
        <p><a href="$(req:link_say({ greeting = "Hi" }, { name = "Foo" }))">Link 1</a></p>
      </body>
    </html>
  ]=],
  ["/default/index.html"] = [=[
    <p>Hello World!</p>
  ]=],
  ["/default/say.html"] = [=[
    <p>$if{req.GET.greeting}[[$(req.GET.greeting)]][[Hello]] $name!</p>
  ]=]
}

function hello:index(req, res)
  res:write(self:layout(req, res,
                        self.theme:load("index.html"):render(req, res)))
end

function hello:say(req, res, params)
  res:write(self:layout(req, res,
                        self.theme:load("say.html"):render(req, res, params)))
end

-- Builds the application's dispatch table

hello.routes = {
  { pattern = "/", name = "index", method = "get" },
  { pattern = "/index", name = "index", method = "get" },
  { pattern = "/say/:name", name = "say", method = "get" },
}

return hello:load()
