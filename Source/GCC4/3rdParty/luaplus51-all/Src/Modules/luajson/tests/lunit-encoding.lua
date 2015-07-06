local json = require("json")
local lunit = require("lunit")

module("lunit-encoding", lunit.testcase, package.seeall)

function test_cloned_array_sibling()
	local obj = {}
	assert_pass(function()
		json.encode({obj, obj})
	end)
end

function test_cloned_object_sibling()
	local obj = {}
	assert_pass(function()
		json.encode({x = obj, y = obj})
	end)
end

function test_cloned_array_deep_sibling()
	local obj = {}
	assert_pass(function()
		json.encode({
			{obj}, {obj}
		})
	end)
end

function test_cloned_array_multilevel_sibling()
	local obj = {}
	assert_pass(function()
		json.encode({
			{obj, {obj}}
		})
	end)
end

function test_recursive_object()
	local obj = {}
	obj.x = obj
	assert_error(function()
		json.encode(obj)
	end)
end

function test_recursive_array()
	local obj = {}
	obj[1] = obj
	assert_error(function()
		json.encode(obj)
	end)
end
