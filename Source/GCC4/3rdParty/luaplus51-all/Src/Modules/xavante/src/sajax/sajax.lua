-- $Id: sajax.lua,v 1.6 2009/03/16 22:53:10 carregal Exp $

local cgilua = cgilua
local table, string, os = table, string, os
local pairs, type, unpack, tostring, xpcall = pairs, type, unpack, tostring, xpcall

module ("sajax")

local export_list = {}
local request_uri

if xavante and xavante.httpd then
	request_uri = function (funcname)
		return cgilua.mkabsoluteurl (cgilua.mkurlpath (cgilua.servervariable"SCRIPT_NAME" .. "/" .. funcname.. "?rs=rs"))
	end
	
	function handle_client_request ()
		return false
	end
	
	local function register_path (funcname)
		return cgilua.mkurlpath (cgilua.servervariable"SCRIPT_NAME" .. "/" .. funcname)
	end
	
	local function handle_prefix (req, res)
		res.headers ["Expires"] = "Mon, 26 Jul 1997 05:00:00 GMT"    -- Date in the past
		res.headers ["Last-Modified"] =  os.date ("!%a, %d %b %Y %H:%M:%S GMT")
		-- always modified
		res.headers ["Cache-Control"] = "no-cache, must-revalidate"		-- HTTP/1.1
		res.headers ["Pragma"] =  "no-cache"							-- HTTP/1.0
		
		local qry = req.parsed_url.query or ""
		local rsargs = {}
		for rs_arg in string.gfind (qry, "rsargs%[%]=([^&=]*)&?") do
			table.insert (rsargs, rs_arg)
		end
		return rsargs
	end
	
	function export (funcname, func)
		export_list[funcname] = func
		
		xavante.httpd.addHandler (nil, register_path (funcname), function (req, res)
				local isok, result = xpcall (function ()
						return func (unpack (handle_prefix (req, res)))
					end, tostring)
				result = result or ""
				if isok then
					res.content = "+:" .. result
				else
					res.content = "-:" .. result
				end
				return res
			end)
	end
	
else

	request_uri = function (funcname)
		local uri = ""
		if string.find (uri, "?") then
			uri = uri .. "&rs=" .. cgilua.urlcode.escape (funcname)
		else
			uri = uri .. "?rs=" .. cgilua.urlcode.escape (funcname)
		end
		
		return uri
	end

	function handle_client_request ()
	
		if not cgilua.QUERY.rs then return end
		
		-- Bust cache in the head
		cgilua.header ("Expires", "Mon, 26 Jul 1997 05:00:00 GMT")    -- Date in the past
		cgilua.header ("Last-Modified", os.date ("!%a, %d %b %Y %H:%M:%S GMT"))
		-- always modified
		cgilua.header ("Cache-Control", "no-cache, must-revalidate")	-- HTTP/1.1
		cgilua.header ("Pragma", "no-cache")							-- HTTP/1.0
		
		local funcname = cgilua.QUERY.rs
		
		if not export_list[funcname] then
			cgilua.put (string.format ("-:%s not callable", funcname))
		else
			local func = export_list[funcname]
			local rsargs = cgilua.QUERY["rsargs[]"]
			local result
			
			if not rsargs then
				result = func()
			elseif type (rsargs) == "string" then
				result = func (rsargs)
			elseif type (rsargs) == "table" then
				result = func (unpack (rsargs))
			else
				return
			end
			
			cgilua.put ("+:")
			cgilua.put (result)
		end
		
		return true
	end
	
	function export (funcname, func)
		export_list[funcname] = func
	end

end
	

local function get_common_js ()
	return [[
		// remote scripting library
        // released under BSD license
		// (c) copyright 2005 modernmethod, inc
		var sajax_debug_mode = false;
		
		function sajax_debug(text) {
			if (sajax_debug_mode)
				alert("RSD: " + text)
		}
		function sajax_init_object() {
			sajax_debug("rs_init_object() called..")
			
			var A;
			try {
				A=new ActiveXObject("Msxml2.XMLHTTP");
			} catch (e) {
				try {
					A=new ActiveXObject("Microsoft.XMLHTTP");
				} catch (oc) {
					A=null;
				}
			}
			if(!A && typeof XMLHttpRequest != "undefined")
				A = new XMLHttpRequest();
			if (!A)
				sajax_debug("Could not create connection object.");
			return A;
		}
		function sajax_do_call(func_name, url, args) {
			var i, x, n;
			for (i = 0; i < args.length-1; i++) 
				url = url + "&rsargs[]=" + escape(args[i]);
			url = url + "&rsrnd=" + new Date().getTime();
			x = sajax_init_object();
			x.open("GET", url, true);
			x.onreadystatechange = function() {
				if (x.readyState != 4) 
					return;
				sajax_debug("received " + x.responseText);
				
				var status;
				var data;
				status = x.responseText.charAt(0);
				data = x.responseText.substring(2);
				if (status == "-") 
					alert("Error: " + data);
				else  
					args[args.length-1](data);
			}
			x.send(null);
			sajax_debug(func_name + " url = " + url);
			sajax_debug(func_name + " waiting..");
			delete x;
		}
		function sajax_call(func_name, script, callback, args) {
			var url = script + "?rs=" + func_name;
			args.push(callback);
			sajax_do_call(func_name, url, args);
		}
	]]
end

function show_common_js()
	cgilua.put (get_common_js ())
end


local function get_one_stub (funcname)
	local uri = request_uri (funcname)
	
	return (string.format ([[
		// wrapper for %s
		
		function x_%s() {
			// count args; build URL
			sajax_do_call ("%s", "%s", x_%s.arguments);
		}
		]], funcname, funcname, funcname, uri, funcname))
--[[
			var i, x, n;
			var url = "%s";
			var a = x_%s.arguments;
			for (i = 0; i < a.length-1; i++) 
				url = url + "&rsargs[]=" + escape(a[i]);
			url = url + "&rsrnd=" + new Date().getTime();
			x = rs_init_object();
			x.open("GET", url, true);
			x.onreadystatechange = function() {
				if (x.readyState != 4) 
					return;
				rs_debug("received " + x.responseText);
				
				var status;
				var data;
				status = x.responseText.charAt(0);
				data = x.responseText.substring(2);
				if (status == "-") 
					alert("Error: " + callback_n);
				else  
					a[a.length-1](data);
			}
			x.send(null);
			rs_debug("x_%s url = " + url);
			rs_debug("x_%s waiting..");
			delete x;
		}
	] ], funcname, funcname, uri, funcname, funcname, funcname))		--]]
end

local function show_one_stub (funcname)
	cgilua.put (get_one_stub (funcname))
end

local js_has_been_shown = false

function get_javascript ()
	local js = {}
	
	if not js_has_been_shown then
		table.insert (js, get_common_js ())
		js_has_been_shown = true
	end
	
	for fn,_ in pairs (export_list) do
		table.insert (js, get_one_stub (fn))
	end
	return (table.concat (js))
end

function show_javascript ()
	cgilua.put (get_javascript ())
end

function init ()
end


