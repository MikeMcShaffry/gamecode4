#!/usr/bin/env wsapi.cgi

local mk = require "mk"
local crypto = require "crypto"
local request = require "wsapi.request"
local response = require "wsapi.response"
local auth = require "mk.auth"

local hello = mk.new()

local login_salt = "LOGIN_SALT"
local session_salt = "SESSION_SALT"

local users = {
  mascarenhas = crypto.hmac.digest("sha1", "foobar", login_salt),
  carregal = crypto.hmac.digest("sha1", "baz", login_salt),
}

function hello.login(user, pass)
  if users[user] == pass then
    return user
  elseif users[user] then
    return nil, "invalid password"
  else
    return nil, "user not found"
  end
end

hello.auth = auth.new{ login = hello.login, 
		       login_salt = login_salt,
		       session_salt = session_salt }

function hello.index(req, res)
  res:write(hello.render_index(req, res))
  return res:finish()
end

function hello.say(wsapi_env, params)
  local req, res = request.new(wsapi_env, { mk_app = hello }), response.new()
  res:write(hello.render_say(req, res, params.name))
  return res:finish()
end

function hello.logoff(wsapi_env)
  local req, res = request.new(wsapi_env, { mk_app = hello }), response.new()
  hello.auth:logoff(res.headers)
  return res:redirect(req:link_index())
end

hello:dispatch_get("index", "/", hello:wrap(hello.index))
hello:dispatch_get("say", "/say/:name", hello.say)
hello:dispatch_get("songs", "/", "/samples/songs.lua")
hello:dispatch_post("login", "/login", hello.auth:provider())
hello:dispatch_get("logoff", "/logoff", hello.logoff)

function hello.render_layout(inner_html)
  return string.format([[
      <html>
        <head><title>Hello</title></head>
        <body>%s</body>
      </html>
    ]], inner_html)
end

function hello.render_login(req, res)
  if req:empty(req.env.MK_AUTH_USER) then
    return string.format([[
      <form method="post" action="%s">
      <p>User: <input type = "text" name = "username"/></p>
      <p>Password: <input type = "password"  name = "password"/></p>
      <input type = "hidden" name = "success" value = "%s"/>
      <input type = "hidden" name = "failure" value = "%s"/>
      <input type = "submit"/>
      </form>
    ]], req:link_login(), req:link_index(), req:link_index())
  else
    return string.format([[<p><a href="%s">Logoff</a>]], req:link_logoff())
  end
end

function hello.render_hello(req, res)
  local user = (req:empty(req.env.MK_AUTH_USER) and "World") or req.env.MK_AUTH_USER
  return string.format([[
      <p><a href="%s">Link 1</a></p>
      <p><a href="%s">Link 2</a></p>
      <p>Hello %s!</p>
    ]], req:link_say({ greeting = "Hi" }, { name = "Foo" }),
        req:link_songs(), user) .. hello.render_login(req, res)
end

function hello.render_index(req, res)
  return hello.render_layout(hello.render_hello(req, res))
end

function hello.render_say(req, res, name)
  return hello.render_layout(hello.render_hello(req, res) .. 
				string.format([[<p>%s %s!</p>]], req.params.greeting or "Hello ", name))
end

return hello.auth:filter(hello.run)
