
ajaxforms = { forms: {} };

// Array Remove - By John Resig (MIT Licensed)
Array.prototype.remove = function(from, to) {
  var rest = this.slice((to || from) + 1 || this.length);
  this.length = from < 0 ? this.length + from : from;
  return this.push.apply(this, rest);
};

// Inspired by base2 and Prototype
(function(){
  var initializing = false, fnTest = /xyz/.test(function(){xyz;}) ? /\b_super\b/ : /.*/;
  // The base Class implementation (does nothing)
  ajaxforms.Class = function(){};
  
  // Create a new Class that inherits from this class
  ajaxforms.Class.extend = function(prop) {
    var _super = this.prototype;
    
    // Instantiate a base class (but only create the instance,
    // don't run the init constructor)
    initializing = true;
    var prototype = new this();
    initializing = false;
    
    // Copy the properties over onto the new prototype
    for (var name in prop) {
      // Check if we're overwriting an existing function
      prototype[name] = typeof prop[name] == "function" && 
        typeof _super[name] == "function" && fnTest.test(prop[name]) ?
        (function(name, fn){
          return function() {
            var tmp = this._super;
            
            // Add a new ._super() method that is the same method
            // but on the super-class
            this._super = _super[name];
            
            // The method only need to be bound temporarily, so we
            // remove it when we're done executing
            var ret = fn.apply(this, arguments);        
            this._super = tmp;
            
            return ret;
          };
        })(name, prop[name]) :
        prop[name];
    }
    
    // The dummy class constructor
    function Class() {
      // All construction is actually done in the init method
      this.__class__ = Class;
      if ( !initializing && this.init )
        this.init.apply(this, arguments);
    }
    
    // Populate our constructed prototype object
    Class.prototype = prototype;
    
    // Enforce the constructor to be what we expect
    Class.constructor = Class;

    // And make this class extendable
    Class.extend = arguments.callee;
    
    return Class;
  };
})();

ajaxforms.Form = ajaxforms.Class.extend({
  init: function (dom, url, fields, buttons) {
    this.dom = dom;
    this.flash = $("[name=flash]", this.dom);
    this.fields = fields;
    this.buttons = buttons;
    this.url = url;
    ajaxforms.forms[url] = this;
  },
  clone: function (url) {
    var dom = this.dom.clone().attr("id", null);
    var fields = {};
    for(var name in this.fields)
      if(this.fields.hasOwnProperty(name))
        fields[name] = this.fields[name].clone();
    var buttons = {};
    for(var name in this.buttons)
      if(this.buttons.hasOwnProperty(name))
        buttons[name] = this.buttons[name].clone();
    return new ajaxforms.Form(dom, url, fields, buttons);	
  },
  load: function (obj) {
    var form = this;
    if(!obj) {
      $.getJSON(this.url, {}, function (obj) { form.load(obj); });
    } else {
      this.obj = obj;
      for(name in this.fields)
        if(this.fields.hasOwnProperty(name)) {
	  this.fields[name].form = this;
	  this.fields[name].draw();
        }
      for(name in this.buttons)
        if(this.buttons.hasOwnProperty(name)) {
	  this.buttons[name].form = this;
	  this.buttons[name].draw();
        }
      for(var name in this.fields) {
        if(this.fields.hasOwnProperty(name)) {
	  var field = this.fields[name];
	  field.set(this.obj[name]);
	}
      }
    }
  },
  hideFlash: function () {
    this.flash.slideUp("fast");
    for(var name in this.fields) {
      if(this.fields.hasOwnProperty(name)) {
	var field = this.fields[name];
	field.hideFlash();
      }
    }
  },
  getData: function () {
    var obj = $.extend({}, this.obj);
    for(var name in this.fields) {
      if(this.fields.hasOwnProperty(name)) {
	obj[name] = this.fields[name].get();
      }
    }
    return obj;
  },
  reset: function () {
    for(var name in this.fields) {
      if(this.fields.hasOwnProperty(name)) {
        var field = this.fields[name];
        field.set(this.obj[name]);
      }
    }
  },
  showFlash: function (msg, field_msgs) {
    if(msg) {
      this.flash.text(msg);
      this.flash.slideDown("fast");
    }
    if(field_msgs) {
      for(var name in this.fields) {
	if(this.fields.hasOwnProperty(name)) {
	  if(field_msgs[name])
	    this.fields[name].showFlash(field_msgs[name]);
	}
      }
    }
  }
});

ajaxforms.Field = ajaxforms.Class.extend({
  init: function (attrs) {
    this.attrs = attrs;
    for(name in this.attrs)
      if(this.attrs.hasOwnProperty(name)) {
	this[name] = this.attrs[name];
      }
  },
  clone: function () {
    return new this.__class__(this.attrs);
  },
  enable: function () {
    this.disabled = false;
    this.dom.attr("disabled", false);
  },
  disable: function () {
    this.disabled = true;
    this.dom.attr("disabled", true);
  },
  hideFlash: function () {
    this.flash.hide();
  },
  showFlash: function (msg) {
    this.flash.text(msg);
    this.flash.slideDown("fast");
  },
  draw: function () {
    this.dom = $("[name='" + this.attrs.field + "']", this.form.dom);
    this.flash = $("[name='"+ this.attrs.field + "_flash']", this.form.dom);
    for(name in this.attrs)
      if(this.attrs.hasOwnProperty(name)) {
        if(name == "class")
          this.dom.addClass(this.attrs[name]);
        else if(name == "field")
          this.dom.attr("name", this.attrs[name]);
        else
          this.dom.attr(name, this.attrs[name]);
    }
    if(this.disabled) this.disable();
  }
});

ajaxforms.DetailBox = ajaxforms.Field.extend({
  init: function (attrs) {
    this.subform = ajaxforms.forms[attrs.form];
    delete attrs['form'];
    this.empty = attrs.empty;
    delete attrs['empty'];
    this._super(attrs);
    this.formlist = [];
  },
  draw: function () {
    this._super();
    this.add = $("[name='add']", this.dom);
    this.detail = $("[name='detail']", this.dom);
    var field = this;
    this.add.click(function () {
      var form = field.subform.clone();
      form.parent = field;
      field.detail.append(form.dom);
      form.load(field.empty || {});
      field.formlist.push(form);
      return false;
    });
  },
  get: function () {
    var data = [];
    for(var i = 0; i < this.formlist.length; i++)
      data.push(this.formlist[i].getData());
    return data;
  },
  set: function (values) {
    for(var i = 0; i < values.length; i++) {
      var form = this.subform.clone();
      form.parent = this;
      this.detail.append(form.dom);
      form.load(values[i]);
      this.formlist.push(form);
    }
  },
  remove: function (subform) {
    for(var i = 0; i < this.formlist.length; i++)
      if(this.formlist[i] == subform) {
	this.formlist.remove(i);
	subform.dom.remove();
        break;	
    }
  }
});

ajaxforms.TextBox = ajaxforms.Field.extend({
  draw: function() {
    this._super();
    this.dom.attr("size", this.size || 100);
  },
  get: function () {
    switch(this.type) {
      case "number":
        return this.dom.val() * 1;
      default:
        var val = this.dom.val();
        return val == '' ? undefined : val;
    }
  },
  set: function (value) {
    this.type = typeof(value);
    this.dom.val(value != undefined ? value : '');
  }
});

ajaxforms.RichTextBox = ajaxforms.Field.extend({
  draw: function() {
    this._super();
    this.dom.attr("cols", this.width || 100);
    this.dom.attr("rows", this.height || 5);
  },
  get: function () {
    return this.dom.val();
  },
  set: function (value) {
    this.type = "string";
    //this.wysiwyg.contents().find('body').html(value || '');
    this.dom.val(value || '');
    this.dom.wysiwyg(); 
  }
});

ajaxforms.DatePicker = ajaxforms.Field.extend({
  draw: function () {
    this._super();
    this.dom.datepicker(); 
  },
  get: function () {
    return this.dom.datepicker('getDate');
  },
  set: function (value) {
    this.type = "date";
    if(typeof(value) == "string") value = new Date(value);
    return this.dom.datepicker('setDate', value);
  }
});

ajaxforms.CheckBox = ajaxforms.Field.extend({
  get: function () {
    if(this.dom.attr('checked'))
      return true;
    else
      return false;
  },
  set: function (value) {
    this.type = "boolean";
    this.dom.attr('checked', value);
  }
});

ajaxforms.ComboBox = ajaxforms.Field.extend({
  get: function () {
    return this.dom.val() * 1;
  },
  set: function (value) {
    var field = this;
    if(!this.list) {
      $.getJSON(this.url, {}, function (obj) { field.list = obj.list; field.set(value); });
    } else {
      this.type = "number";
      var list = this.list;
      this.dom.empty();
      for(var i = 0; i < list.length; i++) {
	var elem = list[i];
	var selected = (elem.id == value);
	if(selected)
	  this.dom.append("<option value='" + elem.id + "' selected = 'true'>" + elem.name + "</option>");
	else
	  this.dom.append("<option value='" + elem.id + "'>" + elem.name + "</option>");
      }
    }
  }
});

ajaxforms.RadioGroup = ajaxforms.Field.extend({
  get: function () {
	for(var i = 0; i < this.children.length; i++)
	  if(this.children[i].attr("checked"))
	    return this.children[i].val() * 1;
  },
  set: function (value) {
    var field = this;
    if(!this.list)
      $.getJSON(this.url, {}, function (obj) { field.list = obj.list; field.set(value); });
    else {
      this.type = "number";
      this.children = [];
      var list = this.list;
      this.dom.empty();
      if(this.wrap_ul) {
	parent = $('<ul></ul>');
	if(this.ul_class) parent.addClass(this.ul_class);
	  this.dom.append(parent);
      } else {
        parent = this.dom;
      }
      for(var i = 0; i < list.length; i++) {
	var elem = list[i];
	var selected = (elem.id == value);
	var child;
	if(this.wrap_ul) {
	  child = $('<li></li>');
	  if(this.li_class) child.addClass(this.li_class);
	  parent.append(child);
	} else {
	  child = parent;
	}
	var rbox;
	if(selected)
	  rbox = $("<input type = 'radio' name = '" + this.name + "_group' value='" + 
	    elem.id + "' checked = 'true' />");
	else
	  rbox = $("<input type = 'radio' name = '" + this.name + "_group' value='" + 
	    elem.id + "' />");
	this.children.push(rbox);
	child.append(rbox); child.append(elem.name); if(!this.wrap_ul) child.append("<br/>");
      }
    }
  }
});

ajaxforms.CheckGroup = ajaxforms.Field.extend({
  get: function () {
    var values = [];
    for(var i = 0; i < this.children.length; i++)
      if(this.children[i].attr("checked"))
        values.push(this.children[i].val() * 1);
    return values;
  },
  set: function (values) {
    var field = this;
    if(!this.list)
      $.getJSON(this.url, {}, function (obj) { field.list = obj.list; field.set(values); });
    else {
      this.type = "number";
      this.children = [];
      var list = this.list;
      this.dom.empty();
      if(this.wrap_ul) {
        parent = $('<ul></ul>');
        if(this.ul_class) parent.addClass(this.ul_class);
        this.dom.append(parent);
      } else {
        parent = this.dom;
      }
      for(var i = 0; i < list.length; i++) {
	var elem = list[i];
	var selected = false;
	for(var j = 0; j < values.length; j++)
	  if(values[j] == elem.id) selected = true;
	var child;
	if(this.wrap_ul) {
	  child = $('<li></li>');
	  if(this.li_class) child.addClass(this.li_class);
	  parent.append(child);
	} else {
	  child = parent;
	}
	var cbox;
	if(selected)
	  cbox = $("<input type = 'checkbox' name = '" + this.name + "_check' value='" + 
	    elem.id + "' checked = 'true' />");
	else
	  cbox = $("<input type = 'checkbox' name = '" + this.name + "_check' value='" + 
	    elem.value + "' />");
	this.children.push(cbox);
	child.append(cbox); child.append(elem.name); if(!this.wrap_ul) child.append("<br/>");
      }
    }
  }
});

ajaxforms.ListBox = ajaxforms.Field.extend({
  get: function () {
    var svals = this.dom.val();
    var vals = [];
    for(var i = 0; i < svals.length; i++)
      vals[i] = svals[i] * 1;
    return vals;
  },
  set: function (values) {
    var field = this;
    if(!this.list)
      $.getJSON(this.url, {}, function (obj) { field.list = obj.list; field.set(values); });
    else {
      this.type = "number";
      var list = this.list;
      this.dom.empty();
      for(var i = 0; i < list.length; i++) {
	var elem = list[i];
	var selected = false;
	for(var j = 0; j < values.length; j++)
	  if(values[j] == elem.id) selected = true;
	if(selected)
	  this.dom.append("<option value='" + elem.id + "' selected = 'true'>" + elem.name + "</option>");
	else
	  this.dom.append("<option value='" + elem.id + "'>" + elem.name + "</option>");
      }
    }
  }
});

ajaxforms.TagBox = ajaxforms.Field.extend({
  draw: function () {
    this._super();
    this.input = $("input", this.dom);
    this.button = $("button", this.dom);
    this.ul = $("ul", this.dom);
    var field = this;
    this.button.click(function () { 
	 field.add_tag(field.input.val());
	 return false;
    });
  },
  disable: function () {
    this.input.attr("disabled", true);
    this.button.attr("disabled", true);
    $("a", this.dom).attr("disabled", true);
  },
  get: function () {
    return this.values;
  },
  set: function (values) {
    var field = this;
    if(!this.list)
      $.getJSON(this.url, {}, function (obj) { field.list = obj.list; field.set(values); });
    else {
      this.type = "number";
      this.values = values;
      var list = this.list;
      var tags = []; var tags_reverse = {}; var tags_values = {};
      for(var i = 0; i < list.length; i++) {
	tags_values[list[i].id] = list[i].name;
	tags.push(list[i].name);
	tags_reverse[list[i].name] = list[i].id;
      }
      this.tags = tags; this.tags_reverse = tags_reverse;
      this.input.empty(); this.ul.empty();
      for(var i = 0; i < values.length; i++) {
	var remove = this.disabled ? false : $("<span>x</span>");
	var item = $("<li></li>");
	item.append(tags_values[values[i]]); if(remove) { item.append("&nbsp;"); item.append(remove); }
	this.ul.append(item);
	if(remove) remove.click((function (item, value) { return function () { field.delete_tag(item, value); return false; } })(item, values[i]));
      }
      this.input.autocomplete(tags); 
    }
  },
  delete_tag: function (item, val) {
    var tags = this.values;
    item.slideUp("fast");
    var new_tags = [];
    for(var i = 0; i < tags.length; i++)
      if(tags[i] != val) {
	new_tags.push(tags[i]);
      }
    this.values = new_tags;
  },
  add_tag: function (text) {
    var field = this;
    var value = this.tags_reverse[text];
    if(value) {
      var tags = this.values;
      for(var i = 0; i < tags.length; i++)
	if(tags[i] == value) return false;
      tags.push(value);
      var remove = this.disabled ? false : $("<span>x</span>");
      var item = $("<li></li>");
      item.append(text); if(remove) { item.append("&nbsp;"); item.append(remove); }
      this.ul.append(item);
      if(remove) remove.click(function () { field.delete_tag(item, value); return false; });
      this.values = tags;
      this.input.val('');
      return true;
    }
    return false;
  }
});

ajaxforms.Button = ajaxforms.Class.extend({
  init: function (attrs) {
    this.attrs = attrs;
    for(var attr in attrs)
      if(attrs.hasOwnProperty(attr))
	this[attr] = attrs[attr];
  },
  clone: function () {
    return new this.__class__(this.attrs);
  },
  draw: function () {
    var button = this;
    this.dom = $("[name=" + this.name + "]", this.form.dom);
    this.dom.click(function (event) {
	  button.disable();
          button.form.hideFlash();
	  var result = button.click();
	  return result;
	});	
    for(name in this.attrs)
      if(this.attrs.hasOwnProperty(name)) {
        if(name == "class")
          this.dom.addClass(this.attrs[name]);
        else
          this.dom.attr(name, this.attrs[name]);
    }
    if(this.disabled) this.disable();
  },
  enable: function () {
    this.dom.attr("disabled", false);
  },
  disable: function () {
    this.dom.attr("disabled", true);
  },
});

ajaxforms.ResetButton = ajaxforms.Button.extend({
  click: function () {
    this.form.reset();
    this.enable();
    return false;
  }
});

ajaxforms.SaveButton = ajaxforms.Button.extend({
  click: function () {
    var button = this;
    $.post(this.url || this.form.url, { json: $.toJSON(this.form.getData()) }, function (response) {
      if(response.message) {
	button.form.showFlash(response.message, response.fields);
      }
      if(!response.message && !response.fields) {
	button.form.showFlash("Saved!");
      }
      button.enable();
    }, 'json');
    return false;
  }
});

ajaxforms.SaveRedirectButton = ajaxforms.Button.extend({
  click: function () {
    var button = this;
    $.post(this.url || this.form.url, { json: $.toJSON(this.form.getData()) }, function (response) {
      if(response.message) {
	button.form.showFlash(response.message, response.fields);
      }
      if(!response.message && !response.fields) {
	window.location.href = button.to;
      } else button.enable();
    }, 'json');
    return false;
  }
});

ajaxforms.SaveRedirectResultButton = ajaxforms.Button.extend({
  click: function () {
    var button = this;
    $.post(this.url || this.form.url, { json: $.toJSON(this.form.getData()) }, function (response) {
      if(response.message) {
	button.form.showFlash(response.message, response.fields);
      }
      if(!response.message && !response.fields) {
	window.location.href = response.to;
      } else button.enable();
    }, 'json');
    return false;
  }
});

ajaxforms.SaveRedirectInlineButton = ajaxforms.Button.extend({
  click: function () {
    var button = this;
    $.post(this.url || this.form.url, { json: $.toJSON(this.form.getData()) }, function (response) {
      if(response.message) {
	button.form.showFlash(response.message, response.fields);
      }
      if(!response.message && !response.fields) {
	    $.get(button.to, {}, function (response) { $('#'+button.container).html(response); });
      } else button.enable();
    }, 'json');
    return false;
  }
});

ajaxforms.RedirectButton = ajaxforms.Button.extend({
  click: function () {
    window.location.href = this.to;
    return false;
  }
});

ajaxforms.DetailDeleteButton = ajaxforms.Button.extend({
  click: function () {
    this.form.parent.remove(this.form);
    return false;
  }
});

