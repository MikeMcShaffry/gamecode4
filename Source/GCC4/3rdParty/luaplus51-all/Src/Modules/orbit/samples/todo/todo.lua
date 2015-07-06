#!/usr/bin/env wsapi.cgi

local orbit = require "orbit"
local schema = require "orbit.schema"

local todo = orbit.new()

todo.config = {
  database = {
    driver = "sqlite3",
    connection = { "todo.db" }
  }
}

todo.mapper = {
  default = true,
  logging = true,
  schema = schema.loadstring([[
    todo_list = entity {
      fields = {
        id = key(),
        title = text(),
        done = boolean(),
        created_at = timestamp()
      }
    }
  ]], "@todo_schema.lua")
}

todo.list = todo:model("todo_list")

function todo.list:find_items()
  return self:find_all{ order = "created_at desc" }
end

function todo:index(req, res)
  local items = self.theme:load("items.html"):render(req, res, { items = self.list:find_items() })
  res:write(self.theme:load("index.html"):render(req, res, { items = items }))
end

function todo:add(req, res)
  local item = todo.list:new()
  item.title = req.POST.item or ""
  item:save()
  res:write(self.theme:load("items.html"):render(req, res, { items = self.list:find_items() }))
end

function todo:remove(req, res)
  local item = todo.list:find(tonumber(req.POST.id))
  item:delete()
  res:write(self.theme:load("items.html"):render(req, res, { items = self.list:find_items() }))
end

function todo:toggle(req, res)
  local item = todo.list:find(tonumber(req.POST.id))
  item.done = not item.done
  item:save()
  res:write("toggle")
end

todo.routes = {
  { pattern = "/", name = "index", method = "get" },
  { pattern = "/add", name = "add", method = "post" },
  { pattern = "/remove", name = "remove", method = "post" },
  { pattern = "/toggle", name = "toggle", method = "post" },
  { pattern = "/*.js", name = "javascript" }
}

todo.config = {
  theme = {
    name = "default", config = {}
  }
}

todo.templates = {
  ["/default/index.html"] = [=[
  <html>
  <head>
  <title>To-do List</title>
    <script type="text/javascript" src="$(req:link_javascript({}, { splat = { 'jquery-1.2.3.min' } }))"></script>
  <script>
  function set_callbacks() {
    $$(".remove").click(function () {
      $$("#items>[item_id=" + $$(this).attr("item_id") +"]").slideUp("slow");
      $$("#items").load("$(req:link_remove())", { 'id': $$(this).attr("item_id") },
        function () { set_callbacks(); });
    });
    $$(".item").click(function () {
      $$.post("$(req:link_toggle())", { 'id': $$(this).attr("item_id") });
    });
  }

  $$(document).ready(function () {
    $$("#add").submit(function () {
      $$("#button").attr("disabled", true);
      $$("#items").load("$(req:link_add())", { item: $$("#title").val()  },
        function () { $$("#title").val(""); set_callbacks();
        $$("#button").attr("disabled",false); });
      return false;
    });
    set_callbacks();
  });
  </script>
  <style>
  ul { padding-left: 0px; }
  li { list-style-type: none;}
  .remove { font-size: xx-small; }
  </style>
  </head>
  <body>
  <h1>To-do</h1>
  <ul id="items">
  $items
  </ul>
  <form id = "add" method = "POST" action = "$(req:link_add())">
  <input id = "title" type = "text" name = "item" size = 30 />
  <input id = "button" type = "submit" value = "Add" />
  </form>
  </body>
  </html>
  ]=],
  ["/default/items.html"] = [=[
    $if{ items[1] }[[
      $items[[
       <li item_id="$id"><input class="item" type="checkbox" $if{done}[[checked]] item_id="$id"/> $title
       <a href = "#" class = "remove" item_id = "$id">Remove</a></li>
      ]]
    ]][[[Nothing to do!]]
  ]=],
}

return todo:load()
