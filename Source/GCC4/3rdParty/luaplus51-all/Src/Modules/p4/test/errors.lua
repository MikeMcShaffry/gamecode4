require 'p4'

p4 = P4.P4()
pcall(function()
	p4:connect()
	p4.exception_level = 1 -- ignore "File(s) up-to-date"
	files = p4:run_sync()
end)

for _, e in ipairs(p4.errors) do
	print(e)
end

p4:disconnect()
