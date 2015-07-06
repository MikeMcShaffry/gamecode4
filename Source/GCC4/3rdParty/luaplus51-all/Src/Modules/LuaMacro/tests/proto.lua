local M = require 'macro'

M.define('SF_',function(get,put)
    local name = get:upto '('
    local args,endt = get:list()
    args = args:strip_spaces()
    local argnames,names = {},{}
    for i,a in ipairs(args) do
        local name = a:pick(1)
        M.assert(a:pick(2) == ':')
        table.remove(a,1)
        table.remove(a,1)
        argnames[i] = {{'iden',name}}
        names[i] = name
    end
    get:expecting ':'
    local rtype, endt = get:upto '\n'
    put :tokens(name) '(' :list(argnames) ')' :space '\n'
    put :space()
    for i,a in ipairs(args) do
        local tp = a:pick(1)
        put :name('assert_arg') '(' :name(names[i]) ',' :number(i) ',' :string(tp) ')' ';'
    end
    return put
end)
