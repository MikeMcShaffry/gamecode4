
local schema = require "orbit.schema"
local forms = require "mk.forms"
local template = require "mk.template"
local json = require "json"

local plugin = { name = "nodes" }

local methods = {}

function methods:view_home(req, res)
  local home_id = self.config.home_page or "/index"
  local node = self.models.node:find_by_nice_id{ home_id }
  if node then
    return self:view_node(req, res, node)
  else
    return self.not_found(req, res)
  end
end

function methods:view_node(req, res, node, raw)
  local type = node.type
  local template
  if node.nice_id then
    template = self.theme:load("pages/" .. node.nice_id:sub(2):gsub("/", "-") .. ".html", self.engine)
  end
  template = template or self.theme:load("pages/" .. type .. ".html", self.engine)
  if template then
    if raw then
      res:write(template:render(req, res, { node = node }))
    else
      res:write(self:layout(req, res, template:render(req, res, { node = node })))
    end
  else
    return self.not_found(req, res)
  end
end

function methods:view_node_id(req, res, params)
  local id = tonumber(params.id)
  local node = self.models.node:find(id)
  if node then
    return self:view_node(req, res, node)
  else
    return self.not_found(req, res)
  end
end

function methods:view_node_id_raw(req, res, params)
  local id = tonumber(params.id)
  local node = self.models.node:find(id)
  if node then
    return self:view_node(req, res, node, true)
  else
    return self.not_found(req, res)
  end
end

function methods:view_nice(req, res, params)
  local nice_handle = "/" .. params.splat[1]
  local node = self.models.node:find_by_nice_id{ nice_handle }
  if node then
    return self:view_node(req, res, node)
  else
    return self.reparse
  end
end

function methods:view_node_type(req, res, params)
  local type, id = params.type, tonumber(params.id)
  if self.models.types[type] and id then
    local node = self.models.types[type]:find(id)
    if node then
      return self:view_node(req, res, node)
    end
  end
  return self.reparse
end

function methods:view_node_type_raw(req, res, params)
  local type, id = params.type, tonumber(params.id)
  if self.models.types[type] and id then
    local node = self.models.types[type]:find(id)
    if node then
      return self:view_node(req, res, node, true)
    end
  end
  return self.reparse
end

function methods:form_node_new(req, res, params)
  local type = params.type
  local node = self.models.types[type]:new()
  local template = self.admin_theme:load("pages/form-new-" .. type .. ".html", self.engine) or
                   self.admin_theme:load("pages/form-new-node.html", self.engine)
  if template then
    res:write(self:admin_layout(req, res, template:render(req, res, { node = node })))
  else
    return self.not_found(req, res)
  end
end

function methods:node_new(req, res, params)
  res:content_type("application/json")
  local raw_node = json.decode(req.POST.json)
  local node = self.models.types[params.type]:new()
  node:from_table(raw_node)
  node:save()
  res:write(string.format([[{ "to": "%s" }]], req:link_view_node_type(nil, { type = node.type, id = node.id })))
end

function methods:form_node_edit(req, res, params)
  local id = tonumber(params.id)
  local node = self.models.node:find(id)
  local type = params.type or node.type
  local template = self.admin_theme:load("pages/form-edit-" .. type .. ".html", self.engine) or
                   self.admin_theme:load("pages/form-edit-node.html", self.engine)
  if template then
    res:write(self:admin_layout(req, res, template:render(req, res, { node = node })))
  else
    return self.not_found(req, res)
  end
end

function methods:node_edit(req, res, params)
  res:content_type("application/json")
  local raw_node = json.decode(req.POST.json)
  local node = self.models.node:find(raw_node.id)
  node:from_table(raw_node)
  node:save()
  res:write(string.format([[{ "to": "%s" }]], req:link_view_node_type(nil, { type = node.type, id = node.id })))
end

function methods:view_terms_json(req, res, params)
  res:content_type("application/json")
  local vocabulary = params.name
  local terms = self.models.term:find_by_vocabulary(vocabulary)
  local list = {}
  for _, term in ipairs(terms) do
    list[#list+1] = { id = term.id, name = term.display_name }
  end
  res:write(json.encode{ list = list })
end

function methods:form_deps(widgets)
  for _, dep in ipairs{ "js", "css" } do
    for _, file in ipairs(forms.dependencies[dep]) do
      if not self[dep][file] then
        self[dep][file] = true
        self[dep][#self[dep]+1] = "ext/forms/" .. file
      end
    end
    for _, w in ipairs(widgets) do
      for _, file in ipairs(forms.controls[w.type][dep] or {}) do
        if not self[dep][file] then
          self[dep][file] = true
          self[dep][#self[dep]+1] = "ext/" .. w.type .. "/" .. file
        end
      end
    end
  end
end

local node_routes = {
    { pattern = '/', name = "view_home", method = "get" },
    { pattern = '/node/:id', name = "view_node_id", method = "get" },
    { pattern = '/node/:id/raw', name = "view_node_id_raw", method = "get" },
    { pattern = '/node/:id', name = "node_edit", method = "post" },
    { pattern = '/node/:id/edit', name = "form_node_edit", method = "get" },
    { pattern = '/terms/:name/json', name = "view_terms_json", method = "get" },
    { pattern = '/:type/:id', name = "view_node_type", method = "get" },
    { pattern = '/:type/:id', name = "node_edit_type",
      handler = methods.node_edit, method = "post" },
    { pattern = '/:type/:id/edit', name = "form_node_edit_type",
      handler = methods.form_node_edit, method = "get" },
    { pattern = '/:type/:id/raw', name = "view_node_type_raw", method = "get" },
    { pattern = '/:type/new', name = "form_node_new", method = "get" },
    { pattern = '/:type/new', name = "node_new", method = "post" },
    { pattern = '/*', name = "view_nice", method = "get" },
}

local blocks = {}

local show_latest_tmpl = [=[
  <div id = "$id">
    <h2>$title</h2>
    <ul>
    $nodes[[
        <li><a href = "$(req:link_view_node_id(nil, { id = self.id }))">$title</a></li>
    ]]
    </ul>
  </div>
]=]

local show_latest_body_tmpl = [=[
  <div id = "$id">
    <h2>$title</h2>
    $nodes[[
        <h3><a href = "$(req:link_view_node_id(nil, { id = self.id }))">$title</a></h3>
        $body
    ]]
  </div>
]=]

function blocks.show_latest(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(show_latest_tmpl)
  return function (req, res, env, name)
           local fields = { "id", "nice_id", "title", unpack(args.includes or {}) }
           local nodes = app.models[args.node or "node"]:find_latest{ count = args.count,
                                                                     fields = fields }
           return tmpl:render(req, res, { title = args.title, nodes = nodes, id = args.id or name })
         end
end

function blocks.show_latest_body(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(show_latest_body_tmpl)
  return function (req, res, env, name)
           local fields = { "id", "nice_id", "title", "body", unpack(args.includes or {}) }
           local nodes = app.models[args.node or "node"]:find_latest{ count = args.count,
                                                                     fields = fields }
           return tmpl:render(req, res, { title = args.title, nodes = nodes, id = args.id or name })
         end
end

local node_info_tmpl = [=[
    <div id = "$id">
      <h2>$(node.title)</h2>
      $(node.body)
    </div>
]=]

function blocks.node_info(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(node_info_tmpl)
  return function (req, res, env, name)
           return tmpl:render(req, res, { node = env.node, id = args.id or name })
         end
end

local node_form = [=[
    $subforms[[$it]]
    $form{ id = form_id, url = url, obj = node.raw }[[
      $flash{ class = "form_flash" }
      $(node.widgets)[[
        $(widgets[type](args))
      ]]
      $button{ id = "save", label = "Save", action = "post_redirect_result" }
      $button{ id = "reset", label = "Reset", action = "reset" }
    ]]
]=]

local form_new_node_tmpl = [=[
    <div id = "$id">
    <h3>New $(node.type)</h3>
    $form
    </div>
]=]

function blocks.form_new_node(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(form_new_node_tmpl)
  local form_tmpl = cosmo.compile(node_form)
  return function (req, res, env, name)
           local div_id = args.id or name
           local node = setmetatable({}, { __index = env.node })
           node.raw = node:to_table()
           node.widgets = app.forms[node.type](app.forms, req, res)
           app:form_deps(node.widgets)
           local subforms = {}
           for i, subform in ipairs(node.widgets.subforms or {}) do
             subforms[#subforms+1] = cosmo.fill(subform, { form = forms.form,
                                                           form_id = "subform_" .. div_id .. "_" .. i })
           end
           local form = function (args)
             args = args or {}
             return form_tmpl{ form = forms.form, subforms = subforms, form_id = args.id or "form_" .. div_id,
                               url = args.url or req:link_node_new(nil, { type = node.type }), node = node }
           end
           return tmpl:render(req, res, { node = node, id = div_id, form = form })
         end
end

local form_edit_node_tmpl = [=[
    <div id = "$id">
    <h3>Edit $(node.type)</h3>
    $form
    </div>
]=]

function blocks.form_edit_node(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(form_edit_node_tmpl)
  local form_tmpl = cosmo.compile(node_form)
  return function (req, res, env, name)
           local div_id = args.id or name
           local node = setmetatable({}, { __index = env.node })
           node.raw = node:to_table()
           node.widgets = app.forms[node.type](app.forms, req, res)
           app:form_deps(node.widgets)
           local subforms = {}
           for i, subform in ipairs(node.widgets.subforms or {}) do
             subforms[#subforms+1] = cosmo.fill(subform, { form = forms.form,
                                                           form_id = "subform_" .. div_id .. "_" .. i })
           end
           local form = function (args)
             args = args or {}
             return form_tmpl{ form = forms.form, subforms = subforms, form_id = args.id or "form_" .. div_id,
                               url = args.url or req:link_node_edit_type(nil, { type = node.type, id = node.id }), node = node }
           end
           return tmpl:render(req, res, { node = node, id = div_id, form = form })
         end
end

local node_widgets = function (self, req, res)
  return {
    { type = "text", args = { label = "Title", field = "title", flash = { class = "field_flash" } } },
    { type = "richtext", args = { label = "Body", field = "body" } },
    { type = "text", args = { label = "Nice URL", field = "nice_id" } },
    { type = "checkgroup", args = { label = "Visibility", field = "visibility",
                                    url = req:link_view_terms_json(nil, { name = "visibility" }) } }
  }
end

local post_widgets = function (self, req, res)
  local node_widgets = self:node(req, res)
  return {
    node_widgets[1],
    { type = "richtext", args = { label = "Teaser", field = "body" } },
    { type = "richtext", args = { label = "Full Body", field = "full_body" } },
    { type = "text", args = { label = "Byline", field = "byline" } },
    { type = "date", args = { label = "Published at", field = "published_at" } },
    node_widgets[3],
    node_widgets[4]
  }
end

local page_widgets = node_widgets

function plugin.new(app)
  schema.loadstring([[
    node = entity {
      fields = {
        id = key(),
        type = text(),
        nice_id = text(),
        title = text(),
        body = long_text(),
        created_at = timestamp(),
        terms = has_and_belongs{ "term" }
      }
    }
    page = entity {
      parent = "node", fields = {}
    }
    post = entity {
      parent = "node",
      fields = {
        byline = text(),
        full_body = long_text(),
        published_at = timestamp(),
      }
    }
    vocabulary = entity {
      fields = {
        id = key(),
        name = text(),
        display_name = text(),
        terms = has_many{ "term" }
      }
    }
    term = entity {
      fields = {
        id = key(),
        vocabulary = belongs_to{ "vocabulary" },
        parent = has_one{ "term" },
        name = text(),
        display_name = text(),
        nodes = has_and_belongs{ "node", order_by = "weight desc" }
      }
    }
    node_term = entity{
      fields = {
        id = key(),
        node = belongs_to{ "node" },
        term = belongs_to{ "term" },
      }
    }
  ]], "@node.lua", app.mapper.schema)
  for k, v in pairs(methods) do
    app[k] = v
  end
  app.models.node = app:model("node")

  function app.models.node:find_latest(args)
    args = args or {}
    count = args.count or 10
    local in_home = { "home", "visibility", entity = "node_term", fields = { "node" },
                      condition = [[node_term.term = term.id and term.vocabulary = vocabulary.id and
                                    term.name = ? and vocabulary.name = ?]], from = { "term", "vocabulary" } }
    return self:find_all("id in ?", { in_home, order = args.order or "created_at desc", count = count })
  end

  function app.models.node:get_visibility()
    if not self.visibility then
      local terms = app.models.term:find_all([[node_term.node = ? and node_term.term = term.id and
         term.vocabulary = vocabulary.id and vocabulary.name = ?]],
         { self.id, "visibility", from = { "node_term", "vocabulary" } })
      self.visibility = terms or {}
    end
    return self.visibility
  end

  function app.models.node:to_table()
    local tab = {}
    for name, field in pairs(self.__schema[self.__name].fields) do
      tab[name] = self[name]
    end
    tab.visibility = {}
    for i, term in ipairs(self:get_visibility()) do
      tab.visibility[i] = term.id
    end
    return tab
  end

  function app.models.node:from_table(tab)
    for name, field in pairs(self.__schema[self.__name].fields) do
      if field.column_name then
        self[name] = tab[name]
      else
        self[name] = {}
        for _, item in ipairs(tab[name]) do
          table.insert(self[name], { id = item })
        end
      end
    end
    for _, term in ipairs(tab.visibility) do
      table.insert(self.terms, { id = term })
    end
  end

  local node_save = app.models.node.save

  function app.models.node:save()
    node_save(self)
    for name, field in pairs(self.__schema[self.__name].fields) do
      if field.join_table then
        local rel_name
        if self.__schema[field.join_table].fields.node then
          rel_name = "node"
        else
          rel_name = self.__name
        end
        app.models[field.join_table]:delete_all(rel_name .. " = ?", { self.id })
        for _, item in ipairs(self[name] or {}) do
          local new = app.models[field.join_table]:new()
          new[rel_name] = self.id
          new[field.entity] = item.id
          new:save()
        end
      elseif field.entity then
        local rel_name
        if self.__schema[field.entity].fields.node then
          rel_name = "node"
        else
          rel_name = self.__name
        end
        app.models[field.entity]:delete_all(rel_name .. " = ?", { self.id })
        for _, item in ipairs(self[name] or {}) do
          local new = app.models[field.entity]:new(item)
          new[rel_name] = self.id
          new:save()
        end
      end
    end
  end

  app.models.post = app:model("post")

  function app.models.post:find_latest(args)
    args = args or {}
    count = args.count or 10
    return app.models.node.find_latest(self, { order = "published_at desc", count = count })
  end

  app.models.page = app:model("page")

  app.models.term = app:model("term")

  function app.models.term:find_by_vocabulary(name)
    return self:find_all("vocabulary = vocabulary.id and vocabulary.name = ?",
                         { name, from = { "vocabulary" }, order = "display_name asc" })
  end

  app.models.vocabulary = app:model("vocabulary")
  app.models.node_term = app:model("node_term")

  app.models.types.post = app.models.post
  app.models.types.page = app.models.page

  for name, block in pairs(blocks) do
    app.blocks.protos[name] = block
  end

  for _, route in ipairs(node_routes) do
    app.routes[#app.routes+1] = route
  end

  app.forms.node = node_widgets
  app.forms.post = post_widgets
  app.forms.page = page_widgets
end

return plugin
