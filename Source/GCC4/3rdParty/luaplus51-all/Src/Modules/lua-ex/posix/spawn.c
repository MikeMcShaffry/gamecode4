/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#include <unistd.h>
#ifdef __APPLE__
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#else
ENVIRON_DECL
#endif
#include <sys/wait.h>
#if MISSING_POSIX_SPAWN
#include "posix_spawn.h"
#else
#include <spawn.h>
#endif

#include "lua.h"
#include "lauxlib.h"

#include "spawn.h"

struct spawn_params {
  lua_State *L;
  const char *command, **argv, **envp;
  posix_spawn_file_actions_t redirect;
  int useshell;
};

extern int push_error(lua_State *L);

struct spawn_params *spawn_param_init(lua_State *L)
{
  struct spawn_params *p = lua_newuserdata(L, sizeof *p);
  p->L = L;
  p->command = 0;
  p->argv = p->envp = 0;
  posix_spawn_file_actions_init(&p->redirect);
  p->useshell = 1;
  return p;
}

void spawn_param_filename(struct spawn_params *p, const char *filename)
{
  p->command = filename;
}

/* Converts a Lua array of strings to a null-terminated array of char pointers.
 * Pops a (0-based) Lua array and replaces it with a userdatum which is the
 * null-terminated C array of char pointers.  The elements of this array point
 * to the strings in the Lua array.  These strings should be associated with
 * this userdatum via a weak table for GC purposes, but they are not here.
 * Therefore, any function which calls this must make sure that these strings
 * remain available until the userdatum is thrown away.
 */
/* ... array -- ... vector */
static const char **make_vector(lua_State *L)
{
  size_t i, n = lua_objlen(L, -1);
  const char **vec = lua_newuserdata(L, (n + 2) * sizeof *vec);
                                        /* ... arr vec */
  for (i = 0; i <= n; i++) {
    lua_rawgeti(L, -2, i);              /* ... arr vec elem */
    vec[i] = lua_tostring(L, -1);
    if (!vec[i] && i > 0) {
      luaL_error(L, "expected string for argument %d, got %s",
                 i, lua_typename(L, lua_type(L, -1)));
      return 0;
    }
    lua_pop(L, 1);                      /* ... arr vec */
  }
  vec[n + 1] = 0;
  lua_replace(L, -2);                   /* ... vector */
  return vec;
}

/* ... argtab -- ... argtab vector */
void spawn_param_args(struct spawn_params *p)
{
  const char **argv = make_vector(p->L);
  if (!argv[0]) argv[0] = p->command;
  p->argv = argv;
}

void spawn_param_show(struct spawn_params *p, int show)
{
  (void)p;
  (void)show;
}

void spawn_param_useshell(struct spawn_params *p, int shell)
{
  p->useshell = shell;
}

/* ... envtab -- ... envtab vector */
void spawn_param_env(struct spawn_params *p)
{
  lua_State *L = p->L;
  size_t i = 0;
  lua_newtable(L);                      /* ... envtab arr */
  lua_pushliteral(L, "=");              /* ... envtab arr "=" */
  lua_pushnil(L);                       /* ... envtab arr "=" nil */
  for (i = 0; lua_next(L, -4); i++) {   /* ... envtab arr "=" k v */
    if (!lua_tostring(L, -2)) {
      luaL_error(L, "expected string for environment variable name, got %s",
                 lua_typename(L, lua_type(L, -2)));
      return;
    }
    if (!lua_tostring(L, -1)) {
      luaL_error(L, "expected string for environment variable value, got %s",
                 lua_typename(L, lua_type(L, -1)));
      return;
    }
    lua_pushvalue(L, -2);               /* ... envtab arr "=" k v k */
    lua_pushvalue(L, -4);               /* ... envtab arr "=" k v k "=" */
    lua_pushvalue(L, -3);               /* ... envtab arr "=" k v k "=" v */
    lua_concat(L, 3);                   /* ... envtab arr "=" k v "k=v" */
    lua_rawseti(L, -5, i);              /* ... envtab arr "=" k v */
    lua_pop(L, 1);                      /* ... envtab arr "=" k */
  }                                     /* ... envtab arr "=" */
  lua_pop(L, 1);                        /* ... envtab arr */
  p->envp = make_vector(L);             /* ... envtab arr vector */
}

void spawn_param_redirect(struct spawn_params *p, const char *stdname, int fd)
{
  int d;
  switch (stdname[3]) {
  case 'i': d = STDIN_FILENO; break;
  case 'o': d = STDOUT_FILENO; break;
  case 'e': d = STDERR_FILENO; break;
  }
  posix_spawn_file_actions_adddup2(&p->redirect, fd, d);
}

struct process {
  int status;
  pid_t pid;
};

int spawn_param_execute(struct spawn_params *p)
{
  lua_State *L = p->L;
  int ret;
  struct process *proc;
  if (!p->argv) {
    p->argv = lua_newuserdata(L, 2 * sizeof *p->argv);
    p->argv[0] = p->command;
    p->argv[1] = 0;
  }
  if (!p->envp)
    p->envp = (const char **)environ;
  proc = lua_newuserdata(L, sizeof *proc);
  luaL_getmetatable(L, PROCESS_HANDLE);
  lua_setmetatable(L, -2);
  proc->status = -1;
  ret = posix_spawnp(&proc->pid, p->command, &p->redirect, 0,
                     (char *const *)p->argv, (char *const *)p->envp);
  posix_spawn_file_actions_destroy(&p->redirect);
  return ret != 0 ? push_error(L) : 1;
}

/* proc -- exitcode/nil error */
int process_wait(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  if (p->status == -1) {
    int status;
    if (-1 == waitpid(p->pid, &status, 0))
      return push_error(L);
    p->status = WEXITSTATUS(status);
  }
  lua_pushnumber(L, p->status);
  return 1;
}

/* proc -- string */
int process_tostring(lua_State *L)
{
  struct process *p = luaL_checkudata(L, 1, PROCESS_HANDLE);
  char buf[40];
  lua_pushlstring(L, buf,
    sprintf(buf, "process (%lu, %s)", (unsigned long)p->pid,
      p->status==-1 ? "running" : "terminated"));
  return 1;
}
