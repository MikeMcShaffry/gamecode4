//Copyright (c) Microsoft Corporation.  All rights reserved.

// Connect.h : Declaration of the CConnect

#pragma once
#include "resource.h"       // main symbols


//The following #import imports the LuaPlusDebuggerControls.CWatchCtl interface based on it's LIBID
#import "libid:8BA18739-40D2-40A3-A1C6-13705A878F63" version("1.0") lcid("0")  raw_interfaces_only named_guids


// CConnect

class ATL_NO_VTABLE CConnect : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CConnect, &CLSID_Connect>,
	public IDispatchImpl<EnvDTE::IDTCommandTarget, &EnvDTE::IID_IDTCommandTarget, &EnvDTE::LIBID_EnvDTE, 7, 0>,
	public IDispatchImpl<AddInDesignerObjects::_IDTExtensibility2, &AddInDesignerObjects::IID__IDTExtensibility2, &AddInDesignerObjects::LIBID_AddInDesignerObjects, 1, 0>
{
public:
	CConnect()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_ADDIN)
DECLARE_NOT_AGGREGATABLE(CConnect)



BEGIN_COM_MAP(CConnect)
	COM_INTERFACE_ENTRY(AddInDesignerObjects::IDTExtensibility2)
	COM_INTERFACE_ENTRY(EnvDTE::IDTCommandTarget)
	COM_INTERFACE_ENTRY2(IDispatch, AddInDesignerObjects::IDTExtensibility2)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:
	//IDTExtensibility2 implementation:
	STDMETHOD(OnConnection)(IDispatch * Application, AddInDesignerObjects::ext_ConnectMode ConnectMode, IDispatch *AddInInst, SAFEARRAY **custom);
	STDMETHOD(OnDisconnection)(AddInDesignerObjects::ext_DisconnectMode RemoveMode, SAFEARRAY **custom );
	STDMETHOD(OnAddInsUpdate)(SAFEARRAY **custom );
	STDMETHOD(OnStartupComplete)(SAFEARRAY **custom );
	STDMETHOD(OnBeginShutdown)(SAFEARRAY **custom );
	
	STDMETHOD(QueryStatus)(BSTR CmdName, EnvDTE::vsCommandStatusTextWanted NeededText, EnvDTE::vsCommandStatus *StatusOption, VARIANT *CommandText);
	STDMETHOD(Exec)(BSTR CmdName, EnvDTE::vsCommandExecOption ExecuteOption, VARIANT *VariantIn, VARIANT *VariantOut, VARIANT_BOOL *Handled);

	CComPtr<EnvDTE::_DTE> m_pDTE;
	CComPtr<EnvDTE::AddIn> m_pAddInInstance;
	CComQIPtr<LuaPlusDebuggerControls::IWatchCtl, &__uuidof(LuaPlusDebuggerControls::IWatchCtl)> m_pWatchCtl;
	CComPtr<EnvDTE::Window> m_pWindow;
};

OBJECT_ENTRY_AUTO(__uuidof(Connect), CConnect)
