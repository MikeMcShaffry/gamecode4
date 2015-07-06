
local _M = {}

local function copy(t1, t2)
  if not t2 then return t1 end
  for k, v in pairs(t2) do 
    if not t1[k] then t1[k] = v end
  end
  return t1
end

local function move(t, k1, k2)
  if t then
    t[k1] = t[k2]
    t[k2] = nil
  end
end

local primitive_types = {
  key = {},
  text = { default = "size" },
  long_text = { default = "size" },
  integer = { default = "size" },
  boolean = {},
  timestamp = {},
  number = {},
  date = {},
  belongs_to = { default = "entity", suffix = "_id" },
  has_many = { default = "entity", virtual = true },
  has_one = { default = "entity", suffix = "_id" },
  has_and_belongs = { default = "entity", virtual = true }
}

local function schema_funcs(extra)
  local i = 0
  local funcs = {}

  local function order(start)
    i = start or (i + 1)
    return i
  end

  local function funcs_index(t, name)
    local prim = primitive_types[name]
    if prim then
      local def = prim.default
      if def then
			t[name] = function (params)
		    if params and params[1] then
		    	move(params, def, 1)
		    end
		    return copy({ order = order(params and params.order), type = name }, params)
		  end
      else
	t[name] = function (params)
		    return copy({ order = order(params and params.order), type = name }, params)
		  end
      end
      return t[name]
    elseif extra and extra[name] then
      t[name] = extra[name]
      return t[name]
    else
      t[name] = function (params)
		  return { order = order(params and params.order), type = name, params = params }
		end
      return t[name]
    end
  end

  function funcs.entity(t)
    if not t.fields.id and not t.parent then
      error("base entity must have an id field")
    end
    return t
  end

  return setmetatable(funcs, { __index = funcs_index })
end

local function fill_entity(schema, alias, entity)
  entity.name = alias
  if entity.parent then
    local parent = schema.entities[entity.parent]
    if not parent then
      parent = fill_entity(schema, entity.parent, schema[entity.parent])
      schema[entity.parent] = nil
      schema.entities[entity.parent] = parent
    end
    entity.table_name = parent.table_name
    entity.column_prefix = entity.name .. "_"
    entity.parent = parent
  else
    entity.table_name = entity.table_name or ((schema.table_prefix or "") .. alias)
  end
  for f_alias, field in pairs(entity.fields) do
    if primitive_types[field.type] and not primitive_types[field.type].virtual then
      field.column_name = field.column_name or ((schema.column_prefix or "") ..
					        (entity.column_prefix or "") .. 
					        f_alias .. (primitive_types[field.type].suffix or "")) 
    end
    if primitive_types[field.type] and primitive_types[field.type].virtual and not field.foreign then
      field.foreign = entity.name
    end
    if field.type == "has_and_belongs" and not field.join_table then
      local names = { alias, field.entity }
      table.sort(names)
      field.join_table = (schema.table_prefix or "") .. table.concat(names, "_")
    end
  end
  if entity.parent then
    for k, v in pairs(entity.parent.fields) do
      entity.fields[k] = v
    end
  end
  return entity
end

local non_entities = { table_prefix = true, column_prefix = true, entities = true }

function _M.expand(def, s, extra)
  s = s or { entities = {} }
  setmetatable(s, { __index = schema_funcs(extra) })
  setfenv(def, s)
  def()
  setmetatable(s, nil)
  for k, v in pairs(s) do
    if not non_entities[k] and not s.entities[k] then
      s.entities[k] = fill_entity(s, k, v)
      s[k] = nil
    end
  end
  return s
end

function _M.load(schema_file, schema, extra)
  local def = assert(loadfile(schema_file))
  return _M.expand(def, schema, extra)
end

function _M.loadstring(schema_str, chunkname, schema, extra)
  local def = assert(loadstring(schema_str, chunkname))
  return _M.expand(def, schema, extra)
end

return _M
