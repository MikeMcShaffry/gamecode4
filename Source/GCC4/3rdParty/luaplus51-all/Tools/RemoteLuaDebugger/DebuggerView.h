#if !defined(AFX_DEBUGGERVIEW_H__9C6B7528_5172_4EC6_BDFD_060F9E02BC9F__INCLUDED_)
#define AFX_DEBUGGERVIEW_H__9C6B7528_5172_4EC6_BDFD_060F9E02BC9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DebuggerView.h : header file
//

#include "VariablesGrid.h"

class CRemoteLuaDebuggerDoc;

/////////////////////////////////////////////////////////////////////////////
// CDebuggerView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CDebuggerView : public cdxCDynamicFormView
{
protected:
	CDebuggerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CDebuggerView)

// Form Data
public:
	//{{AFX_DATA(CDebuggerView)
	enum { IDD = IDD_DEBUGGER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:
	CRemoteLuaDebuggerDoc* GetDocument()  { return (CRemoteLuaDebuggerDoc*)m_pDocument; };

// Operations
public:
	CString m_currentFileName;

	void DebugSetLocation(const CStringW& stateToDebug, const CString& fileName, int lineNumber);
	int LS_DebugUpdateLocals( LuaState* state );
	int LS_DebugUpdateWatch( LuaState* state );
	void DebugUpdateCallStack(const CStringW& stateNameToDebug);
	void DebugAddAllWatches(const CStringW& stateNameToDebug);
	void DebugSendAllBreakpoints(const CStringW& stateNameToDebug);

	void OpenFile(const CString& fileName, int lineNumber);

	ScintillaEditor& GetEditor()			{  return m_editor;  }
	TextToFind& GetTextToFind_s()			{  return m_ft;  }
	const CString& GetFindText()			{  return m_findText;  }
	void SetFindText(const CString& findText){  m_findText = findText;  }
	void FindNext();

	const CStringW& GetStateToDebug() const			{  return m_stateToDebug;  }
		
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebuggerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DrawAllBreakpoints();

	TextToFind m_ft;
	CString m_findText;

	CStringW m_stateToDebug;
	ScintillaEditor m_editor;
	CVariablesGrid m_localsGrid;
	CVariablesGrid m_watchGrid;

	virtual ~CDebuggerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CDebuggerView)
	afx_msg void OnDbStepover();
	afx_msg void OnDbTogglebreakpoint();
	afx_msg void OnDbStepinto();
	afx_msg void OnDbBreak();
	afx_msg void OnDbGo();
	afx_msg void OnDbStepout();
	afx_msg void OnDbGotoline();
	afx_msg void OnDbFind();
	afx_msg void OnEditCopy();
	afx_msg void OnDbFindnext();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGGERVIEW_H__9C6B7528_5172_4EC6_BDFD_060F9E02BC9F__INCLUDED_)
