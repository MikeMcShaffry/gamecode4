-- @module lcs
-- LCS algorithm

-- After pseudo-code in
-- http://www.ics.uci.edu/~eppstein/161/960229.html
-- Lecture notes by David Eppstein, eppstein@ics.uci.edu

module ("lcs", package.seeall)


-- @func commonSubseqs: find common subsequences
--   @param a, b: two sequences
-- @returns
--   @param l: list of common subsequences
--   @param m: the length of a
--   @param n: the length of b
function commonSubseqs (a, b)
  local l, m, n = {}, #a, #b
  for i = m + 1, 1, -1 do
    l[i] = {}
    for j = n + 1, 1, -1 do
      if i > m or j > n then
        l[i][j] = 0
      elseif a[i] == b[j] then
        l[i][j] = 1 + l[i + 1][j + 1]
      else
        l[i][j] = math.max (l[i + 1][j], l[i][j + 1])
      end
    end
  end
  return l, m, n
end

-- @func longestCommonSubseq: find the LCS of two sequences
-- The sequence type must have an __append metamethod. This is
-- provided by string_ext for strings, and by list for lists.
--   @param a, b: two sequences
--   @param s: an empty sequence of the same type
-- @returns
--   @param s_: the LCS of a and b
function longestCommonSubseq (a, b, s)
  local l, m, n = lcs.commonSubseqs (a, b)
  local i, j = 1, 1
  local f = getmetatable (s).__append
  while i <= m and j <= n do
    if a[i] == b[j] then
      s = f (s, a[i])
      i = i + 1
      j = j + 1
    elseif l[i + 1][j] >= l[i][j + 1] then
      i = i + 1
    else
      j = j + 1
    end
  end
  return s
end
