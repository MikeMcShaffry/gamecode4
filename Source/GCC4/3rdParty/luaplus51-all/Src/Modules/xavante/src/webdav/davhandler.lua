-----------------------------------------------------------------------------
-- Xavante webDAV handler
-- Author: Javier Guerra
-- Copyright (c) 2005 Javier Guerra
-----------------------------------------------------------------------------
require "lxp.lom"
require "socket.url"

local url = socket.url

module (arg and arg[1])

-- returns a copy of the string without any
-- leading or trailing whitespace
local function trim (s)	
	return (string.gsub (string.gsub (s, "^%s*", ""), "%s*$", ""))
end

-- reverts a LOM-style attribute array to XML form
local function attrstostr (a)
	local out = {}
	for i,attr in ipairs (a) do
		table.insert (out, string.format ('%s = "%s"', attr,  a[attr]))
	end
	return table.concat (out, " ")
end

-- outputs a LOM XML object back in XML text form
-- params:
-- 		x: LOM XML object
--		of: output function, uses print() if not given
local function lomtoxml (x, of)
	if type (x) == "string" then
		return of (x)
	end
	of = of or print
	local attrs = ""
	-- does it have any attributes?
	if x.attr [1] then
		attrs = " " .. attrstostr (x.attr)
	end
	
	-- is there any content?
	if x[1] then
		of ("<" .. x.tag .. attrs .. ">")
		for i in ipairs (x) do
			lomtoxml (x[i], of)
		end
		of ("</".. x.tag .. ">")
	else
		of ("<" .. x.tag .. attrs .. "/>")
	end
end

-- gets request content, and parses it as XML
-- returns LOM object
local function req_xml (req)
	local sz = req.headers ["content-length"]
	local indata
	if sz then
		indata = req.socket:receive (sz)
	else
		indata = function () return req.socket:receive () end
	end
	return lxp.lom.parse (indata)
end

-- expands namespace tags in-situ
-- returns the same LOM object after processing
local function xml_ns (x, dict)
	if not x then return end
	if type (x) == "string" then return x end
	
	dict = dict or {}
	
	-- adds any new namespace to the dictionary
	for i,attr_name in ipairs (x.attr) do
		-- default namespace
		if attr_name == "xmlns" then
			dict [""] = x.attr [attr_name]
		end
		-- named namespaces
		local _,_, ns_var = string.find (attr_name, "xmlns:(.*)$")
		if ns_var then
			dict [ns_var] = x.attr [attr_name]
		end
	end
	
	-- modifies this node's tag
	local _,_, ns = string.find (x.tag, "^(.*):")
	if ns and dict [ns] then
		local pat = string.format ("^%s:", ns)
		x.tag = string.gsub (x.tag, pat, dict[ns])
	end
	if not ns and dict [""] then
		x.tag = dict [""] .. x.tag
	end
	
	-- recurses to child nodes
	for _, sub in ipairs (x) do
		xml_ns (sub, dict)
	end

	return x
end

-- iterator for traversing only childs in a LOM object
-- whith a given tagname.
local function lomChildsByTagName (x, tagname)
	if not x then
		return function () return nil end
	end

	local function gen (x)
		for _,elem in ipairs (x) do
			if type(elem) == "table" then
				if elem.tag and elem.tag == tagname then
					coroutine.yield (elem)
				end
			end
		end
	end

	return coroutine.wrap (function () gen (x) end)
end

-- iterator for traversing all elements in a LOM object
-- whith a given tagname (at any depth).
local function lomElementsByTagName (x, tagname)
	if not x then
		return function () return nil end
	end

	local function gen (x)
		for _,elem in ipairs (x) do
			if type(elem) == "table" then
				if elem.tag and elem.tag == tagname then
					coroutine.yield (elem)
				end
				gen (elem)
			end
		end
	end

	return coroutine.wrap (function () gen (x) end)
end

-- iterates on the childs of a LOM node
-- use as:
-- for subnode, tagname in lomChilds (node) do ... end
local function lomChilds (x)
	if not x then
		return function () return nil end
	end

	local function gen ()
		for _, elem in ipairs (x) do
			if type (elem) == "table" and elem.tag then
				coroutine.yield (elem, elem.tag)
			end
		end
	end
	return coroutine.wrap (gen)
end

-- returns all the text content of a LOM node
local function lomContent (x)
	if not x then
		return ""
	end

	local out = {}
	local function of (s)
		table.insert (out, s)
	end
	for _, child in ipairs (x) do
		lomtoxml (child, of)
	end
	return table.concat (out)
end

-- returns a table member of a table, creates it if needed
-- params:
--		t: table
--		k: key
local function maketabletable (t,k)
	t[k] = t[k] or {}
	return t[k]
end

-- if a tagname has inconvenient characters
-- replaces part of if with a (possibly new)
-- namespace reference
-- params:
--		ns:	namespace dictionary
--		name:	tagname to reduce
-- returns new tagname, ns is modified in-place if needed
local function reducename (ns, name)
	if string.find (name, "[:/?#]") then
		local _,_,pfx,sfx = string.find (name, "(.*%W)(%w+)")
		local n = 0
		for k,v in pairs (ns) do
			n = n+1
			if v == pfx then
				return string.format ("%s:%s", k, sfx)
			end
		end
		local newns = "lm"..n
		ns [newns] = pfx
		return string.format ("%s:%s", newns, sfx)
	else
		return name
	end
end

-- returns a XML attr string encoding a namespace dictionary
local function nsattr (ns)
	local attr = {}
	for k,v in pairs (ns) do
		table.insert (attr, string.format ([[xmlns:%s="%s"]], k, v))
	end
	return table.concat (attr, " ")
end

local function addProp (outtable, propname, propval)
	if propval then
		local propentry = maketabletable (outtable, "HTTP/1.1 200 OK")
		if propval == "" then
			table.insert (propentry, string.format ([[<%s/>]], propname))
		else
			table.insert (propentry, 
				string.format ([[<%s>%s</%s>]], 
					propname, propval, propname))
		end
	else
		local propentry = maketabletable (outtable, "HTTP/1.1 404 NotFound")
		table.insert (propentry, string.format ([[<%s/>]], propname))
	end
end

local function addPropstat (outtable, propstat)
	local codes = {}
	for stat in pairs (propstat) do
		table.insert (codes, stat)
	end
	table.sort (codes)
	for _, stat in ipairs (codes) do
		props = propstat [stat]
		if props then
			table.insert (outtable, [[<D:propstat>]])
			table.insert (outtable, [[<D:prop>]])
			for _,prop in ipairs (props) do
				table.insert (outtable, prop)
			end
			table.insert (outtable, [[</D:prop>]])
			table.insert (outtable, string.format ([[<D:status>%s</D:status>]], stat))
			table.insert (outtable, [[</D:propstat>]])
		end
	end
end

local function dav_propfind (req, res, repos_b, props_b)
	res.statusline = "HTTP/1.1 207 Multi-Status\r\n"
	res.headers ["Content-Type"] = 'text/xml; charset="utf-8"'

	local depth = req.headers.depth
	local path = req.relpath
	local data = xml_ns (req_xml (req))

--	print ("path:", path, req.relpath)
--	print ("depth:", depth)

--	print ("como xml:") lomtoxml (data) print ()


	local resource_q = repos_b:getResource (req.match, path)
	if not resource_q then
		return httpd.err_404 (req, res)
	end

	local content = {}

	table.insert (content, [[<?xml version="1.0" encoding="utf-8" ?>]])
	table.insert (content, [[<D:multistatus xmlns:D="DAV:">]])
	local namespace = {}

	assert (data.tag == "DAV:propfind")
	for resource in resource_q:getItems (depth) do
	--	print ("resource:", resource.path)
		local propstat = {}
		namespace.D="DAV:"

		for findtype, findtype_tn in lomChilds (data) do

			if findtype_tn == "DAV:prop" then
				for _, propname in lomChilds (findtype) do
					local propval = resource:getProp (propname)
					if not propval and props_b then
						propval = props_b:getProp (req.relpath, propname)
					end
					local shortname = reducename (namespace, propname)
					addProp (propstat, shortname, propval)
				end

			elseif findtype_tn == "DAV:propname" then
				for propname in resource:getPropNames () do
					local shortname = reducename (namespace, propname)
					addProp (propstat, shortname, "")
				end
				if props_b then
					for propname in props_b:getPropNames (req.relpath) do
						local shortname = reducename (namespace, propname)
						addProp (propstat, shortname, "")
					end
				end

			elseif findtype_tn == "DAV:allprop" then
				for propname in resource:getPropNames () do
					local shortname = reducename (namespace, propname)
					local propval =  resource:getProp (propname)
					addProp (propstat, shortname, propval)
				end
				if props_b then
					for propname in props_b:getPropNames (req.relpath) do
						local shortname = reducename (namespace, propname)
						local propval = props_b:getProp (req.relpath, propname)
						addProp (propstat, shortname, propval)
					end
				end
			end
		end
	
		namespace.D = nil

		table.insert (content, string.format ([[<D:response %s>]], nsattr (namespace)))
		table.insert (content, string.format ([[<D:href>%s</D:href>]], resource:getHRef()))
		addPropstat (content, propstat)
		table.insert (content, [[</D:response>]])
	end
	
	table.insert (content, [[</D:multistatus>]])

--	for _,l in ipairs (content) do print (l) end

	res.content = content
	return res
end

local function dav_proppatch (req, res, repos_b, props_b)
	res.statusline = "HTTP/1.1 207 Multi-Status\r\n"
	res.headers ["Content-Type"] = 'text/xml; charset="utf-8"'

	local path = req.relpath
	local data = xml_ns (req_xml (req))
--	print ("como xml:") lomtoxml (data) print ()

	local resource = repos_b:getResource (req.match, path)
	if not resource then
		return httpd.err_404 (req, res)
	end

	local content = {}

	table.insert (content, [[<?xml version="1.0" encoding="utf-8" ?>]])
	local namespace = {D="DAV:"}
	local propstat = {}

	assert (data.tag == "DAV:propertyupdate")
	for cmd_node, cmd_name in lomChilds (data) do
		for nd in lomChildsByTagName (cmd_node, "DAV:prop") do
			local status = nil
			for prop_node, propname in lomChilds (nd) do
				if cmd_name == "DAV:set" then
					props_b:setProp (path, propname, lomContent (prop_node))
					status = "HTTP/1.1 200 OK"
				elseif cmd_name == "DAV:remove" then
					props_b:setProp (path, propname, nil)
					status = "HTTP/1.1 200 OK"
				end

				if status then
					local propentry = maketabletable (content, status)
					table.insert (propentry, string.format ([[<%s/>]], reducename (namespace, propname)))
				end
			end
		end
	end

	table.insert (content, string.format ([[<D:multistatus %s>]], nsattr (namespace)))
	table.insert (content, [[<D:response>]])
	table.insert (content, string.format ([[<D:href>%s</D:href>]], resource:getHRef()))
	addPropstat (content, propstat)
	table.insert (content, [[</D:response>]])
	table.insert (content, [[</D:multistatus>]])

	res.content = content
	return res
end

local function dav_options (req, res, repos_b, props_b)
	res.headers ["DAV"] = "1,2"
	res.content = ""
	return res
end

local function dav_get (req, res, repos_b, props_b)
	local resource = repos_b:getResource (req.match, req.relpath)
	if not resource then
		return httpd.err_404 (req, res)
	end

	res.headers ["Content-Type"] = resource:getContentType ()
	res.headers ["Content-Length"] = resource:getContentSize () or 0

	res:send_headers ()
	for block in resource:getContentData () do
		res:send_data (block)
	end
	return res
end

local function dav_put (req, res, repos_b)
	local path = req.relpath
	local resource = repos_b:getResource (req.match, path)
		or repos_b:createResource (req.match, path)

	if req.headers["content-range"] then
		return httpd.err_405 (req, res)
	end

	local contentlength = assert (req.headers ["content-length"]) + 0
	local bsz
	while contentlength > 0 do
		if contentlength > 4096 then bsz = 4096 else bsz = contentlength end
		resource:addContentData (req.socket:receive (bsz))
		contentlength = contentlength - bsz
	end
	res:send_headers ()
	return res
end

local function dav_mkcol (req, res, repos_b)
	local path = req.relpath
	local resource = repos_b:getResource (req.match, path)
	if resource then
		res.statusline = "HTTP/1.1 405 Method Not Allowed\r\n"
		res.content = ""
		return res
	end
	local done, err = repos_b:createCollection (req.match, path)
	if done then
		res.statusline = "HTTP/1.1 201 Created\r\n"
	else
		res.statusline = "HTTP/1.1 403 Forbidden\r\n"
	end
	res.content = ""
	return res
end

local function dav_delete (req, res, repos_b, props_b)
	local path = req.relpath
	local resource = repos_b:getResource (req.match, path)
	if not resource then
		return http.err_404
	end
	
	-- NOTE: this should iterate depth-first
	for r in resource:getItems ("Infinity") do
		local ok, err = resource:delete ()
		if not ok then
			res.statusline = "HTTP/1.1 207 Multi-Status\r\n"
			res.content = string.format ([[
				<?xml version="1.0" encoding="utf-8" ?>
				<d:multistatus xmlns:d="DAV:">
					<d:response>
						<d:href>%s</d:href>
						<d:status>%s</d:status>
					</d:response>
				</d:multistatus>]], resource:getHRef(), err)
			return res
		end
		props_b:delete (resource:getPath ())		-- TODO
	end
	res.statusline = "HTTP/1.1 204 No Content\r\n"
--[[
	local done, err = resource:delete ()
	if done then
		res.statusline = "HTTP/1.1 204 No Content\r\n"
		props_b:delete (path)
	else
		res.statusline = "HTTP/1.1 403 Forbidden\r\n"
	end
--]]
	res.content = ""
	return res
end

local function dav_lock (req, res, repos_b)
	local data = xml_ns (req_xml (req))
	print ("como xml:") lomtoxml (data) print ()
	return res
end

local dav_cmd_dispatch = {
	PROPFIND = dav_propfind,
	PROPPATCH = dav_proppatch,
	OPTIONS = dav_options,
	GET = dav_get,
	PUT = dav_put,
	MKCOL = dav_mkcol,
	DELETE = dav_delete,
	LOCK = dav_lock,
}

function makeHandler (repos_b, props_b)
	return function (req, res)
--		print (req.cmd_mth, req.parsed_url.path)
--		for k,v in pairs (req.headers) do print (k,v) end

		local dav_handler = dav_cmd_dispatch [req.cmd_mth]
		if dav_handler then
			return dav_handler (req, res, repos_b, props_b)
		end
	end
end
