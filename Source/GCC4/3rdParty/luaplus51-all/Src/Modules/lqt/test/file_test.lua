require'qtcore'

local s = tostring{}

local fn=QString.new'tmp_file'
local f=QFile.new(fn)
print('open file => ', f:open{'WriteOnly'})
print('write to file => ', f:write(s))
print('flush file =>', f:flush())
print('close file => ', f:close())
print('reopen file =>', f:open{'ReadOnly'})
print('correct read => ', f:readAll()==s)
