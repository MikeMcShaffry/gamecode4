--- Getter class. Used to get values from the token stream. The first
-- argument `get` of a macro substitution function is of this type.
--
--    M.define ('\\',function(get,put)
--        local args, body = get:names('('), get:list()
--        return put:keyword 'function' '(' : names(args) ')' :
--             keyword 'return' : list(body) : space() : keyword 'end'
--    end)
--
-- The second argument `put` is a `TokenList` object.
-- @see macro.TokenList
-- @module macro.Getter

local TokenList = require 'macro.TokenList'
local append = table.insert
local setmetatable = setmetatable

local Getter = {
    __call = function(self)
        return self.fun()
    end
}
local M = Getter

Getter.__index = Getter;

local scan_iter

function Getter.new (get)
    return setmetatable({fun=get},Getter)
end

function Getter.from_tl(tl)
    return Getter.new(scan_iter(tl))
end

local Tok = {
    __tostring = function(self)
        return self[2]
    end
}

local function tok_new (t)
    return setmetatable(t,Tok)
end

-- create a token iterator out of a token list
function Getter.scan_iter (tlist)
    local i,n = 1,#tlist
    return function()
        local tv = tlist[i]
        if tv == nil then return nil end
        i = i + 1
        return tv[1],tv[2]
    end
end

scan_iter = Getter.scan_iter

--- get the next non-whitespace token.
-- @return token type
-- @return token value
-- @function Getter.next
function Getter.next(get)
    local t,v = get()
    while t == 'space' or t == 'comment' do
        t,v = get()
    end
    return t,v
end

local TL,LTL = TokenList.new, TokenList.new_list

--- get a delimited list of token lists.
-- Typically used for grabbing argument lists like ('hello',a+1,fred(c,d)); will count parens
-- so that the delimiter (usually a comma) is ignored inside sub-expressions. You must have
-- already read the start token of the list, e.g. open parentheses. It will eat the end token
-- and return the list of TLs, plus the end token. Based on similar code in Penlight's
-- `pl.lexer` module.
-- @param tok the token stream
-- @param endt the end token (default ')')
-- @param delim the delimiter (default ',')
-- @return list of token lists
-- @return end token in form {type,value}
function Getter.list(tok,endtoken,delim)
    endtoken = endtoken or ')'
    delim = delim or ','
    local parm_values = LTL()
    local level = 1 -- used to count ( and )
    local tl = TL()
    local function tappend (tl,t,val)
        val = val or t
        append(tl,{t,val})
    end
    local is_end
    if type(endtoken) == 'function' then
        is_end = endtoken
    elseif endtoken == '\n' then
        is_end = function(t,val)
            return t == 'space' and val:find '\n'
        end
    else
        is_end = function (t)
            return t == endtoken
        end
    end
    local token,value = tok()
    if is_end(token,value) then return parm_values end
    if token == 'space' then
        token,value = tok()
    end
    while true do
        if not token then return nil,'unexpected end of list' end -- end of stream is an error!
        if is_end(token,value) and level == 1 then
            append(parm_values,tl)
            break
        elseif token == '(' then
            level = level + 1
            tappend(tl,'(')
        elseif token == ')' then
            level = level - 1
            if level == 0 then -- finished with parm list
                append(parm_values,tl)
                break
            else
                tappend(tl,')')
            end
        elseif token == '{' then
            level = level + 1
            tappend(tl,'{')
        elseif token == '}' then
            level = level - 1
            tappend(tl,'}')
        elseif token == delim and level == 1 then
            append(parm_values,tl) -- a new parm
            tl = TL()
        else
            tappend(tl,token,value)
        end
        token,value=tok()
    end
    return parm_values,tok_new{token,value}
end

function Getter.upto_keywords (k1,k2)
    return function(t,v)
        return t == 'keyword' and (v == k1 or v == k2)
    end,''
end

local tnext = Getter.next


function Getter.upto(tok,k1,k2)
    local endt = k1
    if type(k1) == 'string' and k1:match '^%a+$' then
        endt = Getter.upto_keywords(k1,k2)
    end
    local ltl,tok = tok:list(endt,'')
    M.assert(ltl ~= nil and #ltl > 0,'failed to grab tokens')
    return ltl[1],tok
end


--- get the next identifier token.
-- (will be an error if the token has wrong type)
-- @return name
function Getter.name(tok)
    local t,v = tnext(tok)
    M.assert(t == 'iden','expecting name')
    return v
end

--- get the next number token.
-- (will be an error if the token has wrong type)
-- @return converted number
function Getter.number(tok)
    local t,v = tnext(tok)
    M.assert(t == 'number','expecting number')
    return tonumber(v)
end

--- get a delimited list of names.
-- works like list.
-- @param tok the token stream
-- @param endt the end token (default ')')
-- @param delim the delimiter (default ',')
-- @see list
function Getter.names(tok,endt,delim)
    local ltl,err = tok:list(endt,delim)
    if not ltl then error('get_names: '..err) end
    local names = {}
    -- list() will return {{}} for an empty list of tlists
    for i,tl in ipairs(ltl) do
        local tv = tl[1]
        if tv then names[i] = tv[2] end
    end
    return names, err
end

--- get the next string token.
-- (will be an error if the token has wrong type)
-- @return string value (without quotes)
function Getter.string(tok)
    local t,v = tok:expecting("string")
    return v:sub(2,-2)
end

--- assert that the next token has the given type. This will throw an
-- error if the next non-whitespace token does not match.
-- @param type a token type ('iden','string',etc)
-- @param value a token value (optional)
-- @usage  get:expecting '('
-- @usage  get:expecting ('iden','bonzo')
function Getter.expecting (tok,type,value)
    local t,v = tnext(tok)
    if t ~= type then M.error ("expected "..type.." got "..t) end
    if value then
        if v ~= value then M.error("expected "..value.." got "..v) end
    end
    return t,v
end

--- peek ahead or before in the token stream.
-- @param k positive delta for looking ahead, negative for looking behind.
-- @param dont_skip true if you want to check for whitespace
-- @return the token type
-- @return the token value
-- @return the token offset
-- @function Getter.peek

--- peek ahead two tokens.
-- @return first token type
-- @return first token value
-- @return second token type
-- @return second token value
-- @function Getter.peek2

--- patch the token stream at the end.
-- @param idx index in output table
-- @param text to replace value at that index
-- @function Getter.patch

--- put out a placeholder for later patching.
-- @param put a putter object
-- @return an index into the output table
-- @function Getter.placeholder

return Getter
