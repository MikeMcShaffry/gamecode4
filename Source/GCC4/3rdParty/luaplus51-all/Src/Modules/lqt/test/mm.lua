#!/usr/bin/lua

require'qtcore'

qapp = QCoreApplication.new_local(1+select("#", ...), {arg[0], ...})

local N = 1
local qo = nil
local del = QObject.delete
for i = 1, 10*N do
  qo = QObject.new_local(qo)
  print('created QObject', qo);
  qo.delete = function(...) del(...) print('deleting', ...) end
  qo.__gc = function(...) del(...) print('deleting', ...) end
end

-- [[
local t = {}
for i = 1, 10*N do
  table.insert(t, qo)
  print("getting parent", qo)
  qo = qo:parent()
end
--]]


qo=nil
print('collecting')
collectgarbage("collect")
collectgarbage("collect")
print('processing')
qapp.processEvents()
t=nil
print('collecting')
collectgarbage("collect")
print('processing')
qapp.processEvents()
print('collecting')
collectgarbage("collect")


