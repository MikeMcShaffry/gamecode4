require 'p4'
p4 = P4.P4()
p4:connect()
spec = p4:fetch_change()
spec.Description = "Automated change"
p4:run_submit(spec)
p4:disconnect()
