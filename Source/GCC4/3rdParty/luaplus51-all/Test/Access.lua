require "com"

app = com.CreateObject("Access.Application")
if not app then
	print("Error creating object")
	io.exit(1)
end

app.Visible = true
app.OpenCurrentDatabase("TestDatabase.mdb")

