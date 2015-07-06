## Introduction

The module *xls* allows high performance read and write access to .xls files.



## Example

<pre>
    require 'xls'

    local doc = xls.BasicExcel()
</pre>



-------------------------
## xls Reference

### workbook = xls.Workbook([filename])

Creates or opens a Workbook object.  Returns a Workbook object.

* `filename` is the optional name of the .xls file to open.  If no `filename` is given, an empty Workbook is created.




-------------------------
## Workbook Reference

### workbook:New(sheets [ = 3])

Creates a new Excel workbook with a given number of worksheets.

* `sheets` is the number of worksheets to create within the workbook.  The minimum is 1.  The default is 3.


### loaded = workbook:Load(filename)

Loads an Excel workbook from a file.

Returns `true` if successful, `false` otherwise.

* `filename` is the path to the .xls file to open.



### saved = workbook:Save()

Saves current Excel workbook to the opened file.

Returns `true` if successful, `false` if unsuccessful.




### saved = workbook:SaveAs(filename)

Loads an Excel workbook from a file.

Returns `true` if successful, `false` otherwise.

* `filename` is the path to the .xls file to save.  `filename` may be a regular string or an `xls.wchar`.



### workbook:Close()

Closes the current Excel workbook.




### totalWorksheets = workbook:GetTotalWorksheets()

Returns the total number of Excel worksheets in the current Excel workbook.




### worksheet = workbook:GetWorksheet(sheetIndex | name)

Returns an object representing the Excel worksheet specified by `sheetIndex` or `name`.  If no worksheet is found with the given `sheetIndex` or `name`, then `GetWorksheet` returns `nil`.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the sheet to retrieve.
* `name` is either a string or an xls.wchar representing the sheet name to retrieve.




### worksheet = workbook:AddWorksheet([sheetIndex] | [name [, sheetIndex]])

Adds a new worksheet to the workbook.  If no `sheetIndex` or `name` is specified, a new worksheet is added to the last position.  

Returns an Excel worksheet specified by `sheetIndex` or `name`.

* `sheetIndex` is an index in the range of -1 <= `sheetIndex` < `GetTotalWorksheets()`.  If `-1` is specified, the new worksheet is added to the last position.  The name given to the new worksheet is `SheetX`, where `X` is a number which starts from 1.
* `name` is either a string or an xls.wchar that will become the name of the worksheet being added.




### deleted = workbook:DeleteWorksheet(sheetIndex | name)

Deletes the specified Excel worksheet from the workbook.

Returns `true` if the worksheet was deleted, `false` otherwise.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.
* `name` is either a string or an xls.wchar representing the sheet name to delete.




### ansiSheetName = workbook:GetAnsiSheetName(sheetIndex)

If possible, returns the ANSI name of the sheet.  Otherwise, returns `nil` if the name is Unicode.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.




### unicodeSheetName = workbook:GetUnicodeSheetName(sheetIndex)

If possible, returns the Unicode name of the sheet formatted as an `xls.wchar`.  Otherwise, returns `nil` if the name is ANSI.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.




### sheetName = workbook:GetSheetName(sheetIndex)

Returns either the ANSI name of the sheet or the Unicode name of the sheet as an `xls.wchar`.

* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.




### renamed = workbook:RenameWorksheet(sheetIndex | from, to)

Renames an Excel worksheet at `sheetIndex` or `from` to the name specified by `to`.

Returns `true` if the operation succeeded, `false` otherwise.


* `sheetIndex` is an index in the range of 0 <= `sheetIndex` < `GetTotalWorksheets()` representing the worksheet to delete.
* `from` is either a string or `xls.wchar` name of the worksheet to rename.
* `to` is either a string or `xls.wchar` name, where the type must match that of `from` if `from` is specified, of the worksheet to rename.




-------------------------
## Worksheet Reference

### ansiSheetName = worksheet:GetAnsiSheetName()

If possible, returns the ANSI name of the sheet.  Otherwise, returns `nil` if the name is Unicode.




### unicodeSheetName = worksheet:GetUnicodeSheetName()

If possible, returns the Unicode name of the sheet formatted as an `xls.wchar`.  Otherwise, returns `nil` if the name is ANSI.




### sheetName = worksheet:GetSheetName(sheetIndex)

Returns either the ANSI name of the sheet or the Unicode name of the sheet as an `xls.wchar`.




### renamed = worksheet:Rename(to)

Renames the Excel worksheet to the name specified by `to`.

Returns `true` if the operation succeeded, `false` otherwise.


* `to` is either a string or `xls.wchar` name, where the type must match that of `from` if `from` is specified, of the worksheet to rename.




### totalRows = worksheet:GetTotalRows()

Returns the total number of rows in the Excel worksheet.




### totalColumns = worksheet:GetTotalCols()

Returns the total number of columns in the Excel worksheet.



### cell = worksheet:Cell(row, col)

Retrieves the contents of a cell from the Excel worksheet.

Returns the cell if the operation succeeded, `nil` if either the row or column are not in range.

* `row` is a value from 0 to 65535, representing the row in the Excel worksheet to retrieve.
* `col` is a value from 0 to 255, representing the column in the Excel worksheet to retrieve.



### erased = worksheet:EraseCell(row, col)

Erases the contents of a cell from the Excel worksheet.

Returns `true` if successful, `false` if either the row or column are not in range.

* `row` is a value from 0 to 65535, representing a row in the Excel worksheet.
* `col` is a value from 0 to 255, representing a column in the Excel worksheet.



### worksheet:SetColWidth(col)

Sets the width of the given column in the Excel worksheet.

* `col` is a value from 0 to 255, representing the column in the Excel worksheet to retrieve.



### columnWidth = worksheet:GetColWidth(col)

Returns the width of the column in the Excel worksheet.

* `col` is a value from 0 to 255, representing a column in the Excel worksheet.




### worksheet:MergeCells(row, col, rowRange, colRange)

Merges the cell specified by `row` and `col` into a single cell consisting in height of `rowRange` rows and `colRange` columns.

* `row` is a value from 0 to 65535, representing a row in the Excel worksheet.
* `col` is a value from 0 to 255, representing a column in the Excel worksheet.
* `rowRange` is a value from 1 to (65535 - row - 1).
* `colRange` is a value from 1 to (255 - row - 1).







-------------------------
## Cell Reference

### cellType = cell:Type()

Returns one of the following as the type of this Excel cell.

* `cell.UNDEFINED`
* `cell.INT`
* `cell.DOUBLE`
* `cell.STRING`
* `cell.WSTRING`
* `cell.FORMULA`



### cellContent = cell:Get()

If the type of the cell is `cell.INT` or `cell.DOUBLE`, the integer or double content of the cell is returned as a Lua number.  If the type of the cell is `cell.STRING`, the ANSI string content of the cell is returned as a Lua string.  If the type of the cell is `cell.WSTRING`, the Unicode string content of the cell are returned as an `xls.wchar`.  Otherwise, `nil` is returned.



### cellContent = cell:GetInteger()

If the type of the cell is `cell.INT`, the integer content of the cell is returned as a Lua number.  Otherwise, `nil` is returned.



### cellContent = cell:GetDouble()

If the type of the cell is `cell.DOUBLE`, the double content of the cell is returned as a Lua number.  Otherwise, `nil` is returned.



### cellContent = cell:GetString()

If the type of the cell is `cell.STRING`, the ANSI string content of the cell is returned as a Lua string.  Otherwise, `nil` is returned.



### cellContent = cell:GetWString()

If the type of the cell is `cell.WSTRING`, the Unicode string content of the cell is returned as an `xls.wchar`.  Otherwise, `nil` is returned.



### cell:Set(value)

Sets the content of the cell to `value`.

* `value` represents the new content of the cell.
** If `value` is a number, the number is assumed to be a double, and the cell type becomes `cell.DOUBLE`.
** If `value` is a string, the cell type becomes `cell.STRING`.
** If `value` is an `xls.wchar`, the cell type becomes `cell.WSTRING`.



### cell:SetInteger(value)

Sets the content of the cell to `value`.

* `value` represents the new integer content of the cell.  The cell type becomes `cell.INT`.



### cell:SetRKValue(value)

To be documented.



### cell:SetString(value)

Sets the content of the cell to `value`.

* `value` represents the new ANSI string content of the cell.  The cell type becomes `cell.STRING`.



### cell:SetWString(value)

Sets the content of the cell to `value`.

* `value` represents the new Unicode string content of the cell.  The cell type becomes `cell.WSTRING`.



### cell:SetFormula(value)

NOT IMPLEMENTED.



### cell:GetXFormatIdx()

To be documented.



### cell:SetXFormatIdx(value)

To be documented.



### cell:SetFormat(value)

To be documented.



### cell:EraseContents()

Erase the content of the current Excel cell.  The cell type becomes `cell.UNDEFINED`.



### numberOfMergedRows = cell:GetMergedRows()

Return the number of merged rows in the current Excel cell.



### numberOfMergedColumns = cell:GetMergedColumns()

Return the number of merged columns in the current Excel cell.



### cell:SetMergedRows(mergedRows)

Sets the number of merged rows in the current Excel cell.



### cell:SetMergedColumns(mergedColumns)

Sets the number of merged columns in the current Excel cell.




-------------------------
## CellFormat Reference




-------------------------
## ExcelFont Reference

### newFont = xls.Font()

Creates or opens a Workbook object.  Returns a Workbook object.

* `filename` is the optional name of the .xls file to open.  If no `filename` is given, an empty Workbook is created.




