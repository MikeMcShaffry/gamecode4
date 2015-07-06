require 'p4'
p4 = P4.P4()
p4.exception_level = 1
p4:connect() -- P4Exception on failure
p4:run_sync() -- File(s) up-to-date is a warning - no exception raised
p4:disconnect()
