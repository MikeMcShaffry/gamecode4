require 'p4'

p4 = P4.P4()
pcall(function()
	p4:connect()
	files = p4:run_sync()
end)

for _, o in ipairs(p4.output) do
	print(o.clientFile)
end

p4:disconnect()
