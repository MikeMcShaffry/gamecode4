-- Rings application for WSAPI

require "rings"

module("wsapi.ringer", package.seeall)

local init = [==[
  local app_name, bootstrap_code, is_file = ...
  if bootstrap_code then
    local bootstrap, err
    if string.match(bootstrap_code, "%w%.lua$") then
      bootstrap, err = loadfile(bootstrap_code)
    else
      bootstrap, err = loadstring(bootstrap_code)
    end
    if bootstrap then
      bootstrap()
    else
      error("could not load " .. bootstrap_code .. ": " .. err)
    end
  else
    _, package.path = remotedostring("return package.path")
    _, package.cpath = remotedostring("return package.cpath")
  end
  local common = require"wsapi.common"
  require"coxpcall"
  pcall = copcall
  xpcall = coxpcall
  local wsapi_error = {
       write = function (self, err)
         remotedostring("env.error:write(...)", err)
       end
  }
  local wsapi_input =  {
       read = function (self, n)
         return coroutine.yield("RECEIVE", n)
       end
  }
  local wsapi_meta = {
       __index = function (tab, k)
        if k == "headers" then
          local headers
          local _, all_headers = remotedostring([[
                if env.headers then
                  local out = {}
                  for k, v in pairs(env.headers) do
                    table.insert(out,
                     "[" .. string.format("%q", k) .. "]=" ..
                     string.format("%q", v))
                  end
                  return "return {" .. table.concat(out, ",") .. "}"
                end
                  ]])
          if all_headers then
      local v = loadstring(all_headers)()
      rawset(tab, k, v)
      return v
          end
        else
          local  _, v = remotedostring("return env[(...)]", k)
          rawset(tab, k, v)
          return v
        end
     end,
       __newindex = function (tab, k, v)
           rawset(tab, k, v)
           remotedostring("local k, v = ...; env[k] = v", k, v)
        end
  }
  local app = common.normalize_app(app_name, is_file)
  main_func = function ()
     local wsapi_env = { error = wsapi_error, input = wsapi_input }
     setmetatable(wsapi_env, wsapi_meta)
     local ok, status, headers, res = common.run_app(app, wsapi_env)
     if not ok then
       local msg = status
       headers = { ["Content-Type"] = "text/html" }
       if wsapi_env.STATUS ~= "" then
         status = wsapi_env.STATUS
         res = coroutine.wrap(function () coroutine.yield(msg) end)
       else
         status = 500
         res = coroutine.wrap(function () coroutine.yield(common.error_html(msg)) end)
       end
     end
     remotedostring("status = ...", status)
     for k, v in pairs(headers) do
       if type(v) == "table" then
         remotedostring("headers[(...)] = {}", k)
         for _, val in ipairs(v) do
           remotedostring("local k, v = ...; table.insert(headers[k], v)", k, val)
         end
       else
         remotedostring("local k, v = ...; headers[k] = v", k, v)
       end
     end
     local s, v = res()
     while s do
       if s == "RECEIVE" and v then
          s, v = res(coroutine.yield(s, v))
       else
          coroutine.yield("SEND", s)
       end
       s, v = res()
     end
     return "SEND", nil
         end
]==]

-- Returns a WSAPI application that runs the provided WSAPI application
-- in an isolated Lua environment
function new(app_name, bootstrap, is_file)
  local data = { created_at = os.time() }
  setmetatable(data, { __index = _G })
  local state = rings.new(data)
  assert(state:dostring(init, app_name, bootstrap, is_file))
  local error = function (msg)
    data.status, data.headers, data.env = nil
    error(msg)
  end
  return function (wsapi_env)
    if state and wsapi_env == "close" then
      state:close()
      state = nil
    end
    if not state then
      return nil
    end
    if rawget(data, "status") then
      error("this state is already in use")
    end
    data.status = 500
    data.headers = {}
    data.env = wsapi_env
    local ok, flag, s =
     state:dostring([[
      main_coro = coroutine.wrap(main_func)
      return main_coro(...)
     ]])
    repeat
      if not ok then error(flag) end
      if flag == "RECEIVE" then
        ok, flag, s = state:dostring("return main_coro(...)",
          wsapi_env.input:read(s))
      elseif flag == "SEND" then
        break
      else
        error("Invalid command: " .. tostring(flag))
      end
    until flag == "SEND"
    local res
    if not s then
      res = function () return nil end
    else
      res = function ()
        if s then
          local res = s
          s = nil
          return res
        end
        local ok, flag, s = state:dostring("return main_coro()")
        while ok and flag and s do
          if flag == "RECEIVE" then
            ok, flag, s = state:dostring("return main_coro(...)",
                wsapi_env.input:read(s))
          elseif flag == "SEND" then
            return s
          else
            error("Invalid command: " .. tostring(flag))
          end
        end
        data.status, data.headers, data.env = nil
        if data.cleanup then
          state:close()
          state = nil
        end
        if not ok then error(flag) end
      end
    end
    return data.status, data.headers, res
  end, data
end