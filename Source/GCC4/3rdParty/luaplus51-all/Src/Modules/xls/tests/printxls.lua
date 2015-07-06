local xls = require 'xls'

-- Load a workbook with one sheet, display its contents and
-- save into another file.
local workbook = xls.Workbook(arg[1])
local sheet1 = workbook:GetWorksheet("Sheet1")
if sheet1 then
    local maxRows = sheet1:GetTotalRows()
    local maxCols = sheet1:GetTotalCols()
    print("Dimension of " .. sheet1:GetSheetName() ..
    	" (" .. maxRows .. ", " .. maxCols .. ")")

	print()
	for c = 0, maxCols - 1 do io.write(("%10d"):format(c + 1)) end
    io.write('\n')

    for r = 0, maxRows - 1 do
    	io.write(("%10d"):format(r + 1))
		for c = 0, maxCols - 1 do
			local cell = sheet1:Cell(r, c)
			local cellType = cell:Type()
			if cellType == cell.UNDEFINED then
				io.write(' ')

			elseif cellType == cell.INT then
				io.write(("%10d"):format(cell:GetInteger()))

			elseif cellType == cell.DOUBLE then
				io.write(("%10.6g"):format(cell:GetDouble()))

			elseif cellType == cell.STRING then
				io.write(("%10s"):format(cell:GetString()))

			elseif cellType == cell.WSTRING then
				io.write(("%10s"):format(tostring(cell:GetWString())))
			end
		end
        io.write('\n')
	end
	io.write('\n')
end

