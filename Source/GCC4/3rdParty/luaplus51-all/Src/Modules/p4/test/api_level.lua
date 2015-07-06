require 'p4'
p4 = P4.P4()
p4.api_level = 57
p4:connect()
p4:disconnect()
