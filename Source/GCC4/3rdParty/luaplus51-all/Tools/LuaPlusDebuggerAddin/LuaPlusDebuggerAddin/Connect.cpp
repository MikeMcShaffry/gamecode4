//Copyright (c) Microsoft Corporation.  All rights reserved.

// Connect.cpp : Implementation of CConnect
#include "stdafx.h"
#include "AddIn.h"
#include "Connect.h"

extern CAddInModule _AtlModule;

// CConnect
STDMETHODIMP CConnect::OnConnection(IDispatch *pApplication, AddInDesignerObjects::ext_ConnectMode ConnectMode, IDispatch *pAddInInst, SAFEARRAY ** /*custom*/ )
{
	m_pDTE = NULL;
	m_pAddInInstance = NULL;

	CComPtr<EnvDTE::Windows> pWindows;
	CComPtr<IDispatch> pCtlObject;
	HRESULT hr = S_OK;
	pApplication->QueryInterface(__uuidof(EnvDTE::_DTE), (LPVOID*)&m_pDTE);
	pAddInInst->QueryInterface(__uuidof(EnvDTE::AddIn), (LPVOID*)&m_pAddInInstance);

	// Chances are, this won't work.
	if ( ConnectMode == AddInDesignerObjects::ext_cm_CommandLine )
		return S_OK;

	CComPtr<EnvDTE::Commands> pCommands;
	CComPtr<Office::_CommandBars> pCommandBars;
	hr = m_pDTE->get_Commands(&pCommands);
	if ( FAILED( hr ) )
		return hr;
	if ( !pCommands )
		return E_FAIL;

	// Get the set of command bars for the application.
	try
	{
		CComPtr<IDispatch> pCommandBarsIDispatch;
		hr = m_pDTE->get_CommandBars(&pCommandBarsIDispatch);
		pCommandBars = pCommandBarsIDispatch;
	}
	catch (...)
	{
		// This will exception in a command-line build.
	}

	// See if the toolbar has been created.
	bool forceRecreateOfToolbarAndCommands = true;

	if (pCommandBars)
	{
		CComPtr<Office::CommandBar> pCommandBar;
		hr = pCommandBars->get_Item(CComVariant(L"Tools"), &pCommandBar );
		if ( SUCCEEDED( hr ) )
		{
			CComPtr<Office::CommandBarControls> pCommandBarControls;
			pCommandBar->get_Controls( &pCommandBarControls );

			CComPtr<Office::CommandBarControl> pCommandBarControl;
			pCommandBarControls->get_Item(CComVariant("LuaPlus Watch"), &pCommandBarControl);
			if (pCommandBarControl)
			{
				forceRecreateOfToolbarAndCommands = false;
			}
		}
	}

	m_pDTE->get_Windows(&pWindows);
	hr = pWindows->CreateToolWindow(m_pAddInInstance, CComBSTR("LuaPlusDebuggerControls.WatchCtl"), CComBSTR("LuaPlus Watch"), CComBSTR("{B8DCB086-5D26-4f18-821D-FA1BA25512B6}"), &pCtlObject, &m_pWindow);
	m_pWatchCtl = pCtlObject;
	m_pWatchCtl->OnConnect(m_pDTE);
	m_pWindow->put_Visible(VARIANT_TRUE);

	if(ConnectMode == 5  ||  forceRecreateOfToolbarAndCommands)
	{
		HRESULT hr = S_OK;
		CComPtr<IDispatch> pCommandBarControl;
		CComPtr<EnvDTE::Command> pCreatedCommand;
		CComPtr<Office::CommandBar> pMenuBarCommandBar;

		if (pCommandBars)
		{
			pCommandBars->get_Item(CComVariant(L"Tools"), &pMenuBarCommandBar);
		}

		try
		{
			if(SUCCEEDED(pCommands->AddNamedCommand(m_pAddInInstance, CComBSTR("LuaPlusWatch"), CComBSTR("LuaPlus Watch Window"), CComBSTR("Show the LuaPlus Watch Window"), VARIANT_TRUE, 59, NULL, EnvDTE::vsCommandStatusSupported+EnvDTE::vsCommandStatusEnabled, &pCreatedCommand)) && (pCreatedCommand))
			{
				//Add a button to the tools menu bar.
				if (pMenuBarCommandBar)
					pCreatedCommand->AddControl(pMenuBarCommandBar, 1, &pCommandBarControl);
			}
		}
		catch (...)
		{
			// This can exception in a command-line build.
		}

		return S_OK;
	}

	return hr;
}

STDMETHODIMP CConnect::OnDisconnection(AddInDesignerObjects::ext_DisconnectMode /*RemoveMode*/, SAFEARRAY ** /*custom*/ )
{
	m_pWatchCtl->OnDisconnect();
	m_pAddInInstance = NULL;
	m_pDTE = NULL;
	return S_OK;
}

STDMETHODIMP CConnect::OnAddInsUpdate (SAFEARRAY ** /*custom*/ )
{
	return S_OK;
}

STDMETHODIMP CConnect::OnStartupComplete (SAFEARRAY ** /*custom*/ )
{
	return S_OK;
}

STDMETHODIMP CConnect::OnBeginShutdown (SAFEARRAY ** /*custom*/ )
{
	return S_OK;
}

STDMETHODIMP CConnect::QueryStatus(BSTR bstrCmdName, EnvDTE::vsCommandStatusTextWanted NeededText, EnvDTE::vsCommandStatus *pStatusOption, VARIANT *pvarCommandText)
{
  if(NeededText == EnvDTE::vsCommandStatusTextWantedNone)
	{
	  if(!_wcsicmp(bstrCmdName, L"LuaPlusDebugger.Connect.LuaPlusWatch"))
	  {
		  // Only activate the command if the debugger is active.
		  EnvDTE::vsIDEMode ideMode;
		  m_pDTE->get_Mode(&ideMode);

		  if (ideMode != EnvDTE::vsIDEModeDesign)
			  *pStatusOption = (EnvDTE::vsCommandStatus)(EnvDTE::vsCommandStatusEnabled+EnvDTE::vsCommandStatusSupported);
		  else
			  *pStatusOption = (EnvDTE::vsCommandStatus)(EnvDTE::vsCommandStatusInvisible);
	  }
  }
	return S_OK;
}


STDMETHODIMP CConnect::Exec(BSTR bstrCmdName, EnvDTE::vsCommandExecOption ExecuteOption, VARIANT * /*pvarVariantIn*/, VARIANT * /*pvarVariantOut*/, VARIANT_BOOL *pvbHandled)
{
	*pvbHandled = VARIANT_FALSE;
	if(ExecuteOption == EnvDTE::vsCommandExecOptionDoDefault)
	{
		if(!_wcsicmp(bstrCmdName, L"LuaPlusDebugger.Connect.LuaPlusWatch"))
		{
			m_pWindow->put_Visible(VARIANT_TRUE);

			*pvbHandled = VARIANT_TRUE;
			return S_OK;
		}
	}
	return S_OK;
}
