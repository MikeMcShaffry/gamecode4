/*|*\
|*|  File:      ACEditCtrl.h
|*|  
|*|  By:        James R. Twine, TransactionWorks, Inc.
|*|             Copyright 1999, TransactionWorks, inc.
|*|  Date:      Tuesday, September 07, 1999
|*|             
|*|  Notes:     This Class Implements A Simple Auto-Completion Edit Control
|*|             
|*|  Revisions: 
|*|             
\*|*/
#if !defined( __TW_ACEDITCTRL_H_ )
#define		__TW_ACEDITCTRL_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ACEditCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CACEditCtrl window

typedef		std::basic_string< TCHAR >	SString;			// String Type
typedef		std::vector< SString* >		SSVect;				// String Vector Type
typedef		SSVect::iterator			SSVectIter;			// String Vector Iterator

#if defined( _USE_FOCUSEDITCTRL_ )
class	CACEditCtrl : public CFocusEditCtrl
#else
class	CACEditCtrl : public CEdit
#endif
{
// Construction
public:
	/**/	CACEditCtrl();									// Constructor

	void	AddACEntry( LPCTSTR cpEntry );					// Add An AutoCompletion Entry
	void	DisableAC( bool bDisable = true );				// Disable AutoCompletion
	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CACEditCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual	~CACEditCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CACEditCtrl)
	afx_msg BOOL OnUpdate();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	CString	m_sTarget;										// Target String To Check
	SSVect	m_vecACStrings;									// Vector Of AutoCompletion Strings
	bool	m_bDisableAC;									// Flag To Disable AutoCompletion
	bool	m_bDisabledInternally;							// Flag For Internal Disabling Of AC
	bool	m_bInUpdate;									// Recursion Flag

private:
	/**/			CACEditCtrl( const CACEditCtrl &rSrc );	// Prevent Copy-Construction
	CACEditCtrl&	operator=( const CACEditCtrl &rSrc );	// Prevent Assignment
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif		// __TW_ACEDITCTRL_H_
