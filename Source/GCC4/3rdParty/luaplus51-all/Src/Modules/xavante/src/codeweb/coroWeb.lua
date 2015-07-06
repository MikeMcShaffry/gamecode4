-----------------------------------------------------------------------------
-- webThreads: Xavante resumable handlers framework
--
-- Author: Javier Guerra
-- Copyright (c) 2005 Kepler Project
-----------------------------------------------------------------------------

local session = require "xavante.session"

module (arg and arg[1])

-- original coroutine functions
local o_resume, o_yield = coroutine.resume, coroutine.yield

-- creates a resume/yield pair
local function _ortoroutines (err)
	err = err or error
	local _tag = {}
	return
		function (co, ...)
			local r,sts
			repeat
				r = { o_resume (co, unpack (arg)) }
				sts = coroutine.status (co)
				
				if not r[1] then err (r[2]) end
				table.remove (r,1)
				
				if r[1] ~= _tag and sts == "suspended" then
					arg = { o_yield (unpack (r)) }
				end
			until r[1] == _tag or sts == "dead"
			table.remove (r,1)
			return unpack (r)
		end,
		function (...)
			return o_yield (_tag, unpack (arg))
		end
end

local function _error (res, err)
	res:send_data ("<h1>error:"..err.."</h1>")
end


resume, yield = _ortoroutines ()

--
-- creates a xavante handler
-- params:
--		name (string) : session name to use
--		h (function) :	thread to handle
--
function handler (name, h)
	return function (req, res)
		local sess = session.open (req, res, name)
		sess.coHandler = sess.coHandler or coroutine.create (h)
		
		resume (sess.coHandler, req, res)
		
		if coroutine.status (sess.coHandler) == "dead" then
			session.close (req, res, name)
		end
		
		return res
	end
end

--
-- gets user actions as events
-- params:
--		in_req (table) :	'req' parameter
--		sh_t (table) :		namesubhandlers
function event (in_req, sh_t, get_all)
	local req, res
	local subH, ret
	repeat
		req, res = yield ()
		subH = sh_t [req.relpath]
		if subH and type (subH) == "function" then
			ret = subH (req, res)

		end
		if ret then
			xavante.httpd.redirect (res, ret)
			
		elseif ret == "refresh" then
			xavante.httpd.redirect (res, in_req.parsed_url.path)
			req, res = yield ()
		end
	until not subH or get_all or ret == "refresh"
	return req, res
end