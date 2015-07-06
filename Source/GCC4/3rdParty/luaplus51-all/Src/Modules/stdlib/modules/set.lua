-- @module set

module ("set", package.seeall)


-- Primitive methods (know about representation)

-- The representation is a table whose tags are the elements, and
-- whose values are true.

-- @func member: Say whether an element is in a set
--   @param s: set
--   @param e: element
-- @returns
--   @param f: true if e is in set, false otherwise
function member (s, e)
  return s[e] == true
end

-- @func insert: Insert an element to a set
--   @param s: set
--   @param e: element
function insert (s, e)
  s[e] = true
end

-- @func new: Make a list into a set
--   @param l: list
-- @returns
--   @param s: set
metatable = {}
function new (l)
  local s = setmetatable ({}, metatable)
  for _, e in ipairs (l) do
    insert (s, e)
  end
  return s
end

-- @func elements: Iterator for sets
-- TODO: Make the iterator return only the key
elements = pairs


-- High level methods (representation unknown)

-- @func difference: Find the difference of two sets
--   @param s, t: sets
-- @returns
--   @param r: s with elements of t removed
function difference (s, t)
  local r = new {}
  for e in elements (s) do
    if not member (t, e) then
      insert (r, e)
    end
  end
  return r
end

-- @func difference: Find the symmetric difference of two sets
--   @param s, t: sets
-- @returns
--   @param r: elements of s and t that are in s or t but not both
function symmetric_difference (s, t)
  return difference (union (s, t), intersection (t, s))
end

-- @func intersection: Find the intersection of two sets
--   @param s, t: sets
-- @returns
--   @param r: set intersection of s and t
function intersection (s, t)
  local r = new {}
  for e in elements (s) do
    if member (t, e) then
      insert (r, e)
    end
  end
  return r
end

-- @func union: Find the union of two sets
--   @param s, t: sets
-- @returns
--   @param r: set union of s and t
function union (s, t)
  local r = new {}
  for e in elements (s) do
    insert (r, e)
  end
  for e in elements (t) do
    insert (r, e)
  end
  return r
end

-- @func subset: Find whether one set is a subset of another
--   @param s, t: sets
-- @returns
--   @param r: true if s is a subset of t, false otherwise
function subset (s, t)
  for e in elements (s) do
    if not member (t, e) then
      return false
    end
  end
  return true
end

-- @func propersubset: Find whether one set is a proper subset of
-- another
--   @param s, t: sets
-- @returns
--   @param r: true if s is a proper subset of t, false otherwise
function propersubset (s, t)
  return subset (s, t) and not subset (t, s)
end

-- @func equal: Find whether two sets are equal
--   @param s, t: sets
-- @returns
--   @param r: true if sets are equal, false otherwise
function equal (s, t)
  return subset (s, t) and subset (t, s)
end

-- @head Metamethods for sets
-- set + table = union
metatable.__add = union
-- set - table = set difference
metatable.__sub = difference
-- set * table = intersection
metatable.__mul = intersection
-- set / table = symmetric difference
metatable.__div = symmetric_difference
-- set <= table = subset
metatable.__le = subset
-- set < table = proper subset
metatable.__lt = propersubset
