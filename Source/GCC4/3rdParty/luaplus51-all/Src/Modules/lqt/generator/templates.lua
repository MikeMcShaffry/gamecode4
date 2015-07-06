module('templates', package.seeall)

-- TODO: maybe automate this?
local translate = dofile(template_file)

--- Creates a deep copy of an object.
local function deepcopy(object)
	local lookup_table = {}
	local function _copy(object)
		if type(object) ~= "table" then
			return object
		elseif lookup_table[object] then
			return lookup_table[object]
		end
		local new_table = {}
		lookup_table[object] = new_table
		for index, value in pairs(object) do
			-- HACK: generate new ids for copied nodes
			if index == "id" then new_table.id = next_id()
			-- deep copy value
			else new_table[_copy(index)] = _copy(value) end
		end
		return new_table
	end
	return _copy(object)
end

-- cannot modify idindex directly while traversing it ->
-- new methods from template classes are added here first and then
-- added to idindex after the traversal
local idindex_add = {}


--- Return true if an instance of templated class should be created.
function should_copy(class)
	return translate[class.xarg.fullname] or
		(translate[module_name] and translate[module_name][class.xarg.fullname])
end

--- Creates instantiated copies of template class.
-- New classes are created according to the 'translate' table as deep copies, and
-- are inserted into the 'ret' table.
function create(class, ret)
	local temps = should_copy(class)

	local replace_in = {name=true, context=true, fullname=true, member_of=true, member_of_class=true,
		scope=true, type_base=true, type_name=true, return_type=true }

	local function template_repare(o, orig, new)
		for k,v in pairs(o) do
			if replace_in[k] then
				o[k] = o[k]:gsub(orig, new)
			elseif k == 'member_template_parameters' then
				-- ignore
				o[k] = nil
			elseif type(v) == "table" then
				template_repare(v, orig, new)
			end
		end
		if o.label and o.label:match'^Function' then
			idindex_add[o] = true -- will be copied to index, so that later it can be picked up by copy_functions
		end
	end

	local name = class.xarg.fullname
	for _, t in ipairs(temps) do
		local oclass, oparams = name:match('^(.+)<([^>]+)>$')
		local tclass, tparams = t:match('^(.+)<([^>]+)>$')
		if tclass == oclass then
			-- TODO: handle multiple template parameters
			local copy = deepcopy(class)
			template_repare(copy, oparams, tparams)
			copy.xarg.cname = copy.xarg.fullname:gsub('[<>*]', '_'):gsub('::', '_LQT_')
			ret[copy] = true
		else
			ignore(name, 'template not bound')
		end
	end
end

--- Append any created classes to the index
function finish(index)
	for f in pairs(idindex_add) do
		index[f] = true
	end
	idindex_add = {}
end
