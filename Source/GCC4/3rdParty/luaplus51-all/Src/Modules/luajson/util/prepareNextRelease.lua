#!/usr/bin/env lua
local io = require("io")
local from, tover = ...
if not from or not tover then
	print("Format: prepareNextRelease.lua <from-tag> <tover-tag>")
	return
end

local f = assert(io.open("docs/ReleaseNotes-" .. tover .. ".txt", "w"))
local headLine = "luajson v" .. tover .. " Release Notes"
f:write(headLine, "\n", ("="):rep(#headLine), "\n\n")

f:write([[
User Visible Changes
--------------------

Plans for next release
----------------------

]])
local tailLine = "Updates since " .. from
f:write(tailLine, "\n", ("="):rep(#tailLine), "\n\n")

local data = assert(io.popen("git shortlog " .. from .. "..HEAD | util/processShortlog.lua", "r"))
local tail = data:read("*a")
data:close()

f:write(tail)
f:close()
