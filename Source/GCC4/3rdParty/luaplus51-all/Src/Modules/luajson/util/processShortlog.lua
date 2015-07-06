#!/usr/bin/env lua
local io = require("io")
local table = require("table")
local authors = {}

local currentAuthor

local function pushValue(t, group, value)
	local items = t[group] or {}
	t[group] = items
	items[#items + 1] = value
end
for line in io.lines() do
	local author = line:match("^(.- %(%d*%):)$")
	line = line:match("^%s*(.-)%s*$")
	if author then
		currentAuthor = authors[author] or {}
		authors[author] = currentAuthor
	elseif currentAuthor and #line > 0 then
		local msg = line:gsub("^%s*","")
		local group, data = msg:match("^([^:]*[%s]*:)[%s]*(.*)$")
		if not group then
			pushValue(currentAuthor, "-ungrouped-", msg)
		else
			pushValue(currentAuthor, group, data)
		end
	end
end
for author, groups in pairs(authors) do
	print(author)
	-- SORT GROUPS
	local sorted = {}
	for group, groupValues in pairs(groups) do
		sorted[#sorted + 1] = {group, groupValues}
	end
	table.sort(sorted, function(a,b) return a[1] < b[1] end)
	for _, gv in ipairs(sorted) do
		local group, groupValues = unpack(gv)
		print("\t" .. group)
		for _, entry in ipairs(groupValues) do
			print("\t\t" .. entry)
		end
	end
end
