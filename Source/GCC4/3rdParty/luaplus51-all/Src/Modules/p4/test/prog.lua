require 'p4'
p4 = P4.P4()
p4.prog = "sync-script"
print( p4.prog )
p4:connect()
