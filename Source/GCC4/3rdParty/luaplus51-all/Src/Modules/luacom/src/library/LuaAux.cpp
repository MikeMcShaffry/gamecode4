// LuaAux.cpp: implementation of the LuaAux class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "LuaAux.h"

extern "C"
{
#include "LuaCompat.h"
}


// RCS Info
static char *rcsid = "$Id: LuaAux.cpp,v 1.2 2007/12/20 06:51:15 dmanura Exp $";
static char *rcsname = "$Name: HEAD $";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LuaAux::LuaAux()
{

}

LuaAux::~LuaAux()
{

}

/*
 * Prints the lua stack
 */

void LuaAux::printLuaStack(lua_State *L)
{
  int size = lua_gettop(L);
  int i = 0;

  for(i = size; i > 0; i--)
  {
    switch(lua_type(L,i))
    {
    case LUA_TNUMBER:
      printf("%d: number = %d", i, lua_tonumber(L, i));
      break;

    case LUA_TSTRING:
      printf("%d: string = \"%s\"", i, lua_tostring(L, i));
      break;

    case LUA_TTABLE:
      printf("%d: table, tag = %p", i, luaCompat_getType2(L, i));
      break;

    case LUA_TUSERDATA:
      printf("%d: userdata = %p, tag = %p", i,
        luaCompat_getPointer(L, i), luaCompat_getType2(L, i));
      break;

    case LUA_TNIL:
      printf("%d: nil", i);
      break;

    case LUA_TBOOLEAN:
      if(lua_toboolean(L, i))
        printf("%d: boolean = true", i);
      else
        printf("%d: boolean = false", i);

      break;

    default:
      printf("%d: unknown type (%d)", i, lua_type(L, i));
      break;
    }      

    printf("\n");
  }

  printf("\n");
}

void LuaAux::printPreDump(int expected) {
  printf("STACK DUMP\n");
  printf("Expected size: %i\n",expected);
}

void LuaAux::printLuaTable(lua_State *L, stkIndex t)
{

  lua_pushnil(L);  /* first key */
  while (lua_next(L, t) != 0) {
   /* `key' is at index -2 and `value' at index -1 */
   printf("%s - %s\n",
     lua_tostring(L, -2), lua_typename(L, lua_type(L, -1)));
   lua_pop(L, 1);  /* removes `value'; keeps `index' for next iteration */
  }
}

const char* LuaAux::makeLuaErrorMessage(int return_value, const char* msg)
{
  static char message[1000];
  message[0] = '\0';

  if(return_value == 0)
    return "No error";
  
  switch(return_value)
  {
  case LUA_ERRRUN:
    {
      strncat(message, 
        "Lua runtime error", 
        sizeof(message) - strlen(message) - 1);

      if(msg)
      {
        strncat(message, ": ", sizeof(message) - strlen(message) - 1);
        strncat(message, msg, sizeof(message) - strlen(message) - 1);
      }
    }

    break;

  case LUA_ERRMEM:
    strcpy(message, "Lua memory allocation error.");
    break;

  case LUA_ERRERR:
    strcpy(message, "Lua error: error during error handler execution.");
    break;

  default:
    strcpy(message, "Unknown Lua error.");
    break;
  }

  return message;
}
