--
--   Enhanced functionality for the LuaCOM library
--


-- startup code: presumes LuaCOM has already been initialized
-- and lies in the luacom table

assert(luacom)
luacomE = luacom

-- tests for other dependencies
assert(table)
assert(string)
assert(io)


--
-- ExportConstants
--
--   Exports all the constants defined in the type library
--   to the global environment
--

function luacomE.ExportConstants(obj, const_table)

  if luacomE.GetType(obj) == "LuaCOM" then
     obj = luacom.GetTypeInfo(obj)
  end
  
  if luacomE.GetType(obj) == "ITypeInfo" then
    obj = obj:GetTypeLib()
  end
  
  assert(luacomE.GetType(obj) == "ITypeLib")
  
  if const_table == nil then
    const_table = _G
  end
  
  obj:ExportConstants(const_table)
end



--
-- Proxies for luacom.CreateObject
--

function luacomE.CreateLocalObject(ID)
  return luacom.CreateObject(ID, "local_server")
end

function luacomE.CreateInprocObject(ID)
  return luacom.CreateObject(ID, "inproc_server")
end

--
-- luacomE.pairs
--
--   Returns an iterator for a COM enumerator
--

function luacomE.pairs(luacom_obj)

  assert(luacom_obj)

  local enumerator = luacom.GetEnumerator(luacom_obj)

  if enumerator == nil then
    error("Could not get an enumerator")
    return
  end

  local function iterator(state, index)
    local value = state:Next()
    
    if value == nil then
      return nil
    else
      return index+1, value
    end

  end

  return iterator, enumerator, 0
end


--
-- GetType
--
--  Returns the type of the object (if it is managed by LuaCOM)
--

function luacomE.GetType(obj)
  local typetable = getmetatable(obj)
  
  if typetable ~= nil then
    return typetable.type
  else
    return nil
  end
  
end


-------------------------------
-- Type library related code --
-------------------------------

-- Copies fields of a table to another

function luacomE._copyFields(dest, src, fields)
  local function copyField(i,field)
    local src_field, dest_field
    if type(field) == "table" then
      src_field = field[2]
      dest_field = field[1]
    else
      src_field = field
      dest_field = field
    end
    
    if src[src_field] ~= "" then
      dest[dest_field] = src[src_field]
    end
  end
  
  table.foreach(fields, copyField)
end

-- FillTypeInfo
--
--  Creates a table filled with the
--  type information contained in a typeinfo

function luacomE.FillTypeInfo(rawTypeInfo)

  if rawTypeInfo.FilledTypeInfo then
    return rawTypeInfo.FilledTypeInfo
  end
  
  local doc, attr
  local typeinfo = {}
  
  rawTypeInfo.FilledTypeInfo = typeinfo
    
  -- Basic information  
  doc = rawTypeInfo:GetDocumentation()
  typeinfo.name = doc.name
  typeinfo.description = doc.helpstring

  -- Now the attributes
  attr = rawTypeInfo:GetTypeAttr()
  
  typeinfo.type = attr.typekind
  typeinfo.guid = attr.GUID

  -- copies flags
  table.foreach(attr.flags, function(i,v) typeinfo[i] = v end)
  
  -- function to fill the different types of elements
  local function fillMethods(methods, num_methods)
    local i, index, method, rawmethod
    
    index = 1
    for i = 0, num_methods - 1 do
      method = {}
      rawmethod = rawTypeInfo:GetFuncDesc(i)

      if rawmethod ~= nil then
        method.rawMethod = rawmethod

        fields = {"name", "description",
          "helpfile", "helpcontext",
          {"dispid", "memid"}, {"typeinv", "invkind"},
          {"num_params", "Params"}, "parameters", "type"
        }
        
        luacomE._copyFields(method, rawmethod, fields)

        local prototype
        if method.type then
          prototype = method.type.." "..method.name
          prototype = prototype.."("
        end
        
        -- builds prototype
        if method.parameters then
          -- builds parameter list
          local first_param = true
          
          local function add_param(i, param)

            if first_param then
              first_param = false
            else
              prototype = prototype..", "
              first_param = false
            end
            
            if param.type then
              prototype = prototype..param.type.." "
            end
            
            prototype = prototype..param.name
          end
          
          table.foreachi(method.parameters, add_param)

        end

        if prototype then      
          prototype = prototype..")"
          method.prototype = prototype
        end
        
        methods[index] = method        
        index = index+1
      end

    end
  end

  local function fillEnum(values, num_values)
    local i, rawConstant
    
    local fields = {"name", "value"}
    
    for i = 0, num_values - 1 do
      rawConstant = rawTypeInfo:GetVarDesc(i)
      constant = {}
      constant.rawConstant = rawConstant
      luacomE._copyFields(constant, rawConstant, fields)
     
      values[i+1] = constant
    end
  end
  
  
  local function fillCoClass(interfaces, num_interfaces)
    local i, interface, rawinterface, typeflags
    
    for i = 0, num_interfaces - 1 do
      rawinterface = rawTypeInfo:GetImplType(i)
      interface = {}
      interface.dispinterface = luacomE.FillTypeInfo(rawinterface)

      -- copies impltypeflags      
      typeflags = rawTypeInfo:GetImplTypeFlags(i)
      table.foreach(typeflags, function(i,v) interface[i] = v end)
  
      interfaces[i+1] = interface      
    end
  end
  
  
  -- Creates tables to hold components of the typeinfo
  
  if attr.typekind == "dispinterface" then
    typeinfo.methods = {}
    fillMethods(typeinfo.methods, attr.Funcs)
  elseif attr.typekind == "coclass" then
    typeinfo.interfaces = {}
    fillCoClass(typeinfo.interfaces, attr.ImplTypes)
  elseif attr.typekind == "enumeration" then
    typeinfo.values = {}
    fillEnum(typeinfo.values, attr.Vars)
  end
  
  return typeinfo

end

--
-- FillTypeLib
--
--  Loads a type library and
--  creates a table that rearranges
--  the information contained in the
--  table library to ease its navigation
--

function luacomE.FillTypeLib(rawtlb)

  if rawtlb._luacom_isfilled then
    return rawtlb
  end
  
  -- Stores type library information
  local tlb = {}
  tlb._luacom_isfilled = true
  
  tlb.rawtlb = rawtlb
  
  local doc = rawtlb:GetDocumentation()
  
  tlb.name = doc.name
  tlb.description = doc.helpstring
  
  -- stores typeinfos
  
  local typeinfo, rawtypeinfo, attr

  for i = 0, rawtlb:GetTypeInfoCount() - 1 do
  
    rawTypeInfo = rawtlb:GetTypeInfo(i)
    typeinfo = luacomE.FillTypeInfo(rawTypeInfo)
    tlb[i+1] = typeinfo
    
  end
    
  return tlb
end


--
--  DumpTypeLib
--    Creates a html page describing a type library,
--

function luacomE.DumpTypeLib(obj, htmlfile)

  local tlb, rawtlb

  if type(obj) == "string" then
    rawtlb = luacom.LoadTypeLibrary(obj)
  elseif luacomE.GetType(obj) == "ITypeLib" then
    rawtlb = obj
  elseif luacomE.GetType(obj) == "LuaCOM" then
    obj = luacom.GetTypeInfo(obj)
    rawtlb = obj:GetTypeLib()
  elseif luacomE.GetType(obj) == "ITypeInfo" then
    rawtlb = obj:GetTypeLib()
  end
  
  if rawtlb == nil then
	  error("Type library not found.")
  end
  
  tlb = luacomE.FillTypeLib(rawtlb)  
  rawtlb = nil
  

  if htmlfile == nil then
  	htmlfile = tlb.name..".html"
  elseif string.sub(htmlfile, -1) == "\\" then
  	htmlfile = htmlfile..tlb.name..".html"
  end
  
  local filehandle = io.open(htmlfile, "w")
  
  if filehandle == nil then
    error("Could not create "..htmlfile..": file exists")
  end
  
  io.output(filehandle)
  
  -- writes html header
  io.write("<html>\n")
  io.write("<head>")
  
  -- writes title
  io.write("<title>")
  
  if tlb.description then
    io.write(tlb.description)
  else
    io.write(tlb.name)
  end
   
  io.write("</title>\n")
  
  io.write("<h1>")
  
  if tlb.description then
    io.write(tlb.description)
  else
    io.write(tlb.name)
  end
  
  io.write(" Type Library")
  
  io.write("</h1>\n")
  io.write("<hr>\n")
  
  --
  -- First, makes an index for the entire type library
  --
  
  io.write("<h2>Summary</h2>\n")

  -- Output function
  local function write_typeinfo(i, typeinfo)
    io.write("<li>")
    io.write("<b>")

    if typeinfo.type ~= "dispinterface" or typeinfo.dispatchable then
      io.write("<a href=\"#"..typeinfo.name.."\">"..typeinfo.name.."</a>")
    else
      io.write(typeinfo.name)
    end
    
    io.write("</b>")
    io.write("</li>")
    
    if typeinfo.description and typeinfo.description ~= "" then
      io.write(" - "..typeinfo.description)
    end

    io.write("</li>\n")
  end

  -- filter function
  local function filter_typeinfo(type)
    local function filter(i,typeinfo)

      if typeinfo.type == type then
        map_function(i, typeinfo)
      end
    end
    
    table.foreachi(tlb, filter)
  end
  
  map_function = write_typeinfo
  
  io.write("<h3>Components</h3>\n")
  filter_typeinfo("coclass")

  io.write("<h3>Enumerations</h3>\n")
  filter_typeinfo("enumeration")

  io.write("<h3>Interfaces</h3>\n")
  filter_typeinfo("dispinterface")
  
  --
  -- Now, describe each element
  --
  
  io.write("<hr>\n")
  io.write("<h2>Detailed description</h2>\n")
  
  -- describes coclasses

  io.write("<h3>Components Classes</h3>\n")

  local function describe_coclass(i, typeinfo)
    assert(typeinfo.type == "coclass")
    
    if typeinfo.restricted or typeinfo.hidden then
      return
    end
    
    io.write("<h4>")
    io.write("<a name=\""..typeinfo.name.."\"></a>")
    io.write(typeinfo.name)
    
    io.write("</h4>\n")
    
    if typeinfo.description then
      io.write(typeinfo.description.."<p>")
    end
    
    io.write("<pre>"..typeinfo.guid.."</pre><p>")
    
    local i, default, source
    
    -- locates the default interface and the source interface
    for i=1, table.getn(typeinfo.interfaces) do
    
      if typeinfo.interfaces[i].source and source == nil then
        source = typeinfo.interfaces[i].dispinterface
      elseif typeinfo.interfaces[i].source 
             and typeinfo.interfaces[i].default then
        source = typeinfo.interfaces[i].dispinterface
      end
      
      if not typeinfo.interfaces[i].source and default == nil then
        default = typeinfo.interfaces[i].dispinterface
      elseif typeinfo.interfaces[i].default 
             and not typeinfo.interfaces[i].source then
        default = typeinfo.interfaces[i].dispinterface
      end
      
    end
    
    if default then    
      if default.dispatchable then
        io.write("<a href=\"#"..default.name.."\">"..default.name.."</a>")
      else
        io.write(default.name)
      end
      
      io.write(" is the default interface for this component.<br>")
    end
    
    if source then    
      if source.dispatchable then
        io.write("<a href=\"#"..source.name.."\">"..source.name.."</a>")
      else
        io.write(source.name)
      end
      
      io.write(" is the default set of events for this component.<br>")
    end

    if typeinfo.appobject then
      io.write("This is the Application object.<br>")
    end
    
    if typeinfo.control then
      io.write("This component is an OLE control.<br>")
    end
    
    if typeinfo.cancreate then
      io.write("Instances of this component can be created.<br>")
    end
   
  end
  
  map_function = describe_coclass
  
  filter_typeinfo("coclass")
  
  
  
  -- describes enumerations
  
  io.write("<hr><h3>Enumerations</h3>\n")
  
  local function describe_enum(i, typeinfo)
    assert(typeinfo.type == "enumeration")
    
    io.write("<h4>")
    io.write("<a name=\""..typeinfo.name.."\"></a>")
    io.write(typeinfo.name)
    
    io.write("</h4>\n")
    
    if typeinfo.description and typeinfo.description ~= nil then
      io.write(typeinfo.description.."<p>")
    end
    
    local function describe_constant(i, constant)
      io.write("<li>")
      io.write(constant.name.." = "..tostring(constant.value))
      io.write("</li>\n")
    end
    
    io.write("<ul>\n")
    table.foreachi(typeinfo.values, describe_constant)
    io.write("</ul>\n")
  end
  
  map_function = describe_enum
  
  filter_typeinfo("enumeration")
    
  
  -- describes interfaces
  
  io.write("<hr><h3>Interfaces</h3>\n")
  
  local function describe_interface(i, typeinfo)
    assert(typeinfo.type == "dispinterface")
    
    io.write("<h4>")
    io.write("<a name=\""..typeinfo.name.."\"></a>")
    io.write(typeinfo.name)
    io.write("</h4>\n")
    io.write("<pre>"..typeinfo.guid.."</pre><p>")
    
    local function describe_method_use(method)
      io.write("<b>LuaCOM examples:</b><br><br>")
      
      local function param_lister()
        local first_param = true
        local function add_param(i, param)
          if param["in"] or (not param.out) then
            local name
            if param.default then
              local default = tostring(param.default)
              if default == "" then default = "?" end
              name = param.name .. "=" .. default
            else
              name = param.name
            end
            if param.opt then name = "[" .. name .. "]" end
            if first_param then
              io.write(name)
              first_param = false
            else
              io.write(", " .. name) 
            end
          end
        end
        return add_param
      end
      
      local function retval_lister(first_retval)
        local has_retval = false
        return function(i, param)
            if param.out then
              if first_retval then
                io.write(param.name)
                first_retval = false
              else
                io.write(", " .. param.name)
              end
              has_retval = true
            end
          end, function() return has_retval end
      end
      
      if method.typeinv == "propget" then
        if method.num_params == 0 then
          io.write("com_obj." .. method.name .. "<br>")
          io.write("com_obj:get" .. method.name .. "()")
        else
          io.write("com_obj:get" .. method.name .. "(")
          table.foreachi(method.parameters, param_lister())
          io.write(")")
        end
      elseif method.typeinv == "propput" then
        if method.num_params == 1 then
          io.write("com_obj." .. method.name .. " = " .. method.parameters[1].name .. "<br>")
          io.write("com_obj:set" .. method.name .. "(" .. method.parameters[1].name .. ")<br>")
        else
          io.write("com_obj:set" .. method.name .. "(")
          table.foreachi(method.parameters, param_lister())
          io.write(")")
        end
      else
          if method.type ~= "void" then
            io.write("retval")
            table.foreachi(method.parameters, retval_lister(false))
            io.write(" = ")
          else
            local lister, checker = retval_lister(true)
            table.foreachi(method.parameters, lister)
            if checker() then io.write(" = ") end
          end
          io.write("com_obj:" .. method.name .. "(")
          table.foreachi(method.parameters, param_lister())
          io.write(")<br>")
      end
    end
    
    local function describe_method(i, method)
      if method.rawMethod == nil then
        return
      end
      
      io.write("<li>")
      io.write(method.name)
      
      if method.prototype then
        io.write("<pre>"..method.prototype.."</pre>\n")
      else
        io.write("<br>")
      end
      
      if method.description then
        io.write(method.description.."<br>\n")
      end
      
      describe_method_use(method)
      
      io.write("<p>")
      
      io.write("</li>\n")
    end
    
    io.write("<ul>\n")
    table.foreachi(typeinfo.methods, describe_method)
    io.write("</ul>\n")
  end
  
  map_function = describe_interface
  
  filter_typeinfo("dispinterface")
  
  io.write("</html>")
  
  io.output():close()
  
  return htmlfile
  
end



--
--    shows TypeLib dump to the user 
--

function luacomE.ViewTypeLib(obj)

	local filename = os.getenv("TEMP")
	
	if filename == nil then
		filename = os.getenv("TMP")
	end
	
	if filename == nil then
	  filename = luacom.GetCurrentDirectory()
  end
  
  if string.sub(filename, -1) ~= "\\" then
  	filename = filename.."\\"
  end
  
  filename = luacomE.DumpTypeLib(obj, filename)

  local browser = luacom.CreateObject("InternetExplorer.Application")
  
  browser.Visible = true
  browser:Navigate2(filename)
 
  return filename
end

local interface_proto = {
  constants = {},
  typedefs = {},
  properties = {},
  methods = {}
}

--[[function interface_proto:AddConstant(type, name, value)
  table.insert(self.constants, { type = type, name = name, value = value })
end]]--

--[[function interface_proto:AddTypedef(type, typedef)
  table.insert(self.typedefs, { type = type, typedef = typedef })
end]]--
  
function interface_proto:AddMethod(methodinfo)
  table.insert(self.methods, methodinfo)
end

function interface_proto:AddProperty(propertyinfo)
  table.insert(self.properties, propertyinfo)
end

function interface_proto:Write(file)
  local start_id = 0

  local function attribute_writer(line_break)
    local first_attribute = true
    return function(name, value)
        if not first_attribute then
          if line_break then
            file:write(",\n  ")
          else
            file:write(", ")
          end
        else
          if line_break then file:write("  ") end
        end
        first_attribute = false
        if(type(name) == "number") then
          file:write(value)
        else 
          file:write(name .. "(" .. value .. ")")
        end
      end
  end

  local function constant_writer(i, constant)
    file:write("\n" .. "  const " .. constant.type .. " " .. constant.name .. " = " ..
      constant.value .. ";\n")
  end

  local function typedef_writer(i, typedef)
    file:write("\n" .. "  typedef " .. typedef.type .. " " .. typedef.typedef .. ";\n")
  end
  
  local function param_writer()
    local first_param = true
    return function(i, param)
        if not first_param then file:write(",\n    ") end
        first_param = false
        if param.attributes then
          file:write("[")
          table.foreach(param.attributes, attribute_writer(false))
          file:write("] ")
        end
        file:write(param.type .. " " .. param.name)
      end
  end

  local function property_writer(i, property)
    if not property.attributes then
      property.attributes = { id = tostring(start_id + i) }
    end
    file:write("\n  [\n")
    if property.attributes.id == nil then
      property.attributes.id = tostring(start_id + i)
    end
    table.foreach(property.attributes, attribute_writer(true))
    file:write("\n  ]\n  ")
    file:write(property.type .. " " .. property.name .. ";\n")
  end
  
  local function method_writer(i, method)
    if not method.attributes then
      method.attributes = { id = tostring(start_id + i) }
    end
    file:write("\n  [\n")
    if method.attributes.id == nil then
      method.attributes.id = tostring(start_id + i)
    end
    table.foreach(method.attributes, attribute_writer(true))
    file:write("\n  ]\n  ")
    if method.type then
      file:write(method.type)
    else
      file:write("void")  
    end
    file:write(" " .. method.name .. "(")
    if method.parameters then
      file:write("\n    ")
      table.foreachi(method.parameters, param_writer())
      file:write("\n  );\n")
    else
      file:write("void" .. ");\n")
    end
  end
  
  if self.attributes then
    file:write("[\n  ")
    table.foreach(self.attributes, attribute_writer(true))
    file:write("\n]\n")
  end
  file:write("dispinterface " .. self.name .. "\n{")
  if self.interface ~= nil then
    file:write("  interface " .. self.interface)
  else
--  table.foreachi(self.constants, constant_writer)
--  file:write("\n")
--  table.foreachi(self.typedefs, typedef_writer)
    file:write("\n  properties:\n")
    table.foreachi(self.properties, property_writer)
    start_id = table.getn(self.properties)
    file:write("\n  methods:\n")
    table.foreachi(self.methods, method_writer)
  end
  file:write("\n};\n\n")
end

local coclass_proto = {
  interfaces = {}
}

function coclass_proto:AddInterface(attributes)
  table.insert(self.interfaces, attributes)
end

function coclass_proto:Write(file)
  local function attribute_writer(line_break)
    local first_attribute = true
    return function(name, value)
        if not first_attribute then
          if line_break then
            file:write(",\n  ")
          else
            file:write(", ")
          end
        end
        first_attribute = false
        if(type(name) == "number") then
          file:write(value)
        else 
          file:write(name .. "(" .. value .. ")")
        end
      end
  end

  local function interface_writer(i, interface)
    local name = interface.name
    interface.name = nil
    file:write("[")
    table.foreach(interface, attribute_writer(false))   
    file:write("] ")
    file:write("dispinterface " .. name .. ";\n")
    interface.name = name
  end

  if self.attributes then
    file:write("[\n  ")
    table.foreach(self.attributes, attribute_writer(true))
    file:write("\n]\n")
  end
  file:write("coclass " .. self.name .. "\n{\n")
  table.foreachi(self.interfaces, interface_writer)
  file:write("};\n\n")
end

local library_proto = {
  interfaces = {},
  coclasses = {},
  imports = {},
  typedefs = {},
  AddTypedef = interface_proto.AddTypedef
}

function library_proto:AddInterface(attributes)
  local name = attributes.name
  attributes.name = nil
  local interface = attributes.interface
  attributes.interface = nil
  local newinterface = { name = name, attributes = attributes, interface = interface, __index = interface_proto }
  setmetatable(newinterface, newinterface)
  table.insert(self.interfaces, newinterface)
  if interface ~= nil then
    return nil
  else
    return newinterface
  end
end

function library_proto:AddCoclass(attributes)
  local name = attributes.name
  attributes.name = nil
  local newcoclass = { name = name, attributes = attributes, __index = coclass_proto }
  setmetatable(newcoclass, newcoclass)
  table.insert(self.coclasses, newcoclass)
  return newcoclass
end

function library_proto:AddImport(library)
  table.insert(self.imports, library)
end
  
function library_proto:WriteODL(filename)
  local file = io.open(filename .. ".odl","w+")
  
  local function attribute_writer(line_break)
    local first_attribute = true
    return function(name, value)
        if not first_attribute then
          if line_break then
            file:write(",\n  ")
          else
            file:write(", ")
          end
        end
        first_attribute = false
        if(type(name) == "number") then
          file:write(value)
        else 
          file:write(name .. "(" .. value .. ")")
        end
      end
  end
  
  local function import_writer(i, import)
    file:write("  " .. "importlib(\"" .. import .. "\");\n")
  end
  
  local function typedef_writer(i, typedef)
    file:write("\n" .. "  typedef " .. typedef.type .. " " .. typedef.typedef .. ";\n")
  end
  
  local function interface_writer(i, interface)
    interface:Write(file)
  end

  local function coclass_writer(i, coclass)
    coclass:Write(file)
  end
  
  if self.attributes then
    file:write("[\n  ")
    table.foreach(self.attributes, attribute_writer(true))
    file:write("\n]\n")
  end
  file:write("library " .. self.name .. "\n{\n")
  table.foreachi(self.imports, import_writer)
  table.foreachi(self.typedefs, typedef_writer)
  table.foreachi(self.interfaces, interface_writer)
  table.foreachi(self.coclasses, coclass_writer)
  file:write("};\n\n")
  file:close()
end

function library_proto:WriteTLB(filename)
  self:WriteODL(filename)
  os.execute("midl " .. filename .. ".odl")
end

function luacomE.NewLibrary(attributes)
  local name = attributes.name
  attributes.name = nil
  local newlibrary = { name = name, attributes = attributes, __index = library_proto }
  setmetatable(newlibrary, newlibrary)
  return newlibrary
end
