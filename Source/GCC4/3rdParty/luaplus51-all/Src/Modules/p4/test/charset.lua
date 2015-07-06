require 'p4'
p4 = P4.P4()
p4.client = "www"
p4.charset = "iso8859-1"
p4:connect()
p4:run_sync()
p4:disconnect()
