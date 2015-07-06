-- Prototype-based objects

module ("object", package.seeall)

require "table_ext"


-- Usage:

-- Create an object/class:
--   object/class = prototype {value, ...; field = value ...}
--   An object's metatable is itself.
--   In the initialiser, unnamed values are assigned to the fields
--   given by _init (assuming the default _clone).
--   Private fields and methods start with "_"

-- Access an object field: object.field
-- Call an object method: object:method (...)
-- Call a class method: Class.method (object, ...)

-- Add a field: object.field = x
-- Add a method: function object:method (...) ... end


-- Root object
_G.Object = {
  -- List of fields to be initialised by the
  -- constructor: assuming the default _clone, the
  -- numbered values in an object constructor are
  -- assigned to the fields given in _init
  _init = {},

  -- @func _clone: Object constructor
  --   @param values: initial values for fields in
  --   _init
  -- @returns
  --   @param object: new object
  _clone = function (self, values)
             local object = table.merge (self, table.rearrange (self._init, values))
             return setmetatable (object, object)
           end,

  -- @func __call: Sugar instance creation
  __call = function (...)
             -- First (...) gets first element of list
             return (...)._clone (...)
           end,
}
setmetatable (Object, Object)
