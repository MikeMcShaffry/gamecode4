
local template = require "mk.template"
local forms = require "mk.forms"
local json = require "json"
local schema = require "orbit.schema"

local plugin = { name = "poll" }

local poll_form = [=[
  $form{ id = form_id, url = url, obj = {} }[[
    $radio{ field = "option", wrap_ul = wrap_ul, ul_class = ul_class, li_class = li_class, list = options }
    $button{ id = "vote", label = "Vote", action = "post_redirect_inline", to = result,
              container = div_id }
  ]]
]=]

local poll_tmpl = [=[
  <div id = "$div_id">
    <h2>$title</h2>
    $body
    $form
  </div>
]=]

local function block_poll(app, args, tmpl)
  args = args or {}
  tmpl = tmpl or template.compile(poll_tmpl)
  local form_tmpl = cosmo.compile(poll_form)
  app:form_deps({ { type = "radio" } })
  return function (req, res, env, name)
           local div_id = args.id or name
	   local poll = app.models.poll:find_latest()
	   local options = {}
	   local _ = poll.options[1]
	   local form = function (args)
	     args = args or {}
             return form_tmpl{ form = forms.form, form_id = "form_" .. div_id, div_id = div_id,
			       wrap_ul = args.wrap_ul, result = req:link_view_node_type_raw(nil, { type = "poll", id = poll.id }),
			       url = req:link_post_vote(nil, { id = poll.id }), options = poll.options,
			       ul_class = args.ul_class, li_class = args.li_class }
	   end	
	   local env = setmetatable({ div_id = args.id or name, form = form }, { __index = poll })
	   return tmpl:render(req, res, env)
	 end
end

local poll_total_tmpl = [=[
  <h2>$title</h2>
  $body
  <ul>
      $options[[
      <li>$name: $votes ($format{ "%.1f", (votes/total) * 100}%)</li>
      ]]
  </ul>
]=]

local function block_poll_total(app, args, tmpl)
  tmpl = tmpl or template.compile(poll_total_tmpl)
  return function (req, res, env, name)
	   local poll = env.node
	   local _ = poll.options[1]
	   return tmpl:render(req, res, poll)
	 end
end

local function post_vote(app, req, res, params)
  res:content_type("application/json")
  local id = tonumber(params.id)
  local poll = app.models.poll:find(id)
  if poll then
    local obj = json.decode(req.POST.json)
    local ok, err = poll:vote(obj.option)
      if ok then
        res:write(json.encode{})
      else
        res:write(json.encode{ message = err })
      end
  else
    res:write(json.encode{ message = "Poll not found" })
  end
end

local poll_widgets = function (self, req, res) 
  local node_widgets = self:node(req, res)
  return {
    node_widgets[1],
    node_widgets[2],
    { type = "checkbox", args = { label = "Closed", field = "closed" } },
    { type = "detail", args = { label = "Options", field = "options", form = "#poll_options" } },
    subforms = {
     [=[
       <div id = "$form_id" style = "display: none">
         $form{ id = "poll_options", url = "#poll_options", hidden = true }[[
           $text{ label = "Option", field = "name" }
           $text{ label = "Weight", field = "weight", size = 10 }
           $button{ id = "delete", label = "Remove", action = "delete_self" }
         ]]
       </div>
     ]=]	
    }
  }
end

function plugin.new(app)
  schema.loadstring([[
    poll = entity {
      parent = "node",
      fields = {
        total = integer(),
	closed = boolean(),
	options = has_many{ "poll_option", order_by = "weight desc" }
      }
    }
    poll_option = entity {
      fields = {
	id = key(),
	name = text(),
	weight = integer(),
	votes = integer(),
	poll = belongs_to{ "poll" }
      }
    }
  ]], "@poll.lua", app.mapper.schema)

  app.models.poll = app:model("poll")
  app.models.poll_option = app:model("poll_option")

  function app.models.poll:find_latest()
    return self:find_first("closed is null or closed != ?", 
	                   { true, order = "created_at desc", count = 1 })
  end

  function app.models.poll:to_table()
    local tab = app.models.node.to_table(self)
    local _ = tab.options[1]
    return tab
  end

  function app.models.poll:from_table(tab)
    app.models.post.from_table(self, tab)
    local total = 0
    for i, option in ipairs(tab.options) do
      for name, value in pairs(option) do
        self.options[i][name] = value
      end
      if not self.options[i].votes then
        self.options[i].votes = 0
      end
      total = total + self.options[i].votes
    end
    self.total = total
  end

  function app.models.poll:vote(option_id)
    local _ = self.options[1]
    for _, option in ipairs(self.options) do
      if option.id == option_id then
        self.total = self.total + 1
        self:save()
        option.votes = option.votes + 1
        option:save()
        break
      end
    end
  end

  local poll_new = app.models.poll.new
  
  function app.models.poll:new(tab)
    local poll = poll_new(self, tab)
    poll.options = {}
    return poll
  end

  app.blocks.protos.latest_poll = block_poll
  app.blocks.protos.poll_total = block_poll_total
  app.post_vote = post_vote
  table.insert(app.routes, { pattern = '/poll/:id/vote', name = "post_vote", method = "post" })
  app.models.types.poll = app.models.poll
  app.forms.poll = poll_widgets
end

return plugin
