
require "lpeg"
require "re"

module("orbit.model", package.seeall)

methods = {}
methods.__index = methods

dao_methods = {}

drivers = {}

drivers.base = {
  convert = {
    key = function (dao, field, v) return tonumber(v) end,
    integer = function (dao, field, v) return tonumber(v) end,
    number = function (dao, field, v) return tonumber(v) end,
    text = function (dao, field, v) if v then return tostring(v) end end,
    long_text = function (dao, field, v) if v then return tostring(v) end end,
    boolean = function (dao, field, v) return tonumber(v) == 1 end,
    timestamp = function (dao, field, v)
                  if v then
                    local year, month, day, hour, min, sec = 
                      string.match(v, "(%d+)%-(%d+)%-(%d+) (%d+):(%d+):(%d+)")
                    return os.time({ year = tonumber(year), month = tonumber(month),
		                day = tonumber(day), hour = tonumber(hour),
		                min = tonumber(min), sec = tonumber(sec) })
		  end
                end,
    date = function (dao, field, v)
             if v then
               local year, month, day = 
                 string.match(v, "(%d+)%-(%d+)%-(%d+)")
               return os.time({ year = tonumber(year), month = tonumber(month),
                                         day = tonumber(day), hour = 12,
                                         min = 0, sec = 0 })
             end
           end,
    belongs_to = function (dao, field, id)
                   return setmetatable({ id = tonumber(id) },
                                       { __index = function (v, name)
                                                     local obj = dao.__models[field.entity]:find(v.id)
                                                     for key, val in pairs(obj) do v[key] = val end
                                                     setmetatable(v, getmetatable(obj))
                                                     return obj[name]
                                                   end })
                 end,
    has_one = function (dao, field, id)
                return setmetatable({ id = tonumber(id) },
                                    { __index = function (v, name)
                                                  local obj = dao.__models[field.entity]:find(v.id)
                                                  for key, val in pairs(obj) do v[key] = val end
                                                  setmetatable(v, getmetatable(obj))
                                                  return obj[name]
                                                end })
              end,
    has_many = function (dao, field, id)
                 return setmetatable({},
                                     { __index = function (list, idx)
                                                   local rel = dao.__models[field.entity]
                                                   local objs = 
                                                     rel:find_all(field.foreign .. " = ?", 
                                                                  { id, order = field.order_by })
                                                   for key, val in ipairs(objs) do list[key] = val end
                                                   setmetatable(list, nil)
                                                   return objs[idx]
                                                 end })
               end,
    has_and_belongs = function (dao, field, id)
                        return setmetatable({},
                                            { __index = function (list, idx)
                                                          local rel = dao.__models[field.entity]
                                                          local query = {
                                                            entity = field.join_table,
                                                            fields = { field.entity },
                                                            condition = field.foreign .. " = ?",
                                                            id
                                                          }
                                                          local objs = rel:find_all("id in ?", { query,
                                                             order = field.order_by })
                                                          for key, val in ipairs(objs) do list[key] = val end
                                                          setmetatable(list, nil)
                                                          return objs[idx]
                                                        end })
                      end,
  },
  escape  = {
    key = function (conn, v) if tonumber(v) then return tostring(v) else return "NULL" end end,
    integer = function (conn, v) if tonumber(v) then return tostring(v) else return "NULL" end end,
    number = function (conn, v) if tonumber(v) then return tostring(v) else return "NULL" end end,
    text = function (conn, v) if v then return "'" .. conn:escape(v) .. "'" else return "NULL" end end,
    long_text = function (conn, v) if v then return "'" .. conn:escape(v) .. "'" else return "NULL" end end,
    boolean = function (conn, v) if v then return "1" else return "0" end end,
    timestamp = function (conn, v)
                  if type(v) == "string" then
	            local year, month, day, hour, min, sec = v:match("(%d+)%-(%d+)%-(%d+).(%d+):(%d+):(%d+)")
	            v = os.time({ year = tonumber(year), month = tonumber(month),
		            day = tonumber(day), hour = tonumber(hour),
		            min = tonumber(min), sec = tonumber(sec) })
                    return "'" .. os.date("%Y-%m-%d %H:%M:%S", v) .. "'"
	          elseif type(v) == "number" then 
                    return "'" .. os.date("%Y-%m-%d %H:%M:%S", v) .. "'"
                  else 
	            return "NULL" 
	          end
                end,
    date = function (conn, v) 
             if type(v) == "string" then
	       local year, month, day = v:match("(%d+)%-(%d+)%-(%d+)")
	       v = os.time({ year = tonumber(year), month = tonumber(month),
	          day = tonumber(day), hour = 12, min = 0, sec = 0 })
               return "'" .. os.date("%Y-%m-%d %H:%M:%S", v) .. "'"
	     elseif type(v) == "number" then 
               return "'" .. os.date("%Y-%m-%d %H:%M:%S", v) .. "'"
             else 
	       return "NULL" 
	     end
           end,
    belongs_to = function (conn, v)
                   if type(v) == "table" then
                     return tostring(v.id)
	           elseif tonumber(v) then
                     return tostring(v) 
                   else
	             return "NULL"
	           end
                 end,
    has_one = function (conn, v) 
                if type(v) == "table" then
	          return tostring(v.id)
	        elseif tonumber(v) then
                  return tostring(v) 
                else
	          return "NULL"
	        end
	      end
  }
}

drivers.sqlite3 = {
  convert = setmetatable({ boolean = function (dao, field, v) return v == "t" end }, 
                         { __index = drivers.base.convert }),
  escape = setmetatable({ boolean = function (conn, v) if v then return "'t'" else return "'f'" end end },
                        { __index = drivers.base.escape })
}

drivers.mysql = {
  convert = drivers.base.convert,
  escape = drivers.base.escape
}

local function log_query(sql)
  io.stderr:write("[orbit.model] " .. sql .. "\n")
end

function recycle(fresh_conn, timeout)
  local created_at = os.time()
  local conn = fresh_conn()
  timeout = timeout or 20000
  return setmetatable({}, { __index = function (tab, meth)
					 tab[meth] = function (tab, ...)
							if created_at + timeout < os.time() then
							   created_at = os.time()
							   pcall(conn.close, conn)
							   conn = fresh_conn()
							end
							return conn[meth](conn, ...)
						     end
					 return tab[meth]
				      end
			 })
end

function new()
  local app_model = { driver = drivers.sqlite3, models = {} }
  setmetatable(app_model, methods)
  return app_model
end

local function mkfield(t)
  if not t.field then
    t.field, t.table = t.table, nil
  end
  return t
end

local sql_condition = re.compile([[
                                     top <- {~ <condition> ~}
				     s <- %s+ -> ' ' / ''
                                     condition <- (<s> '(' <s> <condition> <s> ')' <s> / <simple>) (<conective> <condition>)*
                                     simple <- <s> (%func (<field> <op> {'?'} / <field> <op> <field> /
							 <field> <op>)) -> apply <s>
				     field <- !<conective> ({:table:[%w_]+:}('.'{:field:[%w_]+:})?) -> {} -> mkfield
				     op <- {~ <s> [!<>=~]+ <s> / ((%s+ -> ' ') !<conective> %w+)+ <s> ~}
                                     conective <- [aA][nN][dD] / [oO][rR]
                                 ]], { func = lpeg.Carg(1), mkfield = mkfield,
				       apply = function (f, left, op, right) return f(left, op, right) end })

local sql_order = re.compile([[
				 top <- {~ <order> (',' <order>)* ~}
				 order <- %s* <field> %s+ <ascdesc> %s*
				 field <- (%func ({:table:[%w_]+:}('.'{:field:[%w_]+:})?) -> {} -> mkfield) -> apply
			         ascdesc <- [aA][sS][cC] / [dD][eE][sS][cC]
			     ]], { func = lpeg.Carg(1), mkfield = mkfield,
				   apply = function (f, field) return f(field) end })

local sql_field = re.compile([[ (%func ({:table:[%w_]+:}('.'{:field:'*'/[%w_]+:})?) -> {} -> mkfield) -> apply ]],
			     { func = lpeg.Carg(1), mkfield = mkfield,
			       apply = function (f, field) return f(field) end })

local function build_query(main_entity, dao, condition, args)
  local i = 0
  local schema = dao.__schema
  args = args or {}
  if condition then
    condition = " where " ..
      sql_condition:match(condition, 1,
			  function (left, op, right)
			    if left == "?" then left, right = right, left end
			    left.table = left.table or main_entity
			    local left_field = schema[left.table].table_name .. "." ..
			      schema[left.table].fields[left.field].column_name
			    if not right then
			      return left_field .. op
			    elseif right ~= "?" then
			      local right_field = schema[right.table].table_name .. "." ..
				schema[right.table].fields[right.field].column_name
			      return left_field .. op .. right_field
			    else
			      i = i + 1
			      if type(args[i]) == "table" and args[i].entity then
				return left_field .. op .. "(" ..
				  build_query(args[i].entity, dao, args[i].condition, args[i]) .. ")"
			      elseif type(args[i]) == "table" then
				local values = {}
				for j, value in ipairs(args[i]) do
				  values[#values + 1] = dao:escape(schema[left.table].fields[left.field].type, value)
				end
				return left_field .. op .. "(" .. table.concat(values, ", ") .. ")"
			      else
				return left_field .. op .. dao:escape(schema[left.table].fields[left.field].type, args[i])
			      end
			    end
			  end)
  else condition = "" end
  local order = ""
  if args.order then 
    order = " order by " .. sql_order:match(args.order, 1,
					    function (field)
					      field.table = field.table or main_entity
					      return schema[field.table].table_name .. "." ..
						schema[field.table].fields[field.field].column_name
					    end)
  end
  local field_list, table_list, select, limit
  if args.distinct then select = "select distinct " else select = "select " end
  if tonumber(args.count) then limit = " limit " .. tonumber(args.count) else limit = "" end
  if args.fields then
    local fields = {}
    for _, field in ipairs(args.fields) do
      fields[#fields+1] = sql_field:match(field, 1, function (field)
						      field.table = field.table or main_entity
						      if field.field == "*" then
							return schema[field.table].table_name .. "." .. field.field
						      else
							return schema[field.table].table_name .. "." ..
							  schema[field.table].fields[field.field].column_name
						      end
						    end)
    end
    field_list = table.concat(fields, ", ")
  else
    field_list = schema[main_entity].table_name .. ".*"
  end
  local from = args.from or {}
  local tables = { schema[main_entity].table_name }
  for _, entity in ipairs(from) do
    tables[#tables+1] = schema[entity].table_name
  end
  table_list = table.concat(tables, ", ")
  local sql
  if not args.delete then
    sql = select .. field_list .. " from " .. table_list .. 
      condition .. order .. limit
  else
    sql = "delete from " .. table_list .. condition
  end
  return sql
end

function methods:new(name, dao)
  dao = dao or {}
  dao.__conn, dao.__name, dao.__schema, dao.__driver, dao.__logging, dao.__models = 
    self.conn, name, self.schema.entities, self.driver, self.logging, self.models
  if dao.__schema[name].parent then
    local parent = self.models[dao.__schema[name].parent.name]
    setmetatable(dao, { __index = parent })
  else
    setmetatable(dao, dao_methods)
  end
  self.models[name] = dao
  return dao
end

function dao_methods:escape(type, v)
  return self.__driver.escape[type](self.__conn, v)
end

function dao_methods:from_row(row)
  local schema, driver = self.__schema[self.__name], self.__driver
  if schema.fields.type then
    local type = row[schema.fields.type.column_name]
    if self.__schema[type] and self.__models[type] then
      schema = self.__schema[type]
      self = self.__models[type]
    end
  end
  local obj = {}
  for name, field in pairs(schema.fields) do
    local conv = driver.convert[field.type]
    if conv then
      if field.column_name then
        obj[name] = conv(self, field, row[field.column_name])
      else
	obj[name] = conv(self, field, row[schema.fields["id"].column_name])
      end
    else
      error("no conversion for field " .. name .. " of type " .. field.type)
    end
  end
  return setmetatable(obj, { __index = self })
end

function dao_methods:torow()
  local row = {}
  local schema, driver, conn = self.__schema[self.__name], self.__driver, self.__conn
  for name, field in pairs(schema.fields) do
    if field.column_name then
      local esc = driver.escape[field.type]
      if esc then
	row[field.column_name] = esc(conn, self[name])
      else
	error("no escape function for field " .. name .. " of type " .. field.type)
      end
    end
  end
  return row
end

function dao_methods:fetch_one(sql)
  if self.__logging then log_query(sql) end
  local cursor, err = self.__conn:execute(sql)
  if not cursor then error(err) end
  if type(cursor) == "number" then
    return cursor
  else
    local row = cursor:fetch({}, "a")
    cursor:close()
    if row then
      return self:from_row(row)
    end
    return row
  end
end

function dao_methods:fetch_all(sql)
  local rows = {}
  if self.__logging then log_query(sql) end
  local cursor, err = self.__conn:execute(sql)
  if not cursor then error(err) end
  if type(cursor) == "number" then
    return cursor
  else
    local row = cursor:fetch({}, "a")
    while row do
      rows[#rows + 1] = self:from_row(row)
      row = cursor:fetch({}, "a")
    end
    cursor:close()
    return rows
  end
end

function dao_methods:execute(sql)
  local ok, err = self.__conn:execute(sql)
end

local function parse_by_condition(condition, args)
  condition = string.gsub(condition, "_and_", "|")
  local pairs = {}
  for field in string.gmatch(condition, "[%w_]+") do
    local i = #pairs + 1
    if type(args[i]) == "table" then
      pairs[i] = field .. " in ?"
    else
      pairs[i] = field .. " = ?"
    end
  end
  return table.concat(pairs, " and ")
end

function dao_methods.__index(dao, name)
  local m = dao_methods[name]
  if m then
    return m
  else
    local match = string.match(name, "^find_by_(.+)$")
    if match then
      dao_methods[name] = function (self, args)
                            return self:find_first(parse_by_condition(match, args), args) 
                          end
      return dao_methods[name]
    end
    local match = string.match(name, "^find_all_by_(.+)$")
    if match then
      dao_methods[name] = function (self, args)
                            return self:find_all(parse_by_condition(match, args), args) 
                          end
      return dao_methods[name]
    end
    return nil
  end
end

function dao_methods:find(id)
  if not type(id) == "number" then
    error("find error: id must be a number")
  end
  return self:fetch_one(build_query(self.__name, self, "id = ?", { id, count = 1 }))
end

function dao_methods:find_first(condition, args)
  if type(condition) ~= "string" then
    args, condition = condition, nil
  end
  if self.__schema[self.__name].parent then
    condition = condition and "(" .. condition .. ") and type = ?" or "type = ?"
    args[#args+1] = self.__name
  end
  return self:fetch_one(build_query(self.__name, self, condition, args))
end

function dao_methods:find_all(condition, args)
  if type(condition) ~= "string" then
    args, condition = condition, nil
  end
  if self.__schema[self.__name].parent then
    condition = condition and "(" .. condition .. ") and type = ?" or "type = ?"
    args[#args+1] = self.__name
  end
  return self:fetch_all(build_query(self.__name, self, condition, args))
end

function dao_methods:delete_all(condition, args)
  if type(condition) ~= "string" then
    args, condition = condition, nil
  end
  args.delete = true
  if self.__schema[self.__name].parent then
    condition = "(" .. condition .. ") and type = ?"
    args[#args+1] = self.__name
  end
  return self:fetch_one(build_query(self.__name, self, condition, args))
end

function dao_methods:new(row)
  row = row or {}
  local schema = self.__schema[self.__name]
  for name, field in pairs(schema.fields) do
    row[name] = row[name] or field.default
  end
  row.id = nil
  if schema.parent then row.type = self.__name end
  setmetatable(row, { __index = self })
  return row
end

function dao_methods:update()
  local row = self:torow()
  local updates = {}
  if self.__schema[self.__name].fields["updated_at"] then
    local now = os.time()
    self.updated_at = now
    row[self.__schema[self.__name].fields["updated_at"].column_name] = self:escape("timestamp", now)
  end
  for k, v in pairs(row) do
    table.insert(updates, k .. " = " .. v)
  end
  local sql = "update " .. self.__schema[self.__name].table_name .. " set " ..
    table.concat(updates, ", ") .. " where " .. 
    self.__schema[self.__name].fields["id"].column_name .. " = " .. self.id
  if self.__logging then log_query(sql) end
  local ok, err = self.__conn:execute(sql)
  if not ok then error(err) end
end

function dao_methods:insert()
  local row = self:torow()
  if self.__schema[self.__name].fields["created_at"] then
    local now = os.time()
    self.created_at = now
    row[self.__schema[self.__name].fields["created_at"].column_name] = self:escape("timestamp", now)
  end
  if self.__schema[self.__name].fields["updated_at"] then
    local now = os.time()
    self.updated_at = now
    row[self.__schema[self.__name].fields["updated_at"].column_name] = self:escape("timestamp", now)
  end
  local columns, values = {}, {}
  for k, v in pairs(row) do
    table.insert(columns, k)
    table.insert(values, v)
  end
  local sql = "insert into " .. self.__schema[self.__name].table_name .. 
    " (" .. table.concat(columns, ", ") .. ") values (" ..
    table.concat(values, ", ") .. ")"
  if self.__logging then log_query(sql) end
  local ok, err = self.__conn:execute(sql)
  if ok then 
    self.id = self.id or self.__conn:getlastautoid()
  else 
    error(err)
  end
end

function dao_methods:save(force_insert)
  if self.id and (not force_insert) then
    self:update()
  else
    self:insert()
  end
end

function dao_methods:delete()
  if self.id then
    local sql = "delete from " .. self.__schema[self.__name].table_name .. " where " ..
	self.__schema[self.__name].fields["id"].column_name .. " = " .. self.id
    if self.__logging then log_query(sql) end
    local ok, err = self.__conn:execute(sql)    
    if ok then self.id = nil else error(err) end
  end
end
