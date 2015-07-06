#include <ExcelFormat.h>
using namespace ExcelFormat;

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lstate.h"
}

#define XLS_WCHAR_METATABLE "xls.wcharMetatable"
#define WORKBOOK_METATABLE "xls.WorkbookMetatable"
#define WORKSHEET_METATABLE "xls.WorksheetMetatable"
#define CELL_METATABLE "xls.CellMetatable"
#define FONT_METATABLE "xls.Font"
#define CELLFORMAT_METATABLE "xls.CellFormat"

struct xls_WChar {
	size_t len;

	const wchar_t* data() { return (wchar_t*)(this+1); }
};


xls_WChar* lxls_wchar_new(lua_State* L, const char* str, size_t len) {
	size_t count = mbstowcs(NULL, str, len);
	xls_WChar* xwchar = (xls_WChar*)lua_newuserdata(L, sizeof(xls_WChar) + (count + 1) * sizeof(wchar_t));
	xwchar->len = count;
	mbstowcs((wchar_t*)xwchar->data(), str, len);
	((wchar_t*)xwchar->data())[count] = 0;

	luaL_getmetatable(L, XLS_WCHAR_METATABLE);
	lua_setmetatable(L, -2);

	return xwchar;
}


xls_WChar* lxls_wchar_new(lua_State* L, const wchar_t* str) {
	size_t count = wcslen(str);
	xls_WChar* xwchar = (xls_WChar*)lua_newuserdata(L, sizeof(xls_WChar) + (count + 1) * sizeof(wchar_t));
	xwchar->len = count;
	memcpy((wchar_t*)xwchar->data(), str, (count + 1) * sizeof(wchar_t));

	luaL_getmetatable(L, XLS_WCHAR_METATABLE);
	lua_setmetatable(L, -2);

	return xwchar;
}


static xls_WChar* lxls_wchar_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, XLS_WCHAR_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "xls.wchar expected");
  return (xls_WChar*)ud;
}


static xls_WChar* lxls_wchar_optcheck(lua_State* L, int index) {
	void *p = lua_touserdata(L, index);
	if (p != NULL) {  /* value is a userdata? */
		if (lua_getmetatable(L, index)) {  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, XLS_WCHAR_METATABLE);  /* get correct metatable */
			if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
				lua_pop(L, 2);  /* remove both metatables */
				return (xls_WChar*)p;
			}
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}

	return NULL;
}


static int lxls_wchar___gc(lua_State* L) {
	xls_WChar* xwchar = lxls_wchar_check(L, 1);
	return 0;
}


static int lxls_wchar___tostring(lua_State* L) {
	xls_WChar* xwchar = lxls_wchar_check(L, 1);
	size_t wcstrlen = lua_objlen(L, 1);
	size_t count = wcstombs(NULL, xwchar->data(), xwchar->len);
	char* mbstr = new char[count + 1];
	wcstombs(mbstr, xwchar->data(), xwchar->len);
	lua_pushlstring(L, mbstr, count);
	delete[] mbstr;
	return 1;
}


static int lxls_wchar___len(lua_State* L) {
	xls_WChar* xwchar = lxls_wchar_check(L, 1);
	lua_pushinteger(L, xwchar->len);
	return 1;
}


static const struct luaL_reg xls_wchar_funcs[] = {
	{ "__gc",				lxls_wchar___gc },

	{ "__tostring",			lxls_wchar___tostring },
	{ "__len",				lxls_wchar___len },

	{ NULL, NULL },
};


struct CombinedBasicExcelAndXLSFormatManager {
	CombinedBasicExcelAndXLSFormatManager()
	{
	}

	CombinedBasicExcelAndXLSFormatManager(const char* filename)
		: excel(filename)
	{
	}

	BasicExcel excel;
	XLSFormatManager* formatManager;
};


CombinedBasicExcelAndXLSFormatManager* lBasicExcel_checkmetatable(lua_State* L, int index) {
	void *ud = luaL_checkudata(L, index, WORKBOOK_METATABLE);
	luaL_argcheck(L, ud != NULL, index, "xls.Workbook expected");
	return (CombinedBasicExcelAndXLSFormatManager*)ud;
}


static BasicExcel* lBasicExcel_check(lua_State* L, int index) {
	return &lBasicExcel_checkmetatable(L, index)->excel;
}


static XLSFormatManager* lXlsFormatManager_check(lua_State* L, int index) {
	return lBasicExcel_checkmetatable(L, index)->formatManager;
}


static CellFormat* lCellFormat_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, CELLFORMAT_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "xls.CellFormat expected");
  return (CellFormat*)ud;
}


int lxls_BasicExcel(lua_State* L)
{
	const char* filename = luaL_optstring(L, 1, NULL);
	CombinedBasicExcelAndXLSFormatManager* cbex = (CombinedBasicExcelAndXLSFormatManager*)lua_newuserdata(L, sizeof(CombinedBasicExcelAndXLSFormatManager));
	if (filename)
		::new(cbex) CombinedBasicExcelAndXLSFormatManager(filename);
	else
		::new(cbex) CombinedBasicExcelAndXLSFormatManager();
	cbex->formatManager = new XLSFormatManager(cbex->excel);

	luaL_getmetatable(L, WORKBOOK_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


static int lBasicExcel___gc(lua_State* L) {
	CombinedBasicExcelAndXLSFormatManager* cbex = lBasicExcel_checkmetatable(L, 1);
	delete cbex->formatManager;
	cbex->~CombinedBasicExcelAndXLSFormatManager();
	return 0;
}


void lxls_BasicExcelWorksheet(lua_State* L, BasicExcelWorksheet* worksheet)
{
	void* ud = lua_newuserdata(L, sizeof(BasicExcelWorksheet*));
	*(BasicExcelWorksheet**)ud = worksheet;

	luaL_getmetatable(L, WORKSHEET_METATABLE);
	lua_setmetatable(L, -2);
}


void lxls_BasicExcelCell(lua_State* L, BasicExcelCell* cell)
{
	void* ud = lua_newuserdata(L, sizeof(BasicExcelCell*));
	*(BasicExcelCell**)ud = cell;

	luaL_getmetatable(L, CELL_METATABLE);
	lua_setmetatable(L, -2);
}


int lBasicExcel_CellFormat(lua_State* L) {
	XLSFormatManager* xlsFormatManager = lXlsFormatManager_check(L, 1);
	int numArgs = lua_gettop(L);
	CellFormat* cellFormat = (CellFormat*)lua_newuserdata(L, sizeof(CellFormat));

	if (numArgs != 1  &&  numArgs != 2) {
		luaL_error(L, "Workbook:CellFormat() - expected 0 or 1 arguments");
	}

	if (numArgs == 1) {
		::new(cellFormat) CellFormat(*xlsFormatManager);
	} else if (lua_isuserdata(L, 2)) {
		void* p = lua_touserdata(L, 2);
		if (lua_getmetatable(L, 2)) {
			lua_getfield(L, LUA_REGISTRYINDEX, FONT_METATABLE);
			if (lua_rawequal(L, -1, -2)) {
				::new(cellFormat) CellFormat(*xlsFormatManager, *(ExcelFont*)p);
				lua_pop(L, 2);
			} else {
				lua_pop(L, 1);
				lua_getfield(L, LUA_REGISTRYINDEX, CELL_METATABLE);
				if (lua_rawequal(L, -1, -2)) {
					::new(cellFormat) CellFormat(*xlsFormatManager, *(BasicExcelCell**)p);
					lua_pop(L, 2);
				} else {
					luaL_argerror(L, 2, "expected nothing or xls.ExcelFont or xls.BasicExcelCell");
				}
			}
		}
	}

	luaL_getmetatable(L, CELLFORMAT_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


int lBasicExcel_New(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	int sheets = (int)luaL_optnumber(L, 2, 3);
	xls->New(sheets);
	return 0;
}


int lBasicExcel_Load(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	if (lua_isstring(L, 2)) {
		const char* filename = luaL_checkstring(L, 2);
		lua_pushboolean(L, xls->Load(filename));
	} else {
		xls_WChar* xwchar = lxls_wchar_optcheck(L, 2);
		if (!xwchar)
			luaL_argerror(L, 2, "expected string or xls.wchar");
		lua_pushboolean(L, xls->Load(xwchar->data()));
	}

	return 1;
}


int lBasicExcel_Save(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	lua_pushboolean(L, xls->Save());
	return 1;
}


int lBasicExcel_SaveAs(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	if (lua_isstring(L, 2)) {
		const char* filename = luaL_checkstring(L, 2);
		lua_pushboolean(L, xls->SaveAs(filename));
	} else {
		xls_WChar* xwchar = lxls_wchar_optcheck(L, 2);
		if (!xwchar)
			luaL_argerror(L, 2, "expected string or xls.wchar");
		lua_pushboolean(L, xls->SaveAs(xwchar->data()));
	}
	return 1;
}


int lBasicExcel_Close(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	xls->Close();
	return 0;
}


int lBasicExcel_GetTotalWorksheets(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	lua_pushinteger(L, xls->GetTotalWorkSheets());
	return 1;
}


int lBasicExcel_GetWorksheet(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	BasicExcelWorksheet* worksheet;
	if (lua_isnumber(L, 2)) {
		int sheetIndex = lua_tointeger(L, 2);
		if (sheetIndex >= xls->GetTotalWorkSheets())
			return 0;
		worksheet = xls->GetWorksheet(lua_tointeger(L, 2));
	} else if (lua_isstring(L, 2))
		worksheet = xls->GetWorksheet(lua_tostring(L, 2));
	else {
		xls_WChar* xwchar = lxls_wchar_optcheck(L, 2);
		if (!xwchar)
			luaL_argerror(L, 2, "expected integer or string or xls.wchar");
		worksheet = xls->GetWorksheet(xwchar->data());
	}
	lxls_BasicExcelWorksheet(L, worksheet);
	return 1;
}


int lBasicExcel_AddWorksheet(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	BasicExcelWorksheet* worksheet;
	if (lua_gettop(L) == 1) {
		worksheet = xls->AddWorksheet();
	} else if (lua_isstring(L, 2)) {
		int sheetIndex = luaL_optinteger(L, 3, -1);
		worksheet = xls->AddWorksheet(lua_tostring(L, 2), sheetIndex);
	} else if (lua_isnumber(L, 2))
		worksheet = xls->AddWorksheet(lua_tointeger(L, 2));
	else {
		xls_WChar* xwchar = lxls_wchar_optcheck(L, 2);
		if (!xwchar)
			luaL_argerror(L, 2, "expected integer or string or xls.wchar");
		int sheetIndex = luaL_optinteger(L, 3, -1);
		worksheet = xls->AddWorksheet(xwchar->data(), sheetIndex);
	}
	lxls_BasicExcelWorksheet(L, worksheet);
	return 1;
}


int lBasicExcel_DeleteWorksheet(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	if (lua_isnumber(L, 2))
		lua_pushboolean(L, xls->DeleteWorksheet(lua_tointeger(L, 2)));
	else if (lua_isstring(L, 2))
		lua_pushboolean(L, xls->DeleteWorksheet(lua_tostring(L, 2)));
	else {
		xls_WChar* xwchar = lxls_wchar_optcheck(L, 2);
		if (!xwchar)
			luaL_argerror(L, 2, "expected integer or string or xls.wchar");
		lua_pushboolean(L, xls->DeleteWorksheet(xwchar->data()));
	}
	return 1;
}


int lBasicExcel_GetAnsiSheetName(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	int sheetIndex = luaL_checkinteger(L, 2);
	lua_pushstring(L, xls->GetAnsiSheetName(sheetIndex));
	return 1;
}


int lBasicExcel_GetUnicodeSheetName(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	int sheetIndex = luaL_checkinteger(L, 2);
	lxls_wchar_new(L, xls->GetUnicodeSheetName(sheetIndex));
	return 1;
}


int lBasicExcel_GetSheetName(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	int sheetIndex = luaL_checkinteger(L, 2);

	{
		char ansiSheetName[4096];
		if (xls->GetSheetName(sheetIndex, ansiSheetName)) {
			lua_pushstring(L, ansiSheetName);
			return 1;
		}
	}

	{
		wchar_t unicodeSheetName[4096];
		if (xls->GetSheetName(sheetIndex, unicodeSheetName)) {
			lxls_wchar_new(L, unicodeSheetName);
			return 1;
		}
	}

	return 0;
}


int lBasicExcel_RenameWorksheet(lua_State* L) {
	BasicExcel* xls = lBasicExcel_check(L, 1);
	if (lua_isnumber(L, 2)) {
		int sheetIndex = lua_tointeger(L, 2);
		if (lua_isstring(L, 3)) {
			const char* to = lua_tostring(L, 3);
			lua_pushboolean(L, xls->RenameWorksheet(sheetIndex, to));
		} else {
			xls_WChar* to = lxls_wchar_optcheck(L, 3);
			if (!to)
				luaL_argerror(L, 3, "expected string or xls.wchar");
			lua_pushboolean(L, xls->RenameWorksheet(sheetIndex, to->data()));
		}
	} else if (lua_isstring(L, 2)) {
		const char* from = lua_tostring(L, 2);
		const char* to = luaL_checkstring(L, 3);
		lua_pushboolean(L, xls->RenameWorksheet(from, to));
	} else {
		xls_WChar* from = lxls_wchar_optcheck(L, 2);
		if (!from)
			luaL_argerror(L, 2, "expected integer or string or xls.wchar");
		xls_WChar* to = lxls_wchar_check(L, 3);
		if (!to)
			luaL_argerror(L, 3, "expected string or xls.wchar");
		lua_pushboolean(L, xls->RenameWorksheet(from->data(), to->data()));
	}
	return 1;
}


static const struct luaL_reg BasicExcel_funcs[] = {
	{ "__gc",				lBasicExcel___gc },

	{ "CellFormat",			lBasicExcel_CellFormat },

	{ "New",				lBasicExcel_New },
	{ "Load",				lBasicExcel_Load },
	{ "Save",				lBasicExcel_Save },
	{ "SaveAs",				lBasicExcel_SaveAs },
	{ "Close",				lBasicExcel_Close },

	{ "GetTotalWorksheets",	lBasicExcel_GetTotalWorksheets },
	{ "GetWorksheet",		lBasicExcel_GetWorksheet },
	{ "AddWorksheet",		lBasicExcel_AddWorksheet },
	{ "DeleteWorksheet",	lBasicExcel_DeleteWorksheet },

	{ "GetAnsiSheetName",	lBasicExcel_GetAnsiSheetName },
	{ "GetUnicodeSheetName",lBasicExcel_GetUnicodeSheetName },
	{ "GetSheetName",		lBasicExcel_GetSheetName },

	{ "RenameWorksheet",   	lBasicExcel_RenameWorksheet },
	{ NULL, NULL },
};


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static BasicExcelWorksheet* lBasicExcelWorksheet_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, WORKSHEET_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "xls.BasicExcelWorksheet expected");
  return *(BasicExcelWorksheet**)ud;
}


int lBasicExcelWorksheet_GetAnsiSheetName(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	lua_pushstring(L, worksheet->GetAnsiSheetName());
	return 1;
}


int lBasicExcelWorksheet_GetUnicodeSheetName(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	lxls_wchar_new(L, worksheet->GetUnicodeSheetName());
	return 1;
}


int lBasicExcelWorksheet_GetSheetName(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);

	{
		char ansiSheetName[4096];
		if (worksheet->GetSheetName(ansiSheetName)) {
			lua_pushstring(L, ansiSheetName);
			return 1;
		}
	}

	{
		wchar_t unicodeSheetName[4096];
		if (worksheet->GetSheetName(unicodeSheetName)) {
			lxls_wchar_new(L, unicodeSheetName);
			return 1;
		}
	}

	return 0;
}


int lBasicExcelWorksheet_Rename(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	if (lua_isstring(L, 2)) {
		const char* to = lua_tostring(L, 2);
		lua_pushboolean(L, worksheet->Rename(to));
	} else {
		xls_WChar* to = lxls_wchar_optcheck(L, 2);
		if (!to)
			luaL_argerror(L, 2, "expected string or xls.wchar");
		lua_pushboolean(L, worksheet->Rename(to->data()));
	}
	return 1;
}


int lBasicExcelWorksheet_GetTotalRows(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	lua_pushinteger(L, worksheet->GetTotalRows());
	return 1;
}


int lBasicExcelWorksheet_GetTotalCols(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	lua_pushinteger(L, worksheet->GetTotalCols());
	return 1;
}


int lBasicExcelWorksheet_Cell(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	int row = luaL_checkinteger(L, 2);
	int col = luaL_checkinteger(L, 3);
	BasicExcelCell* cell = worksheet->Cell(row, col);
	lxls_BasicExcelCell(L, cell);
	return 1;
}


int lBasicExcelWorksheet_EraseCell(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	int row = luaL_checkinteger(L, 2);
	int col = luaL_checkinteger(L, 3);
	lua_pushboolean(L, worksheet->EraseCell(row, col));
	return 1;
}


int lBasicExcelWorksheet_SetColWidth(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	int colindex = luaL_checkinteger(L, 2);
	int colwidth = luaL_checkinteger(L, 3);
	worksheet->SetColWidth(colindex, colwidth);
	return 0;
}


int lBasicExcelWorksheet_GetColWidth(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	int colindex = luaL_checkinteger(L, 2);
	lua_pushinteger(L, worksheet->GetColWidth(colindex));
	return 1;
}


int lBasicExcelWorksheet_MergeCells(lua_State* L) {
	BasicExcelWorksheet* worksheet = lBasicExcelWorksheet_check(L, 1);
	int row = luaL_checkinteger(L, 2);
	int col = luaL_checkinteger(L, 3);
	int rowRange = luaL_checkinteger(L, 4);
	int colRange = luaL_checkinteger(L, 5);
	worksheet->MergeCells(row, col, rowRange, colRange);
	return 1;
}


static const struct luaL_reg BasicExcelWorksheet_funcs[] = {
	{ "GetAnsiSheetName",  	lBasicExcelWorksheet_GetAnsiSheetName },
	{ "GetUnicodeSheetName",lBasicExcelWorksheet_GetUnicodeSheetName },
	{ "GetSheetName",  		lBasicExcelWorksheet_GetSheetName },
	{ "Rename",  			lBasicExcelWorksheet_Rename },

	{ "GetTotalRows",  	  	lBasicExcelWorksheet_GetTotalRows },
	{ "GetTotalCols",  	  	lBasicExcelWorksheet_GetTotalCols },

	{ "Cell",  	  			lBasicExcelWorksheet_Cell },
	{ "EraseCell",    		lBasicExcelWorksheet_EraseCell },

	{ "SetColWidth",  		lBasicExcelWorksheet_SetColWidth },
	{ "GetColWidth",  		lBasicExcelWorksheet_GetColWidth },

	{ "MergeCells",  		lBasicExcelWorksheet_MergeCells },
	{ NULL, NULL },
};


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static BasicExcelCell* lBasicExcelCell_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, CELL_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "xls.BasicExcelCell expected");
  return *(BasicExcelCell**)ud;
}


int lBasicExcelCell_Type(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lua_pushinteger(L, cell->Type());
	return 1;
}


int lBasicExcelCell_Get(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	double dval;
	int ival;
	bool ret = cell->Get(dval);
	if (ret) {
		lua_pushnumber(L, dval);
		return 1;
	}
	ret = cell->Get(ival);
	if (ret) {
		lua_pushinteger(L, ival);
		return 1;
	}

	const char* ansiString = cell->GetString();
	if (ansiString) {
		lua_pushstring(L, ansiString);
		return 1;
	}

	const wchar_t* unicodeString = cell->GetWString();
	if (unicodeString) {
		lxls_wchar_new(L, unicodeString);
		return 1;
	}

	return 0;
}


int lBasicExcelCell_GetStringLength(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	if (cell->Type() != BasicExcelCell::STRING  &&  cell->Type() != BasicExcelCell::WSTRING)
		luaL_error(L, "expected cell to be a BasicExcelCell.STRING or BasicExcelCell.WSTRING");
	lua_pushinteger(L, cell->GetStringLength());
	return 1;
}


int lBasicExcelCell_GetInteger(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lua_pushinteger(L, cell->GetInteger());
	return 1;
}


int lBasicExcelCell_GetDouble(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lua_pushnumber(L, cell->GetDouble());
	return 1;
}


int lBasicExcelCell_GetString(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lua_pushstring(L, cell->GetString());
	return 1;
}


int lBasicExcelCell_GetWString(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lxls_wchar_new(L, cell->GetWString());
	return 1;
}


int lBasicExcelCell_Set(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	if (lua_isnumber(L, 2))
		cell->Set(lua_tonumber(L, 2));
	else if (lua_isstring(L, 2))
		cell->Set(lua_tostring(L, 2));
	else {
		xls_WChar* xwchar = lxls_wchar_optcheck(L, 2);
		if (!xwchar)
			luaL_argerror(L, 2, "expected number, string, or xls.wchar");
		cell->Set(xwchar->data());
	}
	return 0;
}


int lBasicExcelCell_SetInteger(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetInteger(luaL_checkinteger(L, 2));
	return 0;
}


int lBasicExcelCell_SetDouble(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetDouble(luaL_checknumber(L, 2));
	return 0;
}


int lBasicExcelCell_SetRKValue(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetRKValue(luaL_checkinteger(L, 2));
	return 0;
}


int lBasicExcelCell_SetString(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetString(luaL_checkstring(L, 2));
	return 0;
}


int lBasicExcelCell_SetWString(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetWString(lxls_wchar_check(L, 2)->data());
	return 0;
}


int lBasicExcelCell_SetFormula(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	luaL_error(L, "SetFormula() is unimplemented.\n");
	return 0;
}


int lBasicExcelCell_GetXFormatIdx(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lua_pushinteger(L, cell->GetXFormatIdx());
	return 1;
}


int lBasicExcelCell_SetXFormatIdx(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetXFormatIdx(luaL_checkinteger(L, 2));
	return 0;
}


int lBasicExcelCell_SetFormat(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	CellFormat* cellFormat = lCellFormat_check(L, 2);
	cell->SetFormat(*cellFormat);
	return 0;
}


int lBasicExcelCell_EraseContents(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->EraseContents();
	return 0;
}


int lBasicExcelCell_GetMergedRows(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lua_pushinteger(L, cell->GetMergedRows());
	return 1;
}


int lBasicExcelCell_GetMergedColumns(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	lua_pushinteger(L, cell->GetMergedColumns());
	return 1;
}


int lBasicExcelCell_SetMergedRows(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetMergedRows(luaL_checkinteger(L, 2));
	return 0;
}


int lBasicExcelCell_SetMergedColumns(lua_State* L) {
	BasicExcelCell* cell = lBasicExcelCell_check(L, 1);
	cell->SetMergedColumns(luaL_checkinteger(L, 2));
	return 0;
}


static const struct luaL_reg BasicExcelCell_funcs[] = {
	{ "Type",  				lBasicExcelCell_Type },

	{ "Get",  				lBasicExcelCell_Get },
	{ "GetStringLength",   	lBasicExcelCell_GetStringLength },

	{ "GetInteger",   		lBasicExcelCell_GetInteger },
	{ "GetDouble",   		lBasicExcelCell_GetDouble },
	{ "GetString",   		lBasicExcelCell_GetString },
	{ "GetWString",   		lBasicExcelCell_GetWString },

	{ "Set",  				lBasicExcelCell_Set },

	{ "SetInteger",    		lBasicExcelCell_SetInteger },
	{ "SetDouble",    		lBasicExcelCell_SetDouble },
	{ "SetRKValue",    		lBasicExcelCell_SetRKValue },
	{ "SetString",    		lBasicExcelCell_SetString },
	{ "SetWString",    		lBasicExcelCell_SetWString },

	{ "SetFormula",    		lBasicExcelCell_SetFormula },

	{ "GetXFormatIdx", 		lBasicExcelCell_GetXFormatIdx },
	{ "SetXFormatIdx", 		lBasicExcelCell_SetXFormatIdx },
	{ "SetFormat", 			lBasicExcelCell_SetFormat },

	{ "EraseContents", 	   	lBasicExcelCell_EraseContents },

	{ "GetMergedRows", 	   	lBasicExcelCell_GetMergedRows },
	{ "GetMergedColumns", 	lBasicExcelCell_GetMergedColumns },
	
	{ "SetMergedRows", 	   	lBasicExcelCell_SetMergedRows },
	{ "SetMergedColumns", 	lBasicExcelCell_SetMergedColumns },
	
	{ NULL, NULL },
};


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
static ExcelFont* lExcelFont_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, FONT_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "xls.ExcelFont expected");
  return (ExcelFont*)ud;
}


int lExcelFont___gc(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	excelFont->~ExcelFont();
	return 0;
}


int lExcelFont_set_height(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	excelFont->set_height(luaL_checkinteger(L, 2));
	lua_pushvalue(L, 1);
	return 1;
}


int lExcelFont_set_weight(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	excelFont->set_weight(luaL_checkinteger(L, 2));
	lua_pushvalue(L, 1);
	return 1;
}


int lExcelFont_set_italic(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	excelFont->set_italic(luaL_checkboolean(L, 2) != 0);
	lua_pushvalue(L, 1);
	return 1;
}


int lExcelFont_set_color_index(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	excelFont->set_color_index(luaL_checkinteger(L, 2));
	lua_pushvalue(L, 1);
	return 1;
}


int lExcelFont_set_underline_type(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	excelFont->set_underline_type(luaL_checkinteger(L, 2));
	lua_pushvalue(L, 1);
	return 1;
}


int lExcelFont_set_escapement(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	excelFont->set_escapement(luaL_checkinteger(L, 2));
	lua_pushvalue(L, 1);
	return 1;
}


int lExcelFont_set_font_name(lua_State* L) {
	ExcelFont* excelFont = lExcelFont_check(L, 1);
	xls_WChar* xwchar = lxls_wchar_check(L, 2);
	excelFont->set_font_name(xwchar->data());
	lua_pushvalue(L, 1);
	return 1;
}


static const struct luaL_reg ExcelFont_funcs[] = {
	{ "__gc",				lExcelFont___gc },
	{ "set_height",    		lExcelFont_set_height },
	{ "set_weight",    		lExcelFont_set_weight },
	{ "set_italic",    		lExcelFont_set_italic },
	{ "set_color_index",    lExcelFont_set_color_index },
	{ "set_underline_type", lExcelFont_set_underline_type },
	{ "set_escapement", 	lExcelFont_set_escapement },
	{ "set_font_name",		lExcelFont_set_font_name },

	{ NULL, NULL },
};


static void l_ExcelFont_index__name(lua_State* L, ExcelFont* excelFont) {
	lxls_wchar_new(L, excelFont->_name.c_str());
}


static void l_ExcelFont_index__height(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_height);
}


static void l_ExcelFont_index__weight(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_weight);
}


static void l_ExcelFont_index__options(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_options);
}


static void l_ExcelFont_index__color_index(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_color_index);
}


static void l_ExcelFont_index__escapement_type(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_escapement_type);
}


static void l_ExcelFont_index__underline_type(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_underline_type);
}


static void l_ExcelFont_index__font_family(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_font_family);
}


static void l_ExcelFont_index__character_set(lua_State* L, ExcelFont* excelFont) {
	lua_pushinteger(L, excelFont->_character_set);
}


typedef void (*l_ExcelFont_index_property_func)(lua_State*, ExcelFont*);
typedef struct l_ExcelFont_index_properties_reg {
	const char *name;
	l_ExcelFont_index_property_func func;
} l_ExcelFont_index_properties_reg;


static const struct l_ExcelFont_index_properties_reg l_ExcelFont_index_properties[] = {
	{ "_name",				l_ExcelFont_index__name },
	{ "_height",			l_ExcelFont_index__height },
	{ "_weight",		   	l_ExcelFont_index__weight },
	{ "_options",		   	l_ExcelFont_index__options },
	{ "_color_index",	   	l_ExcelFont_index__color_index },
	{ "_escapement_type",  	l_ExcelFont_index__escapement_type },
	{ "_underline_type",   	l_ExcelFont_index__underline_type },
	{ "_font_family",		l_ExcelFont_index__font_family },
	{ "_character_set",	   	l_ExcelFont_index__character_set },
	{ NULL, NULL },
};


static int l_ExcelFont_index(lua_State *L) {
	const char* key = luaL_checklstring(L, 2, NULL);
	lua_getfield(L, lua_upvalueindex(1), key);
	if (lua_isfunction(L, -1)) {
		return 1;
	}

	lua_getfield(L, lua_upvalueindex(2), key);
	l_ExcelFont_index_property_func function = (l_ExcelFont_index_property_func)lua_touserdata(L, -1);
	if (function) {
		ExcelFont* excelFont = lExcelFont_check(L, 1);
		function(L, excelFont);
		return 1;
	}
	luaL_argerror(L, 2, "improper key");
	return 1;
}


static void l_ExcelFont_newindex__name(lua_State* L, ExcelFont* excelFont) {
	excelFont->_name = lxls_wchar_check(L, 3)->data();
}


static void l_ExcelFont_newindex__height(lua_State* L, ExcelFont* excelFont) {
	excelFont->_height = luaL_checkinteger(L, 3);
}


static void l_ExcelFont_newindex__weight(lua_State* L, ExcelFont* excelFont) {
	excelFont->_weight = luaL_checkinteger(L, 3);
}


static void l_ExcelFont_newindex__options(lua_State* L, ExcelFont* excelFont) {
	excelFont->_options = luaL_checkinteger(L, 3);
}


static void l_ExcelFont_newindex__color_index(lua_State* L, ExcelFont* excelFont) {
	excelFont->_color_index = luaL_checkinteger(L, 3);
}


static void l_ExcelFont_newindex__escapement_type(lua_State* L, ExcelFont* excelFont) {
	excelFont->_escapement_type = luaL_checkinteger(L, 3);
}


static void l_ExcelFont_newindex__underline_type(lua_State* L, ExcelFont* excelFont) {
	excelFont->_underline_type = luaL_checkinteger(L, 3);
}


static void l_ExcelFont_newindex__font_family(lua_State* L, ExcelFont* excelFont) {
	excelFont->_font_family = luaL_checkinteger(L, 3);
}


static void l_ExcelFont_newindex__character_set(lua_State* L, ExcelFont* excelFont) {
	excelFont->_character_set = luaL_checkinteger(L, 3);
}


static const struct l_ExcelFont_index_properties_reg l_ExcelFont_newindex_properties[] = {
	{ "_name",				l_ExcelFont_newindex__name },
	{ "_height",			l_ExcelFont_newindex__height },
	{ "_weight",		   	l_ExcelFont_newindex__weight },
	{ "_options",		   	l_ExcelFont_newindex__options },
	{ "_color_index",	   	l_ExcelFont_newindex__color_index },
	{ "_escapement_type",  	l_ExcelFont_newindex__escapement_type },
	{ "_underline_type",   	l_ExcelFont_newindex__underline_type },
	{ "_font_family",		l_ExcelFont_newindex__font_family },
	{ "_character_set",	   	l_ExcelFont_newindex__character_set },
	{ NULL, NULL },
};


static int l_ExcelFont_newindex(lua_State *L) {
	l_ExcelFont_index_property_func function;
	ExcelFont* excelFont = (ExcelFont*)luaL_checkudata(L, 1, FONT_METATABLE);
	const char* key = luaL_checklstring(L, 2, NULL);
	lua_getfield(L, lua_upvalueindex(1), key);
	function = (l_ExcelFont_index_property_func)lua_touserdata(L, -1);
	if (function) {
		function(L, excelFont);
		return 0;
	}
	luaL_argerror(L, 2, "improper key");
	return 1;
}


int lxls_ExcelFont(lua_State* L)
{
	ExcelFont* excelFont = (ExcelFont*)lua_newuserdata(L, sizeof(ExcelFont));
	::new(excelFont) ExcelFont;

	luaL_getmetatable(L, FONT_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int lCellFormat___gc(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->~CellFormat();
	return 0;
}


int lCellFormat_get_xf_idx(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_xf_idx());
	return 1;
}


int lCellFormat_flush(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->flush();
	return 0;
}


int lCellFormat_get_font_idx(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_font_idx());
	return 1;
}


int lCellFormat_get_fmt_idx(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_fmt_idx());
	return 1;
}


int lCellFormat_get_font(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lxls_ExcelFont(L);
	ExcelFont* excelFont = lExcelFont_check(L, -1);
	*excelFont = cellFormat->get_font();
	return 1;
}


int lCellFormat_set_font(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	ExcelFont* excelFont = lExcelFont_check(L, 2);
	cellFormat->set_font(*excelFont);
	return 0;
}


int lCellFormat_get_format_string(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lxls_wchar_new(L, cellFormat->get_format_string().c_str());
	return 1;
}


int lCellFormat_set_format_string(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	if (lua_isstring(L, 2))
		cellFormat->set_format_string(lua_tostring(L, 2));
	else {
		xls_WChar* xwchar = lxls_wchar_optcheck(L, 2);
		if (!xwchar)
			luaL_argerror(L, 2, "expected string or xls.wchar");
		cellFormat->set_format_string(xwchar->data());
	}
	return 0;
}


int lCellFormat_get_color1(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_color1());
	return 1;
}


int lCellFormat_set_color1(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->set_color1(luaL_checkinteger(L, 2));
	return 0;
}


int lCellFormat_get_color2(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_color2());
	return 1;
}


int lCellFormat_set_color2(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->set_color2(luaL_checkinteger(L, 2));
	return 0;
}


int lCellFormat_set_background(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->set_background(luaL_checkinteger(L, 2), luaL_optinteger(L, 3, COLOR1_PAT_SOLID));
	return 0;
}


int lCellFormat_get_alignment(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_alignment());
	return 1;
}


int lCellFormat_set_alignment(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->set_alignment(luaL_checkinteger(L, 2));
	return 0;
}


int lCellFormat_is_wrapping(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushboolean(L, cellFormat->is_wrapping());
	return 1;
}


int lCellFormat_set_wrapping(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->set_wrapping(luaL_checkboolean(L, 2) != 0);
	return 0;
}


int lCellFormat_get_rotation(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_rotation());
	return 1;
}


int lCellFormat_set_rotation(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->set_rotation(luaL_checkinteger(L, 2));
	return 0;
}


int lCellFormat_get_text_props(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_text_props());
	return 1;
}


int lCellFormat_set_text_props(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	cellFormat->set_text_props(luaL_checkinteger(L, 2));
	return 0;
}


int lCellFormat_get_borderlines(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	lua_pushinteger(L, cellFormat->get_borderlines());
	return 1;
}


int lCellFormat_set_borderlines(lua_State* L) {
	CellFormat* cellFormat = lCellFormat_check(L, 1);
	int numArgs = lua_gettop(L);
	if (numArgs == 2) {
		cellFormat->set_borderlines(luaL_checkinteger(L, 2));
		return 0;
	} else if (lua_gettop(L) >= 7  &&  lua_isnumber(L, 2)
			&&  lua_isnumber(L, 3)  &&  lua_isnumber(L, 4)
			&&  lua_isnumber(L, 5)  &&  lua_isnumber(L, 6)
			&&  lua_isnumber(L, 7)) {
		if (lua_gettop(L) == 7) {
			cellFormat->set_borderlines(
					(EXCEL_LS)luaL_checkinteger(L, 2),
					(EXCEL_LS)luaL_checkinteger(L, 3), 
					(EXCEL_LS)luaL_checkinteger(L, 4), 
					(EXCEL_LS)luaL_checkinteger(L, 5), 
					luaL_checkinteger(L, 6), 
					luaL_checkinteger(L, 7));
			return 0;
		} else if (lua_gettop(L) == 11) {
			cellFormat->set_borderlines(
					(EXCEL_LS)luaL_checkinteger(L, 2),
					(EXCEL_LS)luaL_checkinteger(L, 3), 
					(EXCEL_LS)luaL_checkinteger(L, 4), 
					(EXCEL_LS)luaL_checkinteger(L, 5), 
					luaL_checkinteger(L, 6), 
					luaL_checkinteger(L, 7),
					luaL_checkinteger(L, 8),
					luaL_checkinteger(L, 9),
					(EXCEL_LS)luaL_checkinteger(L, 10), 
					(COLOR1_PAT)luaL_checkinteger(L, 11));
			return 0;
		}
	}

	luaL_error(L, "wrong number of arguments to CellFormat:set_borderlines()");
	return 0;
}


static const struct luaL_reg CellFormat_funcs[] = {
	{ "__gc",				lCellFormat___gc },
	{ "get_xf_idx",    		lCellFormat_get_xf_idx },
	{ "flush",    			lCellFormat_flush },
	{ "get_font_idx",     	lCellFormat_get_font_idx },
	{ "get_fmt_idx",   		lCellFormat_get_fmt_idx },
	{ "get_font", 	   	    lCellFormat_get_font },
	{ "set_font", 			lCellFormat_set_font },
	{ "get_format_string", 	lCellFormat_get_format_string },
	{ "set_format_string", 	lCellFormat_set_format_string },
	{ "get_color1",	   		lCellFormat_get_color1 },
	{ "set_color1",			lCellFormat_set_color1 },
	{ "get_color2",			lCellFormat_get_color2 },
	{ "set_color2",			lCellFormat_set_color2 },
	{ "set_background",		lCellFormat_set_background },
	{ "get_alignment",		lCellFormat_get_alignment },
	{ "set_alignment",		lCellFormat_set_alignment },
	{ "is_wrapping",		lCellFormat_is_wrapping },
	{ "set_wrapping",		lCellFormat_set_wrapping },
	{ "get_rotation",		lCellFormat_get_rotation },
	{ "set_rotation",		lCellFormat_set_rotation },
	{ "get_text_props",		lCellFormat_get_text_props },
	{ "set_text_props",		lCellFormat_set_text_props },
	{ "get_borderlines",   	lCellFormat_get_borderlines },
	{ "set_borderlines",   	lCellFormat_set_borderlines },

	{ NULL, NULL },
};


int lxls_CellFormat(lua_State* L)
{
	CellFormat* cellFormat = (CellFormat*)lua_newuserdata(L, sizeof(CellFormat));

	XLSFormatManager* xlsFormatManager = lXlsFormatManager_check(L, 1);
	if (lua_gettop(L) == 1) {
		::new(cellFormat) CellFormat(*xlsFormatManager);
	} else if (lua_isuserdata(L, 2)) {
		void* p = lua_touserdata(L, 2);
		if (lua_getmetatable(L, 2)) {
			lua_getfield(L, LUA_REGISTRYINDEX, FONT_METATABLE);
			if (lua_rawequal(L, -1, -2)) {
				::new(cellFormat) CellFormat(*xlsFormatManager, *(ExcelFont*)p);
				lua_pop(L, 2);
			} else {
				lua_pop(L, 1);
				lua_getfield(L, LUA_REGISTRYINDEX, CELL_METATABLE);
				if (lua_rawequal(L, -1, -2)) {
					::new(cellFormat) CellFormat(*xlsFormatManager, *(BasicExcelCell**)p);
					lua_pop(L, 2);
				} else {
					luaL_argerror(L, 2, "expected nothing or xls.ExcelFont or xls.BasicExcelCell");
				}
			}
		}
	}

	luaL_getmetatable(L, CELLFORMAT_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


static void lxls_createmetatables(lua_State* L)
{
	// xls.wchar
	luaL_newmetatable(L, XLS_WCHAR_METATABLE);				// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, xls_wchar_funcs);
	lua_pop(L, 1);

	// xls.Workbook
	luaL_newmetatable(L, WORKBOOK_METATABLE);				// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, BasicExcel_funcs);
	lua_pop(L, 1);

	// xls.Worksheet
	luaL_newmetatable(L, WORKSHEET_METATABLE);				// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, BasicExcelWorksheet_funcs);
	lua_pop(L, 1);

	// xls.Cell
	luaL_newmetatable(L, CELL_METATABLE);				// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, BasicExcelCell_funcs);

	lua_pushinteger(L, BasicExcelCell::UNDEFINED);
	lua_setfield(L, -2, "UNDEFINED");
	lua_pushinteger(L, BasicExcelCell::INT);
	lua_setfield(L, -2, "INT");
	lua_pushinteger(L, BasicExcelCell::DOUBLE);
	lua_setfield(L, -2, "DOUBLE");
	lua_pushinteger(L, BasicExcelCell::STRING);
	lua_setfield(L, -2, "STRING");
	lua_pushinteger(L, BasicExcelCell::WSTRING);
	lua_setfield(L, -2, "WSTRING");
	lua_pushinteger(L, BasicExcelCell::FORMULA);
	lua_setfield(L, -2, "FORMULA");
	lua_pop(L, 1);

	// xls.Font
	luaL_newmetatable(L, FONT_METATABLE);				// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index table
	luaL_register(L, NULL, ExcelFont_funcs);

	lua_newtable(L);										// metatable __index table

	for (const l_ExcelFont_index_properties_reg* l = l_ExcelFont_index_properties; l->name; l++) {
		lua_pushlightuserdata(L, (void*)l->func);
		lua_setfield(L, -2, l->name);
	}

	lua_pushcclosure(L, l_ExcelFont_index, 2);
	lua_settable(L, -3);

	lua_pushliteral(L, "__newindex");							// metatable __index
	lua_newtable(L);										// metatable __index table table

	for (const l_ExcelFont_index_properties_reg* l = l_ExcelFont_newindex_properties; l->name; l++) {
		lua_pushlightuserdata(L, (void*)l->func);
		lua_setfield(L, -2, l->name);
	}

	lua_pushcclosure(L, l_ExcelFont_newindex, 1);
	lua_settable(L, -3);

	lua_pop(L, 1);

	// xls.CellFormat
	luaL_newmetatable(L, CELLFORMAT_METATABLE);				// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, CellFormat_funcs);
	lua_pop(L, 1);

}

#define MAKE_COLOR2(pc, pbc)	((pc) | ((pbc)<<7))

static int lxls_MAKECOLOR2(lua_State* L) {
	lua_pushinteger(L, MAKE_COLOR2(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
	return 1;
}


static int lxls_wchar(lua_State* L) {
	const char* mbstr = luaL_checkstring(L, 1);
	size_t mbstrlen = lua_objlen(L, 1);
	lxls_wchar_new(L, mbstr, mbstrlen);
	return 1;
}


static const struct luaL_reg lxlslib[] = {
	{ "Workbook",			lxls_BasicExcel },
	{ "Font",				lxls_ExcelFont },
	{ "MAKE_COLOR2",		lxls_MAKECOLOR2 },
	{ "wchar",				lxls_wchar },
	//{ "help",				LS_Help },
	{NULL, NULL},
};


#define REGISTER_FIELD(fieldName) \
	lua_pushinteger(L, fieldName); \
	lua_setfield(L, -2, #fieldName);

#define REGISTER_WSTRING(fieldName) \
	lxls_wchar_new(L, fieldName); \
	lua_setfield(L, -2, #fieldName);

extern "C" int luaopen_xls(lua_State* L)
{
	lua_newtable(L);
	luaL_register(L, NULL, lxlslib);

	lxls_createmetatables(L);

	REGISTER_WSTRING(XLS_FORMAT_GENERAL);
	REGISTER_WSTRING(XLS_FORMAT_TEXT);
	REGISTER_WSTRING(XLS_FORMAT_INTEGER);
	REGISTER_WSTRING(XLS_FORMAT_DECIMAL);
	REGISTER_WSTRING(XLS_FORMAT_PERCENT);
	REGISTER_WSTRING(XLS_FORMAT_DATE);
	REGISTER_WSTRING(XLS_FORMAT_TIME);
	REGISTER_WSTRING(XLS_FORMAT_DATETIME);

	REGISTER_FIELD(FIRST_USER_FORMAT_IDX);

	// Font option flags
	REGISTER_FIELD(EXCEL_FONT_BOLD);
	REGISTER_FIELD(EXCEL_FONT_ITALIC);
	REGISTER_FIELD(EXCEL_FONT_UNDERLINED);
	REGISTER_FIELD(EXCEL_FONT_STRUCK_OUT);
	REGISTER_FIELD(EXCEL_FONT_OUTLINED);
	REGISTER_FIELD(EXCEL_FONT_SHADOWED);
	REGISTER_FIELD(EXCEL_FONT_CONDENSED);
	REGISTER_FIELD(EXCEL_FONT_EXTENDED);

	REGISTER_FIELD(EXCEL_ESCAPEMENT_NONE);
	REGISTER_FIELD(EXCEL_ESCAPEMENT_SUPERSCRIPT);
	REGISTER_FIELD(EXCEL_ESCAPEMENT_SUBSCRIPT);

	// Font underline types
	REGISTER_FIELD(EXCEL_UNDERLINE_NONE);
	REGISTER_FIELD(EXCEL_UNDERLINE_SINGLE);
	REGISTER_FIELD(EXCEL_UNDERLINE_DOUBLE);
	REGISTER_FIELD(EXCEL_UNDERLINE_SINGLE_ACCOUNTING);
	REGISTER_FIELD(EXCEL_UNDERLINE_DOUBLE_ACCOUNTING);

	// Excel font family constants, see also FF_DONTCARE, ... in wingdi.h
	REGISTER_FIELD(EXCEL_FONT_FAMILY_DONTCARE);
	REGISTER_FIELD(EXCEL_FONT_FAMILY_ROMAN);
	REGISTER_FIELD(EXCEL_FONT_FAMILY_SWISS);
	REGISTER_FIELD(EXCEL_FONT_FAMILY_MODERN);
	REGISTER_FIELD(EXCEL_FONT_FAMILY_SCRIPT);
	REGISTER_FIELD(EXCEL_FONT_FAMILY_DECORATIVE);

	REGISTER_FIELD(XF_USED_ATTRIB_NUMBER_FORMAT);
	REGISTER_FIELD(XF_USED_ATTRIB_FONT);
	REGISTER_FIELD(XF_USED_ATTRIB_ALIGN);
	REGISTER_FIELD(XF_USED_ATTRIB_BORDER_LINES);
	REGISTER_FIELD(XF_USED_ATTRIB_BKGND_AREA_STYLE);
	REGISTER_FIELD(XF_USED_ATTRIB_CELL_PROTECTION);

	/*
	 Bit Mask Contents of alignment byte
	 2- 0 07H XF_HOR_ALIGN: Horizontal alignment
	 3    08H 1 = Text is wrapped at right border
	 6- 4 70H XF_VERT_ALIGN: Vertical alignment
	 7    80H 1 = Justify last line in justified or distibuted text
	*/
	 // horizontal aligmment
	REGISTER_FIELD(EXCEL_HALIGN_GENERAL);
	REGISTER_FIELD(EXCEL_HALIGN_LEFT);
	REGISTER_FIELD(EXCEL_HALIGN_CENTRED);
	REGISTER_FIELD(EXCEL_HALIGN_RIGHT);
	REGISTER_FIELD(EXCEL_HALIGN_FILLED);
	REGISTER_FIELD(EXCEL_HALIGN_JUSITFIED);
	REGISTER_FIELD(EXCEL_HALIGN_SEL_CENTRED);
	REGISTER_FIELD(EXCEL_HALIGN_DISTRIBUTED);

	 // vertical alignment
	REGISTER_FIELD(EXCEL_VALIGN_TOP);
	REGISTER_FIELD(EXCEL_VALIGN_CENTRED);
	REGISTER_FIELD(EXCEL_VALIGN_BOTTOM);
	REGISTER_FIELD(EXCEL_VALIGN_JUSTIFIED);
	REGISTER_FIELD(EXCEL_VALIGN_DISTRIBUTED);

	REGISTER_FIELD(EXCEL_JUSTIFY_LAST_LINE);

	/*
	Indentation, shrink to cell size, and text direction:
	 Bit Mask Contents
	 3- 0 0FH Indent level
	 4    10H 1 = Shrink content to fit into cell
	 7- 6 C0H Text direction: 0 = According to context; 1 = Left-to-right; 2 = Right-to-left
	*/

	 // shrink flag
	REGISTER_FIELD(EXCEL_TEXT_PROP_SHRINK);

	 // text direction
	REGISTER_FIELD(EXCEL_TEXT_PROP_DEFAULT);
	REGISTER_FIELD(EXCEL_TEXT_PROP_LEFT_TO_RIGHT);
	REGISTER_FIELD(EXCEL_TEXT_PROP_RIGHT_TO_LEFT);


	/*
	  Bit      Mask Contents of cell border lines and background area:
	 3- 0 0000000FH Left line style
	 7- 4 000000F0H Right line style
	11- 8 00000F00H Top line style
	15-12 0000F000H Bottom line style
	22-16 007F0000H Color index for left line colour
	29-23 3F800000H Color index for right line colour
	30    40000000H 1 = Diagonal line from top left to right bottom
	31    80000000H 1 = Diagonal line from bottom left to right top
	*/

	 // line style constants
	REGISTER_FIELD(EXCEL_LS_NO_LINE);
	REGISTER_FIELD(EXCEL_LS_THIN);
	REGISTER_FIELD(EXCEL_LS_MEDIUM);
	REGISTER_FIELD(EXCEL_LS_DASHED);
	REGISTER_FIELD(EXCEL_LS_DOTTED);
	REGISTER_FIELD(EXCEL_LS_THICK);
	REGISTER_FIELD(EXCEL_LS_DOUBLE);
	REGISTER_FIELD(EXCEL_LS_HAIR);
	REGISTER_FIELD(EXCEL_LS_MEDIUM_DASHED);
	REGISTER_FIELD(EXCEL_LS_THIN_DASH_DOTTED);
	REGISTER_FIELD(EXCEL_LS_MEDIUM_DASH_DOTTED);
	REGISTER_FIELD(EXCEL_LS_THIN_DASH_DOT_DOTTED);
	REGISTER_FIELD(EXCEL_LS_MEDIUM_DASH_DOT_DOTTED);
	REGISTER_FIELD(EXCEL_LS_SLANTED_MEDIUM_DASH_DOTTED);

	 // borderline flags for diagonal lines
	REGISTER_FIELD(BORDERLINE_DIAG1);
	REGISTER_FIELD(BORDERLINE_DIAG2);

	/*
	  Bit      Mask Contents of COLOR1
	06- 0 0000007FH Color index for top line colour
	13- 7 00003F80H Color index for bottom line colour
	20-14 001FC000H Color index for diagonal line colour
	24-21 01E00000H Diagonal line style
	31-26 FC000000H Fill pattern
	*/
	 // line style constants for COLOR1
	REGISTER_FIELD(COLOR1_LS_NO_LINE);
	REGISTER_FIELD(COLOR1_LS_THIN);
	REGISTER_FIELD(COLOR1_LS_MEDIUM);
	REGISTER_FIELD(COLOR1_LS_DASHED);
	REGISTER_FIELD(COLOR1_LS_DOTTED);
	REGISTER_FIELD(COLOR1_LS_THICK);
	REGISTER_FIELD(COLOR1_LS_DOUBLE);
	REGISTER_FIELD(COLOR1_LS_HAIR);
	REGISTER_FIELD(COLOR1_LS_MEDIUM_DASHED);
	REGISTER_FIELD(COLOR1_LS_THIN_DASH_DOTTED);
	REGISTER_FIELD(COLOR1_LS_MEDIUM_DASH_DOTTED);
	REGISTER_FIELD(COLOR1_LS_THIN_DASH_DOT_DOTTED);
	REGISTER_FIELD(COLOR1_LS_MEDIUM_DASH_DOT_DOTTED);
	REGISTER_FIELD(COLOR1_LS_SLANTED_MEDIUM_DASH_DOTTED);

	 // pattern constants
	REGISTER_FIELD(COLOR1_PAT_SOLID);
	REGISTER_FIELD(COLOR1_PAT_2);
	REGISTER_FIELD(COLOR1_PAT_3);
	REGISTER_FIELD(COLOR1_PAT_4);
	REGISTER_FIELD(COLOR1_PAT_5);
	REGISTER_FIELD(COLOR1_PAT_6);
	REGISTER_FIELD(COLOR1_PAT_7);
	REGISTER_FIELD(COLOR1_PAT_8);
	REGISTER_FIELD(COLOR1_PAT_9);
	REGISTER_FIELD(COLOR1_PAT_10);
	REGISTER_FIELD(COLOR1_PAT_11);
	REGISTER_FIELD(COLOR1_PAT_12);
	REGISTER_FIELD(COLOR1_PAT_13);
	REGISTER_FIELD(COLOR1_PAT_14);
	REGISTER_FIELD(COLOR1_PAT_15);
	REGISTER_FIELD(COLOR1_PAT_16);
	REGISTER_FIELD(COLOR1_PAT_17);
	REGISTER_FIELD(COLOR1_PAT_18);

	/*
	   Bit Mask Contents of COLOR2
	 6- 0 007FH Color index for pattern colour
	13- 7 3F80H Color index for pattern background
	*/

	 // predefined Excel color definitions
	REGISTER_FIELD(EGA_BLACK);
	REGISTER_FIELD(EGA_WHITE);
	REGISTER_FIELD(EGA_RED);
	REGISTER_FIELD(EGA_GREEN);
	REGISTER_FIELD(EGA_BLUE);
	REGISTER_FIELD(EGA_YELLOW);
	REGISTER_FIELD(EGA_MAGENTA);
	REGISTER_FIELD(EGA_CYAN);

	return 1;
}


