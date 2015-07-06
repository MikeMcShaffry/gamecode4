INCLUDE(../../LuaPlusBuildTypes.cmake)

INCLUDE_DIRECTORIES(../../Src)

SET(SRCS
		CallStackDlg.cpp
		CallStackDlg.h
		CommandLineDlg.cpp
		CommandLineDlg.h
		DebuggerView.cpp
		DebuggerView.h
		FindDlg.cpp
		FindDlg.h
		GotoLineDlg.cpp
		GotoLineDlg.h
		MainFrm.cpp
		MainFrm.h
		OutputDlg.cpp
		OutputDlg.h
		RemoteLuaDebugger.cpp
		RemoteLuaDebugger.h
		RemoteLuaDebugger.rc
		RemoteLuaDebuggerDoc.cpp
		RemoteLuaDebuggerDoc.h
		resource.h
		RemoteLuaDebugger_InternalPch.cpp
		RemoteLuaDebugger_InternalPch.h
		VariablesBar.cpp
		VariablesBar.h
		VariablesGrid.cpp
		VariablesGrid.h
		WindowListDlg.cpp
		WindowListDlg.h
		../Scintilla/ScintillaWrapper/SciLexer.h
		../Scintilla/ScintillaWrapper/Scintilla.h
		../Scintilla/ScintillaWrapper/ScintillaEditor.cpp
		../Scintilla/ScintillaWrapper/ScintillaEditor.h
		../Scintilla/ScintillaWrapper/ScintillaPropertiesFile.cpp
		../Scintilla/ScintillaWrapper/ScintillaPropertiesFile.h
		../Scintilla/ScintillaWrapper/StdAfx.h
		../SharedMFC/ACEditCtrl.cpp
		../SharedMFC/ACEditCtrl.h
		../SharedMFC/ChoiceWindow.cpp
		../SharedMFC/ChoiceWindow.h
		../SharedMFC/ChoiceWindowBtn.cpp
		../SharedMFC/ChoiceWindowBtn.h
		../SharedMFC/MainWndPlacement.h
		../SharedMFC/PropertyView.cpp
		../SharedMFC/PropertyView.h
		../SharedMFC/QTabbedDialog.cpp
		../SharedMFC/QTabbedDialog.h
		../SharedMFC/QTabbedDialog.txt
		../SharedMFC/ResizableDialog.cpp
		../SharedMFC/ResizableDialog.h
		../SharedMFC/ResizablePage.cpp
		../SharedMFC/ResizablePage.h
		../SharedMFC/ResizableSheet.cpp
		../SharedMFC/ResizableSheet.h
		../SharedMFC/StdAfx.cpp
		../SharedMFC/StdAfx.h
		../SharedMFC/Subclass.cpp
		../SharedMFC/Subclass.h
		../SharedMFC/Codex/cdxCDynamicBar.cpp
		../SharedMFC/Codex/cdxCDynamicBar.h
		../SharedMFC/Codex/cdxCDynamicControlsManager.cpp
		../SharedMFC/Codex/cdxCDynamicControlsManager.h
		../SharedMFC/Codex/cdxCDynamicDialog.cpp
		../SharedMFC/Codex/cdxCDynamicDialog.h
		../SharedMFC/Codex/cdxCDynamicFormView.cpp
		../SharedMFC/Codex/cdxCDynamicFormView.h
		../SharedMFC/Codex/cdxCDynamicPropSheet.cpp
		../SharedMFC/Codex/cdxCDynamicPropSheet.h
		../SharedMFC/Codex/cdxCDynamicWnd.cpp
		../SharedMFC/Codex/cdxCDynamicWnd.h
		../SharedMFC/Codex/cdxCDynamicWndEx.cpp
		../SharedMFC/Codex/cdxCDynamicWndEx.h
		../SharedMFC/Codex/cdxCSizeIconCtrl.cpp
		../SharedMFC/Codex/cdxCSizeIconCtrl.h
		../SharedMFC/Codex/scbarcf.cpp
		../SharedMFC/Codex/scbarcf.h
		../SharedMFC/Codex/scbarg.cpp
		../SharedMFC/Codex/scbarg.h
		../SharedMFC/Codex/SizeCBar.cpp
		../SharedMFC/Codex/SizeCBar.h
		../SharedMFC/GridBtnCell_src/BtnDataBase.cpp
		../SharedMFC/GridBtnCell_src/BtnDataBase.h
		../SharedMFC/GridBtnCell_src/GridBtnCell.cpp
		../SharedMFC/GridBtnCell_src/GridBtnCell.h
		../SharedMFC/GridBtnCell_src/GridBtnCellBase.cpp
		../SharedMFC/GridBtnCell_src/GridBtnCellBase.h
		../SharedMFC/GridBtnCell_src/GridBtnCellCombo.cpp
		../SharedMFC/GridBtnCell_src/GridBtnCellCombo.h
		../SharedMFC/GridBtnCell_src/InPlaceList.cpp
		../SharedMFC/GridBtnCell_src/InPlaceList.h
		../SharedMFC/GridCtrl_src/CellRange.h
		../SharedMFC/GridCtrl_src/GridCell.cpp
		../SharedMFC/GridCtrl_src/GridCell.h
		../SharedMFC/GridCtrl_src/GridCellBase.cpp
		../SharedMFC/GridCtrl_src/GridCellBase.h
		../SharedMFC/GridCtrl_src/GridCtrl.cpp
		../SharedMFC/GridCtrl_src/GridCtrl.h
		../SharedMFC/GridCtrl_src/GridDropTarget.cpp
		../SharedMFC/GridCtrl_src/GridDropTarget.h
		../SharedMFC/GridCtrl_src/InPlaceEdit.cpp
		../SharedMFC/GridCtrl_src/InPlaceEdit.h
		../SharedMFC/GridCtrl_src/MemDC.h
		../SharedMFC/GridCtrl_src/TitleTip.cpp
		../SharedMFC/GridCtrl_src/TitleTip.h
		../SharedMFC/GridTreeBtnCell_src/GridTreeBtnCell.cpp
		../SharedMFC/GridTreeBtnCell_src/GridTreeBtnCell.h
		../SharedMFC/TreeColumn_src/GridTreeCell.cpp
		../SharedMFC/TreeColumn_src/GridTreeCell.h
		../SharedMFC/TreeColumn_src/GridTreeCellBase.cpp
		../SharedMFC/TreeColumn_src/GridTreeCellBase.h
		../SharedMFC/TreeColumn_src/TreeColumn.cpp
		../SharedMFC/TreeColumn_src/TreeColumn.h
		../../Src/LuaRemoteDebuggingServer/LuaNetworkClient.cpp
		../../Src/LuaRemoteDebuggingServer/LuaNetworkClient.h
		../../Src/LuaRemoteDebuggingServer/LuaNetworkHelpers.cpp
		../../Src/LuaRemoteDebuggingServer/LuaNetworkHelpers.h
)

IF (MSVC)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF (MSVC)

ADD_DEFINITIONS(-D_AFXDLL -D_UNICODE)
INCLUDE_DIRECTORIES(../SharedMFC ../Scintilla/ScintillaWrapper)

SET(CMAKE_MFC_FLAG 2)
ADD_EXECUTABLE(RemoteLuaDebugger WIN32 ${SRCS})
TARGET_LINK_LIBRARIES(RemoteLuaDebugger LuaPlusShared winmm)
LUAPLUS_FIX_TARGET(RemoteLuaDebugger)

#INSTALL(TARGETS LuaPlus
#	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)