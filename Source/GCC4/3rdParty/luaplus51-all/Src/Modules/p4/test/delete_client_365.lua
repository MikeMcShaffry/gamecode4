require 'p4'

p4 = P4.P4()
now = os.time()

pcall(function()
	p4:connect()
	for i, client in ipairs(p4:run_clients()) do
        -- If the client has not been accessed for a year, delete it
		if client.Access + 365 * 24 * 60 * 60 < now then
--			p4.delete_client( '-f', client[ "client" ] )
			print("Deleting client " .. client.client)
		end
	end
end)

for i, e in ipairs(p4.errors) do
	print(e)
end

p4:disconnect()
