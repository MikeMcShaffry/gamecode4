-- @module list

module ("list", package.seeall)

require "base"
require "table_ext"


-- @func elems: An iterator over the elements of a list
--   @param l: list to iterate over
-- @returns
--   @param f: iterator function
--     @param l: list
--     @param n: index
--   @returns
--     @param v: value (l[n - 1])
--   @param l: the list, as above
--   @param 0
function elems (l)
  local n = 0
  return function (l)
           n = n + 1
           if n <= #l then
             return l[n]
           end
         end,
  l, true
end

-- @func relems: An iterator over the elements of a list, in reverse
--   @param l: list to iterate over
-- @returns
--   @param f: iterator function
--     @param l: list
--     @param n: index
--   @returns
--     @param v: value (l[n - 1))
--   @param l: the list, as above
--   @param n: #l + 1
function relems (l)
  local n = #l + 1
  return function (l)
           n = n - 1
           if n > 0 then
             return l[n]
           end
         end,
  l, true
end

-- @func map: Map a function over a list
--   @param f: function
--   @param l: list
-- @returns
--   @param m: result list {f (l[1]), ..., f (l[#l])}
function map (f, l)
  return _G.map (f, elems, l)
end

-- @func mapWith: Map a function over a list of lists
--   @param f: function
--   @param ls: list of lists
-- @returns
--   @param m: result list {f (unpack (ls[1]))), ...,
--     f (unpack (ls[#ls]))}
function mapWith (f, l)
  return _G.map (compose (f, unpack), elems, l)
end

-- @func filter: Filter a list according to a predicate
--   @param p: predicate
--     @param a: argument
--   @returns
--     @param f: flag
--   @param l: list of lists
-- @returns
--   @param m: result list containing elements e of l for which p (e)
--     is true
function filter (p, l)
  return _G.filter (p, elems, l)
end

-- @func slice: Slice a list
--   @param l: list
--   @param [from], @param [to]: start and end of slice
--     from defaults to 1 and to to #l;
--     negative values count from the end
-- @returns
--   @param m: {l[from], ..., l[to]}
function slice (l, from, to)
  local m = {}
  local len = #l
  from = from or 1
  to = to or len
  if from < 0 then
    from = from + len + 1
  end
  if to < 0 then
    to = to + len + 1
  end
  for i = from, to do
    table.insert (m, l[i])
  end
  return m
end

-- @func tail: Return a list with its first element removed
--   @param l: list
-- @returns
--   @param m: {l[2], ..., l[#l]}
function tail (l)
  return slice (l, 2)
end

-- @func foldl: Fold a binary function through a list left
-- associatively
--   @param f: function
--   @param e: element to place in left-most position
--   @param l: list
-- @returns
--   @param r: result
function foldl (f, e, l)
  return _G.foldl (f, elems, e, l)
end

-- @func foldr: Fold a binary function through a list right
-- associatively
--   @param f: function
--   @param e: element to place in right-most position
--   @param l: list
-- @returns
--   @param r: result
function foldr (f, e, l)
  return _G.foldr (f, relems, e, l)
end

-- @func cons: Prepend an item to a list
--   @param x: item
--   @param l: list
-- @returns
--   @param r: {x, unpack (l)}
function cons (x, l)
  return {x, unpack (l)}
end

-- @func append: Append an item to a list
--   @param x: item
--   @param l: list
-- @returns
--   @param r: {l[1], ..., l[#l], x}
function append (x, l)
  local r = {unpack (l)}
  table.insert(r, x)
  return r
end

-- @func concat: Concatenate lists
--   @param l1, l2, ... ln: lists
-- @returns
--   @param r: result {l1[1], ..., l1[#l1], ...,
--                     ln[1], ..., ln[#ln]}
function concat (...)
  local r = {}
  for _, l in ipairs ({...}) do
    for _, v in ipairs (l) do
      table.insert (r, v)
    end
  end
  return r
end

-- @func rep: Repeat a list
--   @param n: number of times to repeat
--   @param l: list
-- @returns
--   @param r: n copies of l appended together
function rep (n, l)
  local r = {}
  for i = 1, n do
    r = list.concat (r, l)
  end
  return r
end

-- @func reverse: Reverse a list
--   @param l: list
-- @returns
--   @param m: list {l[#l], ..., l[1]}
function reverse (l)
  local m = {}
  for i = #l, 1, -1 do
    table.insert (m, l[i])
  end
  return m
end

-- @func transpose: Transpose a list of lists
--   @param ls: {{l11, ..., l1c}, ..., {lr1, ..., lrc}}
-- @returns
--   @param ms: {{l11, ..., lr1}, ..., {l1c, ..., lrc}}
-- This function is equivalent to zip and unzip in more strongly typed
-- languages
function transpose (ls)
  local ms, len = {}, #ls
  for i = 1, math.max (unpack (map (table.getn, ls))) do
    ms[i] = {}
    for j = 1, len do
      ms[i][j] = ls[j][i]
    end
  end
  return ms
end

-- @func zipWith: Zip lists together with a function
--   @param f: function
--   @param ls: list of lists
-- @returns
--   @param m: {f (ls[1][1], ..., ls[#ls][1]), ...,
--              f (ls[1][N], ..., ls[#ls][N])
--     where N = max {map (table.getn, ls)}
function zipWith (f, ls)
  return mapWith (f, zip (ls))
end

-- @func project: Project a list of fields from a list of tables
--   @param f: field to project
--   @param l: list of tables
-- @returns
--   @param m: list of f fields
function project (f, l)
  return map (function (t) return t[f] end, l)
end

-- @func enpair: Turn a table into a list of pairs
-- FIXME: Find a better name
--   @param t: table {i1=v1, ..., in=vn}
-- @returns
--   @param ls: list {{i1, v1}, ..., {in, vn}}
function enpair (t)
  local ls = {}
  for i, v in pairs (t) do
    table.insert (ls, {i, v})
  end
  return ls
end

-- @func depair: Turn a list of pairs into a table
-- FIXME: Find a better name
--   @param ls: list {{i1, v1}, ..., {in, vn}}
-- @returns
--   @param t: table {i1=v1, ..., in=vn}
function depair (ls)
  local t = {}
  for _, v in ipairs (ls) do
    t[v[1]] = v[2]
  end
  return t
end

-- @func flatten: Flatten a list
--   @param l: list to flatten
-- @returns
--   @param m: flattened list
function flatten (l)
  local m = {}
  for _, v in ipairs (l) do
    if type (v) == "table" then
      m = concat (m, flatten (v))
    else
      table.insert (m, v)
    end
  end
  return m
end

-- @func shape: Shape a list according to a list of dimensions
-- Dimensions are given outermost first and items from the original
-- list are distributed breadth first; there may be one 0 indicating
-- an indefinite number. Hence, {0} is a flat list, {1} is a
-- singleton, {2, 0} is a list of two lists, and {0, 2} is a list of
-- pairs.
--   @param s: {d1, ..., dn}
--   @param l: list to reshape
-- @returns
--   @param m: reshaped list
-- Algorithm: turn shape into all +ve numbers, calculating the zero if
-- necessary and making sure there is at most one; recursively walk
-- the shape, adding empty tables until the bottom level is reached at
-- which point add table items instead, using a counter to walk the
-- flattened original list.
function shape (s, l)
  l = flatten (l)
  -- Check the shape and calculate the size of the zero, if any
  local size = 1
  local zero
  for i, v in ipairs (s) do
    if v == 0 then
      if zero then -- bad shape: two zeros
        return nil
      else
        zero = i
      end
    else
      size = size * v
    end
  end
  if zero then
    s[zero] = math.ceil (#l / size)
  end
  local function fill (i, d)
    if d > #s then
      return l[i], i + 1
    else
      local t = {}
      for j = 1, s[d] do
        local e
        e, i = fill (i, d + 1)
        table.insert (t, e)
      end
      return t, i
    end
  end
  return (fill (1, 1))
end

-- @func indexKey: Make an index of a list of tables on a given
-- field
--   @param f: field
--   @param l: list of tables {t1, ..., tn}
-- @returns
--   @param m: index {t1[f]=1, ..., tn[f]=n}
function indexKey (f, l)
  local m = {}
  for i, v in ipairs (l) do
    local k = v[f]
    if k then
      m[k] = i
    end
  end
  return m
end

-- @func indexValue: Copy a list of tables, indexed on a given
-- field
--   @param f: field whose value should be used as index
--   @param l: list of tables {i1=t1, ..., in=tn}
-- @returns
--   @param m: index {t1[f]=t1, ..., tn[f]=tn}
function indexValue (f, l)
  local m = {}
  for i, v in ipairs (l) do
    local k = v[f]
    if k then
      m[k] = v
    end
  end
  return m
end
permuteOn = indexValue

-- @head Metamethods for lists
metatable = {
  -- list .. table = list.concat
  __concat = list.concat,
  -- @func append metamethod
  --   @param l: list
  --   @param e: list element
  -- @returns
  --   @param l_: {l[1], ..., l[#l], e}
  __append =
    function (l, e)
      local l_ = table.clone (l)
      table.insert (l_, e)
      return l_
    end,
}

-- @func new: List constructor
-- Needed in order to use metamethods
--   @param t: list (as a table)
-- @returns
--   @param l: list (with list metamethods)
function new (l)
  return setmetatable (l, metatable)
end

-- Function forms of operators
_G.op[".."] = list.concat
