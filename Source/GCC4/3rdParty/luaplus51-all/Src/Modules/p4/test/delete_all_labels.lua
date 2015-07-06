require 'p4'
p4 = P4.P4()
p4:connect()
for i, label in ipairs(p4:run_labels()) do
	print("Deleting label " .. label.label)
	p4:delete_label( '-f', label.label )
end
p4:disconnect()
