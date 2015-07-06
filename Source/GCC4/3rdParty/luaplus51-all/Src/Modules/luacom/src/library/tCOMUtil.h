// tCOMUtil.h: interface for the tCOMUtil class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __TCOMUTIL_H
#define __TCOMUTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <oleauto.h>

class tCOMUtil  
{
public:
  static HRESULT GUID2String(GUID& Guid, char** ppGuid);
	static const char* getPrintableTypeKind(const TYPEKIND tkind);
  static const char* getPrintableInvokeKind(const INVOKEKIND invkind);
  static const char* getPrintableTypeDesc(const TYPEDESC& tdesc);
	static void DumpTypeInfo(ITypeInfo* typeinfo);
    static bool GetRegKeyValue(const char* key, char** pValue);
	static bool SetRegKeyValue(const char* key, const char* subkey, const char* value);
    static bool DelRegKey(const char *key, const char *subkey);
	static CLSID GetCLSID(ITypeInfo* coclassinfo);
	static HRESULT ProgID2CLSID(CLSID* pClsid, const char* ProgID);
	static ITypeLib* LoadTypeLibFromCLSID(
    CLSID clsid,
    unsigned short major_version=0);

  static ITypeLib* LoadTypeLibFromProgID(
    const char* ProgID,
    unsigned short major_version=0);

  static ITypeLib* LoadTypeLibByName(const char *pcFilename);

  static CLSID FindCLSID(ITypeInfo* interface_typeinfo);

  static ITypeInfo* GetCoClassTypeInfo(ITypeLib* typelib, CLSID clsid);
  static ITypeInfo* GetCoClassTypeInfo(CLSID clsid);
  static ITypeInfo* GetCoClassTypeInfo(IUnknown* punk);
  static ITypeInfo* GetCoClassTypeInfo(IDispatch* pdisp, CLSID clsid);
  static ITypeInfo* GetCoClassTypeInfo(ITypeLib *typelib,
                                       const char *coclassname);
  static ITypeInfo* GetDefaultInterfaceTypeInfo(ITypeInfo* pCoClassinfo,
                                                bool source);
  static ITypeInfo* GetInterfaceTypeInfo(ITypeLib* typelib,
                                         const char *interface_name);
  static ITypeInfo* GetDispatchTypeInfo(IDispatch* pdisp);

	tCOMUtil();
	virtual ~tCOMUtil();

protected:
	static bool GetDefaultTypeLibVersion(
    const char* libid,
    int* version_major,
    int* version_minor);
};

//#define COM_PRINTREF(x) printf("%p: %d\n", (x), (x)->AddRef()-1); (x)->Release();
#define COM_PRINTREF(x)

#define COM_RELEASE(x) {if(x){(x)->Release(); (x) = NULL;}}



#endif // __TCOMUTIL_H
