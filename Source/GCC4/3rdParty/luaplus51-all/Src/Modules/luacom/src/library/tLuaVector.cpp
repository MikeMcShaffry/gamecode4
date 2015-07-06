// tLuaVector.cpp: implementation of the tLuaVector class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <assert.h>

#include "tLuaVector.h"
#include "tLuaCOM.h"
#include "LuaAux.h"
#include "tLuaCOMException.h"

extern "C"
{
#include "LuaCompat.h"
}


#define LUA_NOOBJECT 0


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

tLuaVector::tLuaVector()
{
  initialized = false;
}

tLuaVector::~tLuaVector()
{
  // destroi escalares ou luavectors
  freeData();
}


unsigned long tLuaVector::get_Dimensions()
{
  CHECKPRECOND(length != 0 && elem_type != UNKNOWN);

  if(elem_type == SCALAR)
    return 1;
  else
    return 1 + vectors[0]->get_Dimensions();
}

unsigned long tLuaVector::get_Nth_Dimension(unsigned long n)
{
  // precond
  CHECKPRECOND(length >= 0 && n >= 0 && (n == 1 || elem_type == VECTOR));
  
  if(n == 1)
    return length;
  else
    return vectors[0]->get_Nth_Dimension(n - 1);
}

stkIndex tLuaVector::getindex(long * indices,
                              unsigned long size,
                              const SAFEARRAYBOUND* bounds )
{
  CHECKPARAM(indices != NULL && size != 0);

  const long index = indices[0] - bounds[0].lLbound;

  CHECKPARAM(index < (long)length);

  if(size > 1)
  {
    CHECKPRECOND(elem_type == VECTOR);

    return vectors[index]->getindex(&indices[1], size - 1, bounds);
  }
  else
  {
    return luavals[index];
  }
}

unsigned long tLuaVector::getLength()
{
  return length;
}

tLuaVector::Type tLuaVector::getType()
{
  return array_type;
}

tLuaVector::Type tLuaVector::lua2Type(lua_State* L, stkIndex luaval)
{
  switch(lua_type(L, luaval))
  {
  case LUA_TNUMBER:
    return NUMBER;
    break;

  case LUA_TSTRING:
    return STRING;
    break;

  case LUA_TUSERDATA:
    return USERDATA;
    break;

  default:
    return OTHER;
    break;
  }
}

void tLuaVector::freeData()
{
  unsigned long i;

  if(vectors != NULL)
  {
    for(i = 0; i < length; i++)
      delete vectors[i];

    delete[] vectors;
    vectors = NULL;
  }
  else if(luavals != NULL)
  {
    delete[] luavals;
    luavals = NULL;
  }

  initialized = false;

  return;
}

//
// Cria uma tabela Lua a partir da matriz contida no luavector
//
bool tLuaVector::CreateTable(lua_State* L)
{
  lua_newtable(L);
  stkIndex table = lua_gettop(L);

  // if no elements have been set, returns an empty table
  if(array_type == NOTYPE)
    return true;

  switch(elem_type)
  {
  case VECTOR:
    {
      unsigned long index = 0;

      for(index = 0; index < length; index++)
      {
        bool succeeded = vectors[index]->CreateTable(L);

        if(!succeeded)
          return false;

        stkIndex child_table = lua_gettop(L);

        lua_pushvalue(L, table);
        lua_pushnumber(L, index + 1);

        lua_pushvalue(L, child_table);

        lua_settable(L, -3);
        lua_settop(L, table);
      }
    }

    break;

  case SCALAR:
    {
      unsigned long int index = 0;

      for(index = 0; index < length; index++)
      {
        if(luavals[index] == LUA_NOOBJECT)
          return false;

        lua_pushvalue(L, table);
        lua_pushnumber(L, index + 1);

        lua_pushvalue(L, luavals[index]);

        lua_settable(L, -3);
        lua_settop(L, table);
      }
    }

    break;

  default:
    return false;
  }

  return true;
}

void tLuaVector::setindex(lua_State* L,
                          stkIndex luaval,
                          long * indices,
                          unsigned long size,
                          const SAFEARRAYBOUND* bounds)
{
  CHECKPARAM(indices != NULL && size != 0);

  const long index = indices[0] - bounds[0].lLbound;

  CHECKPARAM(index < (long)length);

  if(size > 1)
  {
    if(elem_type != VECTOR)
      TYPECONV_ERROR("Cannot convert table to safearray");

    vectors[index]->setindex(L, luaval, &indices[1], size - 1, &bounds[1]);

    if(array_type == NOTYPE)
      array_type = vectors[index]->getType();
    else if(vectors[index]->getType() != array_type)
      array_type = MANY;
  }
  else
  {
    luavals[index] = luaval;

    if(array_type == NOTYPE)
      array_type = lua2Type(L, luaval);
    else if(lua2Type(L, luaval) != array_type)
      array_type = MANY;
  }
}

void tLuaVector::InitVectorFromDimensions(long * dimensions,
                                          unsigned long num_dimensions)
{
  CHECKPRECOND(initialized != true);
  CHECKPARAM(dimensions != NULL && num_dimensions != 0);

  length = 0;
  vectors = NULL;
  luavals = NULL;
  array_type = NOTYPE;
  elem_type = UNKNOWN;
  max_length = 0;

  if(num_dimensions == 1)
  {
    elem_type = SCALAR;
    length = max_length = dimensions[0];

    luavals = new stkIndex[length];
    assert(luavals);

    {
      unsigned long i = 0;

      for(i = 0; i < length; i++)
        luavals[i] = LUA_NOOBJECT;
    }

    // cria array de luavals e inicializa resto

    initialized = true;
  }
  else
  {
    elem_type = VECTOR;
    length = max_length = dimensions[0];

    vectors = new tLuaVector*[length];

    // inicializa vetores recursivamente

    {
      unsigned long i = 0;

      for(i = 0; i < length; i++)
      {
        vectors[i] = new tLuaVector();
        vectors[i]->InitVectorFromDimensions(&dimensions[1], num_dimensions - 1);
      }
    }

    initialized = true;
  }

  return;
}

void tLuaVector::InitVectorFromTable(lua_State* L, stkIndex table)
{
  CHECKPRECOND(!initialized);

  stkIndex luaval = LUA_NOOBJECT;
  
  length = 0;
  vectors = NULL;
  luavals = NULL;
  array_type = NOTYPE;
  elem_type = UNKNOWN;
  max_length = 0;

  // so' podem ser criados vetores a partir
  // de tabelas e de userdata

  if(lua_type(L, table) == LUA_TNONE ||
      (!lua_istable(L, table) && !lua_isuserdata(L, table))
     )
  {
    TYPECONV_ERROR("Can't convert to safearray");
  }


  try
  {
    // Itera em tabela ou userdata, criando LuaVectors recursivamente
    while(1)
    {
      CHECKPRECOND((length <= 0 || elem_type != UNKNOWN) &&
         (length != 0 || elem_type == UNKNOWN));
  
      lua_pushvalue(L, table);

      lua_pushnumber(L, length + 1);

      lua_gettable(L, -2);

      lua_remove(L, -2);

      luaval = lua_gettop(L);

      if(lua_isnil(L, luaval))
      {
        lua_remove(L, luaval);
        break;
      }

      // assures there will be enough stack space
      luaCompat_needStack(L, 1);
      
      // se nao for tabela, termina processo de calculo de dimensoes
      // e guarda elemento
      // se for, continua, chamando recursivamente

      if(lua_istable(L, luaval) && LuaBeans::from_lua(L, luaval) == NULL)
      {
        // garante que tabela e' uma matriz
        if(elem_type == SCALAR)
          TYPECONV_ERROR("Cannot convert table to safearray");

        if(elem_type == UNKNOWN)
        {
          elem_type = VECTOR;

          max_length = 10;

          vectors = new tLuaVector*[max_length];
        }

        // realoca array
        if(length == max_length)
        {
          tLuaVector **new_vectors = NULL;

          max_length *= 2;

          new_vectors = new tLuaVector*[max_length];

          unsigned long i = 0;

          for(i = 0; i < length; i++)
            new_vectors[i] = vectors[i];

          delete[] vectors;
          vectors = new_vectors;
        }

        // obtem novo vetor
        vectors[length] = new tLuaVector();

        vectors[length]->InitVectorFromTable(L, luaval);

        length++;

        // caso nova tabela tenha dimensao diferente, aborta processo
        // (nao e' uma matriz)
        if(length == 1)
        {
          array_type = vectors[length - 1]->getType();
        }
        else if(
          vectors[length - 1]->getLength() != 
          vectors[length - 2]->getLength())
        {
          TYPECONV_ERROR("Cannot convert table to safearray");
        }
        else if(
          vectors[length - 1]->getType() != 
          vectors[length - 2]->getType())
        {
          array_type = MANY;
        }
      }
      else
      {
        if(elem_type == VECTOR)
          TYPECONV_ERROR("Cannot convert table to safearray");

        if(elem_type == UNKNOWN)
        {
          elem_type = SCALAR;

          max_length = 10;

          luavals = new stkIndex[max_length];
        }

        // realoca luavals
        if(length == max_length)
        {
          stkIndex *new_luavals = NULL;

          max_length *= 2;

          new_luavals = new stkIndex[max_length];

          unsigned long i = 0;

          for(i = 0; i < length; i++)
            new_luavals[i] = luavals[i];

          delete[] luavals;
          luavals = new_luavals ;
        }

      
        // guarda luaval
        luavals[length] = luaval;
        length++;

        // verifica se e' do mesmo tipo que o anterior
        if(length > 1 && lua2Type(L, luaval) != array_type)
        {
          array_type = MANY;
        }
        else if(length == 1)
        {
          array_type = lua2Type(L, luaval);
        }
      }
    }
  }
  catch(class tLuaCOMException)
  {
    freeData();
    initialized = false;
    throw;
  }

  initialized = true;

  return;
}

void tLuaVector::InitVectorFromStack(lua_State* L, stkIndex first_elem)
{
  CHECKPRECOND(!initialized);

  // vector initialization
  array_type = MANY;
  elem_type = SCALAR;
  vectors = NULL;

  stkIndex luaval = first_elem;
  stkIndex last_elem = lua_gettop(L);

  length = 0;
  max_length = lua_gettop(L) - first_elem + 1;

  luavals = new stkIndex[max_length];

  try
  {
    while(luaval <= last_elem)
    {
      CHECKPARAM_MSG(lua_type(L, luaval) != LUA_TTABLE,
        "Trying to create nested safearrays.");
    
      luavals[length] = luaval;
      length++;
      luaval++;
    }
  }
  catch(class tLuaCOMException)
  {
    freeData();
    initialized = false;
    throw;
  }

  initialized = true;

  return;
}


long tLuaVector::size()
{
  long size = 0;

  if(vectors != NULL)
  {
    unsigned long i;
    for(i = 0; i < length; i++)
      size += vectors[i]->size();
  }
  else if(luavals != NULL)
  {
    size = length;
  }

  return size;
}
