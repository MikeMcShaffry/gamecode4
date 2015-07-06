#!/usr/bin/lua

require'qtcore'

qa = QCoreApplication.new(1, {'virt_test'})

qo = QObject.new()
qo.event = function(o, e) print('event overload called', e:type()) end
qo.childEvent = function(o, e) print('child event overload called', e:type()) end

ql = {}

for i = 1, 5 do
	table.insert(ql, qo:new())
end

qo.event = nil

for i, o in ipairs(ql) do
	o:delete()
end


