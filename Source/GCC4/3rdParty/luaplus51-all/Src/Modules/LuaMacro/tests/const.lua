local macro = require 'macro'
macro.define ('const',function(get)
   get()
   local vars,values = get:names '=',get:list '\n'
   for i,name in ipairs(vars) do
      macro.assert(values[i],'each constant must be assigned!')
      macro.define_scoped(name,tostring(values[i]))
   end
end)
