require 'p4'

p4 = P4.P4()
pcall(function()
	p4:connect()
	p4.exception_level = 2 -- File(s) up-to-date warning
	files = p4:run_sync()
end)

for _, w in ipairs(p4.warnings) do
	print(w)
end

p4:disconnect()
