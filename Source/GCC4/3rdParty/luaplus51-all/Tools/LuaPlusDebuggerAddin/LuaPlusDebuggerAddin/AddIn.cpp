//Copyright (c) Microsoft Corporation.  All rights reserved.

// AddIn.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"
#include "AddIn.h"

CAddInModule _AtlModule;


// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	_AtlModule.SetResourceInstance(hInstance);
	return _AtlModule.DllMain(dwReason, lpReserved); 
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


void CreateRegistrationKey(const CString& version, const CString& modulePath, const CString& moduleShortName)
{
	CString path = "Software\\Microsoft\\VisualStudio\\" + version;

	CRegKey devKey;
	if (devKey.Open(HKEY_LOCAL_MACHINE, path) == ERROR_SUCCESS)
	{
		// Auto create the addins key if it isn't already there.
		if (devKey.Create(HKEY_LOCAL_MACHINE, path + "\\AddIns") == ERROR_SUCCESS)
		{
			// Create the WorkspaceWhiz.DSAddin.1 key.
			if (devKey.Create(HKEY_LOCAL_MACHINE, path + "\\AddIns\\LuaPlusDebugger.Connect") == ERROR_SUCCESS)
			{
				// Remove all old entries.
				devKey.SetStringValue("SatelliteDLLPath", modulePath);
				devKey.SetStringValue("SatelliteDLLName", moduleShortName);
				devKey.SetDWORDValue("LoadBehavior", 3);
				devKey.SetStringValue("FriendlyName", "LuaPlus Debugger Window");
				devKey.SetStringValue("Description", "The LuaPlus Debugger Window add-in provides support for viewing Lua tables while debugging.");
				devKey.SetDWORDValue("CommandPreload", 1);
			}
		}
	}

	if (devKey.Open(HKEY_CURRENT_USER, path + "\\PreloadAddinState") == ERROR_SUCCESS)
	{
		devKey.SetDWORDValue("LuaPlusDebugger.Connect", 1);
	}
}


void DestroyRegistrationKey(const CString& version)
{
	CString path = "Software\\Microsoft\\VisualStudio\\" + version;

	CRegKey key;
	if (key.Open(HKEY_LOCAL_MACHINE, path + "\\AddIns") == ERROR_SUCCESS)
	{
		// Remove all old entries.
		key.RecurseDeleteKey("LuaPlusDebugger.Connect");
	}
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _AtlModule.DllRegisterServer();

	// Get the module name.
	TCHAR moduleName[_MAX_PATH];
	moduleName[0] = 0;
	::GetModuleFileName(_AtlModule.GetResourceInstance(), (TCHAR*)&moduleName, _MAX_PATH);

	// Get the module path.
	TCHAR modulePath[_MAX_PATH];
	_tcscpy(modulePath, moduleName);
	TCHAR* ptr = _tcsrchr(modulePath, '\\');
	ptr++;
	*ptr++ = 0;

	// Get the short module name.
	TCHAR moduleShortName[_MAX_PATH];
	ptr = _tcsrchr(moduleName, '\\');
	_tcscpy(moduleShortName, ptr + 1);

	// Register the add-in?
	CreateRegistrationKey("7.0", modulePath, moduleShortName);
	CreateRegistrationKey("7.1", modulePath, moduleShortName);
	CreateRegistrationKey("8.0", modulePath, moduleShortName);

	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _AtlModule.DllUnregisterServer();

	// Remove entries.
	DestroyRegistrationKey("7.0");
	DestroyRegistrationKey("7.1");
	DestroyRegistrationKey("8.0");

	return hr;
}
