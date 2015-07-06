
local auth = require "mk.auth"
local crypto = require "crypto"
local util = require "wsapi.util"

local login_salt = "LOGIN_SALT"
local session_salt = "SESSION_SALT"

local users = {
  mascarenhas = crypto.hmac.digest("sha1", "foobar", login_salt),
  carregal = crypto.hmac.digest("sha1", "baz", login_salt),
}

local function login(user, pass)
  if users[user] == pass then
    return user
  elseif users[user] then
    return nil, "invalid password"
  else
    return nil, "user not found"
  end
end

do
  -- basic login
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local user, message = a:login("mascarenhas", "foobar")
  assert(user == "mascarenhas")
  assert(a:authenticate(message) == "mascarenhas")
end

do
  -- make token
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local user, message = "mascarenhas", a:token("mascarenhas")
  assert(a:authenticate(message) == user)
end

do
  -- wrong password
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local user, message = a:login("mascarenhas", "foo")
  assert(not user)
  assert(message == "invalid password")
end

do
  -- unknown user
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local user, message = a:login("fabio", "foo")
  assert(not user)
  assert(message == "user not found")
end

do
  -- login expired
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt,
		      expiration = 0 }
  local user, message = a:login("mascarenhas", "foobar")
  assert(user == "mascarenhas")
  user, message = a:authenticate(message)
  assert(not user)
  assert(message == "login expired")
end

do
  -- try to impersonate user
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local user, message = a:login("mascarenhas", "foobar")
  assert(user == "mascarenhas")
  message = message:gsub("mascarenhas", "carregal")
  user, message = a:authenticate(message)
  assert(not user)
  assert(message == "invalid login")
end

do
  -- try to change expiration
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt, expiration = 0 }
  local user, message = a:login("mascarenhas", "foobar")
  assert(user == "mascarenhas")
  message = message:gsub("exp=%d+", "exp=" .. (os.time() + 3600))
  user, message = a:authenticate(message)
  assert(not user)
  assert(message == "invalid login")
end

do
  -- test salt algorithm
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  assert(users["mascarenhas"] == a:salt_password("foobar"))
end

local function make_wsapi_app(user)
  return function (wsapi_env)
	   assert(wsapi_env.MK_AUTH_USER == (user or ""))
	 end
end

do
  -- successful login with json data
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("json=" .. json.encode({ username = "mascarenhas",
						     password = "foobar",
						     success = "/done",
						     failure = "/fail" }))
  local status, headers, res = a:provider()(env)
  assert(status == 302)
  assert(headers["Location"] == "/done")
  local cookie = util.url_decode(headers["Set-Cookie"]:match("mk_auth_user=(.+)"))
  local user, message = a:authenticate(cookie)
  assert(user == "mascarenhas")
end

do
  -- successful login with json data, json response
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("json=" .. json.encode({ username = "mascarenhas",
							  password = "foobar" }))
  local status, headers, res = a:provider()(env)
  assert(status == 200)
  local res = json.decode(res())
  local cookie = util.url_decode(headers["Set-Cookie"]:match("mk_auth_user=(.+)"))
  local user, message = a:authenticate(cookie)
  assert(user == "mascarenhas")
  assert(res.user == user)
  assert(a:authenticate(res.message) == user)
end

do
  -- successful login with json data, change cookie name
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt, cookie = "cookie" }
  local env = util.make_env_post("json=" .. json.encode({ username = "mascarenhas",
						     password = "foobar",
						     success = "/done",
						     failure = "/fail" }))
  local status, headers, res = a:provider()(env)
  assert(status == 302)
  assert(headers["Location"] == "/done")
  local cookie = util.url_decode(headers["Set-Cookie"]:match("cookie=(.+)"))
  local user, message = a:authenticate(cookie)
  assert(user == "mascarenhas")
end

do
  -- successful persistent login with json data
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("json=" .. json.encode({ username = "mascarenhas",
						     password = "foobar",
						     success = "/done",
						     persistent = true,
						     failure = "/fail" }))
  local status, headers, res = a:provider()(env)
  assert(status == 302)
  assert(headers["Location"] == "/done")
  local cookie = util.url_decode(headers["Set-Cookie"]:match("mk_auth_user=(.-);"))
  local user, message = a:authenticate(cookie)
  assert(user == "mascarenhas")
end

do
  -- bad login with json data, wrong password
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("json=" .. json.encode({ username = "mascarenhas",
						     password = "foo",
						     success = "/done",
						     failure = "/fail" }))
  local status, headers, res = a:provider()(env)
  assert(status == 302)
  assert(headers["Location"] == "/fail?message=invalid+password")
  assert(headers["Set-Cookie"]:match("mk_auth_user=xxx"))
end

do
  -- bad login with json data, wrong password, json response
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("json=" .. json.encode({ username = "mascarenhas",
							  password = "foo" }))
  local status, headers, res = a:provider()(env)
  assert(status == 200)
  assert(headers["Set-Cookie"]:match("mk_auth_user=xxx"))
  local res = json.decode(res())
  assert(not res.user)
  assert(res.message == "invalid password")
end

do
  -- bad login with json data, unknown user
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("json=" .. json.encode({ username = "fabio",
						     password = "foo",
						     success = "/done",
						     failure = "/fail" }))
  local status, headers, res = a:provider()(env)
  assert(status == 302)
  assert(headers["Location"] == "/fail?message=user+not+found")
  assert(headers["Set-Cookie"]:match("mk_auth_user=xxx"))
end

do
  -- successful login with regular post data
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("username=mascarenhas&password=foobar&success=/done&failure=/fail")
  local status, headers, res = a:provider()(env)
  assert(status == 302)
  assert(headers["Location"] == "/done")
  local cookie = util.url_decode(headers["Set-Cookie"]:match("mk_auth_user=(.+)"))
  local user, message = a:authenticate(cookie)
  assert(user == "mascarenhas")
end

do
  -- successful persistent login with regular post data
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_post("username=mascarenhas&password=foobar&persistent=1&success=/done&failure=/fail")
  local status, headers, res = a:provider()(env)
  assert(status == 302)
  assert(headers["Location"] == "/done")
  local cookie = util.url_decode(headers["Set-Cookie"]:match("mk_auth_user=(.-);"))
  local user, message = a:authenticate(cookie)
  assert(user == "mascarenhas")
end

do
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_get()
  local user, message = a:login("mascarenhas", "foobar")
  env.HTTP_COOKIE = "mk_auth_user=" .. util.url_encode(message)
  local ok = pcall(a:filter(make_wsapi_app("mascarenhas")), env)
  assert(ok)
end

do
  -- unsuccessful authorization, no cookie
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt }
  local env = util.make_env_get()
  local ok = pcall(a:filter(make_wsapi_app("mascarenhas")), env)
  assert(not ok)
end

do
  -- unsuccessful authorization, expired cookie
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt, expiration = 0 }
  local user, message = a:login("mascarenhas", "foobar")
  local env = util.make_env_get()
  env.HTTP_COOKIE = "mk_auth_user=" .. util.url_encode(message)
  local ok = pcall(a:filter(make_wsapi_app("mascarenhas")), env)
  assert(not ok)
end

do
  -- unsuccessful authorization, forged cookie
  local a = auth.new{ login = login, login_salt = login_salt, 
		      session_salt = session_salt, expires = 0 }
  local user, message = a:login("mascarenhas", "foobar")
  local env = util.make_env_get()
  env.HTTP_COOKIE = "mk_auth_user=" .. util.url_encode(message:gsub("mascarenhas", "carregal"))
  local ok = pcall(a:filter(make_wsapi_app("carregal")), env)
  assert(not ok)
end
