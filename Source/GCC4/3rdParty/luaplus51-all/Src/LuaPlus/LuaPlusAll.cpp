#define LUA_FORCE_USE_LONGJMP
#ifndef LUAPLUS_ALL
#define LUAPLUS_ALL
#endif
#define loslib_c
#include "LuaLink.h"
LUA_EXTERN_C_BEGIN
#include "src/lapi.c"
#include "src/lauxlib.c"
#include "src/lbaselib.c"
#include "src/lcode.c"
#include "src/ldblib.c"
#include "src/ldebug.c"
#include "src/ldo.c"
#include "src/ldump.c"
#include "src/lfunc.c"
#include "src/lgc.c"
#include "src/linit.c"
#include "src/liolib.c"
#include "src/llex.c"
#include "src/lmathlib.c"
#include "src/lmem.c"
#include "src/lobject.c"
#include "src/lopcodes.c"
#include "src/loslib.c"
#include "src/lparser.c"
#include "src/lstate.c"
#include "src/lstring.c"
#include "src/lstrlib.c"
#include "src/ltable.c"
#include "src/ltablib.c"
#include "src/ltm.c"
#include "src/lundump.c"
#include "src/lvm.c"
#include "src/lzio.c"
#include "lwstrlib.c"
#include "LuaPlusAddons.c"
LUA_EXTERN_C_END
#include "LuaPlus.cpp"
#include "LuaPlus_Libs.cpp"
#include "LuaPlusFunctions.cpp"
#include "LuaState.cpp"
#include "LuaStateOutFile.cpp"
#include "LuaState_DumpObject.cpp"
#include "LuaObject.cpp"
#include "LuaTableIterator.cpp"
LUA_EXTERN_C_BEGIN
#include "src/loadlib.c"
#if defined(LUA_WIN)
#include "popen.c"
#endif // LUA_WIN
LUA_EXTERN_C_END
