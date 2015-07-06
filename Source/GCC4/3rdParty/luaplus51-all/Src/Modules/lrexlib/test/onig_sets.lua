-- See Copyright Notice in the file LICENSE

local luatest = require "luatest"
local N = luatest.NT

local function norm(a) return a==nil and N or a end

local function fill (n, m)
  local t = {}
  for i = n, m, -1 do table.insert (t, i) end
  return t
end

local function set_named_subpatterns (lib, flg)
  return {
    Name = "Named Subpatterns",
    Func = function (methodname, subj, patt, name1, name2)
      local r = lib.new (patt)
      local _,_,caps = r[methodname] (r, subj)
      return norm(caps[name1]), norm(caps[name2])
    end,
    --{}
    { {"tfind", "abcd", "(?<dog>.)b.(?<cat>d)", "dog", "cat"},  {"a","d"} },
    { {"exec",  "abcd", "(?<dog>.)b.(?<cat>d)", "dog", "cat"},  {"a","d"} },
  }
end

local function set_f_find (lib, flg)
  local cp1251 =
    "ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÜÛÚİŞßàáâãäå¸æçèéêëìíîïğñòóôõö÷øùüûúışÿ"
  local loc = "CP1251"
  return {
  Name = "Function find",
  Func = lib.find,
  --{subj,   patt,      st,cf,ef,lo},        { results }
  { {"abcd", ".+",      5},                  { N   } }, -- failing st
  { {"abcd", ".*?"},                         { 1,0 } }, -- non-greedy
  { {"abc",  "aBC",     N,flg.IGNORECASE},   { 1,3 } }, -- cf
  { {"abc",  "aBC",     N,"i"         },     { 1,3 } }, -- cf
  { {cp1251, "[[:upper:]]+", N,N,N, loc},    { 1,33} }, -- locale
  { {cp1251, "[[:lower:]]+", N,N,N, loc},    {34,66} }, -- locale
  { {cp1251, "\\w+",         N,N,N, loc},    {1, 66} }, -- locale
}
end

local function set_f_match (lib, flg)
  return {
  Name = "Function match",
  Func = lib.match,
  --{subj,   patt,      st,cf,ef,lo},        { results }
  { {"abcd", ".+",      5},                  { N    }}, -- failing st
  { {"abcd", ".*?"},                         { ""   }}, -- non-greedy
  { {"abc",  "aBC",     N,flg.IGNORECASE},   {"abc" }}, -- cf
  { {"abc",  "aBC",     N,"i"         },     {"abc" }}, -- cf
}
end

local function set_f_gmatch (lib, flg)
  -- gmatch (s, p, [cf], [ef])
  local pCSV = "(^[^,]*)|,([^,]*)"
  local F = false
  local function test_gmatch (subj, patt)
    local out, guard = {}, 10
    for a, b in lib.gmatch (subj, patt) do
      table.insert (out, { norm(a), norm(b) })
      guard = guard - 1
      if guard == 0 then break end
    end
    return unpack (out)
  end
  return {
    Name = "Function gmatch",
    Func = test_gmatch,
  --{  subj             patt   results }
    { {"a\0c",          "." }, {{"a",N},{"\0",N},{"c",N}} },--nuls in subj
    { {"",              pCSV}, {{"",F}} },
    { {"12",            pCSV}, {{"12",F}} },
    ----{ {",",             pCSV}, {{"", F},{F,""}} },
    { {"12,,45",        pCSV}, {{"12",F},{F,""},{F,"45"}} },
    ----{ {",,12,45,,ab,",  pCSV}, {{"",F},{F,""},{F,"12"},{F,"45"},{F,""},{F,"ab"},{F,""}} },
  }
end

local function set_f_split (lib, flg)
  -- split (s, p, [cf], [ef])
  local function test_split (subj, patt)
    local out, guard = {}, 10
    for a, b, c in lib.split (subj, patt) do
      table.insert (out, { norm(a), norm(b), norm(c) })
      guard = guard - 1
      if guard == 0 then break end
    end
    return unpack (out)
  end
  return {
    Name = "Function split",
    Func = test_split,
  --{  subj             patt      results }
    { {"a,\0,c",       ","},     {{"a",",",N},{"\0",",",N},{"c",N,N},   } },--nuls in subj
    { {"ab",           "$"},     {{"ab","",N}, {"",N,N},               } },
    { {"ab",         "^|$"},     {{"", "", N}, {"ab","",N},  {"",N,N}, } },
    { {"ab45ab","(?<=ab).*?"},   {{"ab","",N}, {"45ab","",N},{"",N,N}, } },
    { {"ab",         "\\b"},     {{"", "", N}, {"ab","",N},  {"",N,N}, } },
  }
end

local function set_m_exec (lib, flg)
  return {
  Name = "Method exec",
  Method = "exec",
--{patt,cf,lo},           {subj,st,ef}              { results }
  { {".+"},               {"abcd",5},               { N }    }, -- failing st
  { {".*?"},              {"abcd"},                 {1,0,{}} }, -- non-greedy
  { {"aBC",flg.IGNORECASE}, {"abc"},                {1,3,{}} }, -- cf
  { {"aBC","i"         }, {"abc"},                  {1,3,{}} }, -- cf
}
end

local function set_m_tfind (lib, flg)
  return {
  Name = "Method tfind",
  Method = "tfind",
--{patt,cf,lo},           {subj,st,ef}              { results }
  { {".+"},               {"abcd",5},               { N }    }, -- failing st
  { {".*?"},              {"abcd"},                 {1,0,{}} }, -- non-greedy
  { {"aBC",flg.IGNORECASE}, {"abc"},                {1,3,{}} }, -- cf
  { {"aBC","i"         }, {"abc"},                  {1,3,{}} }, -- cf
}
end

return function (libname)
  local lib = require (libname)
  local flags = lib.flags ()
  local sets = {
    set_f_match  (lib, flags),
    set_f_find   (lib, flags),
    set_f_gmatch (lib, flags),
    set_f_split  (lib, flags),
    set_m_exec   (lib, flags),
    set_m_tfind  (lib, flags),
  }
  local MAJOR = tonumber(lib.version():match("%d+"))
  if MAJOR >= 0 then
    table.insert (sets, set_named_subpatterns (lib, flags))
  end
  return sets
end
