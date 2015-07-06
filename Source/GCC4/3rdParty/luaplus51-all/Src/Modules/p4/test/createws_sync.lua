require 'p4'

template = "my-client-template"
client_root = "c:\\work\\my-root"

p4 = P4.P4()

pcall(function()
    p4:connect()
    -- Convert client spec into a Lua table
    client = p4:fetch_client("-t", template)
    client._root = client_root
    p4:save_client(client)
    p4:run_sync()
end)
