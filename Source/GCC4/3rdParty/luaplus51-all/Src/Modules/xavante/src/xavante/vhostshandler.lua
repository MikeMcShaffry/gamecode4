-----------------------------------------------------------------------------
-- Xavante virtual hosts handler
--
-- Authors: Javier Guerra
-- Copyright (c) 2006-2007 Kepler Project
--
-- $Id: vhostshandler.lua,v 1.3 2007/08/20 22:20:44 carregal Exp $
-----------------------------------------------------------------------------

function xavante.vhostshandler (vhosts)
	return function (req, res)
		local h = vhosts [req.headers.host] or vhosts [""]
		return h (req, res)
	end
end
