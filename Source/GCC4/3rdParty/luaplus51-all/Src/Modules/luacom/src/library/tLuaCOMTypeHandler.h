
// tLuaCOMTypeHandler.h: interface for the tLuaCOMTypeHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLUACOMTYPEHANDLER_H__2CAF122C_E09B_11D3_BD2F_444553540000__INCLUDED_)
#define AFX_TLUACOMTYPEHANDLER_H__2CAF122C_E09B_11D3_BD2F_444553540000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <ole2.h>

#include "luabeans.h"
#include "tLuaObjList.h"
#include "LuaAux.h"

class tLuaCOM;

class tLuaCOMTypeHandler  
{
public:
  void setOutValues(
    lua_State* L, 
    FUNCDESC* pFuncDesc,
    DISPPARAMS* pDispParams,
    stkIndex outvalue);

  void pushLuaArgs(
    lua_State* L, 
    DISPPARAMS* pDispParams,
    const ELEMDESC* pElemDesc
    );

  void fillDispParams(
    lua_State* L, 
    DISPPARAMS& rDispParams,
    FUNCDESC *pfuncdesc,
    tLuaObjList params,
    int invkind
    );

  void releaseVariants(DISPPARAMS *pDispParams);

  void releaseVariant(VARIANTARG *pvarg, bool release_memory=true);

  void pushTableVarNumber(lua_State *L, VARTYPE vt, double val);

  int pushOutValues(lua_State* L, const DISPPARAMS& dispparams, const FUNCDESC* pfuncdesc);

  bool setRetval(
    lua_State* L,
    const FUNCDESC *funcdesc,
    stkIndex luaval,
    VARIANTARG *pvarg);

  void lua2com(lua_State* L, stkIndex luaval, VARIANTARG& varg, VARTYPE type=VT_VARIANT);

  void com2lua(lua_State* L, VARIANTARG varg, bool is_variant = false);
  tLuaCOMTypeHandler(ITypeInfo *ptypeinfo);
  virtual ~tLuaCOMTypeHandler();

	static TYPEDESC processTYPEDESC(
    ITypeInfo* typeinfo,
    TYPEDESC tdesc);

protected:
  long * dimensionsFromBounds(SAFEARRAYBOUND* bounds, long num_bounds);
  void put_in_array(SAFEARRAY* safearray,
                    VARIANT var_value,
                    long* indices,
                    VARTYPE vt);

  bool inc_indices(long *indices,
                   SAFEARRAYBOUND *bounds,
                   unsigned long dimensions);

  SAFEARRAYBOUND* getRightOrderedBounds(SAFEARRAYBOUND *bounds, 
                                        unsigned long num_dimensions);

	void Coerce(VARIANTARG& dest, VARIANTARG src, VARTYPE vt);
	long VariantSize(VARTYPE vt);
	static TYPEDESC processSAFEARRAY(ITypeInfo* typeinfo, TYPEDESC& tdesc);
	void toByRefParam(VARIANT& var_source, VARIANTARG * pvarg_dest);
	void initByRefParam(VARIANTARG* pvarg, VARTYPE vt, bool alloc_memory = true);
	void pushIUnknown(lua_State* L, IUnknown *punk);
	bool isIUnknown(lua_State* L, stkIndex value);
	static TYPEDESC processAliases(
    ITypeInfo* typeinfo,
    const TYPEDESC& tdesc);
	static TYPEDESC processUSERDEFINED(
    ITypeInfo* typeinfo,
    const TYPEDESC& tdesc);
	static TYPEDESC processPTR(
    ITypeInfo* typeinfo,
    const TYPEDESC& tdesc);

  tLuaCOM * from_lua(lua_State* L, int index);
  tLuaCOM * convert_table(lua_State *L, stkIndex luaval);
  stkIndex get_from_array(
    lua_State* L, 
    SAFEARRAY* safearray,
    long *indices,
    const VARTYPE& vt);

  void safearray_com2lua(lua_State* L, VARIANTARG& varg);

  void safearray_lua2com(
    lua_State* L,
    stkIndex luaval,
    VARIANTARG& varg, VARTYPE vt,
    bool from_stack = false
    );

  void string2safearray(const char* str, size_t len, VARIANTARG& varg);
  void safearray2string(lua_State* L, VARIANTARG & varg);

  ITypeInfo * m_typeinfo;
};

#define LUACOM_IUNKNOWN_TAGNAME "_LuaCOM_IUnknown_tag"

#endif // !defined(AFX_TLUACOMTYPEHANDLER_H__2CAF122C_E09B_11D3_BD2F_444553540000__INCLUDED_)
