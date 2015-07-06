require 'p4'
p4 = P4.P4()
p4:connect()
change = p4:fetch_change()
-- Files were opened elsewhere and we want to
-- submit a subset that we already know about.
myfiles = { '//depot/some/path/file1.c', '//depot/some/path/file1.h' }
change._description = "My changelist\nSubmitted from P4Lua\n"
change._files = myfiles -- This attribute takes a Lua list
p4:run_submit(change)
