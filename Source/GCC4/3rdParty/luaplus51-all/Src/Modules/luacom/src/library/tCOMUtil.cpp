// tCOMUtil.cpp: implementation of the tCOMUtil class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <ocidl.h>
#include <shlwapi.h>
#include <wchar.h>

#include "tCOMUtil.h"
#include "tLuaCOMException.h"
#include "tUtil.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tCOMUtil::tCOMUtil()
{

}

tCOMUtil::~tCOMUtil()
{

}

ITypeInfo *tCOMUtil::GetCoClassTypeInfo(CLSID clsid)
{
  ITypeLib* typelib = tCOMUtil::LoadTypeLibFromCLSID(clsid);

  if(typelib == NULL)
    return NULL;

  return tCOMUtil::GetCoClassTypeInfo(typelib, clsid);
}

ITypeInfo *tCOMUtil::GetCoClassTypeInfo(IUnknown* punk)
{
  HRESULT hr = S_OK;
  IProvideClassInfo *pIProvideClassInfo = NULL;
  ITypeInfo* coclassinfo = NULL;

  hr = punk->QueryInterface(IID_IProvideClassInfo,
    (void **) &pIProvideClassInfo);

  if (FAILED(hr))
    return NULL;

  hr = pIProvideClassInfo->GetClassInfo(&coclassinfo);
  pIProvideClassInfo->Release();

  if(SUCCEEDED(hr))
    return coclassinfo;
  else
    return NULL;
}

ITypeInfo *tCOMUtil::GetCoClassTypeInfo(IDispatch* pdisp, CLSID clsid)
{
  CHECKPARAM(pdisp);

  ITypeInfo* typeinfo = NULL;
  ITypeLib* typelib = NULL;
  HRESULT hr = S_OK;

  {
    unsigned int typeinfocount = 0;

    hr = pdisp->GetTypeInfoCount(&typeinfocount);

    if(FAILED(hr) || typeinfocount == 0)
      return NULL;
  }

  hr = pdisp->GetTypeInfo(0, 0, &typeinfo);

  if(FAILED(hr))
    return NULL;

  {
    unsigned int dumb_index = (unsigned int)-1;
    hr = typeinfo->GetContainingTypeLib(&typelib, &dumb_index);
    COM_RELEASE(typeinfo);
  }

  if(FAILED(hr))
    return NULL;

  ITypeInfo* coclasstypeinfo = tCOMUtil::GetCoClassTypeInfo(typelib, clsid);
  typelib->Release();

  return coclasstypeinfo;
}

ITypeInfo *tCOMUtil::GetCoClassTypeInfo(ITypeLib *typelib,
                                        const char *coclassname)
{
  wchar_t* wcCoClass = (wchar_t*) 
    malloc( (strlen(coclassname) + 1) * sizeof(wchar_t));

  mbstowcs(wcCoClass, coclassname, strlen(coclassname)+1);

  const short max_typeinfos = 1000;

  MEMBERID dumb[max_typeinfos];
  ITypeInfo *typeinfos[max_typeinfos];
  unsigned short number = max_typeinfos;
  
  HRESULT hr = typelib->FindName(wcCoClass, 0, 
    typeinfos, dumb, &number);

  free(wcCoClass);
  wcCoClass = NULL;

  // a interface especificada nao existe nesta TypeLib
  if(FAILED(hr) || number == 0)
    return NULL;

  // tries to find the coclass

  unsigned int i = 0;
  TYPEATTR *typeattr = NULL;
  ITypeInfo *found = NULL;

  for(i = 0; i < number; i++)
  {
    hr = typeinfos[i]->GetTypeAttr(&typeattr);
    
    TYPEKIND typekind = typeattr->typekind;
    typeinfos[i]->ReleaseTypeAttr(typeattr);

    if(!found && typekind == TKIND_COCLASS)
      found = typeinfos[i];
    else
      typeinfos[i]->Release();
  }

  return found;
}



ITypeInfo *tCOMUtil::GetDefaultInterfaceTypeInfo(ITypeInfo* pCoClassinfo,
                                                 bool source)
           
{
  ITypeInfo* typeinfo = NULL;

  // if the component does not have a dispinterface typeinfo 
  // for events, we stay with an interface typeinfo
  ITypeInfo* interface_typeinfo = NULL;

  TYPEATTR* pTA = NULL;
  HRESULT hr = S_OK;

  if (SUCCEEDED(pCoClassinfo->GetTypeAttr(&pTA)))
  {
    UINT i = 0;
    int iFlags = 0;

    for (i=0; i < pTA->cImplTypes; i++)
    {
      //Get the implementation type for this interface
      hr = pCoClassinfo->GetImplTypeFlags(i, &iFlags);

      if (FAILED(hr))
        continue;

      if (iFlags & IMPLTYPEFLAG_FDEFAULT || pTA->cImplTypes == 1)
      {
        if(source == false && !(iFlags & IMPLTYPEFLAG_FSOURCE)
        || source == true && (iFlags & IMPLTYPEFLAG_FSOURCE))
        {
          HREFTYPE    hRefType=0;

          /*
           * This is the interface we want.  Get a handle to
           * the type description from which we can then get
           * the ITypeInfo.
           */
          pCoClassinfo->GetRefTypeOfImplType(i, &hRefType);
          hr = pCoClassinfo->GetRefTypeInfo(hRefType, &typeinfo);

          // gets typeattr info
          TYPEATTR *ptypeattr = NULL;
          GUID guid;
          TYPEKIND typekind;

          hr = typeinfo->GetTypeAttr(&ptypeattr);

          if(FAILED(hr))
          {
            COM_RELEASE(typeinfo);
            break;
          }

          guid = ptypeattr->guid;
          typekind = ptypeattr->typekind;

          typeinfo->ReleaseTypeAttr(ptypeattr);

          if(typekind == TKIND_DISPATCH)
          {
            // found!
            COM_RELEASE(interface_typeinfo);
            break;
          }
          else // hold this pointer. If we do not find
            // anything better, we stay with this typeinfo
          {
            COM_RELEASE(interface_typeinfo);
            interface_typeinfo = typeinfo;
            typeinfo = NULL;
          }
        }
      }
    }

    pCoClassinfo->ReleaseTypeAttr(pTA);
  }

  if(!typeinfo)
    return interface_typeinfo;
  else
    return typeinfo;
}



ITypeInfo *tCOMUtil::GetDispatchTypeInfo(IDispatch* pdisp)
{
  ITypeInfo* typeinfo = NULL;

  HRESULT hr = pdisp->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeinfo);

  if(FAILED(hr))
    return NULL;

  TYPEATTR *ptypeattr = NULL;

  typeinfo->GetTypeAttr(&ptypeattr);

  TYPEKIND typekind = ptypeattr->typekind;

  if(typekind == TKIND_DISPATCH)
  {
    typeinfo->ReleaseTypeAttr(ptypeattr);
    return typeinfo;
  }

  // tries to find another description of the same
  // interface in the typelib with TKIND_DISPATCH

  ITypeLib *ptypelib = NULL;
  unsigned int dumb = 0;
  IID iid = ptypeattr->guid;

  hr = typeinfo->GetContainingTypeLib(&ptypelib, &dumb);

  typeinfo->ReleaseTypeAttr(ptypeattr);

  // if there's no containing type lib, we have to
  // trust this one is the right type info
  if(FAILED(hr))
    return typeinfo;

  // obtem a typeinfo do iid fornecido
  // caso haja uma implementacao dispinterface,
  // esta' e' que sera' retornada (segundo
  // documentacao do ActiveX

  ITypeInfo* typeinfo_guid = NULL;

  hr = ptypelib->GetTypeInfoOfGuid(iid, &typeinfo_guid);

  COM_RELEASE(ptypelib);

  if(FAILED(hr))
    return typeinfo;

  // verifica se e' dispinterface
  TYPEATTR *ptypeattr_iface = NULL;

  hr = typeinfo_guid->GetTypeAttr(&ptypeattr_iface);
  TYPEKIND typekind_iface = ptypeattr_iface->typekind;
  typeinfo_guid->ReleaseTypeAttr(ptypeattr_iface);

  if(typekind_iface == TKIND_DISPATCH)
  {
    // releases original type information
    COM_RELEASE(typeinfo);

    return typeinfo_guid;
  }
  else
  {
    COM_RELEASE(typeinfo_guid);
    
    // returns original type info
    return typeinfo;
  }
}


ITypeInfo *tCOMUtil::GetInterfaceTypeInfo(ITypeLib* typelib,
                                          const char *interface_name)
{
  wchar_t* wcInterface = (wchar_t*) 
    malloc( (strlen(interface_name) + 1) * sizeof(wchar_t));
  mbstowcs(wcInterface, interface_name, strlen(interface_name)+1);

  const int max_typeinfos = 30;

  MEMBERID dumb[max_typeinfos];
  ITypeInfo *typeinfos[max_typeinfos];
  unsigned int number = 30;
  
  HRESULT hr = typelib->FindName(wcInterface, 0, 
    typeinfos, dumb, (unsigned short *) &number);

  free(wcInterface);
  wcInterface = NULL;

  // a interface especificada nao existe nesta TypeLib
  if(FAILED(hr) || number == 0)
    return NULL;

  // Procura por uma implementacao IDispatch
  
  unsigned int i = 0;
  TYPEATTR *typeattr = NULL;
  ITypeInfo *found = NULL;

  for(i = 0; i < number; i++)
  {
    hr = typeinfos[i]->GetTypeAttr(&typeattr);
    
    TYPEKIND typekind = typeattr->typekind;
    typeinfos[i]->ReleaseTypeAttr(typeattr);

    if(!found && typekind == TKIND_DISPATCH)
      found = typeinfos[i];
    else
      typeinfos[i]->Release();
  }

  return found;
}


/*
 * Carrega uma Typelib associada a um ProgID
 */

ITypeLib* tCOMUtil::LoadTypeLibFromProgID(const char* ProgID,
                                          unsigned short major_version)
{
  CLSID clsid = IID_NULL;
  HRESULT hr = tCOMUtil::ProgID2CLSID(&clsid, ProgID);

  if(FAILED(hr))
    return NULL;

  bool version_found = false;
  int version = -1;

  if(major_version <= 0)
  {
    if(sscanf(ProgID, "%*s.%*s.%d", &major_version) == 1)
      version_found = true;
  }

  // tries to get some version information to help finding
  // the right type library
  if(version_found)
    return tCOMUtil::LoadTypeLibFromCLSID(clsid, major_version);
  else
    return tCOMUtil::LoadTypeLibFromCLSID(clsid);

}

/*
 *  LoadTypeLibByName
 *  Carrega uma typelib a partir de um arquivo TLB
 */

ITypeLib *tCOMUtil::LoadTypeLibByName(const char *pcFilename)
{
  HRESULT hr;
  ITypeLib *ptlib = NULL;
  wchar_t *wcFilename = new wchar_t[strlen(pcFilename)+1];

  mbstowcs(wcFilename, pcFilename, strlen(pcFilename)+1);

  hr = LoadTypeLibEx(wcFilename, REGKIND_NONE, &ptlib);

  delete[] wcFilename;
  wcFilename = NULL;

  if(FAILED(hr))
    return NULL;

  return ptlib;
}



ITypeLib* tCOMUtil::LoadTypeLibFromCLSID(CLSID clsid,
                                         unsigned short major_version)
{
  wchar_t* wcClsid = NULL;

  HRESULT hr = StringFromCLSID(clsid, &wcClsid);

  if (FAILED(hr))
    return NULL;

  /* converte CLSID para string normal */
  char* pcClsid = (char*) malloc( (wcslen(wcClsid) + 1) * sizeof(char));
  wcstombs(pcClsid, wcClsid,wcslen(wcClsid)+1);

  CoTaskMemFree(wcClsid);

  DWORD size = 38*3; /*{F37C8063-4AD5-101B-B826-00DD01103DE1}*/
  BYTE *bLibID = (BYTE *) malloc(size);
  BYTE bVersion[100]; // This must hold something like "5.2"
  HKEY iid_key, obj_key, typelib_key, version_key;

  /* extrai do registry type library (GUID e versao) */
  LONG res = 0;
  bool version_info_found = true;

  try
  {
    res = RegOpenKeyEx(HKEY_CLASSES_ROOT,"CLSID", 0, KEY_READ, &iid_key);
    WINCHECK(res == ERROR_SUCCESS);

    res = RegOpenKeyEx(iid_key, pcClsid, 0, KEY_READ, &obj_key);
    RegCloseKey(iid_key);
    free(pcClsid);
    pcClsid = NULL;

    WINCHECK(res == ERROR_SUCCESS);

    res = RegOpenKeyEx(obj_key, "TypeLib",0, KEY_READ, &typelib_key);
    if(res != ERROR_SUCCESS)
    {
      RegCloseKey(obj_key);
      LUACOM_EXCEPTION(WINDOWS_ERROR);
    }

    res = RegOpenKeyEx(obj_key, "version",0, KEY_READ, &version_key);
    RegCloseKey(obj_key);
    if(res != ERROR_SUCCESS)
      version_info_found = false;

    RegQueryValueEx(typelib_key, NULL, NULL, NULL, bLibID, &size);
    RegCloseKey(typelib_key);
        
    RegQueryValueEx(version_key, NULL, NULL, NULL, bVersion, &size);
    RegCloseKey(version_key);
  }
  catch(class tLuaCOMException& e)
  {
    UNUSED(e);

    if(pcClsid)
      free(pcClsid);

    return NULL;
  }

  // converts libID to multibyte string
  wchar_t* wcTypelib= (wchar_t*) 
    malloc( (strlen((char *) bLibID) + 1) * sizeof(wchar_t));
  mbstowcs(wcTypelib, (char *) bLibID, strlen((char *) bLibID)+1);

  // extracts version information

  int version_major = 0, version_minor = 0;

  int found = 0;

  if(version_info_found)
  {
    found = sscanf(
      (const char *) bVersion,
      "%d.%d",
      &version_major, &version_minor);
  }

  if(major_version > 0 &&
      (
        (!version_info_found) ||
        (version_info_found && found == 0)
      )
    )
  {
    version_major = major_version;
    version_minor = 0;
  }
  else if(!version_info_found)
  {
    // tries to load the first type library found in
    // the registry
    bool result = tCOMUtil::GetDefaultTypeLibVersion(
      (const char*) bLibID,
      &version_major,
      &version_minor);

    if(!result)
      return NULL;
  }

  free(bLibID);

  GUID libid = IID_NULL;
  CLSIDFromString(wcTypelib, &libid);
  free(wcTypelib);

  ITypeLib* typelib = NULL;

  hr = LoadRegTypeLib(libid, version_major, version_minor, 0, &typelib);

  if(FAILED(hr))
    return NULL;

  return typelib;
}

ITypeInfo* tCOMUtil::GetCoClassTypeInfo(ITypeLib *typelib, CLSID clsid)
{
  ITypeInfo* coclassinfo = NULL;
  
  HRESULT hr = typelib->GetTypeInfoOfGuid(clsid, &coclassinfo);
  
  if(FAILED(hr))
    return NULL;

  return coclassinfo;
}

HRESULT tCOMUtil::ProgID2CLSID(CLSID *pClsid, const char *ProgID)
{
  CHECKPARAM(pClsid);
  
  // tests whether we already have a CLSID

  wchar_t* wcProgId = NULL;
  HRESULT hr = S_OK;

  if(ProgID[0] == '{')
  {
    wcProgId = (wchar_t*) malloc( (strlen(ProgID) + 1) * sizeof(wchar_t));
    mbstowcs(wcProgId, ProgID, strlen(ProgID)+1);

    hr = CLSIDFromString(wcProgId, pClsid);

    free(wcProgId);
    wcProgId = NULL;

    return hr;
  }

  /* converte ProgID para OLESTR */
  wcProgId = (wchar_t*) malloc( (strlen(ProgID) + 1) * sizeof(wchar_t));
  mbstowcs(wcProgId, ProgID, strlen(ProgID)+1);

  hr = CLSIDFromProgID(wcProgId, pClsid);

  free(wcProgId);
  wcProgId = NULL;

  return hr;
}

CLSID tCOMUtil::GetCLSID(ITypeInfo *coclassinfo)
{
  TYPEATTR* ptypeattr = NULL;

  HRESULT hr = coclassinfo->GetTypeAttr(&ptypeattr);

  if(FAILED(hr))
    return IID_NULL;

  CLSID clsid = ptypeattr->guid;

  coclassinfo->ReleaseTypeAttr(ptypeattr);

  return clsid;
}

bool tCOMUtil::GetDefaultTypeLibVersion(const char* libid,
                                        int *version_major,
                                        int *version_minor)
{
  LONG res = 0;
  HKEY typelib_key, this_typelib_key;

  res = RegOpenKeyEx(HKEY_CLASSES_ROOT,"TypeLib", 0, KEY_READ, &typelib_key);
  RegCloseKey(HKEY_CLASSES_ROOT);

  if(res != ERROR_SUCCESS)
    return false;

  res = RegOpenKeyEx(typelib_key, libid, 0, KEY_READ, &this_typelib_key);
  RegCloseKey(typelib_key);

  if(res != ERROR_SUCCESS)
    return false;

  const int bufsize = 1000;
  char version_info[bufsize];
  DWORD size = bufsize;

  res = RegEnumKeyEx(this_typelib_key, 0, version_info, &size,
    NULL, NULL, NULL, NULL); 
  RegCloseKey(this_typelib_key);

  if(res != ERROR_SUCCESS)
    return false;

  sscanf(version_info, "%d.%d", version_major, version_minor);

  return true;
}

bool tCOMUtil::GetRegKeyValue(const char* key, char** pValue) {
	LONG ec = 0;
	// unused: HKEY hKey;
	LONG cbValue;

	ec = RegQueryValue(HKEY_CLASSES_ROOT,key,NULL,&cbValue);

	if(ERROR_SUCCESS == ec) {
		*pValue = new char[cbValue+1];
		ec = RegQueryValue(HKEY_CLASSES_ROOT,key,*pValue,&cbValue);
  		if(ERROR_SUCCESS == ec)
			return true;
	}

	return false;
}

bool tCOMUtil::SetRegKeyValue(const char *key,
                              const char *subkey,
                              const char *value)
{

  bool ok = false;
  LONG ec = 0;
  HKEY hKey;

  const int bufsize = 10000;
  char Key[bufsize];

  strncpy(Key, key, bufsize - 1);

  if (NULL != subkey)
  {
    strcat(Key, "\\");
    strcat(Key, subkey);
  }

  ec = RegCreateKeyEx(
         HKEY_CLASSES_ROOT,
         Key,
         0,
         NULL,
         REG_OPTION_NON_VOLATILE,
         KEY_ALL_ACCESS,
         NULL,
         &hKey,
         NULL);

  if (ERROR_SUCCESS == ec)
  {
    if (NULL != value)
    {
      ec = RegSetValueEx(
             hKey,
             NULL,
             0,
             REG_SZ,
             (BYTE *)value,
             (DWORD)(strlen(value)+1));
    }
    if (ERROR_SUCCESS == ec)
      ok = TRUE;

    RegCloseKey(hKey);
  }

  return ok;
}

bool tCOMUtil::DelRegKey(const char *key,
                         const char *subkey)
{

  bool ok = false;
  LONG ec = 0;

  const int bufsize = 10000;
  char Key[bufsize];

  strncpy(Key, key, bufsize - 1);

  if (NULL != subkey)
  {
    strcat(Key, "\\");
    strcat(Key, subkey);
  }

  ec = SHDeleteKey(HKEY_CLASSES_ROOT, Key);

  if (ERROR_SUCCESS == ec)
  {
	  return true;
  }

  return false;
}

void tCOMUtil::DumpTypeInfo(ITypeInfo *typeinfo)
{
  HRESULT hr = S_OK;
  TYPEATTR* pta = NULL;

  CHECKPARAM(typeinfo);

  hr = typeinfo->GetTypeAttr(&pta);
  CHK_COM_CODE(hr);

  // prints IID
  LPOLESTR lpsz = NULL;

#ifdef __WINE__
  hr = 0;
  MessageBox(NULL, "FIX - not implemented - StringFromIID", "LuaCOM", MB_ICONEXCLAMATION);
  #warning FIX - not implemented - StringFromIID
#else
  hr = StringFromIID(pta->guid, &lpsz);
#endif

  if(FAILED(hr))
  {
    hr = StringFromCLSID(pta->guid, &lpsz);
  }

  if(SUCCEEDED(hr))
  {
    wprintf(L"\nInterface:  %s\n\n", lpsz);

    CoTaskMemFree(lpsz);
  }

  int i = 0;
  FUNCDESC *pfd = NULL;

  for(i = 0; i < pta->cFuncs; i++)
  {
    typeinfo->GetFuncDesc(i, &pfd);

    BSTR names[1];
    unsigned int dumb;

    typeinfo->GetNames(pfd->memid, names, 1, &dumb);

    printf("%.3d: %-30s\tid=0x%d\t%d param(s)\n", i,
      tUtil::bstr2string(names[0]), pfd->memid, pfd->cParams);

    typeinfo->ReleaseFuncDesc(pfd);
    SysFreeString(names[0]);
  }

  typeinfo->ReleaseTypeAttr(pta);
}


const char* tCOMUtil::getPrintableInvokeKind(const INVOKEKIND invkind)
{
  switch(invkind)
  {
  case INVOKE_PROPERTYGET:
    return "propget";

  case INVOKE_PROPERTYPUT:
    return "propput";

  case INVOKE_PROPERTYPUTREF:
    return "propputref";

  case INVOKE_FUNC:
    return "func";
  }

  return NULL;
}

const char* tCOMUtil::getPrintableTypeDesc(const TYPEDESC& tdesc)
{
  static char buffer[200];
  buffer[0] = '\0';

  switch(tdesc.vt & ~(VT_ARRAY | VT_BYREF))
  {
  case VT_VOID:
    strcat(buffer, "void");
    break;

  case VT_I2:
    strcat(buffer, "short");
    break;

  case VT_I4:
    strcat(buffer, "long");
    break;

  case VT_R4:
    strcat(buffer, "float");
    break;

  case VT_R8:
    strcat(buffer, "double");
    break;

  case VT_CY:
    strcat(buffer, "CY");
    break;

  case VT_DATE:
    strcat(buffer, "DATE");
    break;

  case VT_BSTR:
    strcat(buffer, "BSTR");
    break;

  case VT_DISPATCH:
    strcat(buffer, "IDispatch*");
    break;

  case VT_BOOL:
    strcat(buffer, "VARIANT_BOOL");
    break;

  case VT_VARIANT:
    strcat(buffer, "VARIANT");
    break;

  case VT_UNKNOWN:
    strcat(buffer, "IUnknown*");
    break;

  case VT_DECIMAL:
    strcat(buffer, "Decimal");
    break;

  case VT_UI1:
    strcat(buffer, "unsigned char");
    break;

  case VT_INT:
    strcat(buffer, "int");
    break;

  case VT_HRESULT:
    strcat(buffer, "void");
	break;
  }

  if(tdesc.vt & VT_BYREF)
    strcat(buffer, "*");

  if(tdesc.vt & VT_ARRAY)
    strcat(buffer, "[]");

  return buffer;
}

const char* tCOMUtil::getPrintableTypeKind(const TYPEKIND tkind)
{
  switch(tkind)
  {
  case TKIND_COCLASS:
    return "coclass";
    break;

  case TKIND_ENUM:
    return "enumeration";
    break;

  case TKIND_RECORD:
    return "record";
    break;

  case TKIND_MODULE:
    return "module";
    break;

  case TKIND_INTERFACE:
    return "interface";
    break;

  case TKIND_DISPATCH:
    return "dispinterface";
    break;

  case TKIND_ALIAS:
    return "alias";
    break;

  case TKIND_UNION:
    return "union";
    break;

  default:
    return "";
    break;
  }
}


HRESULT tCOMUtil::GUID2String(GUID& Guid, char** ppGuid)
{
  wchar_t* wcGuid = NULL;

  HRESULT hr = StringFromCLSID(Guid, &wcGuid);

  if (FAILED(hr))
    return hr;

  *ppGuid = new char[wcslen(wcGuid) + 1];
  wcstombs(*ppGuid, wcGuid, wcslen(wcGuid)+1);

  CoTaskMemFree(wcGuid);

  return S_OK;
}

CLSID tCOMUtil::FindCLSID(ITypeInfo* interface_typeinfo)
{
  ITypeLib* ptypelib    = NULL;
  ITypeInfo* ptypeinfo  = NULL;
  long count            = 0;
  IID iid               = IID_NULL;
  TYPEATTR* ptypeattr   = NULL;
  TYPEKIND tkind;
  bool found            = false;
  CLSID clsid           = IID_NULL;

  // gets IID
  interface_typeinfo->GetTypeAttr(&ptypeattr);

  iid = ptypeattr->guid;
  interface_typeinfo->ReleaseTypeAttr(ptypeattr);
  ptypeattr = NULL;

  // Gets type library
  interface_typeinfo->GetContainingTypeLib(&ptypelib, NULL);

  // iterates looking for IID inside some coclass
  count = ptypelib->GetTypeInfoCount();

  while(count-- && !found)
  {
    ptypelib->GetTypeInfoType(count, &tkind);

    if(tkind != TKIND_COCLASS)
      continue;

    // look inside
    ptypelib->GetTypeInfo(count, &ptypeinfo);

    // gets counts and clsid
    ptypeinfo->GetTypeAttr(&ptypeattr);
    long ifaces_count   = ptypeattr->cImplTypes;
    clsid = ptypeattr->guid;

    ptypeinfo->ReleaseTypeAttr(ptypeattr);
    ptypeattr = NULL;

    HREFTYPE RefType;
    ITypeInfo* piface_typeinfo = NULL;

    while(ifaces_count-- && !found)
    {
      ptypeinfo->GetRefTypeOfImplType(ifaces_count, &RefType);
      ptypeinfo->GetRefTypeInfo(RefType, &piface_typeinfo);
      piface_typeinfo->GetTypeAttr(&ptypeattr);

      if(IsEqualIID(ptypeattr->guid, iid))
      {
        found = true;
      }

      piface_typeinfo->ReleaseTypeAttr(ptypeattr);
      ptypeattr = NULL;

      COM_RELEASE(piface_typeinfo);
    }

    COM_RELEASE(ptypeinfo);
  }

  COM_RELEASE(ptypelib);

  return clsid;
}
