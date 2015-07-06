#!/usr/bin/env wsapi.cgi

local mk = require "mk"
local request = require "wsapi.request"
local response = require "wsapi.response"

local hello = mk.new()

function hello.index(wsapi_env)
  local req, res = request.new(wsapi_env, { mk_app = hello }), response.new()
  res:write(hello.render_index(req, res))
  return res:finish()
end

function hello.say(wsapi_env, params)
  local req, res = request.new(wsapi_env, { mk_app = hello }), response.new()
  res:write(hello.render_say(req, res, params.name))
  return res:finish()
end

hello:dispatch_get("index", "/", hello.index)
hello:dispatch_get("say", "/say/:name", hello.say)
hello:dispatch_get("songs", "/", "/samples/songs.lua")

function hello.render_layout(inner_html)
  return string.format([[
      <html>
        <head><title>Hello</title></head>
        <body>%s</body>
      </html>
    ]], inner_html)
end

function hello.render_hello(req, res)
  return string.format([[
      <p><a href="%s">Link 1</a></p>
      <p><a href="%s">Link 2</a></p>
      <p>Hello World!</p>
    ]], req:link_say({ greeting = "Hi" }, { name = "Foo" }),
        req:link_songs())
end

function hello.render_index(req, res)
  return hello.render_layout(hello.render_hello(req, res))
end

function hello.render_say(req, res, name)
  return hello.render_layout(hello.render_hello(req, res) .. 
				string.format([[<p>%s %s!</p>]], req.params.greeting or "Hello ", name))
end

return hello
