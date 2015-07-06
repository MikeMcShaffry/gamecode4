-- Given an .ini file, convert it into a Lua table.
module(..., package.seeall)

local lpeg = require "lpeg"
local P, R, S, C, Carg, Cc, Cf, Cg, Ct = lpeg.P, lpeg.R, lpeg.S, lpeg.C, lpeg.Carg, lpeg.Cc, lpeg.Cf, lpeg.Cg, lpeg.Ct

local whitespace = S(" \t\r\n\f")^1
local space = S(' \t')
local eol = space^0 * S('\r\n')
local comment = space^0 * P("#") * (1 - S('\r\n\f'))^0
local key = (1 - whitespace - P('='))^1
local value = ((1 - eol - comment)^0)
local key_value = Cg(C(key) * space^0 * P('=') * space^0 * C(value))
local list = Cf(Ct("") * (comment + key_value + whitespace)^0, rawset)
local section = Cg((P('[') * C((1 - P(']'))^1) * P(']')) * list)
local tokens = Cf(Ct("") * (comment + section + whitespace)^0, rawset)

-- public interface
getmetatable(getfenv(1)).__call = function(self, input)
   assert(type(input) == 'string', 'bad argument #1 (expected string)')
   return lpeg.match(tokens, input)
end
