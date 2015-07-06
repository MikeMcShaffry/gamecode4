// WatchCtl.h : Declaration of the CWatchCtl
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>
#include "LuaPlusTypes.h"

// IWatchCtl
[
	object,
	uuid(9D9A543B-0982-4D85-8B10-0E49683F8C59),
	dual,
	helpstring("IWatchCtl Interface"),
	pointer_default(unique)
]
__interface IWatchCtl : public IDispatch
{
	[id(1), helpstring("method OnConnect")] HRESULT OnConnect([in] IDispatch* pApplication);
	[id(2), helpstring("method OnDisconnect")] HRESULT OnDisconnect();
};


// CWatchCtl
[
	coclass,
	threading(single),
	vi_progid("LuaPlusDebuggerControls.WatchCtl"),
	progid("LuaPlusDebuggerControls.WatchCtl.1"),
	version(1.0),
	uuid("D08C4F5B-5C34-412F-8D80-86C30BC4F938"),
	helpstring("WatchCtl Class"),
	support_error_info(IWatchCtl),
	registration_script("control.rgs")
]
class ATL_NO_VTABLE CWatchCtl : 
	public IWatchCtl,
	public IPersistStreamInitImpl<CWatchCtl>,
	public IOleControlImpl<CWatchCtl>,
	public IOleObjectImpl<CWatchCtl>,
	public IOleInPlaceActiveObjectImpl<CWatchCtl>,
	public IViewObjectExImpl<CWatchCtl>,
	public IOleInPlaceObjectWindowlessImpl<CWatchCtl>,
	public IPersistStorageImpl<CWatchCtl>,
	public ISpecifyPropertyPagesImpl<CWatchCtl>,
	public IQuickActivateImpl<CWatchCtl>,
	public IDataObjectImpl<CWatchCtl>,
	public IProvideClassInfo2Impl<&__uuidof(CWatchCtl), NULL>,
	public CComCompositeControl<CWatchCtl>,
	public CScrollImpl<CWatchCtl>,
	public LayoutMgr<CWatchCtl>
//	public CDialogResize<CWatchCtl>
{
public:

	CWatchCtl() :
		m_processHandle(NULL)
	{
		m_bWindowOnly = TRUE;
		CalcExtent(m_sizeExtent);
	}

DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE | 
	OLEMISC_CANTLINKINSIDE | 
	OLEMISC_INSIDEOUT | 
	OLEMISC_ACTIVATEWHENVISIBLE | 
	OLEMISC_SETCLIENTSITEFIRST
)


BEGIN_PROP_MAP(CWatchCtl)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	// Example entries
	// PROP_ENTRY("Property Description", dispid, clsid)
	// PROP_PAGE(CLSID_StockColorPage)
END_PROP_MAP()

/*    BEGIN_DLGRESIZE_MAP(CVariablesDlg)
        DLGRESIZE_CONTROL(IDC_VD_ACCEPTLUASTATENAME, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDCANCEL, DLSZ_MOVE_X)
        DLGRESIZE_CONTROL(IDC_TABLES, DLSZ_SIZE_Y)
        DLGRESIZE_CONTROL(IDC_KEYVALUES, DLSZ_SIZE_X | DLSZ_SIZE_Y)
    END_DLGRESIZE_MAP()
*/
	BEGIN_MSG_MAP(CWatchCtl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_HANDLER(IDC_LOCALS, BN_CLICKED, OnBnClickedLocals)
		CHAIN_MSG_MAP(CScrollImpl<CWatchCtl>)
//		CHAIN_MSG_MAP(CDialogResize<CWatchCtl>)
		COMMAND_HANDLER(IDC_VD_ACCEPTLUASTATENAME, BN_CLICKED, OnBnClickedAccept)
		NOTIFY_HANDLER(IDC_TABLES, TVN_ITEMEXPANDING, OnTvnItemexpandingTables)
		NOTIFY_HANDLER(IDC_TABLES, TVN_SELCHANGED, OnTvnSelchangedTables)
		COMMAND_HANDLER(IDC_GLOBALS, BN_CLICKED, OnBnClickedGlobals)
		COMMAND_HANDLER(IDC_REGISTRY, BN_CLICKED, OnBnClickedRegistry)
		COMMAND_HANDLER(IDC_EVALUATESYMBOL, BN_CLICKED, OnBnClickedEvaluatesymbol)
		COMMAND_HANDLER(IDC_SYMBOL, EN_CHANGE, OnEnChangeSymbol)
		MESSAGE_HANDLER(WM_SIZE, LayoutMgr<CWatchCtl>::OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, LayoutMgr<CWatchCtl>::OnEraseBackground)
		CHAIN_MSG_MAP(CComCompositeControl<CWatchCtl>)
	END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

// IViewObjectEx
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

// IWatchCtl

	enum { IDD = IDD_WATCHCTL };

	DECLARE_PROTECT_FINAL_CONSTRUCT()

// LayoutMgr
	//! Returns the client area of the container
	void GetDialogRect(LPRECT r)
	{
		CWatchCtl* pT = static_cast<CWatchCtl*>(this);
		// NB : GetClientRect returns always a rectangle with left an top are set to 0
		pT->GetClientRect(r);	
		if (r->right < pT->m_sizeAll.cx)
		{
			r->right = pT->m_sizeAll.cx;
		}
		if (r->bottom < pT->m_sizeAll.cy)
		{
			r->bottom = pT->m_sizeAll.cy;
		}
	}

	//! This methods returns the offset for positionning the controls in the client rect
	void GetOffset(LPPOINT offset)
	{
		CWatchCtl* pT = static_cast<CWatchCtl*>(this);
		*offset = pT->m_ptOffset;
	}

	//! Paints the view
	virtual void DoPaint(CDCHandle /*dc*/)
	{
		// Just avoid the assertion
	}

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	void Initialize();

	STDMETHOD(OnConnect)(IDispatch *pApplication);
	STDMETHOD(OnDisconnect)();

	LRESULT OnBnClickedAccept(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	void OnPlusExpand(CTreeItem treeItem);
	void FillInKeyValues(void* inTableObj, CTreeItem* treeItem);
	void ReadAddresses();

	void UpdateLuaObjectAddress(void);
	void UpdateLuaStateAddress(void);
	void Refresh();
	TValue LookupSymbol(TValue obj, CString symbol);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTvnItemexpandingTables(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnTvnSelchangedTables(int /*idCtrl*/, LPNMHDR pNMHDR, BOOL& /*bHandled*/);
	LRESULT OnBnClickedGlobals(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedRegistry(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedEvaluatesymbol(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnChangeSymbol(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	CComPtr<EnvDTE::_DTE> m_pDTE;
	CComPtr<EnvDTE::Debugger> m_pDebugger;
	long m_processID;
	HANDLE m_processHandle;

	void* m_LuaStackObjectPtr;
	void* m_LuaObjectPtr;
	void* m_LuaStateOwnerPtr;
	void* m_LuaStatePtr;
	void* m_luaStatePtr;
	TValue* m_lookupObject;

	CWindow m_luaStateNameWnd;
	CWindow m_symbolWnd;
	CButton m_evaluateSymbolRadio;
	CButton m_globalsRadio;
	CButton m_registryRadio;
	CButton m_localsRadio;
	CButton m_xbox360Check;
//	CSplitterWindow m_splitterWnd;
	CTreeViewCtrlEx m_tableTree;
	CListViewCtrl m_keyValueList;

	CString m_symbol;
	LRESULT OnBnClickedLocals(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};

