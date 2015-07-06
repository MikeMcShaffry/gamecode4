require 'p4'
p4 = P4.P4()
p4:connect()
clientspec = p4:run_client( "-o" )[1]
clientspec.Description = "Build client"
p4.input = clientspec
p4:run_client( "-i" )
p4:disconnect()
