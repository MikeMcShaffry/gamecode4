local xls = require 'xls'

local FW_NORMAL = 400
local FW_BOLD = 700

function example1(path)
	local workbook = xls.Workbook()

    -- create sheet 1 and get the associated worksheet
    workbook:New(1)
	local sheet = workbook:GetWorksheet(0)

	-- Create a table containing an header row in bold and four rows below.

	font_bold = xls.Font()
	font_bold._weight = FW_BOLD

	fmt_bold = workbook:CellFormat()
	fmt_bold:set_font(font_bold)

	local col, row = 0, 0
    for col = 0, 9 do
		local cell = sheet:Cell(row, col)

		cell:Set("TITLE")
		cell:SetFormat(fmt_bold)
	end

	while true do
		row = row + 1
		if row >= 4 then break end
		for col = 0, 9 do
			sheet:Cell(row, col):Set("text")
		end
	end

	row = row + 1

	font_red_bold = xls.Font()
	font_red_bold._weight = FW_BOLD
	font_red_bold._color_index = xls.EGA_RED

	fmt_red_bold = workbook:CellFormat(font_red_bold)
	fmt_red_bold:set_color1(xls.COLOR1_PAT_SOLID);				-- solid background
	fmt_red_bold:set_color2(xls.MAKE_COLOR2(xls.EGA_BLUE,0))	-- blue background

	fmt_green = workbook:CellFormat(xls.Font():set_color_index(xls.EGA_GREEN))

	local col = 0
	while col < 10 do
		local cell = sheet:Cell(row, col)

		cell:Set("xxx")
		cell:SetFormat(fmt_red_bold)

		col = col + 1
		cell = sheet:Cell(row, col)
		cell:Set("yyy")
		cell:SetFormat(fmt_green)

		col = col + 1
	end


	workbook:SaveAs(path)
end


function example2(path)
	local workbook = xls.Workbook()

	-- create sheet 1 and get the associated worksheet
	workbook:New(1)
	local sheet = workbook:GetWorksheet(0)

	local font_header = xls.Font()
	font_header:set_weight(FW_BOLD)
	font_header:set_underline_type(xls.EXCEL_UNDERLINE_SINGLE)
	font_header:set_font_name(xls.wchar("Times New Roman"))
	font_header:set_color_index(xls.EGA_BLUE)
	font_header._options = xls.EXCEL_FONT_STRUCK_OUT

	local fmt_header = workbook:CellFormat(font_header)
	fmt_header:set_rotation(30); -- rotate the header cell text 30° to the left

	local row = 0

	for col = 0, 9 do
		local cell = sheet:Cell(row, col)

		cell:Set("TITLE")
		cell:SetFormat(fmt_header)
	end

	while true do
		row = row + 1
		if row >= 10 then break end
		for col = 0, 9 do
			local buffer = ("text %d/%d"):format(row, col)

			sheet:Cell(row, col):Set(buffer)
		end
	end

	workbook:SaveAs(path)
end

function example3(path)
	local workbook = xls.Workbook()

	 -- create sheet 1 and get the associated worksheet
	workbook:New(1)
	local sheet = workbook:GetWorksheet(0)

	local fmt = workbook:CellFormat()

	 -- row 1

	fmt:set_format_string(xls.XLS_FORMAT_INTEGER)
	local cell = sheet:Cell(0, 0)
	cell:Set(1.0)
	cell:SetFormat(fmt)

	fmt:set_format_string(xls.XLS_FORMAT_DECIMAL)
	cell = sheet:Cell(0, 1)
	cell:Set(2.0)
	cell:SetFormat(fmt)

	fmt:set_format_string(xls.XLS_FORMAT_DATE)
	fmt:set_font(xls.Font():set_weight(FW_BOLD))
	cell = sheet:Cell(0, 2)
	cell:Set("03.03.2000")
	cell:SetFormat(fmt)


	 -- row 2

	fmt:set_font(xls.Font():set_weight(FW_NORMAL))
	fmt:set_format_string(xls.XLS_FORMAT_GENERAL)
	cell = sheet:Cell(1, 0)
	cell:Set("normal")
	cell:SetFormat(fmt)

	fmt:set_format_string(xls.XLS_FORMAT_TEXT)
	cell = sheet:Cell(1, 1)
	cell:Set("Text")
	cell:SetFormat(fmt)

	fmt:set_format_string(xls.XLS_FORMAT_GENERAL)
	fmt:set_font(xls.Font():set_weight(FW_BOLD))
	cell = sheet:Cell(1, 2)
	cell:Set("bold")
	cell:SetFormat(fmt)


	workbook:SaveAs(path)
	workbook:Close()
end


function example_read_write(from, to)
	io.write("read: " .. from .. '\n')

	local workbook = xls.Workbook(from)

	local sheet = workbook:GetWorksheet(0)

	local fmt_general = workbook:CellFormat()

	fmt_general:set_format_string("0.000")

	for y = 0, 1 do
		for x = 0, 1 do
			io.write(tostring(y) .. '/' .. tostring(x))

			local cell = sheet:Cell(y, x)

			local fmt = workbook:CellFormat(cell)

--			cout << " - xf_idx=" << cell:GetXFormatIdx()

			local font = fmt:get_font()
			local font_name = font._name
			io.write("  font name: " .. tostring(font_name))

			local fmt_string = fmt:get_format_string()
			io.write("  format: " .. tostring(fmt_string))

			cell:SetFormat(fmt_general)

			io.write('\n')
		end
	end

	io.write("write: " .. to .. '\n')
	workbook:SaveAs(to)
end


function example4(path)
	local workbook = xls.Workbook()
	workbook:New(1)

	local sheet = workbook:GetWorksheet(0)

	local i = 0

	for row = 0, 8 do
		local color = i
		i = i + 1
		local height = 100 * i

		local buffer = ("Times New Roman %d"):format(height / 20)

		local font = xls.Font()
		font:set_color_index(color)
		font:set_height(height)
		font:set_font_name(xls.wchar("Times New Roman"))

		local fmt = workbook:CellFormat(font)
		fmt:set_background(xls.MAKE_COLOR2(xls.EGA_MAGENTA,0))	-- solid magenta background

		local cell = sheet:Cell(row, 0)
		cell:Set(buffer)
		cell:SetFormat(fmt)
	end

	workbook:SaveAs(path)
end


function copy_sheet(from, to)
	local workbook = xls.Workbook()

	workbook:Load(from)
	workbook:SaveAs(to)
end


function write_big_sheet(path, row_max, col_max)
	local workbook = xls.Workbook()
	local buffer

	 -- create sheet 1 and get the associated BasicExcelWorksheet pointer
	workbook:New(1)
	local sheet = workbook:GetWorksheet(0)


	 -- Create a table containing header row and column in bold.

	local font_bold = xls.Font()
	font_bold._weight = FW_BOLD; -- 700

	local fmt_bold = workbook:CellFormat()
	fmt_bold:set_font(font_bold)

	local col, row

	local cell = sheet:Cell(0, 0)
	cell:Set("Row / Column")
	cell:SetFormat(fmt_bold)

	for col = 1, col_max do
		cell = sheet:Cell(0, col)

		buffer = ("Column %d"):format(col)
		cell:Set(buffer)
		cell:SetFormat(fmt_bold)
	end

	for row = 1, row_max do
		cell = sheet:Cell(row, 0)

		buffer = ("Row %d"):format(row)
		cell:Set(buffer)
		cell:SetFormat(fmt_bold)
	end

	for row = 1, row_max do
		for col = 1, col_max do
			buffer = ("%d / %d"):format(row, col)

			sheet:Cell(row, col):Set(buffer)
		end
	end

	workbook:SaveAs(path)
end

-- call example1()
example1("example1.xls")

-- call example2()
example2("example2.xls")

-- call example3()
example3("example3.xls")

-- dump out cell contents of example3.xls and write modified to example3-out.xls
example_read_write("example3.xls", "example3-out.xls")

-- call example4()
example4("example4.xls")

-- create a table containing 500 x 100 cells
write_big_sheet("big-example.xls", 500, 100)
