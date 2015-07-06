require 'p4'
p4 = P4.P4()
p4:connect()
clientspec = p4:fetch_client()
clientspec.Description = "Build client 2"
p4:save_client( clientspec )
p4:disconnect()
