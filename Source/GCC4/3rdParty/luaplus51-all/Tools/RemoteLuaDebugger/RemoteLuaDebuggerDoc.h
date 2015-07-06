#pragma once

#include "DebuggerView.h"

class CRemoteLuaDebuggerDoc : public CDocument
{
protected: // create from serialization only
	CRemoteLuaDebuggerDoc();
	DECLARE_DYNCREATE(CRemoteLuaDebuggerDoc)

// Attributes
public:
	CDebuggerView* GetDebuggerView();
	LuaState* GetState()			{  return m_state;  }
	void GetActiveFiles(CStringArray& fileList);

// Operations
public:
	void OnIdleCustom();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemoteLuaDebuggerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRemoteLuaDebuggerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	LuaState* m_state;

// Generated message map functions
protected:
	//{{AFX_MSG(CRemoteLuaDebuggerDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

