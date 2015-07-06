
local crypto = require "crypto"
local json = require "json"
local request = require "wsapi.request"
local response = require "wsapi.response"
local util = require "wsapi.util"

local auth = {}

local methods = {}
methods.__index = methods

function auth.new(params)
  return setmetatable({ login_function = params.login,
                        login_salt = params.login_salt, 
                        session_salt = params.session_salt,
                        expiration = params.expiration or 3600,
                        cookie_name = params.cookie or "mk_auth_user"}, methods)
end

function methods:token(user, expiration)
  local expiration = expiration or (os.time() + self.expiration)
  local message = "exp=" .. expiration .. "&data=" .. json.encode(user)
  message = message .. "&digest=" .. crypto.hmac.digest("sha1", message, self.session_salt)
  return message
end

function methods:login(username, password, expiration)
  local expiration = expiration or (os.time() + self.expiration)
  local salted_password = crypto.hmac.digest("sha1", password, self.login_salt)
  local user, message = self.login_function(username, salted_password)
  if user then
    return user, self:token(user, expiration)
  end
  return user, message
end

function methods:salt_password(password)
  return crypto.hmac.digest("sha1", password, self.login_salt)
end

function methods:logoff(headers)
  local res = response.new(nil, headers)
  res:delete_cookie(self.cookie_name)
end

function methods:authenticate(message)
  local message, digest = message:match("^(.-)&digest=(.*)$")
  if message and digest == crypto.hmac.digest("sha1", message, self.session_salt) then
    local exp, data = message:match("^exp=(.-)&data=(.+)$")
    local expiration, user = tonumber(exp), json.decode(data)
    if os.time() < expiration then
      return user
    else
      return nil, "login expired"
    end
  end
  return nil, "invalid login"
end

function methods:filter(wsapi_app)
  return function (wsapi_env, ...)
           local message = (";" .. (wsapi_env.HTTP_COOKIE or "")
                          .. ";"):match(";%s*" .. self.cookie_name .. "=(.-)%s*;")
           if message then
             message = util.url_decode(message) 
             wsapi_env.MK_AUTH_USER, wsapi_env.MK_AUTH_ERROR = self:authenticate(message)
           end
           return wsapi_app(wsapi_env, ...)
         end
end

function methods:provider()
  return function (wsapi_env)
           local req = request.new(wsapi_env)
           local res = response.new()
           local data = req.POST.json and json.decode(req.POST.json)
           if not data then
             data = { username = req.POST.username, 
                      password = req.POST.password,
                      persistent = req.POST.persistent,
                      success = req.POST.success, failure = req.POST.failure }
           end
           local expires = (data.persistent and (os.time() + self.expiration)) or nil
           local user, message = self:login(data.username, data.password)
	   local redirect_or_json = function (url)
				      if url then
					return res:redirect(url)
				      else
					res:content_type("application/json")
					res:write(json.encode{ user = user, 
							       message = message,
							       expires = expires })
					return res:finish()
				      end
				    end
           if user then
             res:set_cookie(self.cookie_name, { value = message, expires = expires })
	     return redirect_or_json(data.success)
           else
             res:delete_cookie(self.cookie_name)
	     return redirect_or_json(data.failure and data.failure .. "?message=" .. util.url_encode(message))
           end
         end
end

return auth
