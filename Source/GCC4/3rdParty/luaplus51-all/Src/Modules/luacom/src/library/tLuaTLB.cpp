// tLuaTLB.cpp: implementation of the tLuaTLB class.
//
//////////////////////////////////////////////////////////////////////

#include "tLuaTLB.h"

#include "tCOMUtil.h"
#include "tLuaCOMException.h"
#include "tUtil.h"
#include "luacom_internal.h"
#include "LuaAux.h"

#include "tLuaCOM.h"
#include "tLuaCOMTypeHandler.h"

extern "C"
{
#include "LuaCompat.h"
}

// dynamic_cast if RTTI enabled.
// static_cast  if RTTI disabled.
#ifdef _CPPRTTI   // MSVC RTTI
#define DYNAMIC_CAST dynamic_cast
#else
#define DYNAMIC_CAST static_cast
#endif

const char tLuaTLB::type_name[] = "ITypeLib";
const char tLuaTLB::pointer_type_name[] = "ITypeLib_pointer";

const char tLuaTypeInfo::type_name[] = "ITypeInfo";
const char tLuaTypeInfo::pointer_type_name[] = "ITypeInfo_pointer";


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tLuaTLB::tLuaTLB(lua_State* L, ITypeLib *p_typelib)
{
  method_table.Add("ShowHelp", ShowHelp, FUNC);
  method_table.Add("GetDocumentation", GetDocumentation, FUNC);
  method_table.Add("GetTypeInfo", GetTypeInfo, FUNC);
  method_table.Add("GetTypeInfoCount", GetTypeInfoCount, FUNC);
  method_table.Add("ExportConstants", ExportConstants, FUNC);
  method_table.Add("ExportEnumerations", ExportEnumerations, FUNC);

  typelib = p_typelib;
  typelib->AddRef();
}

tLuaTLB::~tLuaTLB()
{
  COM_RELEASE(typelib);
}

int tLuaTLB::index(lua_State* L)
{
  return 0;
}

int tLuaTLB::pushNew(lua_State *L, ITypeLib *p_typelib)
{
  CHECKPRECOND(p_typelib);

  // tests whether we have a cached instance
  if(pushCachedObject(L, p_typelib))
    return 1;

  LUASTACK_SET(L);

  tLuaTLB* lua_tlb = new tLuaTLB(L, p_typelib);

  int retval = generic_PushNew(L, lua_tlb, type_name, pointer_type_name);
  cacheObject(L, (void*) p_typelib);

  LUASTACK_CLEAN(L, 1);

  return retval;
}

int tLuaTLB::GetTypeInfoCount(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTLB* lua_tlb = DYNAMIC_CAST<tLuaTLB*>(lua_obj);
  
  CHECKPRECOND(lua_tlb);

  int count = lua_tlb->typelib->GetTypeInfoCount();

  lua_pushnumber(L, count);

  return 1;
}

int tLuaTLB::GetTypeInfo(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTLB* lua_tlb = DYNAMIC_CAST<tLuaTLB*>(lua_obj);
  CHECKPRECOND(lua_tlb);

  UINT typeinfo_pos = (UINT)lua_tointeger(L, 2);

  ITypeInfo* typeinfo = NULL;
  HRESULT hr = lua_tlb->typelib->GetTypeInfo(typeinfo_pos , &typeinfo);

  CHK_COM_CODE(hr);

  tLuaTypeInfo::pushNew(L, typeinfo);
  COM_RELEASE(typeinfo);

  return 1;
}

int tLuaTLB::GetDocumentation(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTLB* lua_tlb = DYNAMIC_CAST<tLuaTLB*>(lua_obj);
  
  CHECKPRECOND(lua_tlb);

  HRESULT hr = S_OK;
  BSTR name;
  BSTR helpstring;
  BSTR helpfile;
  ULONG helpcontext;

  hr = lua_tlb->typelib->GetDocumentation(-1, 
    &name, &helpstring, &helpcontext, &helpfile);

  CHK_COM_CODE(hr);

  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushstring(L, tUtil::bstr2string(name));
  lua_settable(L, -3);

  lua_pushstring(L, "helpstring");
  lua_pushstring(L, tUtil::bstr2string(helpstring));
  lua_settable(L, -3);

  lua_pushstring(L, "helpfile");
  lua_pushstring(L, tUtil::bstr2string(helpfile));
  lua_settable(L, -3);

  lua_pushstring(L, "helpcontext");
  lua_pushnumber(L, helpcontext);
  lua_settable(L, -3);
 
  SysFreeString(name);
  SysFreeString(helpstring);
  SysFreeString(helpfile);

  return 1;
}

int tLuaTLB::ShowHelp(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTLB* lua_tlb = DYNAMIC_CAST<tLuaTLB*>(lua_obj);
  
  CHECKPRECOND(lua_tlb);

  HRESULT hr = S_OK;
  BSTR helpfile;
  ULONG helpcontext;

  hr = lua_tlb->typelib->GetDocumentation(
    -1, NULL, NULL, &helpcontext, &helpfile);
    
  CHK_COM_CODE(hr);
  
  tUtil::ShowHelp(tUtil::bstr2string(helpfile), helpcontext);

  SysFreeString(helpfile);

  return 0;
}

int tLuaTLB::ExportConstants(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTLB* lua_tlb = DYNAMIC_CAST<tLuaTLB*>(lua_obj);
  CHECKPRECOND(lua_tlb);
  CHECKPARAM(lua_type(L, -1) == LUA_TTABLE);

  TYPEKIND tkind;
  ITypeInfo* ptypeinfo  = NULL;
  VARDESC* pvardesc     = NULL;
  TYPEATTR* ptypeattr   = NULL;
  BSTR name;
  HRESULT hr = S_OK;
  tLuaCOMTypeHandler* typehandler = NULL;

  long count = lua_tlb->typelib->GetTypeInfoCount();
  
  while(count--)
  {
    lua_tlb->typelib->GetTypeInfoType(count, &tkind);

    if(tkind == TKIND_ENUM)
    {
      hr = lua_tlb->typelib->GetTypeInfo(count, &ptypeinfo);
      CHK_COM_CODE(hr);

      typehandler = new tLuaCOMTypeHandler(ptypeinfo);

      hr = ptypeinfo->GetTypeAttr(&ptypeattr);
      CHK_COM_CODE(hr);

      long var_count = ptypeattr->cVars;
      
      ptypeinfo->ReleaseTypeAttr(ptypeattr);
      ptypeattr = NULL;

      while(var_count--)
      {
        hr = ptypeinfo->GetVarDesc(var_count, &pvardesc);
        CHK_COM_CODE(hr);

        ptypeinfo->GetDocumentation(pvardesc->memid, &name, NULL, NULL, NULL);

        lua_pushstring(L, tUtil::bstr2string(name));
        SysFreeString(name);

        typehandler->com2lua(L, *pvardesc->lpvarValue);

        // sets in the table
        lua_settable(L, -3);

        ptypeinfo->ReleaseVarDesc(pvardesc);
        pvardesc = NULL;
      }

      SAFEDELETE(typehandler);
      COM_RELEASE(ptypeinfo);
    }
  }

  return 1;
}


int tLuaTLB::ExportEnumerations(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTLB* lua_tlb = DYNAMIC_CAST<tLuaTLB*>(lua_obj);
  CHECKPRECOND(lua_tlb);
  if(lua_gettop(L) == 0)
	  lua_newtable(L);
  else CHECKPARAM(lua_type(L, -1) == LUA_TTABLE);

  TYPEKIND tkind;
  ITypeInfo* ptypeinfo  = NULL;
  VARDESC* pvardesc     = NULL;
  TYPEATTR* ptypeattr   = NULL;
  BSTR name;
  HRESULT hr = S_OK;
  tLuaCOMTypeHandler* typehandler = NULL;

  long count = lua_tlb->typelib->GetTypeInfoCount();
  
  while(count--)
  {
    lua_tlb->typelib->GetTypeInfoType(count, &tkind);

    if(tkind == TKIND_ENUM)
    {

	  hr = lua_tlb->typelib->GetTypeInfo(count, &ptypeinfo);
      CHK_COM_CODE(hr);

	  hr = lua_tlb->typelib->GetDocumentation(count, &name, NULL, NULL, NULL);	
      CHK_COM_CODE(hr);
      lua_pushstring(L, tUtil::bstr2string(name));
      SysFreeString(name);
	  lua_newtable(L);

      typehandler = new tLuaCOMTypeHandler(ptypeinfo);

      hr = ptypeinfo->GetTypeAttr(&ptypeattr);
      CHK_COM_CODE(hr);

      long var_count = ptypeattr->cVars;
      
      ptypeinfo->ReleaseTypeAttr(ptypeattr);
      ptypeattr = NULL;

      while(var_count--)
      {
        hr = ptypeinfo->GetVarDesc(var_count, &pvardesc);
        CHK_COM_CODE(hr);

        ptypeinfo->GetDocumentation(pvardesc->memid, &name, NULL, NULL, NULL);

        lua_pushstring(L, tUtil::bstr2string(name));
        SysFreeString(name);

        typehandler->com2lua(L, *pvardesc->lpvarValue);

        // sets in the table
        lua_settable(L, -3);

        ptypeinfo->ReleaseVarDesc(pvardesc);
        pvardesc = NULL;
      }

	  lua_settable(L, -3);

      SAFEDELETE(typehandler);
      COM_RELEASE(ptypeinfo);
    }
  }

  return 1;
}



////////////////////////
// tLuaTypeInfo class //
////////////////////////


tLuaTypeInfo::tLuaTypeInfo(lua_State* L, ITypeInfo *p_typeinfo)
{
  method_table.Add("GetTypeLib", GetTypeLib, FUNC);
  method_table.Add("GetFuncDesc", GetFuncDesc, FUNC);
  method_table.Add("GetVarDesc", GetVarDesc, FUNC);
  method_table.Add("GetDocumentation", GetDocumentation, FUNC);
  method_table.Add("GetTypeAttr", GetTypeAttr, FUNC);
  method_table.Add("GetImplType", GetImplType, FUNC);
  method_table.Add("GetImplTypeFlags", GetImplTypeFlags, FUNC);
  
  typeinfo = p_typeinfo;
  typeinfo->AddRef();

  typehandler = new tLuaCOMTypeHandler(p_typeinfo);
}

tLuaTypeInfo::~tLuaTypeInfo()
{
  delete typehandler;
  COM_RELEASE(typeinfo);
}


void tLuaTypeInfo::pushNew(lua_State *L, ITypeInfo *p_typeinfo)
{
  CHECKPRECOND(p_typeinfo);

  // tests whether we have a cached instance
  if(pushCachedObject(L, p_typeinfo))
    return;

  tLuaTypeInfo* lua_typeinfo = new tLuaTypeInfo(L, p_typeinfo);
  generic_PushNew(L, lua_typeinfo, type_name, pointer_type_name);
  cacheObject(L, (void*) p_typeinfo);

  return;
}

void tLuaTLB::Init(lua_State* L)
{
  RegisterType(L, type_name, pointer_type_name);
  tLuaTypeInfo::Init(L);
}


void tLuaTypeInfo::Init(lua_State* L)
{
  RegisterType(L, type_name, pointer_type_name);
}

int tLuaTypeInfo::GetFuncDesc(tLuaObject* lua_obj, lua_State* L)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  tLuaTypeInfo* lua_typeinfo = DYNAMIC_CAST<tLuaTypeInfo*>(lua_obj);
  CHECKPRECOND(lua_typeinfo);

  // we just deal with oleautomation-compatible interfaces
  unsigned short flags = 0 ;
  {
    TYPEATTR* ptypeattr = NULL;

    hr = lua_typeinfo->typeinfo->GetTypeAttr(&ptypeattr);
    CHK_COM_CODE(hr);

    flags = ptypeattr->wTypeFlags;

    lua_typeinfo->typeinfo->ReleaseTypeAttr(ptypeattr);

    if(!(flags & TYPEFLAG_FDISPATCHABLE))
      return 0;
  }

  // gets funcdesc

  FUNCDESC* pfuncdesc = NULL;
  {
    UINT i = (UINT)lua_tointeger(L, 2);

    hr = lua_typeinfo->typeinfo->GetFuncDesc(i, &pfuncdesc);
    CHK_COM_CODE(hr);
  }

  if(pfuncdesc->wFuncFlags & FUNCFLAG_FRESTRICTED)
  {
    lua_typeinfo->typeinfo->ReleaseFuncDesc(pfuncdesc);
    return 0;
  }

  // creates table to hold the funcdesc
  lua_newtable(L);

  // stores the various fields
  lua_pushstring(L, "memid");
  lua_pushnumber(L, pfuncdesc->memid);
  lua_settable(L, -3);

  lua_pushstring(L, "invkind");
  lua_pushstring(L, tCOMUtil::getPrintableInvokeKind(pfuncdesc->invkind));
  lua_settable(L, -3);

  lua_pushstring(L, "Params");
  lua_pushnumber(L, pfuncdesc->cParams);
  lua_settable(L, -3);

  lua_pushstring(L, "ParamsOpt");
  lua_pushnumber(L, pfuncdesc->cParamsOpt);
  lua_settable(L, -3);

  // gets all the names
  BSTR* names = new BSTR[pfuncdesc->cParams+1];
  UINT found = 0;

  // gets the name of the functions and the names of the
  // parameters
  lua_typeinfo->typeinfo->GetNames(
    pfuncdesc->memid, names, pfuncdesc->cParams+1, &found);

  lua_pushstring(L, "name");
  lua_pushstring(L, tUtil::bstr2string(names[0]));
  lua_settable(L, -3);

  SysFreeString(names[0]);

  // gets other documentation
  {
    BSTR description, helpfile;
    ULONG helpcontext;

    lua_typeinfo->typeinfo->GetDocumentation(
      pfuncdesc->memid, NULL, &description, &helpcontext, &helpfile);

    lua_pushstring(L, "description");
    lua_pushstring(L, tUtil::bstr2string(description));
    lua_settable(L, -3);

    lua_pushstring(L, "helpfile");
    lua_pushstring(L, tUtil::bstr2string(helpfile));
    lua_settable(L, -3);

    lua_pushstring(L, "helpcontext");
    lua_pushnumber(L, helpcontext);
    lua_settable(L, -3);

    SysFreeString(description);
    SysFreeString(helpfile);
  }

  // now stores information for each parameter (if
  // this is an oleautomation interface

  // table to hold all parameters and stores it
  lua_newtable(L);
  lua_pushstring(L, "parameters");
  lua_pushvalue(L, -2);
  lua_settable(L, -4);

  SHORT i = 0;

  while(i < pfuncdesc->cParams)
  {
    // creates table to hold information for this parameter
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_rawseti(L, -3, i+1);

    // gets the name
    lua_pushstring(L, "name");

    if((i+1) < (SHORT)found)
    {
      lua_pushstring(L, tUtil::bstr2string(names[i+1]));
      SysFreeString(names[i+1]);
    }
    else
    {
      char buffer[50];
      sprintf(buffer, "p%d", i+1);
      lua_pushstring(L, buffer);
    }

    lua_settable(L, -3);

    // gets the type
    TYPEDESC tdesc = pfuncdesc->lprgelemdescParam[i].tdesc;
    bool incompatible_type = false;

    try
    {
      tdesc = 
        tLuaCOMTypeHandler::processTYPEDESC(lua_typeinfo->typeinfo, tdesc);
    }
    catch(class tLuaCOMException& e)
    {
      UNUSED(e);
      incompatible_type = true;
    }

    if(!incompatible_type)
    {
      lua_pushstring(L, "type");
      lua_pushstring(L, tCOMUtil::getPrintableTypeDesc(tdesc));
      lua_settable(L, -3);
    }

	USHORT paramFlags = pfuncdesc->lprgelemdescParam[i].paramdesc.wParamFlags;

    lua_pushstring(L, "in");
	luaCompat_pushBool(L,paramFlags & PARAMFLAG_FIN);
	lua_settable(L, -3);

    lua_pushstring(L, "out");
	luaCompat_pushBool(L,paramFlags & PARAMFLAG_FOUT);
	lua_settable(L, -3);

    lua_pushstring(L, "opt");
	luaCompat_pushBool(L,paramFlags & PARAMFLAG_FOPT);
	lua_settable(L, -3);

	lua_pushstring(L, "default");
	if(paramFlags & PARAMFLAG_FHASDEFAULT) {
		lua_typeinfo->typehandler->com2lua(L,pfuncdesc->lprgelemdescParam[i].paramdesc.pparamdescex->varDefaultValue);
	} else lua_pushnil(L);
	lua_settable(L, -3);

    // removes copy of the table
    lua_pop(L, 1);

    i++;
  }

  // removes copy of the table
  lua_pop(L, 1);

  SAFEDELETEARR(names);

  // Stores the type for the return value
  TYPEDESC tdesc = pfuncdesc->elemdescFunc.tdesc;
  bool incompatible_type = false;

  try
  {
    tdesc = 
      tLuaCOMTypeHandler::processTYPEDESC(lua_typeinfo->typeinfo, tdesc);
  }
  catch(class tLuaCOMException& e)
  {
    UNUSED(e);
    incompatible_type = true;
  }

  if(!incompatible_type)
  {
    lua_pushstring(L, "type");
    lua_pushstring(L, tCOMUtil::getPrintableTypeDesc(tdesc));
    lua_settable(L, -3);
  }

  lua_typeinfo->typeinfo->ReleaseFuncDesc(pfuncdesc);
  
  LUASTACK_CLEAN(L, 1);

  return 1;
}


int tLuaTypeInfo::GetVarDesc(tLuaObject* lua_obj, lua_State* L)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  tLuaTypeInfo* lua_typeinfo = DYNAMIC_CAST<tLuaTypeInfo*>(lua_obj);
  CHECKPRECOND(lua_typeinfo);

  // gets vardesc

  VARDESC* pvardesc = NULL;
  {
    UINT i = (UINT)lua_tointeger(L, 2);

    hr = lua_typeinfo->typeinfo->GetVarDesc(i, &pvardesc);
    CHK_COM_CODE(hr);
  }

  // table to hold the vardesc
  lua_newtable(L);

  // stores variable name
  lua_pushstring(L, "name");
  {
    BSTR name;
    lua_typeinfo->typeinfo->GetDocumentation(
      pvardesc->memid, &name, NULL, NULL, NULL);

    lua_pushstring(L, tUtil::bstr2string(name));
    SysFreeString(name);
  }
  lua_settable(L, -3);

  // if it is a constant, stores it's value
  if(pvardesc->varkind == VAR_CONST)
  {
    lua_pushstring(L, "value");
  
    lua_typeinfo->typehandler->com2lua(L, *pvardesc->lpvarValue);

    // sets in the table
    lua_settable(L, -3);
  }
    
  lua_typeinfo->typeinfo->ReleaseVarDesc(pvardesc);
  pvardesc = NULL;

  LUASTACK_CLEAN(L, 1);

  return 1;
}

int tLuaTypeInfo::GetImplType(tLuaObject* lua_obj, lua_State* L)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  tLuaTypeInfo* lua_typeinfo = DYNAMIC_CAST<tLuaTypeInfo*>(lua_obj);
  CHECKPRECOND(lua_typeinfo);

  // gets implemented type

  ITypeInfo* ptinfo = NULL;
  {
    UINT i = (UINT)lua_tointeger(L, 2);

    HREFTYPE hreftype;
    hr = lua_typeinfo->typeinfo->GetRefTypeOfImplType(i, &hreftype);
    CHK_COM_CODE(hr);

    hr = lua_typeinfo->typeinfo->GetRefTypeInfo(hreftype, &ptinfo);
    CHK_COM_CODE(hr);
  }

  tLuaTypeInfo::pushNew(L, ptinfo);
  COM_RELEASE(ptinfo);
 
  LUASTACK_CLEAN(L, 1);

  return 1;
}

int tLuaTypeInfo::GetImplTypeFlags(tLuaObject* lua_obj, lua_State* L)
{
  LUASTACK_SET(L);

  HRESULT hr = S_OK;

  tLuaTypeInfo* lua_typeinfo = DYNAMIC_CAST<tLuaTypeInfo*>(lua_obj);
  CHECKPRECOND(lua_typeinfo);

  int typeflags = 0;
  UINT i = (UINT)lua_tointeger(L, 2);

  hr = lua_typeinfo->typeinfo->GetImplTypeFlags(i, &typeflags);
  CHK_COM_CODE(hr);

  lua_newtable(L);

  lua_pushstring(L, "default");
  luaCompat_pushBool(L, typeflags & IMPLTYPEFLAG_FDEFAULT);
  lua_settable(L, -3);

  lua_pushstring(L, "source");
  luaCompat_pushBool(L, typeflags & IMPLTYPEFLAG_FSOURCE);
  lua_settable(L, -3);

  lua_pushstring(L, "restricted");
  luaCompat_pushBool(L, typeflags & IMPLTYPEFLAG_FRESTRICTED);
  lua_settable(L, -3);

  lua_pushstring(L, "defaultvtable");
  luaCompat_pushBool(L, typeflags & IMPLTYPEFLAG_FDEFAULTVTABLE);
  lua_settable(L, -3);

  LUASTACK_CLEAN(L, 1);

  return 1;
}



int tLuaTypeInfo::GetDocumentation(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTypeInfo* lua_typeinfo = DYNAMIC_CAST<tLuaTypeInfo*>(lua_obj);
  
  CHECKPRECOND(lua_typeinfo);

  HRESULT hr = S_OK;
  BSTR name;
  BSTR helpstring;
  BSTR helpfile;
  ULONG helpcontext;

  hr = lua_typeinfo->typeinfo->GetDocumentation(MEMBERID_NIL, 
    &name, &helpstring, &helpcontext, &helpfile);

  CHK_COM_CODE(hr);

  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushstring(L, tUtil::bstr2string(name));
  lua_settable(L, -3);

  lua_pushstring(L, "helpstring");
  lua_pushstring(L, tUtil::bstr2string(helpstring));
  lua_settable(L, -3);

  lua_pushstring(L, "helpfile");
  lua_pushstring(L, tUtil::bstr2string(helpfile));
  lua_settable(L, -3);

  lua_pushstring(L, "helpcontext");
  lua_pushnumber(L, helpcontext);
  lua_settable(L, -3);
 
  SysFreeString(name);
  SysFreeString(helpstring);
  SysFreeString(helpfile);

  return 1;
}

int tLuaTypeInfo::GetTypeAttr(tLuaObject* lua_obj, lua_State* L)
{
  tLuaTypeInfo* lua_typeinfo = DYNAMIC_CAST<tLuaTypeInfo*>(lua_obj);
  CHECKPRECOND(lua_typeinfo);

  TYPEATTR* ptypeattr = NULL;

  HRESULT hr = lua_typeinfo->typeinfo->GetTypeAttr(&ptypeattr);
  CHK_COM_CODE(hr);

  // creates table to hold type attributes
  lua_newtable(L);

  // stores guid
  {
    char *pGuid = NULL;

    hr = tCOMUtil::GUID2String(ptypeattr->guid, &pGuid);
    if(SUCCEEDED(hr))
    {
      lua_pushstring(L, "GUID");
      lua_pushstring(L, pGuid);
      lua_settable(L, -3);
      delete pGuid;
    }
  }

  // stores typekind
  lua_pushstring(L, "typekind");
  lua_pushstring(L, tCOMUtil::getPrintableTypeKind(ptypeattr->typekind));
  lua_settable(L, -3);

  // stores numbers
  lua_pushstring(L, "Funcs");
  lua_pushnumber(L, ptypeattr->cFuncs);
  lua_settable(L, -3);

  lua_pushstring(L, "Vars");
  lua_pushnumber(L, ptypeattr->cVars);
  lua_settable(L, -3);

  lua_pushstring(L, "ImplTypes");
  lua_pushnumber(L, ptypeattr->cImplTypes);
  lua_settable(L, -3);


  // stores flags in a separate table
  lua_newtable(L);

  lua_pushstring(L, "flags");
  lua_pushvalue(L, -2);
  lua_settable(L, -4);

  lua_pushstring(L, "hidden");
  luaCompat_pushBool(L, ptypeattr->wTypeFlags & TYPEFLAG_FHIDDEN);
  lua_settable(L, -3);

  lua_pushstring(L, "control");
  luaCompat_pushBool(L, ptypeattr->wTypeFlags & TYPEFLAG_FCONTROL);
  lua_settable(L, -3);

  lua_pushstring(L, "appobject");
  luaCompat_pushBool(L, ptypeattr->wTypeFlags & TYPEFLAG_FAPPOBJECT);
  lua_settable(L, -3);

  lua_pushstring(L, "dispatchable");
  luaCompat_pushBool(L, ptypeattr->wTypeFlags & TYPEFLAG_FDISPATCHABLE);
  lua_settable(L, -3);

  lua_pushstring(L, "oleautomation");
  luaCompat_pushBool(L, ptypeattr->wTypeFlags & TYPEFLAG_FOLEAUTOMATION);
  lua_settable(L, -3);

  lua_pushstring(L, "cancreate");
  luaCompat_pushBool(L, ptypeattr->wTypeFlags & TYPEFLAG_FCANCREATE);
  lua_settable(L, -3);


  // removes attributes table
  lua_pop(L, 1);

  return 1;
}

int tLuaTypeInfo::GetTypeLib(tLuaObject* lua_obj, lua_State* L)
{
  LUASTACK_SET(L);

  tLuaTypeInfo* lua_typeinfo = DYNAMIC_CAST<tLuaTypeInfo*>(lua_obj);
  
  CHECKPRECOND(lua_typeinfo);

  // stores a reference to the type library
  ITypeLib* typelib = NULL;
  unsigned int dumb = 0;

  HRESULT hr = lua_typeinfo->typeinfo->GetContainingTypeLib(&typelib, &dumb);

  if(SUCCEEDED(hr))
  {
    tLuaTLB::pushNew(L, typelib);
  }
  COM_RELEASE(typelib);

  LUASTACK_CLEAN(L, 1);

  return 1;
}
