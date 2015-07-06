-- @module base
-- Adds to the existing global functions

module ("base", package.seeall)

require "table_ext"
--require "list" FIXME: sort out op table
require "string_ext"
--require "io_ext" FIXME: allow loops


-- @func metamethod: Return given metamethod, if any, or nil
--   @param x: object to get metamethod of
--   @param n: name of metamethod to get
-- @returns
--   @param m: metamethod function or nil if no metamethod or not a
--     function
function _G.metamethod (x, n)
  local _, m = pcall (function (x)
                        return getmetatable (x)[n]
                      end,
                      x)
  if type (m) ~= "function" then
    m = nil
  end
  return m
end

-- @func render: Turn tables into strings with recursion detection
-- N.B. Functions calling render should not recurse, or recursion
-- detection will not work
--   @param x: object to convert to string
--   @param open: open table renderer
--     @t: table
--   @returns
--     @s: open table string
--   @param close: close table renderer
--     @t: table
--   @returns
--     @s: close table string
--   @param elem: element renderer
--     @e: element
--   @returns
--     @s: element string
--   @param pair: pair renderer
--     N.B. this function should not try to render i and v, or treat
--     them recursively
--     @t: table
--     @i: index
--     @v: value
--     @is: index string
--     @vs: value string
--   @returns
--     @s: element string
--   @param sep: separator renderer
--     @t: table
--     @i: preceding index (nil on first call)
--     @v: preceding value (nil on first call)
--     @j: following index (nil on last call)
--     @w: following value (nil on last call)
--   @returns
--     @s: separator string
-- @returns
--   @param s: string representation
function _G.render (x, open, close, elem, pair, sep, roots)
  local function stopRoots (x)
    if roots[x] then
      return roots[x]
    else
      return render (x, open, close, elem, pair, sep, table.clone (roots))
    end
  end
  roots = roots or {}
  local s
  if type (x) ~= "table" or metamethod (x, "__tostring") then
    s = elem (x)
  else
    s = open (x)
    roots[x] = elem (x)
    local i, v = nil, nil
    for j, w in pairs (x) do
      s = s .. sep (x, i, v, j, w) .. pair (x, j, w, stopRoots (j), stopRoots (w))
      i, v = j, w
    end
    s = s .. sep(x, i, v, nil, nil) .. close (x)
  end
  return s
end

-- @func tostring: Extend tostring to work better on tables
--   @param x: object to convert to string
-- @returns
--   @param s: string representation
local _tostring = tostring
function _G.tostring (x)
  return render (x,
                 function () return "{" end,
                 function () return "}" end,
                 _tostring,
                 function (t, _, _, i, v)
                   return i .. "=" .. v
                 end,
                 function (_, i, _, j)
                   if i and j then
                     return ","
                   end
                   return ""
                 end)
end

-- @func print: Make print use tostring, so that improvements to tostring
-- are picked up
--   @param arg: objects to print
local _print = print
function _G.print (...)
  local arg = {...}
  for i = 1, select ("#", ...) do
    arg[i] = tostring (arg[i])
  end
  _print (...)
end

-- @func prettytostring: pretty-print a table
--   @t: table to print
--   @indent: indent between levels ["\t"]
--   @spacing: space before every line
-- @returns
--   @s: pretty-printed string
function _G.prettytostring (t, indent, spacing)
  indent = indent or "\t"
  spacing = spacing or ""
  return render (t,
                 function ()
                   local s = spacing .. "{"
                   spacing = spacing .. indent
                   return s
                 end,
                 function ()
                   spacing = string.gsub (spacing, indent .. "$", "")
                   return spacing .. "}"
                 end,
                 function (x)
                   if type (x) == "string" then
                     return string.format ("%q", x)
                   else
                     return tostring (x)
                   end
                 end,
                 function (x, i, v, is, vs)
                   local s = spacing .. "["
                   if type (i) == "table" then
                     s = s .. "\n"
                   end
                   s = s .. is
                   if type (i) == "table" then
                     s = s .. "\n"
                   end
                   s = s .. "] ="
                   if type (v) == "table" then
                     s = s .. "\n"
                   else
                     s = s .. " "
                   end
                   s = s .. vs
                   return s
                 end,
                 function (_, i)
                   local s = "\n"
                   if i then
                     s = "," .. s
                   end
                   return s
                 end)
end

-- @func totable: Turn an object into a table according to __totable
-- metamethod
--   @param x: object to turn into a table
-- @returns
--   @param t: table or nil
function _G.totable (x)
  local m = metamethod (x, "__totable")
  if m then
    return m (x)
  elseif type (x) == "table" then
    return x
  else
    return nil
  end
end

-- @func pickle: Convert a value to a string
-- The string can be passed to dostring to retrieve the value
-- TODO: Make it work for recursive tables
--   @param x: object to pickle
-- @returns
--   @param s: string such that eval (s) is the same value as x
function _G.pickle (x)
  if type (x) == "string" then
    return string.format ("%q", x)
  elseif type (x) == "number" or type (x) == "boolean" or
    type (x) == "nil" then
    return tostring (x)
  else
    x = totable (x) or x
    if type (x) == "table" then
      local s, sep = "{", ""
      for i, v in pairs (x) do
        s = s .. sep .. "[" .. pickle (i) .. "]=" .. pickle (v)
        sep = ","
      end
      s = s .. "}"
      return s
    else
      die ("cannot pickle " .. tostring (x))
    end
  end
end

-- @func id: Identity
--   @param ...
-- @returns
--   @param ...: the arguments passed to the function
function _G.id (...)
  return ...
end

-- @func pack: Turn a tuple into a list
--   @param ...: tuple
-- @returns
--   @param l: list
function _G.pack (...)
  return {...}
end

-- @func bind: Partially apply a function
--   @param f: function to apply partially
--   @param a1 ... an: arguments to bind
-- @returns
--   @param g: function with ai already bound
function _G.bind (f, ...)
  local fix = {...}
  return function (...)
           return f (unpack (list.concat (fix, {...})))
         end
end

-- @func curry: Curry a function
--   @param f: function to curry
--   @param n: number of arguments
-- @returns
--   @param g: curried version of f
function _G.curry (f, n)
  if n <= 1 then
    return f
  else
    return function (x)
             return curry (bind (f, x), n - 1)
           end
  end
end

-- @func compose: Compose functions
--   @param f1 ... fn: functions to compose
-- @returns
--   @param g: composition of f1 ... fn
--     @param args: arguments
--   @returns
--     @param f1 (...fn (args)...)
function _G.compose (...)
  local arg = {...}
  local fns, n = arg, #arg
  return function (...)
           local arg = {...}
           for i = n, 1, -1 do
             arg = {fns[i] (unpack (arg))}
           end
           return unpack (arg)
         end
end

-- @func eval: Evaluate a string
--   @param s: string
-- @returns
--   @param v: value of string
function _G.eval (s)
  return loadstring ("return " .. s)()
end

-- @func ripairs: An iterator like ipairs, but in reverse
--   @param t: table to iterate over
-- @returns
--   @param f: iterator function
--     @param t: table
--     @param n: index
--   @returns
--     @param i: index (n - 1)
--     @param v: value (t[n - 1])
--   @param t: the table, as above
--   @param n: #t + 1
function _G.ripairs (t)
  return function (t, n)
           n = n - 1
           if n > 0 then
             return n, t[n]
           end
         end,
  t, #t + 1
end

-- @func collect: collect the results of an iterator
--   @param i: iterator
--   @param ...: arguments
-- @returns
--   @t: results of running the iterator on its arguments
function _G.collect (i, ...)
  local t = {}
  for e in i (...) do
    table.insert (t, e)
  end
  return t
end

-- @func map: Map a function over an iterator
--   @param f: function
--   @param i: iterator
--   @param ...: iterator's arguments
-- @returns
--   @param t: result table
function _G.map (f, i, ...)
  local t = {}
  for e in i (...) do
    local r = f (e)
    if r then
      table.insert (t, r)
    end
  end
  return t
end

-- @func filter: Filter an iterator with a predicate
--   @param p: predicate
--   @param i: iterator
--   @param ...:
-- @returns
--   @param t: result table containing elements e for which p (e)
function _G.filter (p, i, ...)
  local t = {}
  for e in i (...) do
    if p (e) then
      table.insert (t, e)
    end
  end
  return t
end

-- @func fold: Fold a function into an iterator leftwards
--   @param f: function
--   @param d: element to place in left-most position
--   @param i: iterator
--   @param ...:
-- @returns
--   @param r: result
function _G.foldl (f, i, ...)
  local r = d
  for e in i (...) do
    r = f (r, e)
  end
  return r
end

-- @func treeIter: tree iterator
--   @param t: tree to iterate over
-- @returns
--   @param f: iterator function
--   @returns
--     @param e: event
--     @param t: table of values
function _G.treeIter (t)
  return coroutine.wrap (function ()
                           if not coroutine.yield ("branch", t) then
                             for i, v in ipairs (t) do
                               if type (v) ~= "table" then
                                 if coroutine.yield ("leaf", {i, v}) then
                                   break
                                 end
                               else
                                 for e, u in treeIter (v) do
                                   if coroutine.yield (e, u) then
                                     break
                                   end
                                 end
                               end
                             end
                             coroutine.yield ("join", t)
                           end
                         end)
end

-- FIXME: this version is more obvious but has an illegal yield
-- @func treeIter: tree iterator
--   @param t: tree to iterate over
-- @returns
--   @param f: iterator function
--   @returns
--     @param e: event
--     @param t: table of values
-- function _G.treeIter (t)
--   if not coroutine.yield ("branch", t) then
--     for i, v in ipairs (t) do
--       if type (v) ~= "table" then
--         if coroutine.yield ("leaf", {i, v}) then
--           break
--         end
--       else
--         f (v)
--       end
--     end
--     coroutine.yield ("join", t)
--   end
-- end

-- @func assert: Extend to allow formatted arguments
--   @param v: value
--   @param f, ...: arguments to format
-- @returns
--   @param v: value
function _G.assert (v, f, ...)
  if not v then
    if f == nil then
      f = ""
    end
    error (string.format (f, ...))
  end
  return v
end

-- @func warn: Give warning with the name of program and file (if any)
--   @param ...: arguments for format
function _G.warn (...)
  if prog.name then
    io.stderr:write (prog.name .. ":")
  end
  if prog.file then
    io.stderr:write (prog.file .. ":")
  end
  if prog.line then
    io.stderr:write (tostring (prog.line) .. ":")
  end
  if prog.name or prog.file or prog.line then
    io.stderr:write (" ")
  end
  io.writeLine (io.stderr, string.format (...))
end

-- @func die: Die with error
--   @param ...: arguments for format
function _G.die (...)
  warn (unpack (arg))
  error ()
end

-- Function forms of operators
_G.op = {
  ["+"] = function (...)
            return list.foldr (function (a, b)
                                 return a + b
                               end,
                               0, {...})
          end,
  ["-"] = function (...)
            return list.foldr (function (a, b)
                                 return a - b
                               end,
                               0, {...})
          end,
  ["*"] = function (...)
            return list.foldr (function (a, b)
                                 return a * b
                               end,
                               1, {...})
          end,
  ["/"] = function (a, b)
            return a / b
          end,
  ["and"] = function (...)
              return list.foldl (function (a, b)
                                   return a and b
                                 end, true, {...})
            end,
  ["or"] = function (...)
             return list.foldl (function (a, b)
                                  return a or b
                                end,
                                false, {...})
           end,
  ["not"] = function (x)
              return not x
            end,
  ["=="] = function (x, ...)
             for _, v in ipairs ({...}) do
               if v ~= x then
                 return false
               end
             end
             return true
           end,
  ["~="] = function (...)
             local t = {}
             for _, v in ipairs ({...}) do
               if t[v] then
                 return false
               end
               t[v] = true
             end
             return true
           end,
}
