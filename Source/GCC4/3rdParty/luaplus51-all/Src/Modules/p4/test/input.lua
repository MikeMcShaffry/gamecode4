require 'p4'
p4 = P4.P4()
p4:connect()
change = p4:run_change( "-o" )[1]
change.Description = "Autosubmitted changelist"
p4.input = change
p4:run_submit( "-i" )
p4:disconnect()
