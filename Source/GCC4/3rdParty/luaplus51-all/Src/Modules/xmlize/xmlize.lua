module(..., package.seeall)

-- Loosely based on a PHP implementation.
local lom = require "lxp.lom"

local function xml_depth(vals)
	local valCount = #vals
	if valCount == 1  and  type(vals[1]) ~= 'table' then
		return vals[1]
	end

	local children = {}

	for i = 1, #vals do
		local val = vals[i]
		if type(val) == "table" then
			children[#children + 1] = val.tag
			local tagEntry = children[val.tag]
			if not tagEntry then
				tagEntry = {}
				children[val.tag] = tagEntry
			end

			entry = {}
			tagEntry[#tagEntry + 1] = entry

			entry['@'] = val.attr
			entry['#'] = xml_depth(val)
		else
			children[#children + 1] = val
		end
	end

	return children
end


function luaize(data)
	data = data:gsub('<%?xml.-%?>(.+)', "%1")
	data = '<root>' .. data .. '</root>'

	local vals, err = lom.parse(data)

    array = xml_depth(vals);

	return array
end


---------------------------------------------------------------------------------
-- Encoding routines stolen from Lua Element Tree.
local mapping = { ['&']  = "&amp;"  ,
                  ['<']  = "&lt;"   ,
                  ['>']  = "&gt;"   ,
                  ['"']  = "&quot;" ,
                  ["'"]  = "&apos;" , -- not used
                  ["\t"] = "&#9;"    ,
                  ["\r"] = "&#13;"   ,
                  ["\n"] = "&#10;"   }

local function map(symbols)
  local array = {}
  for _, symbol in ipairs(symbols) do
    table.insert(array, {symbol, mapping[symbol]})
  end
  return array
end

encoding = {}

encoding[1] = { map{'&', '<'}      ,
                map{'&', '<', '"'} }

encoding[2] = { map{'&', '<', '>'}      ,
                map{'&', '<', '>', '"'} }

encoding[3] = { map{'&', '\r', '<', '>'}                  ,
                map{'&', '\r', '\n', '\t', '<', '>', '"'} }

encoding[4] = { map{'&', '\r', '\n', '\t', '<', '>', '"'} ,
                map{'&', '\r', '\n', '\t', '<', '>', '"'} }

encoding["minimal"]   = encoding[1]
encoding["standard"]  = encoding[2]
encoding["strict"]    = encoding[3]
encoding["most"]      = encoding[4]

local _encode = function(text, encoding)
	for _, key_value in pairs(encoding) do
		text = text:gsub(key_value[1], key_value[2])
	end
	return text
end
---------------------------------------------------------------------------------

local srep = string.rep

local function xmlsave_recurse(indent, luaTable, xmlTable, maxIndentLevel)
	local tabs = ''
	if indent then
		if not maxIndentLevel  or indent <= maxIndentLevel then
			tabs = srep('\t', indent)
		end
	end
	local keys = {}
	local entryOrder
	if luaTable[1] then
		for _, key in ipairs(luaTable) do
			local whichIndex = keys[key]
			if not whichIndex then
				keys[key] = 0
				whichIndex = 0
			end
			whichIndex = whichIndex + 1
			keys[key] = whichIndex

			local section = luaTable[key]
			if not section then
				if not indent then
					-- Generally whitespace.
					xmlTable[#xmlTable + 1] = key
				end
			else
				local entry = section[whichIndex]
				if not entry then
					error('xmlsave: syntax bad')
				end

				xmlTable[#xmlTable + 1] = tabs .. '<' .. key

				local attributes = entry['@']
				if attributes then
					if not indent then
						for _, attrKey in ipairs(attributes) do
							xmlTable[#xmlTable + 1] = ' ' .. attrKey .. '="' .. attributes[attrKey] .. '"'
						end
					else
						for attrKey, attrValue in pairs(attributes) do
							if type(attrKey) == 'string' then
								xmlTable[#xmlTable + 1] = ' ' .. attrKey .. '="' .. attrValue .. '"'
							end
						end
					end
				end

				xmlTable[#xmlTable + 1] = '>'

				local elements = entry['#']
				if type(elements) == 'table' then
					if indent then
						xmlTable[#xmlTable + 1] = '\n'
					end
					xmlsave_recurse(indent and (indent + 1) or nil, elements, xmlTable, maxIndentLevel)
				else
					xmlTable[#xmlTable + 1] = _encode(elements, encoding[4][1])
				end

				if indent and type(elements) == 'table' then
					xmlTable[#xmlTable + 1] = tabs
				end
				xmlTable[#xmlTable + 1] = '</' .. key .. '>'
				if indent then
					xmlTable[#xmlTable + 1] = '\n'
				end
			end
		end
	else
		for key, value in pairs(luaTable) do
			if type(value) == 'table' then
				for _, entry in ipairs(value) do
					xmlTable[#xmlTable + 1] = tabs .. '<' .. key

					local attributes = entry['@']
					if attributes then
						if not indent then
							for _, attrKey in ipairs(attributes) do
								xmlTable[#xmlTable + 1] = ' ' .. attrKey .. '="' .. attributes[attrKey] .. '"'
							end
						else
							for attrKey, attrValue in pairs(attributes) do
								if type(attrKey) == 'string' then
									xmlTable[#xmlTable + 1] = ' ' .. attrKey .. '="' .. attrValue .. '"'
								end
							end
						end
					end

					xmlTable[#xmlTable + 1] = '>'

					local elements = entry['#']
					if type(elements) == 'table' then
						if indent then
							xmlTable[#xmlTable + 1] = '\n'
						end
						xmlsave_recurse(indent and (indent + 1) or nil, elements, xmlTable, maxIndentLevel)
					else
						xmlTable[#xmlTable + 1] = _encode(elements, encoding[4][1]) 
					end

					if indent and type(elements) == 'table' then
						xmlTable[#xmlTable + 1] = tabs
					end
					xmlTable[#xmlTable + 1] = '</' .. key .. '>'
					if indent then
						xmlTable[#xmlTable + 1] = '\n'
					end
				end
			end
		end
	end
end


function xmlize(outFilename, luaTable, indent, maxIndentLevel)
	local xmlTable = {}
	xmlsave_recurse(indent, luaTable, xmlTable, maxIndentLevel)
	local outText = table.concat(xmlTable)
	if outFilename == ':string' then
		return outText
	else
		local file = io.open(outFilename, "wt")
		file:write(table.concat(xmlTable))
		file:close()
	end
end

