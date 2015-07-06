require 'lanes'
require 'wx'

-- This is our communications pipeline with the thread.  It is inherited
-- as an upvalue for the runp4 lane.
local linda = lanes.linda()

-- Create the lane, giving it full access to all the standard libraries.
runp4 = lanes.gen('*', function()
	require 'p4'
	p4 = P4.P4()

	pcall(function()
		p4:connect()
		p4.keepalive = function()
			-- If the linda receives the finished message, then abort.
			if linda:receive(0, "finished") then return false end
			return true
		end
		files = p4:run{ 'sync', '...', OutputText = function(info) print(info.clientFile) end }
		p4.keepalive = nil
	end)

	for _, w in ipairs(p4.warnings) do
		print(w)
	end

	for _, e in ipairs(p4.errors) do
		print(e)
	end

	p4:disconnect()	
end)

-- Start the thread.
p4result = runp4()

-- Pop up the dialog.
wx.wxMessageBox("Press OK to cancel sync.")

-- After the dialog OK button has been pressed, send the finished message.
linda:send('finished', true)

-- Wait for the thread to exit.
p4result:join()
