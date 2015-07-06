// tLuaVector.h: interface for the tLuaVector class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TLUAVECTOR_H__BAD115E6_A282_11D4_9DF6_0000B45D6611__INCLUDED_)
#define AFX_TLUAVECTOR_H__BAD115E6_A282_11D4_9DF6_0000B45D6611__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

extern "C"
{
#include "lua.h"
}

#include "tLuaCOM.h"
#include "luabeans.h"


class tLuaVector  
{
public:
	long size(void);
  enum Type {NOTYPE, NUMBER, STRING, USERDATA, OTHER, MANY};

	void InitVectorFromTable(lua_State* L, stkIndex table);
  void InitVectorFromStack(lua_State* L, stkIndex first_elem);
  void InitVectorFromDimensions(
    long *dimensions,
    unsigned long num_dimensions);
	
	void setindex(
    lua_State* L,
    stkIndex luaval,
    long *dimensions,
    unsigned long size,
    const SAFEARRAYBOUND* bounds);

	bool CreateTable(lua_State* L);

	Type lua2Type(lua_State* L, stkIndex luaval);
	Type getType(void);
  unsigned long getLength(void);
	stkIndex getindex(
    long *indices,
    unsigned long size,
    const SAFEARRAYBOUND* bounds );

	unsigned long get_Nth_Dimension(unsigned long n);
	unsigned long get_Dimensions(void);
	tLuaVector();
	virtual ~tLuaVector();

protected:
  enum {UNKNOWN, VECTOR, SCALAR} elem_type;

	void freeData(void);

	unsigned long max_length;
	unsigned long length;
	stkIndex*  luavals;
	tLuaVector** vectors;
	bool initialized;

  Type array_type;
};

#endif // !defined(AFX_TLUAVECTOR_H__BAD115E6_A282_11D4_9DF6_0000B45D6611__INCLUDED_)
