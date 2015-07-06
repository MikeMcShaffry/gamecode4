local M = require 'macro'

local cf,copy,null
if package.config:sub(1,1) == '\\' then
    cf = 'fc'; copy = 'copy'; null = ' > null'
else
    cf = 'diff'; copy = 'cp'; null = ' > /dev/null'
end

local f,hname,mname

M.keyword_handler('BEGIN',function()
    hname = M.filename:gsub('%.c$','')..'.h'
    mname = hname:gsub('%.','_'):upper()
    f = io.open(M.filename..'.h','w')
    f:write('#ifndef ',mname,'\n')
    f:write('#define ',mname,'\n')
end)

M.keyword_handler ('END',function()
    f:write('#endif\n')
    f:close()
    local tmpf = M.filename..'.h'
    if os.execute(cf..' '..hname..' '..tmpf..null) ~= 0 then
        os.execute(copy..' '..tmpf..' '..hname..null)
    end
end)

M.define('export',function(get)
    local t,v = get:next()
    local decl,out
    if v == '{' then
        decl = tostring(get:upto '}')
        f:write(decl,'\n')
    else
        decl = v .. ' ' .. tostring(get:upto '{')
        f:write(decl,';\n')
        out = decl .. '{'
    end
    return out
end)

M.define('finis',close_header)

