/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#ifndef SPAWN_H
#define SPAWN_H

#include <windows.h>
#include "lua.h"

#define PROCESS_HANDLE "process"
struct process;
struct spawn_params;

struct spawn_params *spawn_param_init(lua_State *L);
void spawn_param_filename(struct spawn_params *p);
void spawn_param_args(struct spawn_params *p);
void spawn_param_env(struct spawn_params *p);
void spawn_param_show(struct spawn_params *p, int show);
void spawn_param_useshell(struct spawn_params *p, int shell);
void spawn_param_redirect(
  struct spawn_params *p,
  const char *stdname,
  HANDLE h);
int spawn_param_execute(struct spawn_params *p);

int process_wait(lua_State *L);
int process_tostring(lua_State *L);
int process_close(lua_State *L);

#endif/*SPAWN_H*/
