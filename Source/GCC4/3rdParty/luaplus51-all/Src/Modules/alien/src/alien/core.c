/* Lua FFI using libffi */
/* Author: Fabio Mascarenhas */
/* License: MIT/X11 */

#ifdef WINDOWS
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ffi.h"

#if LUA_VERSION_NUM == 502
#define lua_setfenv lua_setuservalue
#define lua_getfenv lua_getuservalue
#define lua_objlen lua_rawlen

static int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

#undef luaL_register
#define luaL_register(L, n, f) luaL_setfuncs(L, f, 0)
#endif

#ifdef WINDOWS
#include <windows.h>
#define ALLOCA _alloca
#else
#include <sys/mman.h>
#include <unistd.h>
# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#  define MAP_ANONYMOUS MAP_ANON
# endif
#define ALLOCA alloca
#endif

#define BLOCKSIZE _pagesize

#define ALIEN_LIBRARY_META "alien_library"
#define ALIEN_FUNCTION_META "alien_function"
#define ALIEN_BUFFER_META "alien_buffer"
#define ALIEN_LBUFFER_META "alien_lbuffer"
#define ALIEN_CALLBACK_META "alien_callback"

#ifndef uchar
#define uchar unsigned char
#endif

typedef enum {
  AT_SHORT,
  AT_USHORT,
  AT_INT,
  AT_UINT,
  AT_LONG,
  AT_ULONG,
  AT_VOID,
  AT_FLOAT,
  AT_DOUBLE,
  AT_CHAR,
  AT_BYTE,
  AT_STRING,
  AT_PTR,
  AT_REFINT,
  AT_REFUINT,
  AT_REFCHAR,
  AT_REFDOUBLE,
  AT_CALLBACK
} alien_Type;

typedef struct _alien_Library {
  void *lib;
  char *name;
} alien_Library;

typedef struct _alien_Function {
  alien_Library *lib;
  void *fn;
  char *name;
  alien_Type ret_type;
  ffi_cif cif;
  ffi_type *ffi_ret_type;
  int nparams;
  alien_Type *params;
  ffi_type **ffi_params;
} alien_Function;

typedef struct _alien_Callback {
  /* alien_Function part! */
  alien_Library *lib;
  void *fn;
  char *name;
  alien_Type ret_type;
  ffi_cif cif;
  ffi_type *ffi_ret_type;
  int nparams;
  alien_Type *params;
  ffi_type **ffi_params;
  /* callback part */
  lua_State *L;
  int fn_ref;
} alien_Callback;

typedef struct _alien_Wrap {
  alien_Type tag;
  union {
    void *p;
    int i;
  } val;
} alien_Wrap;

typedef union _tagITEM {
        ffi_closure closure;
        union _tagITEM *next;
} ITEM;

static ITEM *free_list;
int _pagesize;

/* Information to compute strucuture access */

typedef struct { char c; char x; } s_char;
typedef struct { char c; short x; } s_short;
typedef struct { char c; int x; } s_int;
typedef struct { char c; long x; } s_long;
typedef struct { char c; float x; } s_float;
typedef struct { char c; double x; } s_double;
typedef struct { char c; char *x; } s_char_p;
typedef struct { char c; void *x; } s_void_p;

#define AT_CHAR_ALIGN (sizeof(s_char) - sizeof(char))
#define AT_SHORT_ALIGN (sizeof(s_short) - sizeof(short))
#define AT_INT_ALIGN (sizeof(s_int) - sizeof(int))
#define AT_LONG_ALIGN (sizeof(s_long) - sizeof(long))
#define AT_FLOAT_ALIGN (sizeof(s_float) - sizeof(float))
#define AT_DOUBLE_ALIGN (sizeof(s_double) - sizeof(double))
#define AT_CHAR_P_ALIGN (sizeof(s_char_p) - sizeof(char*))
#define AT_VOID_P_ALIGN (sizeof(s_void_p) - sizeof(void*))

static void more_core(void)
{
        ITEM *item;
        int count, i;

/* determine the pagesize */
#ifdef WINDOWS
        if (!_pagesize) {
                SYSTEM_INFO systeminfo;
                GetSystemInfo(&systeminfo);
                _pagesize = systeminfo.dwPageSize;
        }
#else
        if (!_pagesize) {
                _pagesize = getpagesize();
        }
#endif

        /* calculate the number of nodes to allocate */
        count = BLOCKSIZE / sizeof(ITEM);

        /* allocate a memory block */
#ifdef WINDOWS
        item = (ITEM *)VirtualAlloc(NULL,
                                               count * sizeof(ITEM),
                                               MEM_COMMIT,
                                               PAGE_EXECUTE_READWRITE);
        if (item == NULL)
                return;
#else
        item = (ITEM *)mmap(NULL,
                            count * sizeof(ITEM),
                            PROT_READ | PROT_WRITE | PROT_EXEC,
                            MAP_PRIVATE | MAP_ANONYMOUS,
                            -1,
                            0);
        if (item == (void *)MAP_FAILED)
                return;
#endif

        /* put them into the free list */
        for (i = 0; i < count; ++i) {
                item->next = free_list;
                free_list = item;
                ++item;
        }
}

/******************************************************************/

/* put the item back into the free list */
void free_closure(void *p)
{
        ITEM *item = (ITEM *)p;
        item->next = free_list;
        free_list = item;
}

/* return one item from the free list, allocating more if needed */
void *malloc_closure(void)
{
        ITEM *item;
        if (!free_list)
                more_core();
        if (!free_list)
                return NULL;
        item = free_list;
        free_list = item->next;
        return item;
}

#if defined(LINUX)
#define PLATFORM "linux"
#define USE_DLOPEN
#elif defined(BSD)
#define PLATFORM "bsd"
#define USE_DLOPEN
#elif defined(DARWIN)
#define PLATFORM "darwin"
#define USE_DLOPEN
#endif

#if defined(USE_DLOPEN)

#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT 0
#endif

#include <dlfcn.h>

static void alien_unload (void *lib) {
  if(lib && (lib != RTLD_DEFAULT))
    dlclose(lib);
}

static void *alien_load (lua_State *L, const char *libname) {
  void *lib;
  lib = dlopen(libname, RTLD_NOW);
  if(lib == NULL) lua_pushstring(L, dlerror());
  return lib;
}

static void *alien_loadfunc (lua_State *L, void *lib, const char *sym) {
  if(!lib) lib = RTLD_DEFAULT;
  void *f = dlsym(lib, sym);
  if (f == NULL) lua_pushstring(L, dlerror());
  return f;
}

#elif defined(WINDOWS)

#define PLATFORM "windows"

static void pusherror (lua_State *L) {
  int error = GetLastError();
  char buffer[128];
  if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
      NULL, error, 0, buffer, sizeof(buffer), NULL))
    lua_pushstring(L, buffer);
  else
    lua_pushfstring(L, "system error %d\n", error);
}

static void alien_unload (void *lib) {
  if(lib)
    FreeLibrary((HINSTANCE)lib);
}

static void *alien_load (lua_State *L, const char *libname) {
  HINSTANCE lib = LoadLibrary(libname);
  if (lib == NULL) pusherror(L);
  return lib;
}

static void *alien_loadfunc (lua_State *L, void *lib, const char *sym) {
  HINSTANCE module;
  void *f;
  module = (HINSTANCE)lib;
  if(!module) module = GetModuleHandle(NULL);
  f = (lua_CFunction)GetProcAddress(module, sym);
  if (f == NULL) pusherror(L);
  return f;
}

#else

#define DLMSG   "dynamic libraries not enabled; check your Lua installation"

#define PLATFORM "unknown"

static void alien_unload (void *lib) {
  (void)lib;  /* to avoid warnings */
}

static void *alien_load (lua_State *L, const char *path) {
  (void)path;  /* to avoid warnings */
  lua_pushliteral(L, DLMSG);
  return NULL;
}

static void *alien_loadfunc (lua_State *L, void *lib, const char *sym) {
  (void)lib; (void)sym;  /* to avoid warnings */
  lua_pushliteral(L, DLMSG);
  return NULL;
}

#endif

#ifndef WINDOWS
#define FFI_STDCALL FFI_DEFAULT_ABI
#endif

#ifdef _WIN64
#define FFI_STDCALL FFI_DEFAULT_ABI
#endif

#ifdef DARWIN
#define FFI_SYSV FFI_DEFAULT_ABI
#endif

static const ffi_abi ffi_abis[] = { FFI_DEFAULT_ABI, FFI_SYSV, FFI_STDCALL };
static const char *const ffi_abi_names[] = { "default", "cdecl", "stdcall", NULL };

static void *alien_checkudata(lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if(p == NULL) return NULL;
  if(!lua_getmetatable(L, ud)) return NULL;
  lua_getfield(L, LUA_REGISTRYINDEX, tname);
  if(!lua_rawequal(L, -1, -2)) {
    lua_pop(L, 2);
    return NULL;
  } else {
    lua_pop(L, 2);
    return p;
  }
}

static alien_Library *alien_checklibrary(lua_State *L, int index) {
  void *ud = alien_checkudata(L, index, ALIEN_LIBRARY_META);
  luaL_argcheck(L, ud != NULL, index, "alien library expected");
  return (alien_Library *)ud;
}

static alien_Function *alien_checkfunction(lua_State *L, int index) {
  void *ud = alien_checkudata(L, index, ALIEN_FUNCTION_META);
  luaL_argcheck(L, ud != NULL, index, "alien function expected");
  return (alien_Function *)ud;
}

static alien_Function *alien_tofunction(lua_State *L, int index) {
  void *ud = alien_checkudata(L, index, ALIEN_FUNCTION_META);
  if(ud) return (alien_Function *)ud;
  ud = alien_checkudata(L, index, ALIEN_CALLBACK_META);
  if(ud) {
    ffi_closure *ud2 = *((ffi_closure**)ud);
    return (alien_Function *)ud2->user_data;
  }
  luaL_argcheck(L, 0, index, "alien function or callback expected");
  return NULL;
}

static ffi_closure *alien_tocallback(lua_State *L, int index) {
  void *ud = alien_checkudata(L, index, ALIEN_CALLBACK_META);
  if(ud) return *((ffi_closure **)ud);
  ud = alien_checkudata(L, index, ALIEN_FUNCTION_META);
  if(ud) return ((alien_Function *)ud)->fn;
  luaL_argcheck(L, 0, index, "alien function or callback expected");
  return NULL;
}

static ffi_closure *alien_checkcallback(lua_State *L, int index) {
  void *ud = alien_checkudata(L, index, ALIEN_CALLBACK_META);
  luaL_argcheck(L, ud != NULL, index, "alien callback expected");
  return *((ffi_closure **)ud);
}

static int alien_iscallback(lua_State *L, int index) {
  void *ud = alien_checkudata(L, index, ALIEN_CALLBACK_META);
  return ud != NULL;
}

static char *alien_checkbuffer(lua_State *L, int index) {
  void *ud = alien_checkudata(L, index, ALIEN_BUFFER_META);
  if(ud == NULL) {
    void **ud2 = alien_checkudata(L, index, ALIEN_LBUFFER_META);
    ud = *ud2;
  }
  luaL_argcheck(L, ud != NULL, index, "alien buffer expected");
  return (char *)ud;
}

static int alien_get(lua_State *L) {
  char *name;
  const char *libname;
  size_t len;
  libname = luaL_checklstring(L, lua_gettop(L), &len);
  name = (char*)malloc(sizeof(char) * (len + 1));
  if(name) {
    void *lib;
    alien_Library *al;
    strcpy(name, libname);
    al = (alien_Library *)lua_newuserdata(L, sizeof(alien_Library));
    if(!al) luaL_error(L, "out of memory!");
    lib = alien_load(L, libname);
    if(lib) {
      lua_newtable(L);
      lua_setfenv(L, -2);
      luaL_getmetatable(L, ALIEN_LIBRARY_META);
      lua_setmetatable(L, -2);
      al->lib = lib;
      al->name = name;
      return 1;
    } else {
      lua_error(L);
    }
  } else {
    luaL_error(L, "out of memory!");
  }
  return 0;
}

static int alien_makefunction(lua_State *L, void *lib, void *fn, char *name) {
  alien_Function *af;
  af = (alien_Function *)lua_newuserdata(L, sizeof(alien_Function));
  if(af) {
    luaL_getmetatable(L, ALIEN_FUNCTION_META);
    lua_setmetatable(L, -2);
    af->fn = fn;
    af->name = name;
    af->lib = lib;
    af->nparams = 0;
    af->ret_type = AT_VOID;
    af->params = NULL;
    af->ffi_params = NULL;
  } else luaL_error(L, "out of memory!");
  return 1;
}

static int alien_library_get(lua_State *L) {
  const char *funcname;
  char *name;
  size_t len;
  alien_Library *al;
  int cache;
  al = alien_checklibrary(L, 1);
  funcname = luaL_checklstring(L, 2, &len);
  lua_getfenv(L, 1);
  cache = lua_gettop(L);
  lua_getfield(L, cache, funcname);
  if(!lua_isnil(L, -1)) return 1;
  name = (char*)malloc(sizeof(char) * (len + 1));
  if(name) {
    void *fn;
    strcpy(name, funcname);
    fn = alien_loadfunc(L, al->lib, funcname);
    if(fn) {
      alien_makefunction(L, al, fn, name);
      lua_newtable(L);
      lua_pushvalue(L, 1);
      lua_rawseti(L, -2, 1);
      lua_setfenv(L, -2);
      lua_pushvalue(L, -1);
      lua_setfield(L, cache, funcname);
      return 1;
    } else {
      free(name);
      lua_error(L);
    }
  } else {
    luaL_error(L, "out of memory!");
  }
  return 0;
}

static int alien_function_new(lua_State *L) {
  void *fn;
  if(lua_isuserdata(L, 1)) {
    void *fn = lua_touserdata(L, 1);
    return alien_makefunction(L, NULL, fn, NULL);
  } else luaL_error(L, "alien: not an userdata");
  return 0;
}

static int alien_library_tostring(lua_State *L) {
  alien_Library *al;
  al = alien_checklibrary(L, 1);
  lua_pushfstring(L, "alien library %s", (al->name ? al->name : "default"));
  return 1;
}

static void alien_callback_call(ffi_cif *cif, void *resp, void **args, void *data) {
  alien_Callback *ac;
  int nparams, i;
  void *ptr;
  ac = (alien_Callback *)data;
  lua_rawgeti(ac->L, LUA_REGISTRYINDEX, ac->fn_ref);
  nparams = ac->nparams;
  for(i = 0; i < nparams; i++) {
    switch(ac->params[i]) {
    case AT_BYTE: lua_pushnumber(ac->L, (signed char)*((int*)args[i])); break;
    case AT_CHAR: lua_pushnumber(ac->L, (uchar)*((int*)args[i])); break;
    case AT_SHORT: lua_pushnumber(ac->L, (short)*((int*)args[i])); break;
    case AT_LONG:
      lua_pushnumber(ac->L, (long)*((long*)args[i])); break;
    case AT_INT: lua_pushnumber(ac->L, *((int*)args[i])); break;
    case AT_USHORT: lua_pushnumber(ac->L, (unsigned short)*((unsigned int*)args[i])); break;
    case AT_ULONG:
      lua_pushnumber(ac->L, (unsigned long)*((unsigned long*)args[i])); break;
    case AT_UINT: lua_pushnumber(ac->L, *((unsigned int*)args[i])); break;
    case AT_FLOAT: lua_pushnumber(ac->L, (float)*((float*)args[i])); break;
    case AT_DOUBLE: lua_pushnumber(ac->L, *((double*)args[i])); break;
    case AT_STRING: lua_pushstring(ac->L, *((char**)args[i])); break;
    case AT_REFINT:
      lua_pushnumber(ac->L, **((int**)args[i])); break;
    case AT_REFUINT:
      lua_pushnumber(ac->L, **((unsigned int**)args[i])); break;
    case AT_REFCHAR:
      lua_pushnumber(ac->L, **((uchar**)args[i])); break;
    case AT_REFDOUBLE:
      lua_pushnumber(ac->L, **((double**)args[i])); break;
    case AT_PTR:
      ptr = *((void**)args[i]);
      ptr ? lua_pushlightuserdata(ac->L, ptr) : lua_pushnil(ac->L);
      break;
    default: luaL_error(ac->L, "alien: unknown parameter type in callback");
    }
  }
  lua_call(ac->L, nparams, 1);
  switch(ac->ret_type) {
  case AT_VOID: break;
  case AT_SHORT: *((int*)resp) = (short)lua_tonumber(ac->L, -1); break;
  case AT_LONG:
    *((long*)resp) = (long)lua_tonumber(ac->L, -1); break;
  case AT_INT: *((int*)resp) = (int)lua_tonumber(ac->L, -1); break;
  case AT_USHORT: *((unsigned int*)resp) = (unsigned short)lua_tonumber(ac->L, -1); break;
  case AT_ULONG:
    *((unsigned long*)resp) = (unsigned long)lua_tonumber(ac->L, -1); break;
  case AT_UINT: *((unsigned int*)resp) = (unsigned int)lua_tonumber(ac->L, -1); break;
  case AT_CHAR: *((int*)resp) = (uchar)lua_tointeger(ac->L, -1); break;
  case AT_BYTE: *((int*)resp) = (signed char)lua_tointeger(ac->L, -1); break;
  case AT_FLOAT: *((float*)resp) = (float)lua_tonumber(ac->L, -1); break;
  case AT_DOUBLE: *((double*)resp) = (double)lua_tonumber(ac->L, -1); break;
  case AT_STRING:
    if(lua_isuserdata(ac->L, -1))
      *((char**)resp) = lua_touserdata(ac->L, -1);
    else
      *((const char**)resp) = lua_tostring(ac->L, -1);
    break;
  case AT_PTR:
    if(lua_isstring(ac->L, -1))
      *((void**)resp) = (void*)lua_tostring(ac->L, -1);
    else
      *((void**)resp) = lua_touserdata(ac->L, -1);
    break;
  default: luaL_error(ac->L, "alien: unknown return type in callback");
  }
}

static int alien_callback_new(lua_State *L) {
  int fn_ref;
  alien_Callback *ac;
  ffi_closure **ud;
  ffi_status status;
  ffi_abi abi;
  luaL_checktype(L, 1, LUA_TFUNCTION);
  ac = (alien_Callback *)malloc(sizeof(alien_Callback));
  ud = (ffi_closure **)lua_newuserdata(L, sizeof(ffi_closure**));
  if(ac != NULL && ud != NULL) {
    int j;
    *ud = malloc_closure();
    if(*ud == NULL) { free(ac); luaL_error(L, "alien: cannot allocate callback"); }
    ac->L = L;
    ac->ret_type = AT_VOID;
    ac->ffi_ret_type = &ffi_type_void;
    abi = FFI_DEFAULT_ABI;
    ac->nparams = 0;
    ac->params = NULL;
    ac->ffi_params = NULL;
    lua_pushvalue(L, 1);
    ac->fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    luaL_getmetatable(L, ALIEN_CALLBACK_META);
    lua_setmetatable(L, -2);
    status = ffi_prep_cif(&(ac->cif), abi, ac->nparams,
                          ac->ffi_ret_type, ac->ffi_params);
    if(status != FFI_OK) luaL_error(L, "alien: cannot create callback");
    status = ffi_prep_closure(*ud, &(ac->cif), &alien_callback_call, ac);
    ac->fn = *ud;
    ac->lib = NULL;
    ac->name = NULL;
    if(status != FFI_OK) luaL_error(L, "alien: cannot create callback");
    return 1;
  } else {
    if(ac) free(ac);
    luaL_error(L, "alien: cannot allocate callback");
  }
  return 0;
}

static int alien_sizeof(lua_State *L) {
  static const int sizes[] = {sizeof(int), sizeof(double), sizeof(uchar), sizeof(char*),
                              sizeof(unsigned int), sizeof(unsigned short), sizeof(unsigned long), sizeof(unsigned int*),
                              sizeof(void*), sizeof(char),
                              sizeof(short), sizeof(long), sizeof(float),
                              sizeof(void*), sizeof(char*), sizeof(int*),
                              sizeof(double*)};
  static const char *const typenames[] = {"int", "double", "char", "string",
                                          "uint", "ushort", "ulong", "ref uint",
                                          "pointer", "byte", "short", "long",
                                          "float", "callback", "ref char",
                                          "ref int", "ref double", NULL};
  lua_pushnumber(L, sizes[luaL_checkoption(L, 1, "int", typenames)]);
  return 1;
}

static int alien_align(lua_State *L) {
  static const int aligns[] = {AT_INT_ALIGN, AT_DOUBLE_ALIGN, AT_CHAR_ALIGN, AT_CHAR_P_ALIGN,
                               AT_INT_ALIGN, AT_SHORT_ALIGN, AT_LONG_ALIGN, AT_VOID_P_ALIGN,
                               AT_VOID_P_ALIGN, AT_CHAR_ALIGN,
                              AT_SHORT_ALIGN, AT_LONG_ALIGN, AT_FLOAT_ALIGN,
                              AT_VOID_P_ALIGN, AT_CHAR_P_ALIGN, AT_VOID_P_ALIGN,
                               AT_VOID_P_ALIGN};
  static const char *const typenames[] = {"int", "double", "char", "string",
                                          "uint", "ushort", "ulong", "ref uint",
                                          "pointer", "byte", "short", "long",
                                          "float", "callback", "ref char",
                                          "ref int", "ref double", NULL};
  lua_pushnumber(L, aligns[luaL_checkoption(L, 1, "char", typenames)]);
  return 1;
}

static int alien_function_types(lua_State *L) {
  static ffi_type* ffitypes[] = {&ffi_type_void, &ffi_type_sint, &ffi_type_double,
                                 &ffi_type_uint, &ffi_type_ushort,
                                 &ffi_type_uchar, &ffi_type_pointer, &ffi_type_pointer,
                                 &ffi_type_pointer, &ffi_type_pointer, &ffi_type_pointer,
                                 &ffi_type_pointer, &ffi_type_sshort, &ffi_type_schar,
#ifndef WINDOWS
                                 &ffi_type_ulong,
                                 &ffi_type_slong,
#else
                                 &ffi_type_uint,
                                 &ffi_type_sint,
#endif
                                 &ffi_type_float, &ffi_type_pointer};
  static const int types[] = {AT_VOID, AT_INT, AT_DOUBLE, AT_UINT, AT_USHORT, AT_CHAR, AT_STRING, AT_PTR, AT_REFINT,
                              AT_REFDOUBLE, AT_REFCHAR, AT_CALLBACK, AT_SHORT, AT_BYTE, AT_ULONG, AT_LONG,
                              AT_FLOAT, AT_REFUINT};
  static const char *const typenames[] =
    {"void", "int", "double", "uint", "ushort", "char", "string", "pointer",
     "ref int", "ref double", "ref char", "callback",
     "short", "byte", "ulong", "long", "float", "ref uint", NULL};
  ffi_status status;
  ffi_abi abi;
  alien_Function *af = alien_tofunction(L, 1);
  int i, j, ret_type;
  if(lua_istable(L, 2)) {
    lua_getfield(L, 2, "ret");
    ret_type = luaL_checkoption(L, -1, "int", typenames);
    af->ret_type = types[ret_type];
    af->ffi_ret_type = ffitypes[ret_type];
    lua_getfield(L, 2, "abi");
    abi = ffi_abis[luaL_checkoption(L, -1, "default", ffi_abi_names)];
    lua_pop(L, 2);
  } else {
    ret_type = luaL_checkoption(L, 2, "int", typenames);
    af->ret_type = types[ret_type];
    af->ffi_ret_type = ffitypes[ret_type];
    abi = FFI_DEFAULT_ABI;
  }
  if(af->params) {
    free(af->params); free(af->ffi_params);
    af->params = NULL; af->ffi_params = NULL;
  }
  if(lua_istable(L, 2)) {
    af->nparams = lua_objlen(L, 2);
  } else {
    af->nparams = lua_gettop(L) - 2;
  }
  if(af->nparams > 0) {
    af->ffi_params = (ffi_type **)malloc(sizeof(ffi_type *) * af->nparams);
    if(!af->ffi_params) luaL_error(L, "alien: out of memory");
    af->params = (alien_Type *)malloc(af->nparams * sizeof(alien_Type));
    if(!af->params) luaL_error(L, "alien: out of memory");
  } else {
    af->ffi_params = NULL;
    af->params = NULL;
  }
  if(lua_istable(L, 2)) {
    for(i = 0, j = 1; i < af->nparams; i++, j++) {
      lua_rawgeti(L, 2, j);
      af->ffi_params[i] = ffitypes[luaL_checkoption(L, -1, "int", typenames)];
      af->params[i] = types[luaL_checkoption(L, -1, "int", typenames)];
      lua_pop(L, 1);
    }
  } else {
    for(i = 0, j = 3; i < af->nparams; i++, j++) {
      int type = luaL_checkoption(L, j, "int", typenames);
      af->ffi_params[i] = ffitypes[type];
      af->params[i] = types[type];
    }
  }
  status = ffi_prep_cif(&(af->cif), abi, af->nparams,
                        af->ffi_ret_type,
                        af->ffi_params);
  if(status != FFI_OK)
    luaL_error(L, "alien: error in libffi preparation");
  if(alien_iscallback(L, 1)) {
    alien_Callback *ac = (alien_Callback*)af;
    status = ffi_prep_closure(ac->fn, &(ac->cif), &alien_callback_call, ac);
    if(status != FFI_OK) luaL_error(L, "alien: cannot create callback");
  }
  return 0;
}

static int alien_function_tostring(lua_State *L) {
  alien_Function*af;
  af = alien_tofunction(L, 1);
  lua_pushfstring(L, "alien function %s, library %s", af->name ? af->name : "anonymous",
                   ((af->lib && af->lib->name) ? af->lib->name : "default"));
  return 1;
}

static int alien_function_call(lua_State *L) {
  int i, j, nargs, nparams;
  int iret; double dret; void *pret; long lret; unsigned long ulret; float fret;
  int *refi_args, nrefi, nrefui, nrefd, nrefc;
  double *refd_args;
  char *refc_args;
  unsigned int *refui_args;
  void **args;
  ffi_cif *cif;
  alien_Function *af = alien_tofunction(L, 1);
  cif = &(af->cif);
  nparams = af->nparams;
  nargs = lua_gettop(L) - 1;
  if(nargs < nparams)
    luaL_error(L, "alien: too few arguments (function %s)", af->name ?
               af->name : "anonymous");
  else if(nargs > nparams)
    luaL_error(L, "alien: too many arguments (function %s)", af->name ?
               af->name : "anonymous");
  for(i = 0, nrefi = 0, nrefui = 0, nrefd = 0, nrefc = 0; i < nparams; i++) {
    switch(af->params[i]) {
    case AT_REFINT: nrefi++; break;
    case AT_REFUINT: nrefui++; break;
    case AT_REFDOUBLE: nrefd++; break;
    case AT_REFCHAR: nrefc++; break;
    }
  }
  if(nrefi > 0) refi_args = (int*)ALLOCA(sizeof(int) * nrefi);
  if(nrefui > 0) refui_args = (unsigned int*)ALLOCA(sizeof(unsigned int) * nrefui);
  if(nrefd > 0) refd_args = (double*)ALLOCA(sizeof(double) * nrefd);
  if(nrefc > 0) refc_args = (char*)ALLOCA(sizeof(char) * nrefc);
  if(nargs > 0) args = ALLOCA(sizeof(void*) * nargs);
  for(i = 0, j = 2; i < nparams; i++, j++) {
    void *arg;
    switch(af->params[i]) {
    case AT_SHORT:
      arg = ALLOCA(sizeof(short)); *((short*)arg) = (short)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_LONG:
      arg = ALLOCA(sizeof(long)); *((long*)arg) = (long)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_INT:
      arg = ALLOCA(sizeof(int)); *((int*)arg) = (int)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_USHORT:
      arg = ALLOCA(sizeof(unsigned short)); *((unsigned short*)arg) = (unsigned short)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_ULONG:
      arg = ALLOCA(sizeof(unsigned long)); *((unsigned long*)arg) = (unsigned long)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_UINT:
      arg = ALLOCA(sizeof(unsigned int)); *((unsigned int*)arg) = (unsigned int)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_CHAR:
      arg = ALLOCA(sizeof(uchar)); *((uchar*)arg) = (uchar)lua_tointeger(L, j);
      args[i] = arg; break;
    case AT_BYTE:
      arg = ALLOCA(sizeof(char)); *((char*)arg) = (signed char)lua_tointeger(L, j);
      args[i] = arg; break;
    case AT_FLOAT:
      arg = ALLOCA(sizeof(float)); *((float*)arg) = (float)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_DOUBLE:
      arg = ALLOCA(sizeof(double)); *((double*)arg) = (double)lua_tonumber(L, j);
      args[i] = arg; break;
    case AT_STRING:
      arg = ALLOCA(sizeof(char*));
      if(lua_isuserdata(L, j))
        *((char**)arg) = lua_isnil(L, j) ? NULL : lua_touserdata(L, j);
      else
        *((const char**)arg) = lua_isnil(L, j) ? NULL : lua_tostring(L, j);
      args[i] = arg;
      break;
    case AT_CALLBACK:
      arg = ALLOCA(sizeof(void*));
      *((void**)arg) = alien_tocallback(L, j);
      args[i] = arg;
      break;
    case AT_PTR:
      arg = ALLOCA(sizeof(char*));
      *((void**)arg) = lua_isnil(L, j) ? NULL :
             (lua_isstring(L, j) ? (void*)lua_tostring(L, j) :
              lua_touserdata(L, j));
      args[i] = arg;
      break;
    case AT_REFINT:
      *refi_args = (int)lua_tonumber(L, j);
      arg = ALLOCA(sizeof(int*));
      *((int**)arg) = refi_args;
      args[i] = arg; refi_args++; break;
      break;
    case AT_REFUINT:
      *refui_args = (unsigned int)lua_tonumber(L, j);
      arg = ALLOCA(sizeof(unsigned int*));
      *((unsigned int**)arg) = refui_args;
      args[i] = arg; refui_args++; break;
      break;
    case AT_REFCHAR:
      *refc_args = (char)lua_tonumber(L, j);
      arg = ALLOCA(sizeof(char*));
      *((char**)arg) = refc_args;
      args[i] = arg; refc_args++; break;
      break;
    case AT_REFDOUBLE:
      *refd_args = lua_tonumber(L, j);
      arg = ALLOCA(sizeof(double*));
      *((double**)arg) = refd_args;
      args[i] = arg; refd_args++; break;
      break;
    default:
      luaL_error(L, "alien: parameter %i is of unknown type (function %s)", j,
                 af->name ? af->name : "anonymous");
    }
  }
  pret = NULL;
  switch(af->ret_type) {
  case AT_VOID: ffi_call(cif, af->fn, NULL, args); lua_pushnil(L); break;
  case AT_SHORT: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (short)iret); break;
  case AT_LONG:
    ffi_call(cif, af->fn, &lret, args); lua_pushnumber(L, lret); break;
  case AT_INT: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, iret); break;
  case AT_USHORT: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (unsigned short)iret); break;
  case AT_ULONG:
    ffi_call(cif, af->fn, &ulret, args); lua_pushnumber(L, (unsigned long)ulret); break;
  case AT_UINT: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (unsigned int)iret); break;
  case AT_CHAR: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (uchar)iret); break;
  case AT_BYTE: ffi_call(cif, af->fn, &iret, args); lua_pushnumber(L, (signed char)iret); break;
  case AT_FLOAT: ffi_call(cif, af->fn, &fret, args); lua_pushnumber(L, fret); break;
  case AT_DOUBLE: ffi_call(cif, af->fn, &dret, args); lua_pushnumber(L, dret); break;
  case AT_STRING: ffi_call(cif, af->fn, &pret, args);
    if(pret) lua_pushstring(L, (const char *)pret); else lua_pushnil(L); break;
  case AT_PTR: ffi_call(cif, af->fn, &pret, args);
    (pret ? lua_pushlightuserdata(L, pret) : lua_pushnil(L)); break;
  default:
    luaL_error(L, "alien: unknown return type (function %s)", af->name ?
               af->name : "anonymous");
  }
  refi_args -= nrefi; refd_args -= nrefd; refc_args -= nrefc; refui_args -= nrefui;
  for(i = 0; i < nparams; i++) {
    switch(af->params[i]) {
    case AT_REFINT: lua_pushnumber(L, *refi_args); refi_args++; break;
    case AT_REFUINT: lua_pushnumber(L, *refui_args); refui_args++; break;
    case AT_REFDOUBLE: lua_pushnumber(L, *refd_args); refd_args++; break;
    case AT_REFCHAR: lua_pushnumber(L, *refc_args); refc_args++; break;
    }
  }
  return 1 + nrefi + nrefui + nrefc + nrefd;
}

static int alien_library_gc(lua_State *L) {
  alien_Library *al = alien_checklibrary(L, 1);
  if(al->lib) {
    alien_unload(al->lib);
    al->lib = NULL;
    if(al->name) { free(al->name); al->name = NULL; }
  }
  return 0;
}

static int alien_function_gc(lua_State *L) {
  alien_Function *af = alien_checkfunction(L, 1);
  if(af->name) free(af->name);
  if(af->params) free(af->params);
  if(af->ffi_params) free(af->ffi_params);
  return 0;
}

static int alien_callback_gc(lua_State *L) {
  ffi_closure *ud = alien_checkcallback(L, 1);
  alien_Callback *ac = (alien_Callback *)ud->user_data;
  luaL_unref(ac->L, LUA_REGISTRYINDEX, ac->fn_ref);
  if(ac->params) free(ac->params);
  if(ac->ffi_params) free(ac->ffi_params);
  free_closure(ud);
  return 0;
}

static int alien_register(lua_State *L) {
  const char *meta = luaL_checkstring(L, 1);
  luaL_getmetatable(L, meta);
  if(lua_isnil(L, -1))
    luaL_newmetatable(L, meta);
  return 1;
}

static int alien_pack(lua_State *L) {
  int i, top;
  alien_Wrap *ud;
  const char *meta = luaL_checkstring(L, 1);
  ud = (alien_Wrap*)lua_newuserdata(L, sizeof(alien_Wrap) * lua_gettop(L));
  top = lua_gettop(L);
  for(i = 2; i < top ; i++) {
    if(lua_isnil(L, i)) {
      ud[i - 2].tag = AT_PTR;
      ud[i - 2].val.p = NULL;
    }
    else if(lua_isuserdata(L, i)) {
      ud[i - 2].tag = AT_PTR;
      ud[i - 2].val.p = lua_touserdata(L, i);
    } else {
      ud[i - 2].tag = AT_INT;
      ud[i - 2].val.i = lua_tointeger(L, i);
    }
  }
  ud[lua_gettop(L) - 2].tag = AT_VOID;
  luaL_getmetatable(L, meta);
  lua_setmetatable(L, -2);
  return 1;
}

static int alien_unpack(lua_State *L) {
  int size, i;
  alien_Wrap *ud;
  const char *meta = luaL_checkstring(L, 1);
  ud = (alien_Wrap *)luaL_checkudata(L, 2, meta);
  while(ud->tag != AT_VOID) {
    switch(ud->tag) {
    case AT_INT: lua_pushnumber(L, ud->val.i); break;
    case AT_PTR: ud->val.p ? lua_pushlightuserdata(L, ud->val.p) :
      lua_pushnil(L); break;
    default: luaL_error(L, "wrong type in wrapped value");
    }
    ud++;
  }
  return lua_gettop(L) - 2;
}

static int alien_repack(lua_State *L) {
  int size, i, top;
  alien_Wrap *ud;
  const char *meta = luaL_checkstring(L, 1);
  ud = (alien_Wrap *)luaL_checkudata(L, 2, meta);
  i = 3;
  top = lua_gettop(L);
  while(ud->tag != AT_VOID) {
    if(i > top || lua_isnil(L, i)) {
      ud->tag = AT_PTR;
      ud->val.p = NULL;
    }
    else if(lua_isuserdata(L, i)) {
      ud->tag = AT_PTR;
      ud->val.p = lua_touserdata(L, i);
    } else {
      ud->tag = AT_INT;
      ud->val.i = lua_tointeger(L, i);
    }
    ud++; i++;
  }
  return 0;
}

static int alien_buffer_new(lua_State *L) {
  const char *s; char *b;
  size_t size;
  if(lua_type(L, 1) == LUA_TSTRING) {
    s = lua_tolstring(L, 1, &size);
    size++;
  } else if(lua_type(L, 1) == LUA_TLIGHTUSERDATA) {
  	void *p = lua_touserdata(L, 1);
	void **ud = lua_newuserdata(L, sizeof(void*));
	*ud = p;
    luaL_getmetatable(L, ALIEN_LBUFFER_META);
    lua_setmetatable(L, -2);
    return 1;
  } else {
    s = NULL;
    size = luaL_optint(L, 1, BUFSIZ);
  }
  b = (char *)lua_newuserdata(L, size);
  if(b) {
    if(s) {
      memcpy(b, s, size - 1);
      b[size - 1] = '\0';
    }
    lua_newtable(L);
    lua_pushnumber(L, size);
    lua_setfield(L, -2, "size");
    lua_setfenv(L, -2);
    luaL_getmetatable(L, ALIEN_BUFFER_META);
    lua_setmetatable(L, -2);
    return 1;
  } else {
    luaL_error(L, "cannot allocate buffer");
  }
  return 0;
}

static int alien_buffer_tostring(lua_State *L) {
  char *b;
  int size, offset;
  b = alien_checkbuffer(L, 1);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2)) {
    size = strlen(b);
    offset = 0;
  } else {
    size = luaL_checkinteger(L, 2);
    offset = luaL_optint(L, 3, 1) - 1;
  }
  lua_pushlstring(L, b + offset, size);
  return 1;
}

static int alien_buffer_len(lua_State *L) {
  char *b = alien_checkbuffer(L, 1);
  lua_pushinteger(L, strlen(b));
  return 1;
}

static int alien_buffer_topointer(lua_State *L) {
  char *b = alien_checkbuffer(L, 1);
  int offset = luaL_optint(L, 2, 1) - 1;
  lua_pushlightuserdata(L, b + offset);
  return 1;
}

static int alien_buffer_put(lua_State *L);

static int alien_buffer_get(lua_State *L) {
  static const void* funcs[] = {&alien_buffer_tostring,
                                &alien_buffer_topointer,
                                &alien_buffer_len,
                                &alien_buffer_get,
                                &alien_buffer_put};
  static const char *const funcnames[] = { "tostring", "topointer", "len", "get", "set", NULL };
  static const int types[] = {AT_VOID, AT_INT, AT_DOUBLE, AT_CHAR, AT_STRING, AT_PTR, AT_REFINT,
                              AT_UINT, AT_USHORT, AT_ULONG, AT_REFUINT,
                              AT_REFDOUBLE, AT_REFCHAR, AT_CALLBACK, AT_SHORT, AT_BYTE, AT_LONG,
                              AT_FLOAT};
  static const char *const typenames[] =
    {"void", "int", "double", "char", "string", "pointer",
     "ref int", "uint", "ushort", "ulong", "ref uint", "ref double", "ref char", "callback",
     "short", "byte", "long", "float", NULL};
  char *b = alien_checkbuffer(L, 1);
  if(lua_type(L, 2) == LUA_TSTRING) {
    lua_getfenv(L, 1);
    if(!lua_isnil(L, -1))
      lua_getfield(L, -1, lua_tostring(L, 2));
    if(lua_isnil(L, -1))
      lua_pushcfunction(L,
                        (lua_CFunction)funcs[luaL_checkoption(L, 2, "tostring", funcnames)]);
  } else {
    void *p;
    int offset = luaL_checkinteger(L, 2) - 1;
    int type = types[luaL_checkoption(L, 3, "char", typenames)];
    switch(type) {
    case AT_SHORT: lua_pushnumber(L, *((short*)(&b[offset]))); break;
    case AT_INT: lua_pushnumber(L, *((int*)(&b[offset]))); break;
    case AT_LONG: lua_pushnumber(L, *((long*)(&b[offset]))); break;
    case AT_USHORT: lua_pushnumber(L, *((unsigned short*)(&b[offset]))); break;
    case AT_UINT: lua_pushnumber(L, *((unsigned int*)(&b[offset]))); break;
    case AT_ULONG: lua_pushnumber(L, *((unsigned long*)(&b[offset]))); break;
    case AT_BYTE: lua_pushnumber(L, (signed char)b[offset]); break;
    case AT_CHAR: lua_pushnumber(L, b[offset]); break;
    case AT_FLOAT: lua_pushnumber(L, *((float*)(&b[offset]))); break;
    case AT_DOUBLE: lua_pushnumber(L, *((double*)(&b[offset]))); break;
    case AT_STRING:
      p = *((void**)&b[offset]);
      if(p) lua_pushstring(L, (const char *)p); else lua_pushnil(L);
      break;
    case AT_CALLBACK:
      p = *((void**)&b[offset]);
      p ? alien_makefunction(L, NULL, p, NULL) : lua_pushnil(L);
      break;
    case AT_PTR:
      p = *((void**)&b[offset]);
      p ? lua_pushlightuserdata(L, p) : lua_pushnil(L);
      break;
    default:
      luaL_error(L, "alien: unknown type in buffer:get");
    }
  }
  return 1;
}

static int alien_buffer_put(lua_State *L) {
  static const int types[] = {AT_VOID, AT_INT, AT_DOUBLE, AT_CHAR, AT_STRING, AT_PTR, AT_REFINT,
                              AT_UINT, AT_USHORT, AT_ULONG, AT_REFUINT,
                              AT_REFDOUBLE, AT_REFCHAR, AT_CALLBACK, AT_SHORT, AT_BYTE, AT_LONG,
                              AT_FLOAT};
  static const char *const typenames[] =
    {"void", "int", "double", "char", "string", "pointer",
     "ref int", "uint", "ushort", "ulong", "ref uint", "ref double", "ref char", "callback",
     "short", "byte", "long", "float", NULL};
  char *b = alien_checkbuffer(L, 1);
  int offset = luaL_checkinteger(L, 2) - 1;
  int type = types[luaL_checkoption(L, 4, "char", typenames)];
  switch(type) {
  case AT_SHORT: *((short*)(&b[offset])) = (short)lua_tonumber(L, 3); break;
  case AT_INT: *((int*)(&b[offset])) = (int)lua_tonumber(L, 3); break;
  case AT_LONG: *((long*)(&b[offset])) = (long)lua_tonumber(L, 3); break;
  case AT_USHORT: *((unsigned short*)(&b[offset])) = (unsigned short)lua_tonumber(L, 3); break;
  case AT_UINT: *((unsigned int*)(&b[offset])) = (unsigned int)lua_tonumber(L, 3); break;
  case AT_ULONG: *((unsigned long*)(&b[offset])) = (unsigned long)lua_tonumber(L, 3); break;
  case AT_BYTE: b[offset] = (signed char)lua_tointeger(L, 3); break;
  case AT_CHAR: b[offset] = (char)lua_tointeger(L, 3); break;
  case AT_FLOAT: *((float*)(&b[offset])) = (float)lua_tonumber(L, 3); break;
  case AT_DOUBLE: *((double*)(&b[offset])) = (double)lua_tonumber(L, 3); break;
  case AT_PTR: {
        if(lua_isnil(L, 3) || lua_isuserdata(L, 3)) {
          *((void**)(&b[offset])) =
              (lua_isnil(L, 3) ? NULL : lua_touserdata(L, 3));
          break;
        }
      }
  case AT_STRING: {
       const char *s;
       size_t size;
       s = lua_tolstring(L, 3, &size);
       memcpy(*((char**)(&b[offset])), s, size + 1);
       break;
      }
  case AT_CALLBACK: *((void**)(&b[offset])) = alien_tocallback(L, 3); break;
  default:
    luaL_error(L, "alien: unknown type in buffer:put");
  }
  return 0;
}

static int alien_register_library_meta(lua_State *L) {
  luaL_newmetatable(L, ALIEN_LIBRARY_META);
  lua_pushliteral(L, "__gc");
  lua_pushcfunction(L, alien_library_gc);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, alien_library_tostring);
  lua_settable(L, -3);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, alien_library_get);
  lua_settable(L, -3);
  lua_pop(L, 1);
  return 0;
}

static int alien_register_callback_meta(lua_State *L) {
  luaL_newmetatable(L, ALIEN_CALLBACK_META);
  lua_pushliteral(L, "__index");
  lua_newtable(L);
  lua_pushliteral(L, "types");
  lua_pushcfunction(L, alien_function_types);
  lua_settable(L, -3);
  lua_settable(L, -3);
  lua_pushliteral(L, "__call");
  lua_pushcfunction(L, alien_function_call);
  lua_settable(L, -3);
  lua_pushliteral(L, "__gc");
  lua_pushcfunction(L, alien_callback_gc);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, alien_function_tostring);
  lua_settable(L, -3);
  lua_pop(L, 1);
  return 0;
}

static int alien_register_function_meta(lua_State *L) {
  luaL_newmetatable(L, ALIEN_FUNCTION_META);
  lua_pushliteral(L, "__index");
  lua_newtable(L);
  lua_pushliteral(L, "types");
  lua_pushcfunction(L, alien_function_types);
  lua_settable(L, -3);
  lua_settable(L, -3);
  lua_pushliteral(L, "__call");
  lua_pushcfunction(L, alien_function_call);
  lua_settable(L, -3);
  lua_pushliteral(L, "__gc");
  lua_pushcfunction(L, alien_function_gc);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, alien_function_tostring);
  lua_settable(L, -3);
  lua_pop(L, 1);
  return 0;
}

static int alien_register_buffer_meta(lua_State *L) {
  luaL_newmetatable(L, ALIEN_BUFFER_META);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, alien_buffer_get);
  lua_settable(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, alien_buffer_put);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, alien_buffer_tostring);
  lua_settable(L, -3);
  lua_pop(L, 1);
  luaL_newmetatable(L, ALIEN_LBUFFER_META);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, alien_buffer_get);
  lua_settable(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, alien_buffer_put);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, alien_buffer_tostring);
  lua_settable(L, -3);
  lua_pop(L, 1);
  return 0;
}

static int alien_errno(lua_State *L) {
  lua_pushnumber(L, errno);
  return 1;
}

static int alien_udata2str(lua_State *L) {
  char *ud;
  int size;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  ud = (char *)lua_touserdata(L, 1);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = strlen(ud);
  else
    size = luaL_checkinteger(L, 2);
  lua_pushlstring(L, ud, size);
  return 1;
}

static int alien_udata2double(lua_State *L) {
  double *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (double *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++)
    lua_pushnumber(L, ud[i]);
  return size;
}

static int alien_udata2int(lua_State *L) {
  int *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (int *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++) {
    lua_pushnumber(L, ud[i]);
  }
  return size;
}

static int alien_udata2uint(lua_State *L) {
  unsigned int *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (unsigned int *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++) {
    lua_pushnumber(L, ud[i]);
  }
  return size;
}

static int alien_udata2short(lua_State *L) {
  short *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (short *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++)
    lua_pushnumber(L, ud[i]);
  return size;
}

static int alien_udata2ushort(lua_State *L) {
  unsigned short *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (unsigned short *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++)
    lua_pushnumber(L, ud[i]);
  return size;
}

static int alien_udata2char(lua_State *L) {
  char *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (char *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++)
    lua_pushnumber(L, ud[i]);
  return size;
}

static int alien_udata2long(lua_State *L) {
  long *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (long *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++)
    lua_pushnumber(L, ud[i]);
  return size;
}

static int alien_udata2ulong(lua_State *L) {
  unsigned long *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (unsigned long *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++)
    lua_pushnumber(L, ud[i]);
  return size;
}

static int alien_udata2float(lua_State *L) {
  float *ud;
  int size, i;
  if(lua_isnil(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  if(lua_gettop(L) < 2 || lua_isnil(L, 2))
    size = 1;
  else
    size = luaL_checkinteger(L, 2);
  ud = (float *)lua_touserdata(L, 1);
  for(i = 0; i < size; i++)
    lua_pushnumber(L, ud[i]);
  return size;
}

static int alien_isnull(lua_State *L) {
  void *ud;
  luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
  ud = lua_touserdata(L, 1);
  lua_pushboolean(L, ud == NULL);
  return 1;
}

static int alien_table_new(lua_State *L) {
  int narray, nhash;
  narray = luaL_optint(L, 1, 0);
  nhash = luaL_optint(L, 2, 0);
  lua_createtable(L, narray, nhash);
  return 1;
}

static int alien_memcpy(lua_State *L) {
  void* dst;
  void* src;
  size_t size;
  dst = lua_touserdata(L, 1);
  if(!dst)
    luaL_typerror(L, 1, "userdata, or light userdata");
  if (!(lua_isuserdata(L, 2) || lua_isstring(L, 2)))
    luaL_typerror(L, 2, "string, userdata, or light userdata");
  if (lua_isuserdata(L, 2)) {
    src = lua_touserdata(L, 2);
    size = luaL_checkint(L, 3);
  } else {
    src = (void*)lua_tolstring(L, 2, &size);
    size = luaL_optint(L, 3, size);
  }
  if (size > 0)
    memcpy(dst, src, size);
  return 0;
}

static int alien_memset(lua_State *L) {
  void* dst;
  int c;
  size_t n;
  dst = lua_touserdata(L, 1);
  if(!dst)
    luaL_typerror(L, 1, "userdata, or light userdata");
  c = luaL_checkinteger(L, 2);
  n = luaL_checkinteger(L, 3);
  memset(dst, c, n);
  return 0;
}

static const luaL_Reg alienlib[] = {
  {"load", alien_get},
  {"align", alien_align},
  {"tag", alien_register},
  {"wrap", alien_pack},
  {"rewrap", alien_repack},
  {"unwrap", alien_unpack},
  {"errno", alien_errno},
  {"tostring", alien_udata2str},
  {"isnull", alien_isnull},
  {"sizeof", alien_sizeof},
  {"todouble", alien_udata2double},
  {"toint", alien_udata2int},
  {"tolong", alien_udata2long},
  {"touint", alien_udata2uint},
  {"toulong", alien_udata2ulong},
  {"tofloat", alien_udata2float},
  {"toshort", alien_udata2short},
  {"toushort", alien_udata2ushort},
  {"tochar", alien_udata2char},
  {"buffer", alien_buffer_new},
  {"callback", alien_callback_new},
  {"funcptr", alien_function_new},
  {"table", alien_table_new},
  {"memcpy", alien_memcpy },
  {"memset", alien_memcpy },
  {NULL, NULL},
};

static int alien_register_main(lua_State *L) {
  alien_Library *al;
  luaL_register (L, NULL, alienlib);
  lua_pushliteral(L, PLATFORM);
  lua_setfield(L, -2, "platform");
  al = (alien_Library *)lua_newuserdata(L, sizeof(alien_Library));
  al->lib = NULL;
  al->name = "default";
  luaL_getmetatable(L, ALIEN_LIBRARY_META);
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "default");
  return 1;
}

int luaopen_alien_core(lua_State *L) {
  alien_register_library_meta(L);
  alien_register_callback_meta(L);
  alien_register_function_meta(L);
  alien_register_buffer_meta(L);
  lua_getglobal(L, "alien");
  if(lua_isnil(L, -1)) {
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "alien");
  }
  lua_newtable(L);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, "core");
  alien_register_main(L);
  return 1;
}
