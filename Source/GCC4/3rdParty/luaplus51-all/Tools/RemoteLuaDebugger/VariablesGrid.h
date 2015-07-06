#pragma once

#include "GridCtrl_src\GridCtrl.h"
#include "GridBtnCell_src\BtnDataBase.h"
#include "TreeColumn_src\TreeColumn.h"

class CVariablesGrid : public CGridCtrl
{
public:
	CVariablesGrid();
	virtual ~CVariablesGrid();

	void CreateGrid(CWnd* parentWnd, UINT windowId);
	void Init();

	void BuildLocals( LuaObject tableObj );
	void BuildWatch();
	void UpdateWatch( LuaObject tableObj );
	void DebugAddAllWatches(const CStringW& stateNameToDebug);
	void Update(LuaStackObject& inTable);
	
	void AddSection(const CString& title, const CString& type);

	int AddEditProperty( int treeDepth, const CStringW& title, bool expanded );
	void SetEditProperty(int row, const CStringW& data);

protected:
	void FixupRows(bool readOnly);
	int FindWatchActualIndex( int row );
	CString GetFullName( int row );

	DECLARE_MESSAGE_MAP()

	afx_msg void OnGridCellEndLabelEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnTreeClick(NMHDR *pNotifyStruct, LRESULT* pResult);

	void SaveExpandInfo(bool extraRow);
	void RestoreExpandInfo(bool extraRow);
		
	enum GridType
	{
		LOCALS,
		WATCH,
	};

	GridType m_gridType;

	CBtnDataBase m_buttonDatabase;
	CImageList m_imageList;
	CTreeColumn m_treeColumn;
	CStringArray m_titleList;
	CArray<BYTE> m_treeLevelArray;
	CArray<bool> m_treeForcePlusArray;

	struct ControlInfo
	{
		bool m_hasSpin;
		float m_rangeMin;
		float m_rangeMax;
		float m_step;
	};
	CArray<ControlInfo> m_controlInfo;

	struct ExpandInfo
	{
		CString m_fullName;
	};

	CArray<ExpandInfo> m_expandInfo;

	CFont m_titleFont;
	CFont m_dataFont;

	CArray<CString> m_watchArray;
};
