require 'p4'                              -- Import the module
p4 = P4.P4()                              -- Create the P4 instance
p4.port = "1666"
p4.user = "fred"
p4.client = "fred-ws"                     -- Set some environment variables
ret, err = pcall( function()              -- Catch exceptions
	p4:connect()                          -- Connect to the Perforce Server
	info = p4:run("info")                 -- Run "p4 info" (returns a table)
	for key, value in pairs(info[1]) do   -- and display all key-value pairs
		print(key .. '=' .. value)
	end
	p4:run("edit", "file.txt")            -- Run "p4 edit file.txt"
end)

for _, e in ipairs(p4.errors) do          -- Display errors
	print(e)
end

p4:disconnect()                       -- Disconnect from the Server
