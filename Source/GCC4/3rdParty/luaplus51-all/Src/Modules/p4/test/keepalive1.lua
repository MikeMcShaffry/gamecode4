require 'p4'
p4 = P4.P4()
p4:connect()
p4.keepalive = function(self)
	-- abort immediately
	return false
end
files = p4:run_sync('...')
p4.keepalive = nil  -- Remove the keep alive function for future commands
p4:disconnect()