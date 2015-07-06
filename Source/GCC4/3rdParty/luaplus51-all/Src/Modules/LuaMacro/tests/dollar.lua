local macro = require 'macro'

macro.define('$',function(get)
   local t,v = get()
   if t == 'iden' then
      return 'os.getenv("'..v..'")'
   elseif t == '(' then
      local rest = get:upto ')'
      return 'os.execute("'..tostring(rest)..'")'
   end
end)
