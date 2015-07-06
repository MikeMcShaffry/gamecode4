require("luacom")

app = luacom.CreateObject("Excel.Application")
if not app then
	print("Error creating object")
	exit(1)
end

app.Visible = true
wb = app.Workbooks:Add()
app:Range("C3"):Select()
app.ActiveCell.FormulaR1C1 = "5"
app:Range("C6"):Select()
app.ActiveCell.FormulaR1C1 = 10

--[[    Range("C3").Select
    ActiveCell.FormulaR1C1 = "5"
    Range("C6").Select
    ActiveCell.FormulaR1C1 = ""
    Range("D6").Select
    ActiveCell.FormulaR1C1 = "10"
    Range("D7").Select
]]
