#include "RemoteLuaDebugger_InternalPch.h"
#include "RemoteLuaDebugger.h"
#include "VariablesGrid.h"
#include "GridBtnCell_src\GridBtnCell.h"
#include "GridBtnCell_src\GridBtnCellCombo.h"
#include "TreeColumn_src\GridTreeCell.h"
#include "GridTreeBtnCell_src\GridTreeBtnCell.h"

const int ROW_HEIGHT = 15;
const int NAME_COL = 0;
const int NAME_WIDTH = 100;

const int DATA_COL = 1;
const int DATA_WIDTH = 150;

BEGIN_MESSAGE_MAP(CVariablesGrid, CGridCtrl)
	ON_NOTIFY_REFLECT(GVN_ENDLABELEDIT, OnGridCellEndLabelEdit)
	ON_NOTIFY_REFLECT(TC_NOTIFY_TREECLICK, OnTreeClick)
END_MESSAGE_MAP()


CVariablesGrid::CVariablesGrid()
{
}


CVariablesGrid::~CVariablesGrid()
{
}


static CVariablesGrid* ScriptHelper_GetAddInfo(LuaState& state, int& treeDepth,
											  CString& title)
{
	LuaStackObject gridObj(state, 1);
	LuaStackObject treeDepthObj(state, 2);
	LuaStackObject titleObj(state, 3);
	
	treeDepth = treeDepthObj.GetInteger();
	title = titleObj.GetString();
	
	return static_cast<CVariablesGrid*>(gridObj.GetUserData());
}


void CVariablesGrid::CreateGrid(CWnd* parentWnd, UINT windowID)
{
	CWnd* tempWnd = parentWnd->GetDlgItem(windowID);
	CRect gridRect;
	tempWnd->GetWindowRect(gridRect);
	tempWnd->DestroyWindow();

	CRect dialogRect;
	parentWnd->GetClientRect(dialogRect);
	parentWnd->ClientToScreen(dialogRect);

	gridRect.OffsetRect(-dialogRect.TopLeft());

	Create(gridRect, parentWnd, windowID, WS_VISIBLE);
}

void CVariablesGrid::Init()
{
	LuaState* state = theApp.GetDocument()->GetState();
	
	m_buttonDatabase.SetGrid(this);
	m_imageList.Create(MAKEINTRESOURCE(IDB_FOLDERS), 16, 1, RGB(255,255,255));
    SetImageList(&m_imageList);

	SetColumnCount(2);
	SetDefCellHeight(ROW_HEIGHT);
	SetRowCount(0);
	SetColumnWidth(NAME_COL, NAME_WIDTH);
	SetColumnWidth(DATA_COL, DATA_WIDTH);

	CString fontName = "Arial";
	
	LOGFONT logFont;
	memset(&logFont, 0, sizeof(LOGFONT));
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfWeight = 700;
	logFont.lfHeight = 80;
	lstrcpyn(logFont.lfFaceName, fontName, fontName.GetLength());

	m_titleFont.CreatePointFontIndirect(&logFont);
	m_dataFont.CreatePointFont(80, fontName);
}


CString MakeValueTextFromObject(LuaStackObject value)
{
	CString valueStr;
	if (value.IsNumber())
	{
		valueStr.Format(_T("%f"), value.GetNumber());
	}
	else if (value.IsString())
	{
		valueStr = value.GetString();
		if (valueStr == "!!!NIL!!!")
			valueStr = _T("nil");
		else if (valueStr == "!!!USERDATA!!!")
			valueStr = _T("[userdata]");
		else if (valueStr == "!!!FUNCTION!!!")
			valueStr = _T("[function]");
		else if (valueStr == "!!!FUNCTION!!!")
			valueStr = _T("[cfunction]");
		else
			valueStr = _T("\"") + valueStr + _T("\"");
	}
	else if (value.IsTable())
	{
		valueStr = _T("[table]");
	}

	return valueStr;
}


void CVariablesGrid::FixupRows(bool readOnly)
{
	// Nothing here...
//	if (m_titleList.GetSize() == 0)
//		return;

	int numRows = GetRowCount();
	if (readOnly  &&  numRows == 1)
		return;

	GV_ITEM Item;

	m_treeColumn.TreeSetup( this, NAME_COL, readOnly ? numRows - 1 : numRows, 1, m_treeLevelArray.GetData(), TRUE, FALSE);
	m_treeColumn.SetTreeLineColor( RGB( 0, 0, 0xFF) );
//	m_treeColumn.SetDefTreeIndent( 8 );
//	m_treeColumn.TreeDisplayOutline(100);

	int curWatchIndex = -1;
	for (int i = 1; i < numRows; i++)
	{
		CGridTreeCell* treeCell = (CGridTreeCell*)GetCell(i, 0);
		treeCell->SetForcePlus( m_treeForcePlusArray[ i - 1 ] );
		int curLevel = treeCell->GetLevel();
		if (curLevel == 1)
		{
			curWatchIndex++;
		}

		Item.mask = GVIF_TEXT|GVIF_FORMAT|GVIF_FONT|GVIF_PARAM;
		Item.row = i;
		Item.col = NAME_COL;

		Item.mask = GVIF_TEXT|GVIF_FORMAT|GVIF_STATE;
		Item.nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS;
		Item.nState = (readOnly || curLevel > 1) ? GVIS_READONLY : 0;
		m_titleFont.GetLogFont(&Item.lfFont);
		Item.lParam = curWatchIndex;

		SetItem(&Item);
	}

	for (int i = 1; i  <= m_titleList.GetSize(); ++i)
	{
//		SetFont(&m_titleFont);
		SetItemText(i, NAME_COL, m_titleList[i - 1]);
	}

//	for (i = 0; i < numRows; ++i)
//		m_treeColumn.TreeDataCollapseAllSubLevels(i);
}


void CVariablesGrid::SaveExpandInfo(bool extraRow)
{
	m_expandInfo.RemoveAll();

	// Get the full name.
	CString fullName;

	int numRows = GetRowCount();
	if (extraRow)
		numRows--;
	for (int i = 1; i < numRows; ++i)
	{
		CGridTreeCell* treeCell = (CGridTreeCell*)GetCell(i, 0);
		int level = treeCell->GetLevel();
		if (level == 1)
		{
			fullName = GetItemText(i, 0);
		}
		else
		{
			fullName += "." + GetItemText(i, 0);
		}

		if (treeCell->IsViewable())
		{
			ExpandInfo info;
			info.m_fullName = fullName;
			m_expandInfo.Add(info);
		}
	}
}


void CVariablesGrid::RestoreExpandInfo(bool extraRow)
{
	if (m_expandInfo.GetSize() == 0)
		return;

	// Get the full name.
	CString fullName;

	int curExpandPos = 0;
	int numRows = GetRowCount();
	if (extraRow)
		numRows--;
	for (int i = 1; i < numRows; ++i)
	{
		CGridTreeCell* treeCell = (CGridTreeCell*)GetCell(i, 0);
		int level = treeCell->GetLevel();
		if (level == 1)
		{
			fullName = GetItemText(i, 0);
		}
		else
		{
			fullName += "." + GetItemText(i, 0);
		}

		if (m_expandInfo[curExpandPos].m_fullName == fullName)
		{
//			m_treeColumn.TreeDataExpandOneLevel(i);
			treeCell->SetViewable(TRUE);
			curExpandPos++;
			if (curExpandPos == m_expandInfo.GetSize())
			{
				m_treeColumn.TreeRefreshRows();
				return;
			}
		}
	}

	m_treeColumn.TreeRefreshRows();
}


/*LuaStackObject GetValueFromTextName(CString prefix, CString name)
{
	CString out;
	CString findName = name;
	LuaState* state = theApp.GetDocument()->GetState();
	LuaObject curTable = state->GetGlobal(prefix);

	while (true)
	{
		int dotPos = name.Find('.');
		if (dotPos != -1)
		{
			findName = name.Left(dotPos);
			name = name.Mid(dotPos + 1);
		}
		else
		{
			findName = name;
			break;
		}
		
		curTable = curTable.GetByName(findName);
		if (!curTable.IsTable())
		{
			state->PushString("!!!NIL!!!");
			return LuaStackObject(state, state->GetTop());
		}
	}

	LuaStackObject obj = curTable.GetByName(findName);
	return obj;
}
*/


CStringW DebugBasicSerialize( LuaObject& obj )
{
	if ( obj.Type() == LUA_TNUMBER )
	{
		CStringW str;
		str.Format( L"%.16g", obj.GetNumber() );
		return str;
	}

	else if ( obj.Type() == LUA_TBOOLEAN )
	{
		return obj.GetBoolean() ? L"true" : L"false";
	}

	else if (obj.Type() == LUA_TSTRING)
	{
		CString str = obj.GetString();
		if ( str == "!!!NIL!!!" )
			return L"nil";
		else if ( str == "!!!FUNCTION!!!" )
			return L"{function}";
		else if ( str == "!!!CFUNCTION!!!" )
			return L"{cfunction}";
		else if ( str == "!!!USERDATA!!!" )
			return L"{userdata}";
		else if ( str == "!!!TABLE!!!" )
			return L"{table}";
		return CStringW(str);
	}
#if LUA_WIDESTRING
	else if (obj.Type() == LUA_TWSTRING)
	{
		return CStringW((wchar_t*)obj.GetWString());
	}
#endif // LUA_WIDESTRING

	else
		return _T("Unknown");
}


void CVariablesGrid::BuildLocals( LuaObject tableObj )
{
    SetRedraw(FALSE);
	
	m_gridType = LOCALS;

	m_titleList.RemoveAll();
	m_treeLevelArray.RemoveAll();
	m_treeForcePlusArray.RemoveAll();
	AddSection("Name", "Value");
	SetFixedRowCount(1);
	SetRowCount(1);

	LuaState* state = tableObj.GetState();

	int tableCount = tableObj.GetCount();
	for ( int i = 0; i < tableCount; ++i )
	{
		LuaObject entryObj = tableObj[ i + 1 ];
		LuaObject indentLevelObj = entryObj[ 2 ];
		LuaObject valueObj = entryObj[ 3 ];

		CStringW ident = DebugBasicSerialize(entryObj[ 1 ]);
		int indentLevel = indentLevelObj.GetInteger();
		CStringW value = DebugBasicSerialize( valueObj );

		int curRow = AddEditProperty( indentLevel, ident, true );
		SetEditProperty( curRow, value );

		// Make an extra row.
		if ( valueObj.IsString()  &&  strcmp( valueObj.GetString(), "!!!TABLE!!!" ) == 0 )
		{
			bool addTreeRowForPlusMinus = false;

			if ( i < tableCount - 1 )
			{
				LuaObject nextIndentLevelObj = tableObj[ i + 2 ][ 2 ];
				int nextIndentLevel = nextIndentLevelObj.GetInteger();
				if ( nextIndentLevel != indentLevel + 1 )
				{
					addTreeRowForPlusMinus = true;
				}
			}
			else
			{
				addTreeRowForPlusMinus = true;
			}

			if ( addTreeRowForPlusMinus )
			{
				m_treeForcePlusArray[ curRow - 1 ] = true;
			}
		}
 
		m_treeLevelArray[ curRow - 1 ] |= 0x80;
	}

	FixupRows(true);

    SetRedraw(TRUE);

}


void CVariablesGrid::BuildWatch()
{
	m_gridType = WATCH;

	m_titleList.RemoveAll();
	m_treeLevelArray.RemoveAll();
	m_treeForcePlusArray.RemoveAll();
	AddSection("Name", "Value");
	SetFixedRowCount(1);
	SetRowCount(2);
}


void CVariablesGrid::UpdateWatch( LuaObject tableObj )
{
    SetRedraw(FALSE);

//	SaveExpandInfo(true);
	BuildWatch();

	SetRowCount(1);

	LuaState* state = tableObj.GetState();

	int tableCount = tableObj.GetCount();
	for ( int i = 0; i < tableCount; ++i )
	{
		LuaObject entryObj = tableObj[ i + 1 ];
		LuaObject indentLevelObj = entryObj[ 2 ];
		LuaObject valueObj = entryObj[ 3 ];

		CStringW ident = DebugBasicSerialize(entryObj[ 1 ]);
		int indentLevel = indentLevelObj.GetInteger();
		CStringW value = DebugBasicSerialize( valueObj );

		int curRow = AddEditProperty( indentLevel, ident, true );
		SetEditProperty( curRow, value );

		// Make an extra row.
		if ( valueObj.IsString()  &&  strcmp( valueObj.GetString(), "!!!TABLE!!!" ) == 0 )
		{
			bool addTreeRowForPlusMinus = false;

			if ( i < tableCount - 1 )
			{
				LuaObject nextIndentLevelObj = tableObj[ i + 2 ][ 2 ];
				int nextIndentLevel = nextIndentLevelObj.GetInteger();
				if ( nextIndentLevel != indentLevel + 1 )
				{
					addTreeRowForPlusMinus = true;
				}
			}
			else
			{
				addTreeRowForPlusMinus = true;
			}

			if ( addTreeRowForPlusMinus )
			{
				m_treeForcePlusArray[ curRow - 1 ] = true;
			}
		}
 
		m_treeLevelArray[ curRow - 1 ] |= 0x80;
	}

	m_treeLevelArray.Add(0);
	FixupRows(false);

    SetRedraw(TRUE);
}


void CVariablesGrid::DebugAddAllWatches(const CStringW& stateNameToDebug)
{
	int watchCount = m_watchArray.GetSize();
	for (int i = 0; i < watchCount; ++i)
	{
		LPCTSTR name = m_watchArray[i];

		CString command;
		command.Format(_T("DebugAddWatch('%s',%d,'%s',false)"), stateNameToDebug, i, name);
		theApp.GetNetworkClient().SendCommand(command);
	}

	CString command;
	command.Format(_T("DebugReadWatchVariables('%s')"), stateNameToDebug);
	theApp.GetNetworkClient().SendCommand(command);
}


void CVariablesGrid::Update(LuaStackObject& inTable)
{
//	LuaAutoBlock block(*m_script);
//	m_script->GetGlobal("UpdatePropertyGrid");
//	m_script->PushUserData(this);
//	m_script->GetGlobal(m_propertyDefinitionsName);
//	m_script->PushObject(inTable);
//	m_script->Call(3, 0);
}

	
void CVariablesGrid::AddSection(const CString& title, const CString& type)
{
	int curRow = GetRowCount();
	SetRowCount(curRow + 1);

	SetFont(&m_dataFont);

	SetItemText(curRow, NAME_COL, title);
	SetItemText(curRow, DATA_COL, type);
}


int CVariablesGrid::AddEditProperty( int treeDepth, const CStringW& title, bool expanded )
{
	int curRow = GetRowCount();
	SetRowCount(curRow + 1);
	SetFont(&m_dataFont);
	m_titleList.Add(title);
	m_treeLevelArray.Add(treeDepth | ( expanded ? 0x80 : 0 ) );
	m_treeForcePlusArray.Add( false );

	ControlInfo controlInfo;
	controlInfo.m_hasSpin = false;
	m_controlInfo.Add(controlInfo);
	
	return curRow;
}

void CVariablesGrid::SetEditProperty(int row, const CStringW& data)
{
	CGridCell* gridCell = (CGridCell*)GetCell(row, DATA_COL);
	gridCell->SetText(data);
}


void CVariablesGrid::OnTreeClick(NMHDR *pNotifyStruct, LRESULT* pResult)
{
    NM_TREECLICK* pItem = (NM_TREECLICK*)pNotifyStruct;

	CString fullIdent = GetFullName( pItem->iRow );
	int actualIndex = FindWatchActualIndex( pItem->iRow );

	CString command;
	if ( pItem->expand )
	{
		if (m_gridType == WATCH)
			command.Format( _T("DebugAddWatchExpandIdent('%s',%d,'%s')"), theApp.GetDebuggerView()->GetStateToDebug(), actualIndex, fullIdent );
		else
			command.Format( _T("DebugAddLocalExpandIdent('%s',%d,'%s')"), theApp.GetDebuggerView()->GetStateToDebug(), actualIndex, fullIdent );
		theApp.GetNetworkClient().SendCommand(command);
	}
	else
	{
		if (m_gridType == WATCH)
			command.Format( _T("DebugRemoveWatchExpandIdent('%s',%d,'%s')"), theApp.GetDebuggerView()->GetStateToDebug(), actualIndex, fullIdent );
		else
			command.Format( _T("DebugRemoveLocalExpandIdent('%s',%d,'%s')"), theApp.GetDebuggerView()->GetStateToDebug(), actualIndex, fullIdent );
		theApp.GetNetworkClient().SendCommand(command);
	}

	command.Format(_T("DebugReadWatchVariables('%s')"), theApp.GetDebuggerView()->GetStateToDebug());
	theApp.GetNetworkClient().SendCommand(command);
}

	
CString MakeOutputValue(const CString& value)
{
	CString outValue;
	if (value == "nil")
	{
		outValue = "nil";
	}
	else if (value == "[table]"  ||  value == "[function]"  ||  value == "[cfunction]"  ||  value == "[userdata]")
	{
		// Nothing.
	}
	else if (value[0] == '"')
	{
		// String.
		outValue = "\"" + value.Mid(1, value.GetLength() - 2) + "\"";
	}
	else
	{
		// Number?
		for (int i = 0; i < value.GetLength(); ++i)
		{
			TCHAR ch = value[i];
			if (!_istdigit(ch)  &&  ch != '.'  &&  ch != 'e'  &&  ch != '-')
			{
				// Invalid.
				return "";
			}
		}

		outValue.Format(_T("%f"), _tstof(value));
	}

	return outValue;
}


int CVariablesGrid::FindWatchActualIndex( int row )
{
	CString name = GetItemText( row, 0 );
	int actualIndex = -1;
	int lastCheckRow = row;
	if (!name.IsEmpty()  &&  row == GetRowCount() - 1)
	{
		actualIndex++;
		lastCheckRow--;
	}

	// Determine the true index.
	for (int i = 1; i <= lastCheckRow; ++i)
	{
		CGridTreeCell* treeCell = (CGridTreeCell*)GetCell(i, 0);
		if (treeCell->IsKindOf(RUNTIME_CLASS(CGridTreeCell)))
		{
			int level = treeCell->GetLevel();
			if (level == 1)
				actualIndex++;
		}
		else
			actualIndex++;
	}

	return actualIndex;
}


CString CVariablesGrid::GetFullName( int row )
{
	// Get the full name.
	CString fullName = GetItemText( row, 0 );
	CGridTreeCell* treeCell = (CGridTreeCell*)GetCell( row, 0 );
	int curLevel = treeCell->GetLevel();
	if (curLevel > 1)
	{
		for (int i = row - 1; i >= 1; --i)
		{
			if (curLevel <= 1)
				break;
			CGridTreeCell* treeCell = (CGridTreeCell*)GetCell(i, 0);
			int level = treeCell->GetLevel();
			if (level == curLevel - 1)
			{
				// Found a new part to tack on.
				fullName = GetItemText(i, 0) + "." + fullName;
				curLevel--;
			}
		}
	}

	return fullName;
}


void CVariablesGrid::OnGridCellEndLabelEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*)pNotifyStruct;
    if (pItem->iRow < 1)
        return;

	CString name = GetItemText(pItem->iRow, 0);
	CString value = GetItemText(pItem->iRow, 1);

	// Get the full name.
	CString fullName = GetFullName( pItem->iRow );

	if (m_gridType == WATCH)
	{
		int actualIndex = FindWatchActualIndex( pItem->iRow );
			
		// Is the name being modified?
		if (pItem->iColumn == 0)
		{
			if (name.IsEmpty())
			{
//				DeleteRow(pItem->iRow);

				m_watchArray.RemoveAt(actualIndex);
				
				CString command;
				command.Format(_T("DebugRemoveWatch('%s',%d)"), theApp.GetDebuggerView()->GetStateToDebug(), actualIndex);
				theApp.GetNetworkClient().SendCommand(command);

				command.Format(_T("DebugReadWatchVariables('%s')"), theApp.GetDebuggerView()->GetStateToDebug());
				theApp.GetNetworkClient().SendCommand(command);
			}
			else // Add the watch.
			{
				// Did the data actually change (bug in the stupid grid)...
				if (m_watchArray.GetSize() > actualIndex  &&  fullName != m_watchArray[actualIndex])
				{
					return;
				}

				// Did they enter the value on the last column?
				if (pItem->iRow == GetRowCount() - 1)
				{
					// Yes.  Add a new row.
					SetRowCount(GetRowCount() + 1);
				}

				// Since they changed the name, invalidate the data.
				SetItemText(pItem->iRow, 1, _T(""));

				m_watchArray.SetAtGrow(actualIndex, fullName);
				
				CString command;
				command.Format(_T("DebugAddWatch('%s',%d,'%s',false)"), theApp.GetDebuggerView()->GetStateToDebug(), actualIndex, fullName);
				theApp.GetNetworkClient().SendCommand(command);

				command.Format(_T("DebugReadWatchVariables('%s')"), theApp.GetDebuggerView()->GetStateToDebug());
				theApp.GetNetworkClient().SendCommand(command);
			}
		}
		else if (pItem->iColumn == 1)
		{
			// Don't let data be entered on a column when there is no name.
			if (name.IsEmpty())
			{
				// Clear out the data.
				SetItemText(pItem->iRow, 1, _T(""));
				return;
			}

			CString outValue = MakeOutputValue(value);
			if (!outValue.IsEmpty())
			{
				CString command;
				command.Format(_T("DebugSetVariable('%s','%s',%s)"), theApp.GetDebuggerView()->GetStateToDebug(), fullName, outValue);
				theApp.GetNetworkClient().SendCommand(command);

				command.Format(_T("DebugReadWatchVariables('%s')"), theApp.GetDebuggerView()->GetStateToDebug());
				theApp.GetNetworkClient().SendCommand(command);
			}
		}

		return;
	}

	CString outValue = MakeOutputValue(value);
	if (!outValue.IsEmpty())
	{
		CString command;
		command.Format(_T("DebugSetVariable('%s','%s',%s)"), theApp.GetDebuggerView()->GetStateToDebug(), fullName, outValue);
		theApp.GetNetworkClient().SendCommand(command);

		command.Format(_T("DebugReadLocalVariables('%s')"), theApp.GetDebuggerView()->GetStateToDebug());
		theApp.GetNetworkClient().SendCommand(command);
	}

	// Validate data.
//	LuaAutoBlock block(*m_script);
//	m_script->GetGlobal("ValidatePropertyGridRow");
//	m_script->PushUserData(this);
//	m_script->PushNumber(pItem->iRow);
//	m_script->Call(2, 0);

	// Write data
//	LuaAutoBlock block(*m_script);
//	LuaStackObject newTable = m_script->NewTable();

//	m_script->GetGlobal("WriteOutputFromPropertyGridRow");
//	m_script->PushUserData(this);
//	m_script->GetGlobal(m_propertyDefinitionsName);
//	m_script->PushObject(newTable);
//	m_script->PushNumber(pItem->iRow);
//	m_script->Call(4, 0);

//	if (m_propertyGridCallbacks)
//		m_propertyGridCallbacks->OnGridCellEndLabelEdit(pItem->iRow, newTable);
}

