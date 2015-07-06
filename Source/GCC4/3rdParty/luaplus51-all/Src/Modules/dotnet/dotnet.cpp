#include "../../LuaPlus/LuaPlus.h"
#include <windows.h>
#include <assert.h>
#include "mscoree.h"

/*
using namespace System;
using namespace System::IO;
using namespace System::Collections;
using namespace System::Reflection;
using System::Runtime::InteropServices::GCHandle;

using namespace LuaPlus;

struct lua_State {};


static int GCHandle_gc(LuaState* state)
{
	LuaStack args(state);
	if ( !args[1].IsUserData() )
		assert( 0 );		// How did we get here?
	GCHandle::op_Explicit((int)args[1].GetUserData()).Free();

	System::GC::Collect();
	System::GC::WaitForPendingFinalizers();

	return 0;
}
*/

#import "mscorlib.tlb" raw_interfaces_only, rename("ReportEvent","ReportCLREvent")

using namespace mscorlib;

static struct luaL_reg luanet_funcs[] = { { NULL, NULL} };

int collect_runtime(lua_State *L)
{
	lua_pushstring(L,"interpreter");
	lua_rawget(L,-2);
	_ObjectHandle *objectHandle = (_ObjectHandle *)lua_touserdata(L,-1);
	objectHandle->Release();
	lua_pushstring(L,"runtime");
	lua_rawget(L,-2);
	ICorRuntimeHost *runtimeHost = (ICorRuntimeHost*)lua_touserdata(L,-1);
	runtimeHost->Stop();
	runtimeHost->Release();
	return 0;
}

void p(lua_State* L,char *str) {
	lua_getglobal(L,"print");
	lua_pushstring(L,str);
	lua_call(L,1,0);
}

extern "C" LUAMODULE_API int luaopen_luanet(lua_State* L)
{
	lua_pushstring(L, "LUAINTERFACE LOADED");
        lua_gettable(L, LUA_REGISTRYINDEX);
        if(lua_toboolean(L,-1)) {
        	return 0;
	}
	ICorRuntimeHost *runtimeHost = NULL;
	HRESULT hr = CorBindToRuntimeEx(
					NULL,   //Retrieve latest version by default
					NULL, //Request a WorkStation build of the CLR
					STARTUP_LOADER_OPTIMIZATION_SINGLE_DOMAIN |
					STARTUP_CONCURRENT_GC,
					CLSID_CorRuntimeHost,
					IID_ICorRuntimeHost,
					(void**)&runtimeHost
                 );
	if(!SUCCEEDED(hr))
	{
		lua_pushstring(L,"LuaInterface: cannot load CLR");
		lua_error(L);
		return 0;
	}

	IUnknown *unknown = NULL;
	_AppDomain *appDomain = NULL;
	runtimeHost->Start();
	hr = runtimeHost->GetDefaultDomain(&unknown);
    	if(!SUCCEEDED(hr))
	{
		runtimeHost->Release();
		lua_pushstring(L,"LuaInterface: cannot load app domain");
		lua_error(L);
		return 0;
	}
	hr = unknown->QueryInterface(__uuidof(_AppDomain),(void**)&appDomain);
    if(!SUCCEEDED(hr))
	{
		unknown->Release();
		runtimeHost->Release();
		lua_pushstring(L,"LuaInterface: cannot query app domain");
		lua_error(L);
		return 0;
	}
	_ObjectHandle *objectHandle = NULL;
    SAFEARRAY *args = NULL;
	args = SafeArrayCreateVector(VT_VARIANT,0,1);
	if(args == NULL)
	{
		appDomain->Release();
		unknown->Release();
		runtimeHost->Release();
		lua_pushstring(L,"LuaInterface: cannot instantiate parameter array");
		lua_error(L);
		return 0;
	}
	VARIANT *argsVal = NULL;
	hr = SafeArrayAccessData(args,(void**)&argsVal);
	if(!SUCCEEDED(hr)) {
		SafeArrayDestroy(args);
		appDomain->Release();
		unknown->Release();
		runtimeHost->Release();
		lua_pushstring(L,"LuaInterface: cannot access parameter array");
		lua_error(L);
		return 0;
	}
	VariantInit(&argsVal[0]);
	argsVal[0].vt = VT_I8;
	argsVal[0].lVal = (unsigned __int64)L;
//	BSTR assembly = SysAllocString(L"LuaInterface.dll");

	wchar_t path[ _MAX_PATH ];
#ifdef _DEBUG
	const wchar_t* dllName = L"luanet.debug.dll";
#else !_DEBUG
	const wchar_t* dllName = L"luanet.dll";
#endif _DEBUG
	GetModuleFileNameW(GetModuleHandleW(dllName), path, _MAX_PATH);
	wchar_t* slashPtr = wcsrchr( path, '\\' );
	slashPtr++;
	*slashPtr = 0;
#ifdef _DEBUG
	wcscat(path, L"Debug\\");
#endif _DEBUG
	wcscat(path, L"dotnetinterface.dll");
	BSTR assembly = SysAllocString(path);

	BSTR type = SysAllocString(L"LuaInterface.Lua");
	luaL_register(L, "luanet", luanet_funcs);	// steffenj: Lua compat-5.1 "luaL_module" replaced by proper register function
	hr = appDomain->CreateInstanceFrom_3(
			assembly,type,
			VARIANT_TRUE,BindingFlags_Default,NULL,args,NULL,
			NULL,NULL,&objectHandle);
	SysFreeString(assembly);
	SysFreeString(type);
	SafeArrayUnaccessData(args);
	SafeArrayDestroy(args);
	if(!SUCCEEDED(hr))
	{
		appDomain->Release();
		unknown->Release();
		runtimeHost->Release();
		lua_pushstring(L,"LuaInterface: cannot instantiate interpreter");
		lua_error(L);
		return 0;
	}
	lua_newtable(L);
	lua_pushstring(L,"interpreter");
	lua_pushlightuserdata(L,(void*)objectHandle);
	lua_rawset(L,-3);
	lua_pushstring(L,"runtime");
	lua_pushlightuserdata(L,(void*)runtimeHost);
	lua_rawset(L,-3);
	lua_newtable(L);
	lua_pushstring(L,"__gc");
	lua_pushcfunction(L,collect_runtime);
	lua_rawset(L,-3);
	lua_setmetatable(L,-2);
	lua_setglobal(L,"runtime_data");
	appDomain->Release();
	unknown->Release();
	return 0;
/*
	LuaState* state = LuaState::CastState(L);
	LuaObject obj = state->GetGlobals()["dotnet"];
	if (obj.IsTable())
		return 0;

	obj = state->GetGlobals().CreateTable( "dotnet" );

	char path[ _MAX_PATH ];
#ifdef _DEBUG
	const char* dllName = "ldotnet.dlld";
#else !_DEBUG
	const char* dllName = "ldotnet.dll";
#endif _DEBUG
	GetModuleFileName(GetModuleHandle(dllName), path, _MAX_PATH);
	char* slashPtr = strrchr( path, '\\' );
	slashPtr++;
	*slashPtr = 0;
#ifdef _DEBUG
	strcat(path, "Debug\\");
#endif _DEBUG
	strcat(path, "dotnetinterface.dll");

	Assembly* assembly = Assembly::LoadFrom(path);
	Type* type = assembly->GetType("LuaInterface.Lua");
	Object* parms[] = new Object*[1];
	parms[0] = __box((IntPtr)state->GetCState());
	Object* lua = Activator::CreateInstance(type, parms);

	LuaObject metaTableObj;
	metaTableObj.AssignNewTable(state);
	metaTableObj.Register("__gc", GCHandle_gc);

	LuaObject dotNetObj = state->NewUserDataBox((void*)(GCHandle::op_Explicit(GCHandle::Alloc(lua))).ToInt32());
	dotNetObj.SetMetaTable(metaTableObj);

	obj.SetObject("__dotNetUserData", dotNetObj);

	return 0;
*/
}


