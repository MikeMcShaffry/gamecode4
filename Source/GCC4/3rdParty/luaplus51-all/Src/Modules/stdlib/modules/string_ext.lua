-- String

module ("string", package.seeall)


-- TODO: Pretty printing
--
--   (Use in getopt)
--
--   John Hughes's and Simon Peyton Jones's Pretty Printer Combinators
--
--   Based on The Design of a Pretty-printing Library in Advanced
--   Functional Programming, Johan Jeuring and Erik Meijer (eds), LNCS 925
--   http://www.cs.chalmers.se/~rjmh/Papers/pretty.ps
--   Heavily modified by Simon Peyton Jones, Dec 96
--
--   Haskell types:
--   data Doc     list of lines
--   quote :: Char -> Char -> Doc -> Doc    Wrap document in ...
--   (<>) :: Doc -> Doc -> Doc              Beside
--   (<+>) :: Doc -> Doc -> Doc             Beside, separated by space
--   ($$) :: Doc -> Doc -> Doc              Above; if there is no overlap it "dovetails" the two
--   nest :: Int -> Doc -> Doc              Nested
--   punctuate :: Doc -> [Doc] -> [Doc]     punctuate p [d1, ... dn] = [d1 <> p, d2 <> p, ... dn-1 <> p, dn]
--   render      :: Int                     Line length
--               -> Float                   Ribbons per line
--               -> (TextDetails -> a -> a) What to do with text
--               -> a                       What to do at the end
--               -> Doc                     The document
--               -> a                       Result


-- @func __index: Give strings a subscription operator
--   @param s: string
--   @param n: index
-- @returns
--   @param s_: string.sub (s, n, n)
local oldmeta = getmetatable ("").__index
getmetatable ("").__index =
  function (s, n)
    if type (n) == "number" then
      return sub (s, n, n)
    -- Fall back to old metamethods
    elseif type (oldmeta) == "function" then
      return oldmeta (s, n)
    else
      return oldmeta[n]
    end
  end

-- @func __append: Give strings an append metamethod
--   @param s: string
--   @param c: character (1-character string)
-- @returns
--   @param s_: s .. c
getmetatable ("").__append =
  function (s, c)
    return s .. c
  end

-- @func caps: Capitalise each word in a string
--   @param s: string
-- @returns
--   @param s_: capitalised string
function caps (s)
  return (gsub (s, "(%w)([%w]*)",
                function (l, ls)
                  return upper (l) .. ls
                end))
end

-- @func chomp: Remove any final newline from a string
--   @param s: string to process
-- @returns
--   @param s_: processed string
function chomp (s)
  return (gsub (s, "\n$", ""))
end

-- @func escapePattern: Escape a string to be used as a pattern
--   @param s: string to process
-- @returns
--   @param s_: processed string
function escapePattern (s)
  return (gsub (s, "(%W)", "%%%1"))
end

-- @param escapeShell: Escape a string to be used as a shell token
-- Quotes spaces, parentheses, brackets, quotes, apostrophes and \s
--   @param s: string to process
-- @returns
--   @param s_: processed string
function escapeShell (s)
  return (gsub (s, "([ %(%)%\\%[%]\"'])", "\\%1"))
end

-- @func ordinalSuffix: Return the English suffix for an ordinal
--   @param n: number of the day
-- @returns
--   @param s: suffix
function ordinalSuffix (n)
  n = math.mod (n, 100)
  local d = math.mod (n, 10)
  if d == 1 and n ~= 11 then
    return "st"
  elseif d == 2 and n ~= 12 then
    return "nd"
  elseif d == 3 and n ~= 13 then
    return "rd"
  else
    return "th"
  end
end

-- @func format: Extend to work better with one argument
-- If only one argument is passed, no formatting is attempted
--   @param f: format
--   @param ...: arguments to format
-- @returns
--   @param s: formatted string
local _format = format
function format (f, arg1, ...)
  if arg1 == nil then
    return f
  else
    return _format (f, arg1, ...)
  end
end

-- @func pad: Justify a string
-- When the string is longer than w, it is truncated (left or right
-- according to the sign of w)
--   @param s: string to justify
--   @param w: width to justify to (-ve means right-justify; +ve means
--     left-justify)
--   @param [p]: string to pad with [" "]
-- @returns
--   s_: justified string
function pad (s, w, p)
  p = rep (p or " ", abs (w))
  if w < 0 then
    return sub (p .. s, -w)
  end
  return sub (s .. p, 1, w)
end

-- @func wrap: Wrap a string into a paragraph
--   @param s: string to wrap
--   @param w: width to wrap to [78]
--   @param ind: indent [0]
--   @param ind1: indent of first line [ind]
-- @returns
--   @param s_: wrapped paragraph
function wrap (s, w, ind, ind1)
  w = w or 78
  ind = ind or 0
  ind1 = ind1 or ind
  assert (ind1 < w and ind < w,
          "the indents must be less than the line width")
  s = rep (" ", ind1) .. s
  local lstart, len = 1, len (s)
  while len - lstart > w - ind do
    local i = lstart + w - ind
    while i > lstart and sub (s, i, i) ~= " " do
      i = i - 1
    end
    local j = i
    while j > lstart and sub (s, j, j) == " " do
      j = j - 1
    end
    s = sub (s, 1, j) .. "\n" .. rep (" ", ind) ..
      sub (s, i + 1, -1)
    local change = ind + 1 - (i - j)
    lstart = j + change
    len = len + change
  end
  return s
end

-- @func numbertosi: Write a number using SI suffixes
-- The number is always written to 3 s.f.
--   @param n: number
-- @returns
--   @param n_: string
function numbertosi (n)
  local SIprefix = {
    [-8] = "y", [-7] = "z", [-6] = "a", [-5] = "f",
    [-4] = "p", [-3] = "n", [-2] = "mu", [-1] = "m",
    [0] = "", [1] = "k", [2] = "M", [3] = "G",
    [4] = "T", [5] = "P", [6] = "E", [7] = "Z",
    [8] = "Y"
  }
  local t = format("% #.2e", n)
  local _, _, m, e = t:find(".(.%...)e(.+)")
  local man, exp = tonumber (m), tonumber (e)
  local siexp = math.floor (exp / 3)
  local shift = exp - siexp * 3
  local s = SIprefix[siexp] or "e" .. tostring (siexp)
  man = man * (10 ^ shift)
  return tostring (man) .. s
end


-- @func findl: Do find, returning captures as a list
--   @param s: target string
--   @param p: pattern
--   @param [init]: start position [1]
--   @param [plain]: inhibit magic characters [nil]
-- @returns
--   @param from, to: start and finish of match
--   @param capt: table of captures
function findl (s, p, init, plain)
  local function pack (from, to, ...)
    return from, to, {...}
  end
  return pack (p.find (s, p, init, plain))
end

-- @func finds: Do multiple find's on a string
--   @param s: target string
--   @param p: pattern
--   @param [init]: start position [1]
--   @param [plain]: inhibit magic characters [nil]
-- @returns
--   @param l: list of {from, to; capt = {captures}}
function finds (s, p, init, plain)
  init = init or 1
  local l = {}
  local from, to, r
  repeat
    from, to, r = findl (s, p, init, plain)
    if from ~= nil then
      table.insert (l, {from, to, capt = r})
      init = to + 1
    end
  until not from
  return l
end

-- @func gsubs: Perform multiple calls to gsub
--   @param s: string to call gsub on
--   @param sub: {pattern1=replacement1 ...}
--   @param [n]: upper limit on replacements [infinite]
-- @returns
--   @param s_: result string
--   @param r: number of replacements made
function gsubs (s, sub, n)
  local r = 0
  for i, v in pairs (sub) do
    local rep
    if n ~= nil then
      s, rep = gsub (s, i, v, n)
      r = r + rep
      n = n - rep
      if n == 0 then
        break
      end
    else
      s, rep = i.gsub (s, i, v)
      r = r + rep
    end
  end
  return s, r
end

-- FIXME: Consider Perl and Python versions.
-- @func split: Split a string at a given separator
--   @param sep: separator regex
--   @param s: string to split
-- @returns
--   @param l: list of strings
function split (sep, s)
  -- finds gets a list of {from, to, capt = {}} lists; we then
  -- flatten the result, discarding the captures, and prepend 0 (1
  -- before the first character) and append 0 (1 after the last
  -- character), and then read off the result in pairs.
  local pairs = list.concat ({0}, list.flatten (finds (s, sep)), {0})
  local l = {}
  for i = 1, #pairs, 2 do
    table.insert (l, sub (s, pairs[i] + 1, pairs[i + 1] - 1))
  end
  return l
end

-- @func ltrim: Remove leading matter from a string
--   @param [r]: leading regex ["%s+"]
--   @param s: string
-- @returns
--   @param s_: string without leading r
function ltrim (r, s)
  if s == nil then
    s, r = r, "%s+"
  end
  return (r.gsub (s, "^" .. r, ""))
end

-- @func rtrim: Remove trailing matter from a string
--   @param [r]: trailing regex ["%s+"]
--   @param s: string
-- @returns
--   @param s_: string without trailing r
function rtrim (r, s)
  if s == nil then
    s, r = r, "%s+"
  end
  return (r.gsub (s, r .. "$", ""))
end

-- @func trim: Remove leading and trailing matter from a
-- string
--   @param [r]: leading/trailing regex ["%s+"]
--   @param s: string
-- @returns
--   @param s_: string without leading/trailing r
function trim (r, s)
  return ltrim (rtrim (r, s))
end
