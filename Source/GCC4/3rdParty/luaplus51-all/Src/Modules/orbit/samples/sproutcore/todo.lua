#!/usr/bin/env wsapi.cgi

require "orbit"
require "orbit.cache"
require "luasql.sqlite3"

local schema = require "orbit.schema"
local encode = require "json.encode"
local decode = require "json.decode"

local todo = orbit.new()
todo.mapper.conn = luasql.sqlite3():connect(todo.real_path .. "/todo.db")

todo.mapper.schema = schema.loadstring([[
  todo_list = entity {
    fields = {
       id = key(),
       description = text(),
       is_done = boolean(),
       created_at = timestamp()
    }
  }
]], "@todo_schema.lua")

local todo_list = todo:model("todo_list")
local cache = orbit.cache.new(todo)

function todo_list:url()
  return "/tasks/" .. self.id
end

function todo_list:to_json()
  return { guid = self:url(), description = self.description, isDone = self.is_done, order = self.id }
end

function todo.index(web)
  return todo:serve_static(web, todo.real_path .. "/static/todos/en/ee972277c11ed2d7cf0765e9c5b9738575b9248d/index.html")
end

todo:dispatch_get(todo.index, "/")

function todo.get_tasks(web)
  web:content_type("application/json")
  local tasks, json = todo_list:find_all{ order = "id asc" }, {}
  for i, task in ipairs(tasks) do json[i] = task:to_json() end
  return encode{ content = json }
end

todo:dispatch_get(cache(todo.get_tasks), "/tasks")

function todo.add_task(web)
  local opt = decode(web.input.post_data)
  local task = todo_list:new(opt)
  task:save()
  web.headers["Location"] = task:url()
  web.status = "201 Created"
  web:content_type("application/json")
  cache:invalidate("/tasks")
  return encode{ content = task:to_json() }  
end

todo:dispatch_post(todo.add_task, "/tasks")

function todo.get_task(web, id)
  local task = todo_list:find(tonumber(id))
  if not task then return todo.not_found(web) end
  web:content_type("application/json")
  return encode{ content = task:to_json() }  
end

todo:dispatch_get(cache(todo.get_task), "/tasks/(%d+)")

function todo.put_task(web, id)
  local task = todo_list:find(tonumber(id))
  if not task then return todo.not_found(web) end
  local opt = decode(web.input.post_data)
  task.description = opt.description
  task.is_done = opt.isDone
  task:save()
  cache:invalidate(task:url())
  cache:invalidate("/tasks")
  web:content_type("application/json")
  return encode{ content = task:to_json() }  
end

todo:dispatch_put(todo.put_task, "/tasks/(%d+)")

function todo.delete_task(web, id)
  local task = todo_list:find(tonumber(id))
  if not task then return todo.not_found(web) end
  task:delete()
  cache:invalidate("/tasks")
end

todo:dispatch_delete(todo.delete_task, "/tasks/(%d+)")

todo:dispatch_static("/static/.*")

return todo
