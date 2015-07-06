/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#include <sched.h>
#include <signal.h>
#include <sys/types.h>

#if __STDC_VERSION__ < 199901L
#define restrict
#endif

typedef void *posix_spawnattr_t;

enum {
  POSIX_SPAWN_RESETIDS,
  POSIX_SPAWN_SETPGROUP,
  POSIX_SPAWN_SETSCHEDPARAM,
  POSIX_SPAWN_SETSCHEDULER,
  POSIX_SPAWN_SETSIGDEF,
  POSIX_SPAWN_SETSIGMASK,
};

int posix_spawnattr_init(posix_spawnattr_t *attrp);
int posix_spawnattr_getflags(
  const posix_spawnattr_t *restrict attrp,
  short *restrict flags);
int posix_spawnattr_setflags(
  posix_spawnattr_t *attrp,
  short flags);
int posix_spawnattr_getpgroup(
  const posix_spawnattr_t *restrict attrp,
  pid_t *restrict pgroup);
int posix_spawnattr_setpgroup(
  posix_spawnattr_t *attrp,
  pid_t pgroup);
int posix_spawnattr_getschedparam(
  const posix_spawnattr_t *restrict attrp,
  struct sched_param *restrict schedparam);
int posix_spawnattr_setschedparam(
  posix_spawnattr_t *restrict attrp,
  const struct sched_param *restrict schedparam);
int posix_spawnattr_getschedpolicy(
  const posix_spawnattr_t *restrict attrp,
  int *restrict schedpolicy);
int posix_spawnattr_setschedpolicy(
  posix_spawnattr_t *attrp,
  int schedpolicy);
int posix_spawnattr_getsigdefault(
  const posix_spawnattr_t *restrict attrp,
  sigset_t *restrict sigdefault);
int posix_spawnattr_setsigdefault(
  posix_spawnattr_t *restrict attrp,
  const sigset_t *restrict sigdefault);
int posix_spawnattr_getsigmask(
  const posix_spawnattr_t *restrict attrp,
  sigset_t *restrict sigmask);
int posix_spawnattr_setsigmask(
  posix_spawnattr_t *restrict attrp,
  const sigset_t *restrict sigmask);
int posix_spawnattr_destroy(posix_spawnattr_t *attrp);

typedef struct posix_spawn_file_actions posix_spawn_file_actions_t;
struct posix_spawn_file_actions {
  int dups[3];
};

int posix_spawn_file_actions_init(posix_spawn_file_actions_t *file_actions);
int posix_spawn_file_actions_adddup2(
  posix_spawn_file_actions_t *file_actions,
  int filedes,
  int newfiledes);
int posix_spawn_file_actions_addclose(
  posix_spawn_file_actions_t *file_actions,
  int filedes);
int posix_spawn_file_actions_addopen(
  posix_spawn_file_actions_t *restrict file_actions,
  int filedes,
  const char *restrict path,
  int oflag,
  mode_t mode);
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *file_actions);

int posix_spawn(
  pid_t *restrict,
  const char *restrict,
  const posix_spawn_file_actions_t *,
  const posix_spawnattr_t *restrict,
  char *const argv[restrict],
  char *const envp[restrict]);
int posix_spawnp(
  pid_t *restrict,
  const char *restrict,
  const posix_spawn_file_actions_t *,
  const posix_spawnattr_t *restrict,
  char *const argv[restrict],
  char *const envp[restrict]);
