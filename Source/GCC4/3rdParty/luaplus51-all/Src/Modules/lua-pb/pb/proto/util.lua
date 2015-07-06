-- Copyright (c) 2007 Humberto Saraiva Nazareno dos Anjos.
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.

-- Copied from module "leg.grammar" in "Leg - LPeg-powered Lua 5.1 grammar"

-- basic functions
local assert  = assert
local pairs   = pairs
local type    = type

-- imported modules
local lpeg = require 'lpeg'

-- imported functions
local P, V = lpeg.P, lpeg.V

-- module declaration
module(...)

function listOf(patt, sep)
	patt, sep = P(patt), P(sep)

	return patt * (sep * patt)^0
end

function complete (dest, orig)
	for rule, patt in pairs(orig) do
		if not dest[rule] then
			dest[rule] = patt
		end
	end

	return dest
end

function apply (grammar, rules, captures)
	if rules == nil then
		rules = {}
	elseif type(rules) ~= 'table' then
		rules = { rules }
	end

	complete(rules, grammar)

	if type(grammar[1]) == 'string' then
		rules[1] = V(grammar[1])
	end

	if captures ~= nil then
		assert(type(captures) == 'table', 'captures must be a table')

		for rule, cap in pairs(captures) do
			rules[rule] = rules[rule] / cap
		end
	end

	return rules
end
