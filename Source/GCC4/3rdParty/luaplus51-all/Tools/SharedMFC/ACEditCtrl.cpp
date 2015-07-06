/*|*\
|*|  File:      ACEditCtrl.cpp
|*|  
|*|  By:        James R. Twine, TransactionWorks, Inc.
|*|             Copyright 1999, TransactionWorks, inc.
|*|  Date:      Tuesday, September 07, 1999
|*|             
|*|  Notes:     This Class Implements A Simple Auto-Completion Edit Control
|*|             Note That This Class Is Not Designed To Handle Large Amounts
|*|             Of Completion Strings, But For Things Like Recent Paths,
|*|             Names, Etc. It Works Fine.
|*|             
|*|             May Be Freely Incorporated Into Projects Of Any Type
|*|             Subject To The Following Conditions:
|*|             
|*|             o This Header Must Remain In This File, And Any
|*|               Files Derived From It
|*|             o Do Not Misrepresent The Origin Of This Code
|*|               (IOW, Do Not Claim You Wrote It)
|*|             
|*|             A "Mention In The Credits", Or Similar Acknowledgement,
|*|             Is *NOT* Required.  It Would Be Nice, Though! :)
\*|*/
//
//	Copy The Following #define To Your StdAfx.h File In 
//	Order To Use My Custom "CFocusEdit" Control As A 
//	Base Instead Of The Standard MFC CEdit Control.
//
//#define		_USE_FOCUSEDITCTRL_		1
#include	"stdafx.h"

#pragma warning (disable : 4018)	// '<':  signed/unsigned mismatch
#pragma warning (disable : 4100)	// unreferenced formal parameter
#pragma warning (disable : 4127)	// conditional expression is constant
#pragma warning (disable : 4244)	// conv from X to Y, possible loss of data
#pragma warning (disable : 4310)	// cast truncates constant value
#pragma warning (disable : 4505)	// X: unreference local function removed
#pragma warning (disable : 4510)	// X: default ctor could not be generated
#pragma warning (disable : 4511)	// X: copy constructor could not be generated
#pragma warning (disable : 4512)	// assignment operator could not be generated
#pragma warning (disable : 4514)	// debug symbol exceeds 255 chars
#pragma warning (disable : 4610)	// union X can never be instantiated
#pragma warning (disable : 4663)	// to explicitly spec class template X use ...
#pragma warning (disable : 4710)	// function 'XXX' not expanded
#pragma	warning	(disable : 4786)	// X: identifier truncated to '255' chars
#pragma warning( disable:4706 )

#include	<vector>
#include	<string>

#if defined( _USE_FOCUSEDITCTRL_ )
#pragma message( "ACEditCtrl: Using Focus Edit Control For AutoCompletion Edit Control..." )
#include	"FocusEditCtrl.h"
#else
#pragma message( "ACEditCtrl: Using Standard Edit Control For AutoCompletion Edit Control..." )
#endif

#include	"ACEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CACEditCtrl

/**/	CACEditCtrl::CACEditCtrl() :
	m_bDisableAC( false ),									// Default To AC Enabled
	m_bDisabledInternally( false ),							// Not Disabled Internally
	m_bInUpdate( false )									// Not In The Update Handler
{
	return;													// Done!
}


/**/	CACEditCtrl::~CACEditCtrl()
{
	SSVectIter	iterString = m_vecACStrings.begin();
	SSVectIter	iterEnd = m_vecACStrings.end();
	SString		*pString = NULL;

	while( iterString != iterEnd )							// For All Available String Objects
	{
		pString = *iterString;								// Get String Pointer
		iterString++;										// Increment Iterator
		if( pString )										// If Not NULL
		{
			delete pString;									// Dealocate String
		}
	}
	return;													// Done!
}


void	CACEditCtrl::AddACEntry( LPCTSTR cpEntry )
{
	_ASSERTE( ( !cpEntry ) || ( !::IsBadStringPtr( cpEntry, // If Not NULL, Then...
			1 ) ) );										// Assert Valid For At Least One Character
	
	if( ( cpEntry ) && ( _tcslen( cpEntry ) >= 2 ) )		// If A String Is There
	{
		//
		//	NOTE: No Duplicate Checking Is Done Here, It Is 
		//	Supposed To Be Handled By The Data Provider!!!!
		//
		m_vecACStrings.push_back( new SString( cpEntry ) );	// Add String To Vector
	}
	return;													// Done!
}


void	CACEditCtrl::DisableAC( bool bDisable )
{
	m_bDisableAC = bDisable;								// Set Flag As Requested
	m_bDisabledInternally = false;							// Unset Internal Flag

	return;													// Done!
}


#if	defined( _USE_FOCUSEDITCTRL_ )
BEGIN_MESSAGE_MAP(CACEditCtrl, CFocusEditCtrl)
#else
BEGIN_MESSAGE_MAP(CACEditCtrl, CEdit)
#endif
	//{{AFX_MSG_MAP(CACEditCtrl)
	ON_CONTROL_REFLECT_EX(EN_UPDATE, OnUpdate)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CACEditCtrl message handlers

void CACEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( ( nChar == VK_DELETE ) || ( nChar == VK_BACK ) )	// If <DELETE> or <BACKSPACE>
	{
		m_bDisableAC = true;								// Disable AutoComplete
		m_bDisabledInternally = true;
	}
	else if( ( m_bDisableAC ) && ( m_bDisabledInternally ) )// Else If Some Other Key, And Disabled Internally
	{
		m_bDisableAC = false;								// Enable AC
		m_bDisabledInternally = false;						// Unset Internal Flag
		UpdateWindow();										// Trigger Update
	}
#if	defined( _USE_FOCUSEDITCTRL_ )
	CFocusEditCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
#else
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
#endif
}


BOOL CACEditCtrl::OnUpdate() 
{
	if( m_bInUpdate )										// If Already Here
	{
		return( FALSE );									// Just Stop Here
	}
	int		iTLength = 0;
	
	m_bInUpdate = true;										// Set Recursion Flag
	
	if( ( m_vecACStrings.size() ) && ( iTLength =			// If Text There And Strings To Check Aganist
			GetWindowTextLength() ) && ( !m_bDisableAC ) )	// And Matching Not Disabled
	{
		SSVectIter	iterString = m_vecACStrings.begin();
		SSVectIter	iterEnd = m_vecACStrings.end();

		GetWindowText( m_sTarget );							// Get Target Text
		//
		//	Is The The Fastest Way?  No.  Is It Good Enough
		//	For The Current Specifications?  Yes.
		//	Comments/Suggestions Are Encouraged.
		//
		while( iterString != iterEnd )						// While Strings Available To Check
		{
			LPCTSTR		cpTest = (*iterString) -> c_str();	// Get Text To Test Aganist
			
			++iterString;									// Increment Iterator
			
			if( ( cpTest ) && ( !_tcsncmp( m_sTarget, 		// If String Pointer Not NULL
					cpTest, iTLength ) ) )					// If Match On Substring Found
			{
				int		iStart = 0;
				int		iEnd = 0;
				
				GetSel( iStart, iEnd );						// Get Selection (Cursor Position)
				SetWindowText( cpTest );					// Set New Window Text
				SetSel( iStart, -1 );						// Reset Selection
				
				UpdateWindow();								// Force Update
				
				break;										// Stop Here
			}
		}
	}
	m_bInUpdate = false;									// Unset Recursion Flag
	
	return( TRUE );											// Done!
}
