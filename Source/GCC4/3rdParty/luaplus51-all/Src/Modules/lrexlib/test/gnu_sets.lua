-- See Copyright Notice in the file LICENSE

local luatest = require "luatest"
local N = luatest.NT

local function norm(a) return a==nil and N or a end

local function set_f_gmatch (lib, flg)
  local downcase = {}
  for i = 0, 255 do -- 255 == UCHAR_MAX
    downcase[i] = string.gsub(string.char (i), ".", function (s) return string.lower(s) end)
  end
  -- gmatch (s, p, [cf], [ef], [tr])
  local function test_gmatch (subj, patt)
    local out, guard = {}, 10
    for a, b in lib.gmatch (subj, patt, nil, nil, downcase) do
      table.insert (out, { norm(a), norm(b) })
      guard = guard - 1
      if guard == 0 then break end
    end
    return unpack (out)
  end
  return {
    Name = "Function gmatch",
    Func = test_gmatch,
  --{  subj             patt         results }
    { {"abA",           "a"},        {{"a",N}, {"A",N} } },
  }
end

local function set_f_match (lib, flg)
return {
  Name = "Function match",
  Func = lib.match,
  --{subj,   patt,      st,cf,ef},           { results }
  { {"abcd", ".+",      5},                  { N }    }, -- failing st
  { {"abc",  "^abc"},                        {"abc" } }, -- anchor
  { {"abc",  "^abc",    N,N,flg.not_bol},    { N }    }, -- anchor + ef
  { {"abc",  "abc$",    N,N,flg.not_eol},    { N }    }, -- anchor + ef
  { {"cabcaab", "ca+b", N,N,flg.backward},    {"caab" }  }, -- reverse search
}
end

return function (libname)
  local lib = require (libname)
  local flags = lib.flags ()
  return {
    set_f_match     (lib, flags),
    set_f_gmatch    (lib),
  }
end
