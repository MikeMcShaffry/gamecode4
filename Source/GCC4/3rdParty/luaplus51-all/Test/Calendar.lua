require("com")

calendar = com.CreateObject("MSCAL.Calendar.7")
if not calendar then
	print("Error creating object")
	exit(1)
end

calendar:AboutBox()

current_day = calendar.Day
print( current_day )
--calendar.Month = calendar.Month + 1
