
local cosmo = require "cosmo"
local json = require "json"

local _M = {}

local form_preamble = cosmo.compile[=[
  <form $if{ id }[[ id = "$id" ]] action = "$url" method = "post">
]=]

local text_control = [=[$if{label}[[<p>$label<br/>]]
    <input name = "$name" type = "text" size = "$size" $attrs[[ $name = "$value" ]]/>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local textarea_control = [=[$if{label}[[<p>$label<br/>]]
    <textarea name = "$name" cols = "$width" rows = "$height" $attrs[[ $name = "$value" ]]></textarea>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local checkbox_control = [=[$if{label}[[<p>$label ]]
    <input name = "$name" type = "checkbox" value = "1" $attrs[[ $name = "$value" ]]/>
    $if{ flash }[[<span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local richtext_control = [=[$if{label}[[<p>$label<br/>]]
    <textarea name = "$name" cols = "$width" rows = "$height" $attrs[[ $name = "$value" ]]></textarea>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]]
]=]

local combobox_control = [=[$if{label}[[<p>$label<br/>]]
    <select name = "$name" $if{size}[[size = "$size"]] $attrs[[ $name = "$value" ]]></select>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local radiogroup_control = [=[$if{label}[[<p>$label<br/>]]
    <div name = "$name" $attrs[[ $name = "$value" ]]></div>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local detailbox_control = [=[$if{label}[[<p>$label<br/>]]
    <div name = "$name" $attrs[[ $name = "$value" ]]>
      <div name = "detail"></div><br/>
      <button name = "add">Add New</button>
    </div>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local checkgroup_control = [=[$if{label}[[<p>$label<br/>]]
    <div name = "$name" $attrs[[ $name = "$value" ]]></div>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local multibox_control = [=[$if{label}[[<p>$label<br/>]]
    <select multiple = "true" name = "$name" $if{size}[[size = "$size"]] $attrs[[ $name = "$value" ]]></select>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]] ]=]

local tagbox_control = [=[$if{label}[[<p>$label<br/>]]
    <div name = "$name">
      <input name = "$(name)_text" type = "text" size = "$size" $attrs[[ $name = "$value" ]]/> 
      <button name = "$(name)_button" $attrs[[ $name = "$value" ]]>Add</button><br/>
      <ul $attrs[[ $name = "$value" ]]></ul>
    </div>
    $if{ flash }[[<br/><span name = "$(name)_flash"  $flash[[ $name = "$value" ]]></span>]]$if{label}[[</p>]]
]=]

local button_control = cosmo.compile[=[<button name = "$name" value = "1" $attrs[[ $name = "$value" ]]>$label</button>]=]

local form_postamble = cosmo.compile[=[
  </form>
  <script type = "text/javascript">
    $$(document).ready(function() {
       var fields = { $concat{ fields }[['$name': new $type($json{ attrs })]]
      };
       var buttons = { $concat{ buttons }[['$name': new $type($json{ attrs })]]
      };
      var form = new ajaxforms.Form($if{ id }[[$$('#$id')]][[$dom]], '$url', fields, buttons);
      $if{ not hidden }[[ form.load($if{obj}[[$json{obj}]]); ]]
    });
  </script>
]=]

local form_flash = [=[<div name = "flash" $attrs[[ $name = "$value" ]]></div>]=]

local field_flash = [=[<span name = "$(name)_flash"  $attrs[[ $name = "$value" ]]></span>]=]

local function tab2list(tab)
  if tab then
    local list = {}
    for k, v in pairs(tab) do list[#list+1] = { name = k, value = v } end
    return list
  end
end

local function make_field(args, type)
  local field = { name = args.field, type = type, attrs = {} }
  for k, v in pairs(args) do 
    if not field[k] then
      field.attrs[k] = v
    end
  end
  return field
end

local function make_button(args, type)
  local field = { name = args.id, type = type, attrs = { name = args.id } }
  for k, v in pairs(args) do 
    if not field[k] then
      field.attrs[k] = v
    end
  end
  return field
end

local function make_control(template, args, defaults)
  local control = { name = args.field, flash = tab2list(args.flash), 
		    concat = cosmo.concat, ["if"] = cosmo.cif, attrs = {} }
  if args.attrs then
    for k, v in pairs(args.attrs) do
      control.attrs[#control.attrs+1] = { name = k, value = v }
    end
    args.attrs = nil
  end
  for k, v in pairs(args) do
    if not control[k] then
      control[k] = v
    end
  end
  for k, v in pairs(defaults) do
    if not control[k] then
      control[k] = v
    end
  end
  if type(template) == "string" then
    return cosmo.fill(template, control)
  else
    return template(control)
  end
end

_M.actions = {
  post = "ajaxforms.SaveButton",
  post_redirect = "ajaxforms.SaveRedirectButton",
  link = "ajaxforms.RedirectButton",
  post_redirect_inline = "ajaxforms.SaveRedirectInlineButton",
  post_redirect_result = "ajaxforms.SaveRedirectResultButton",
  reset = "ajaxforms.ResetButton",
  delete_self = "ajaxforms.DetailDeleteButton"
}

_M.controls = {
  text = { type = "ajaxforms.TextBox", template = text_control, defaults = { size = 100 } },
  textarea = { type = "ajaxforms.TextBox", template = textarea_control, defaults = { width = 100, height = 10 } },
  richtext = { type = "ajaxforms.RichTextBox", template = richtext_control, defaults = { width = 100, height = 10 },
	       js = { "jquery.wysiwyg.js" }, css = { "jquery.wysiwyg.css" } },
  checkbox = { type = "ajaxforms.CheckBox", template = checkbox_control, defaults = {} },
  combobox = { type = "ajaxforms.ComboBox", template = combobox_control, defaults = {} },
  radio = { type = "ajaxforms.RadioGroup", template = radiogroup_control, defaults = {} },
  detail = { type = "ajaxforms.DetailBox", template = detailbox_control, defaults = {} },
  checkgroup = { type = "ajaxforms.CheckGroup", template = checkgroup_control, defaults = {} },
  multi = { type = "ajaxforms.ListBox", template = multibox_control, defaults = { size = 5 } },
  tag = { type = "ajaxforms.TagBox", template = tagbox_control, defaults = { size = 50 },
	  js = { "jquery.autocomplete.min.js" }, css = { "jquery.autocomplete.css" } },
  date = { type = "ajaxforms.DatePicker", template = text_control, defaults = { size = 10 },
	   js = { "jquery-ui-1.7.2.custom.min.js" }, css = { "smoothness/jquery-ui-1.7.2.custom.css"} },
}

_M.dependencies = {
  js = { "form_support.js", "jquery-1.4.2.min.js", "jquery.json-2.2.min.js" },
  css = {}
}

function _M.form(args)
  local id, url = args.id, args.url
  cosmo.yield(form_preamble{ id = id, url = url, ["if"] = cosmo.cif }, true)
  local fields, buttons = {}, {}
  env = {
    flash = function(args)
	      local attrs = {}
	      for k, v in pairs(args) do attrs[#attrs+1] = { name = k, value = v } end
	      return cosmo.fill(form_flash, { attrs = attrs })
	    end,
    flash_for = function(args)
		  local attrs = {}
		  local field = args.field
		  args.field = nil
		  for k, v in pairs(args) do attrs[#attrs+1] = { name = k, value = v } end
		  return cosmo.fill(field_flash, { name = field, attrs = attrs })
		end,
    button = function (args)
	       local attrs = {}
	       if args.attrs then
		 for k, v in pairs(args.attrs) do
		   attrs[#attrs+1] = { name = k, value = v }
		 end
		 args.attrs = nil
	       end
	       buttons[#buttons+1] = make_button(args, _M.actions[args.action]) 
	       return button_control{ name = args.id, label = args.label, disabled = args.disabled,
				      attrs = attrs }
	     end
  }
  for name, control in pairs(_M.controls) do
    env[name] = function (args)
		  fields[#fields+1] = make_field(args, control.type)
		  return make_control(control.template, args, control.defaults)
		end
  end
  env.widgets = env
  cosmo.yield(env)
  cosmo.yield(form_postamble{ id = id, url = url, concat = cosmo.concat, buttons = buttons, 
			      fields = fields, ["if"] = cosmo.cif, obj = args.obj, hidden = args.hidden,
			      json = function (args) return json.encode(args[1]) end }, true)
end

function _M.dependency(dep)
  return function (args)
	   local control = (args and args[1]) or "forms"
	   if control == "all" or control == "forms" then
	     for _, file in ipairs(_M.dependencies[dep]) do
	       cosmo.yield{ control = "forms", [dep] = file }
	     end
	     if control == "all" then
	       for name, control in pairs(_M.controls) do
		 if control[dep] then
		   for _, file in ipairs(control[dep]) do
		     cosmo.yield{ control = name, [dep] = file }
		   end
		 end
	       end
	     end
	   else
	     local files = _M.controls[control][dep] or {}
	     for _, file in ipairs(files) do
	       cosmo.yield{ control = control, [dep] = file }
	     end
	   end
	 end
end

_M.javascript = _M.dependency("js")
_M.css = _M.dependency("css")

return _M
