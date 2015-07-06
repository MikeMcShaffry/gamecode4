-- Load the p4 module.
local p4api = require 'p4.p4api'

--*******************************************************************************
-- Copyright (c) 2007-2008, Perforce Software, Inc.  All rights reserved.
-- Portions Copyright (c) 1999, Mike Meyer. All rights reserved.
-- Portions Copyright (c) 2004-2007, Robert Cowham. All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--
-- 1.  Redistributions of source code must retain the above copyright
-- notice, this list of conditions and the following disclaimer.
--
-- 2.  Redistributions in binary form must reproduce the above copyright
-- notice, this list of conditions and the following disclaimer in the
-- documentation and/or other materials provided with the distribution.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
-- ARE DISCLAIMED. IN NO EVENT SHALL PERFORCE SOFTWARE, INC. BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
-- SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--*******************************************************************************

--
-- P4Integration objects hold details about the integrations that have
-- been performed on a particular revision. Used primarily with the
-- P4Revision class
--
local IntegrationMetatable = {}
IntegrationMetatable.__index = IntegrationMetatable

function IntegrationMetatable:__tostring()
	return string.format("Integration (how = %s, file = %s, srev = %s, erev = %s)", self.how, self.file, self.srev, self.erev)
end

function P4.Integration( how, file, srev, erev )
	local self = {}
    self.how = how
    self.file = file
    self.srev = srev
    self.erev = erev
    return setmetatable(self, IntegrationMetatable)
end

--
-- Each P4Revision object holds details about a particular revision
-- of a file. It may also contain the history of any integrations 
-- to/from the file
--

local RevisionMetatable = {}
RevisionMetatable.__index = RevisionMetatable

function RevisionMetatable:integration( how, file, srev, erev )
    local rec = P4.Integration( how, file, srev, erev )
    self.integrations[ #self.integrations + 1 ] = rec
    return rec
end

-- iterator over the collection calling a provided function
-- Python's poor man version of the Ruby block 
function RevisionMetatable:each_integration()
	local i = 0
	return function()
		i = i + 1
		return self.integrations[i]
	end
end

function RevisionMetatable:__tostring()
	return string.format("Revision (depotFile = %s, rev = %d, change = %s, action = %s, type = %s, time = %d, user = %s, client = %s, %d integrations)",
              self.depotFile, self.rev or -1, self.change or "", self.action or "", self.type or "",
              self.time or -1, self.user or "", self.client or "", #self.integrations)
end

function P4.Revision( depotFile )
	local self = {}
	self.depotFile = depotFile
    self.integrations = {}
    self.rev = nil
    self.change = nil
    self.action = nil
    self.type = nil
    self.time = nil
    self.user = nil
    self.client = nil
    self.desc = nil
    self.digest = nil
    self.fileSize = nil
    return setmetatable(self, RevisionMetatable)
end


local DepotFileMetatable =
{
	new_revision = function(self)
		local r = P4.Revision(self.depotFile)
		table.insert(self.revisions, r)
		return r
	end,
	
	each_revision = function(self)
		local i = 0
		return function()
			i = i + 1
			return self.revisions[i]
		end
	end,	

	__tostring = function(self)
		return string.format("DepotFile (depotFile = %s, %d revisions)", self.depotFile, #self.revisions )
	end
}
DepotFileMetatable.__index = DepotFileMetatable

--
-- Each DepotFile entry contains details about one depot file. 
-- 
function P4.DepotFile(name)
	return setmetatable(
		{
			depotFile = name,
			revisions = {},
		},
		DepotFileMetatable)
end



--
-- Resolver class used in p4.run_resolve()
--
-- The default simply checks that p4.input is set to sensible value
--
local defaultResolver =
{
	resolve = function(self, mergeInfo)
        if mergeInfo.merge_hint == "e" then
            print "Standard resolver encountered merge conflict, skipping resolve"
            return "s"
        else
            return mergeInfo.merge_hint
        end
    end
}

	

local oldP4 = p4api.P4
function p4api.P4()
	local out = oldP4()
	local out_mt = getmetatable(out)

	out_mt.__getattr = function(self, key)
		if key:sub(1, 4) == 'run_' then
			return function(self, ...) return self:run(key:sub(5), ...) end
        elseif key:sub(1, 7) == 'delete_' then
			return function(self, ...) return self:run(key:sub(8), '-d', ...) end
        elseif key:sub(1, 6) == 'fetch_' then
			return function(self, ...) return self:run(key:sub(7), '-o', ...)[1] end
        elseif key:sub(1, 5) == 'save_' then
			return function(self, ...)
				local args = {...}
				self.input = args[1]
				table.remove(args, 1)
				return self:run(key:sub(6), '-i', args)
			end
        elseif key:sub(1, 6) == 'parse_' then
			return function(self, ...) return self:parse_spec(key:sub(7), ...) end
        elseif key:sub(1, 6) == 'format_' then
			return function(self, ...) return self:format_spec(key:sub(8), ...) end
		end
	end

	out_mt.last_gui_settings = function(self)
		for line in io.lines(os.getenv("HOMEDRIVE") .. os.getenv("HOMEPATH") .. "/.p4qt/settings") do
			local connection = line:match('^PrevConnection=(.+)')
			if connection then
				connection = connection:gsub('(%%)(..)',
						function(value1, value2) return string.char(tonumber(value2, 16)) end)
				local settings = {}
				for setting in (connection .. ', '):gmatch('(.-), ') do
					settings[#settings + 1] = setting
				end
				if #settings == 3 then
					self.port, self.user, self.client = unpack(settings)
					return true
				else
					return false
				end
			end
		end
	end
	
    --
    -- run_filelog: convert "p4 filelog" responses into objects with useful
    --              methods
    --
    -- Requires tagged output to be of any real use. If tagged output it not 
    -- enabled then you just get the raw data back
    --
    out_mt.run_filelog = function( self, ... )
		local raw = self:run( 'filelog', ... )
		if not self.tagged then			
			-- untagged mode returns simple strings, which breaks the code below
			return raw
		end

		local result = {}
		for _, h in ipairs(raw) do
			local r
			local df
			if type(h) == 'table' then
				df = P4.DepotFile( h.depotFile )
				for n, rev in ipairs(h.rev) do repeat
					-- Create a new revision of this file ready for populating
					r = df:new_revision()
					-- Populate the base attributes of each revision
					r.rev = rev
					r.change = h.change[ n ]
					r.action = h.action[ n ] 
					r.type = h.type[ n ]
					r.time = h.time[ n ]
					r.user = h.user[ n ]
					r.client = h.client[ n ]
					r.desc = h.desc[ n ]
					if h.digest then
						r.digest = h.digest[ n ]
					end
					if h.fileSize then
						r.fileSize = h.fileSize[ n ]
					end

					-- Now if there are any integration records for this revision,
					-- add them in too
					if not h.how  or  n > #h.how  or  not h.how[n] then
						break
					else
						for m, how in ipairs(h.how[ n ]) do
							file = h.file[ n ][ m ]
							srev = h.srev[ n ][ m ]:gsub('^#', '')
							erev = h.erev[ n ][ m ]:gsub('^#', '')
                          
							if srev == "none" then
								srev = 0
							else
								srev = tonumber( srev )
							end
                        
							if erev == "none" then
								erev = 0
							else
								erev = tonumber( erev )
							end
                              
							r:integration( how, file, srev, erev )
						end
					end
				until true end
			else
				r = h
			end
          
			result[#result + 1] = df
		end
		
		return result
	end



	-- Simple interface to make login easier
    out_mt.run_login = function(self, ...)
        self.input = self.password
        return self:run("login", ...)
	end

	-- Simple interface to allow setting of the password
	out_mt.run_password = function(self, oldpass, newpass)
        if oldpass and #oldpass > 0 then
            self.input = { oldpass, newpass, newpass }
        else
            self.input = { newpass, newpass }
		end

        return self:run( "password" )
	end

	-- Simplified submit - if any arguments is a dict, assume it to be the changeform
    out_mt.run_submit = function(self, ...)
		local nargs = {...}
        for n, arg in ipairs(nargs) do
			if type(arg) == "table" then
                self.input = arg
                table.remove(nargs, n)
				table.insert(nargs, '-i')
                break
			end
		end
        return self:run("submit", nargs)
	end

    out_mt.run_print = function(self, ...)
		local raw, err = self:run('print', ...)
		if raw then
			table.remove(raw, 1)
		end
		return raw, err
	end
	    
    out_mt.run_resolve = function(self, ...)
		local args = {...}
		local myResolver = defaultResolver
		for n, arg in ipairs(args) do
			if type(arg) == 'table' then
				if arg.resolver then
					myResolver = arg.resolver
					break
				end
			end
		end

        local savedResolver = self.resolver
        self.resolver = myResolver
        result, err = self:run("resolve", args)
        self.resolver = savedResolver
        
        return result, err
    end
    
	return out
end
