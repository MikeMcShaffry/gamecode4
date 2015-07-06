/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#include <stdlib.h>
#include <windows.h>
#include <io.h>

#include "lua.h"
#include "lauxlib.h"

#include "spawn.h"
#include "pusherror.h"

#if _MSC_VER  &&  _MSC_VER <= 1300
#define debug() /* fprintf(stderr, __VA_ARGS__) */
#define debug_stack(L) /* #include "../lds.c" */
#else
#define debug(...) /* fprintf(stderr, __VA_ARGS__) */
#define debug_stack(L) /* #include "../lds.c" */
#endif

#define file_handle(fp) (HANDLE)_get_osfhandle(fileno(fp))

struct spawn_params {
  lua_State *L;
  const char *cmdline;
  const char *environment;
  STARTUPINFO si;
  int useshell;
};

/* quotes and adds argument string to b */
static int add_argument(luaL_Buffer *b, const char *s) {
  int oddbs = 0;
//  luaL_addchar(b, '"');
  for (; *s; s++) {
    switch (*s) {
//    case '\\':
//      luaL_addchar(b, '\\');
//      oddbs = !oddbs;
//      break;
//    case '"':
//      luaL_addchar(b, '\\');
//      oddbs = 0;
//      break;
    default:
      oddbs = 0;
      break;
    }
    luaL_addchar(b, *s);
  }
//  luaL_addchar(b, '"');
  return oddbs;
}

struct spawn_params *spawn_param_init(lua_State *L)
{
  static const STARTUPINFO si = {sizeof si, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, STARTF_USESHOWWINDOW, SW_HIDE, 0, NULL, 0, 0, 0 };
  struct spawn_params *p = lua_newuserdata(L, sizeof *p);
  p->L = L;
  p->cmdline = p->environment = 0;
  p->si = si;
  p->useshell = 1;
  return p;
}

/* cmd ... -- cmd ... */
void spawn_param_filename(struct spawn_params *p)
{
  lua_State *L = p->L;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  if (add_argument(&b, lua_tostring(L, 1))) {
    luaL_error(L, "argument ends in odd number of backslashes");
    return;
  }
  luaL_pushresult(&b);
  lua_replace(L, 1);
  p->cmdline = lua_tostring(L, 1);
}

/* cmd ... argtab -- cmdline ... */
void spawn_param_args(struct spawn_params *p)
{
  lua_State *L = p->L;
  int argtab = lua_gettop(L);
  size_t i, n = lua_objlen(L, argtab);
  luaL_Buffer b;
  debug("spawn_param_args:"); debug_stack(L);
  lua_pushnil(L);                 /* cmd opts ... argtab nil */
  luaL_buffinit(L, &b);           /* cmd opts ... argtab nil b... */
  lua_pushvalue(L, 1);            /* cmd opts ... argtab nil b... cmd */
  luaL_addvalue(&b);              /* cmd opts ... argtab nil b... */
  /* concatenate the arg array to a string */
  for (i = 1; i <= n; i++) {
    const char *s;
    lua_rawgeti(L, argtab, i);    /* cmd opts ... argtab nil b... arg */
    lua_replace(L, argtab + 1);   /* cmd opts ... argtab arg b... */
    luaL_addchar(&b, ' ');
    /* XXX checkstring is confusing here */
    s = lua_tostring(L, argtab + 1);
    if (!s) {
      luaL_error(L, "expected string for argument %d, got %s",
                 i, lua_typename(L, lua_type(L, argtab + 1)));
      return;
    }
    add_argument(&b, luaL_checkstring(L, argtab + 1));
  }
  luaL_pushresult(&b);            /* cmd opts ... argtab arg cmdline */
  lua_replace(L, 1);              /* cmdline opts ... argtab arg */
  lua_pop(L, 2);                  /* cmdline opts ... */
  p->cmdline = lua_tostring(L, 1);
}

void spawn_param_show(struct spawn_params *p, int show)
{
  p->si.wShowWindow = show ? SW_SHOW : SW_HIDE;
}

void spawn_param_useshell(struct spawn_params *p, int shell)
{
  p->useshell = shell;
}

/* ... tab nil nil [...] -- ... tab envstr */
static char *add_env(lua_State *L, int tab, size_t where) {
  char *t;
  lua_checkstack(L, 2);
  lua_pushvalue(L, -2);
  if (lua_next(L, tab)) {
    size_t klen, vlen;
    const char *key = lua_tolstring(L, -2, &klen);
    const char *val = lua_tolstring(L, -1, &vlen);
    t = add_env(L, tab, where + klen + vlen + 2);
    memcpy(&t[where], key, klen);
    t[where += klen] = '=';
    memcpy(&t[where + 1], val, vlen + 1);
  }
  else {
    t = lua_newuserdata(L, where + 1);
    t[where] = '\0';
    lua_replace(L, tab + 1);
  }
  return t;
}

/* ... envtab -- ... envtab envstr */
void spawn_param_env(struct spawn_params *p)
{
  lua_State *L = p->L;
  int envtab = lua_gettop(L);
  lua_pushnil(L);
  lua_pushnil(L);
  p->environment = add_env(L, envtab, 0);
  lua_settop(L, envtab + 1);
}

void spawn_param_redirect(struct spawn_params *p, const char *stdname, HANDLE h)
{
  SetHandleInformation(h, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
  if (!(p->si.dwFlags & STARTF_USESTDHANDLES)) {
    p->si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    p->si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    p->si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
    p->si.dwFlags |= STARTF_USESTDHANDLES;
  }
  switch (stdname[3]) {
  case 'i': p->si.hStdInput = h; break;
  case 'o': p->si.hStdOutput = h; break;
  case 'e': p->si.hStdError = h; break;
  }
}

struct process {
  int status;
  HANDLE hProcess;
  DWORD dwProcessId;
};

int spawn_param_execute(struct spawn_params *p)
{
  lua_State *L = p->L;
  char *c, *e;
  const char* comspec;
  PROCESS_INFORMATION pi;
  BOOL ret;
  struct process *proc = lua_newuserdata(L, sizeof *proc);
  luaL_getmetatable(L, PROCESS_HANDLE);
  lua_setmetatable(L, -2);
  proc->status = -1;

  if (p->useshell) {
    comspec = getenv("COMSPEC");
    if (!comspec)
      comspec = "cmd.exe";

    c = (char*)malloc(1 + strlen(comspec) + 1 + 4 + strlen(p->cmdline) + 2 + 1);
    strcpy(c, "\"");
    strcat(c, comspec);
    strcat(c, "\" /C ");
    strcat(c, "\"");
    strcat(c, p->cmdline);
    strcat(c, "\"");
  } else {
    c = strdup(p->cmdline);
  }

  e = (char *)p->environment; /* strdup(p->environment); */
  /* XXX does CreateProcess modify its environment argument? */
  ret = CreateProcess(0, c, 0, 0, TRUE, 0, e, 0, &p->si, &pi);
  if (ret)
	CloseHandle(pi.hThread);
  /* if (e) free(e); */
  free(c);
  if (!ret)
    return windows_pushlasterror(L);
  proc->hProcess = pi.hProcess;
  proc->dwProcessId = pi.dwProcessId;
  return 1;
}

/* proc -- exitcode/nil error */
int process_wait(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  DWORD timeout = luaL_optinteger(L, 2, INFINITE);
  if (p->status == -1) {
    DWORD exitcode;
	DWORD result = WaitForSingleObject(p->hProcess, timeout);
	if (WAIT_TIMEOUT == result)
		return 0;
    if (WAIT_FAILED == result
        || !GetExitCodeProcess(p->hProcess, &exitcode))
      return windows_pushlasterror(L);
    p->status = exitcode;
  }
  process_close(L);
  lua_pushnumber(L, p->status);
  return 1;
}

/* proc -- string */
int process_tostring(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  char buf[40];
  lua_pushlstring(L, buf,
    sprintf(buf, "process (%lu, %s)", (unsigned long)p->dwProcessId,
        p->dwProcessId ? (p->status==-1 ? "running" : "terminated") : "terminated"));
  return 1;
}

/* proc -- exitcode/nil error */
int process_close(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  if (p->hProcess != INVALID_HANDLE_VALUE) {
    if (p->status == -1) {
      DWORD exitcode;
      GetExitCodeProcess(p->hProcess, &exitcode);
      p->status = exitcode;
	}

    CloseHandle(p->hProcess);
    p->hProcess = INVALID_HANDLE_VALUE;
  }
  return 0;
}

