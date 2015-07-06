// LuaPlusDebuggerControls.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{8BA18739-40D2-40A3-A1C6-13705A878F63}", 
		 name = "LuaPlusDebuggerControls", 
		 helpstring = "LuaPlusDebuggerControls 1.0 Type Library",
		 resource_name = "IDR_LUAPLUSDEBUGGERCONTROLS") ];

/*
extern "C" const __declspec(selectany) GUID __LIBID_ = {0x8ba18739,0x40d2,0x40a3,{0xa1,0xc6,0x13,0x70,0x5a,0x87,0x8f,0x63}};
struct __declspec(uuid("8ba18739-40d2-40a3-a1c6-13705a878f63")) LuaPlusDebuggerControls;

CAppModule _Module;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	return _Module.DllMain(hInstance, dwReason, lpReserved, NULL, NULL); 
}


// Used to determine whether the DLL can be unloaded by OLE
STDAPI DllCanUnloadNow(void)
{
	return _Module.DllCanUnloadNow();
}


// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.DllGetClassObject(rclsid, riid, ppv);
}


// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	HRESULT hr = _Module.DllRegisterServer();
	return hr;
}


// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	HRESULT hr = _Module.DllUnregisterServer();
	return hr;
}
*/