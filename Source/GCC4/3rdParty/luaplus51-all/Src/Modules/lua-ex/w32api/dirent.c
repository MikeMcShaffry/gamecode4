/*
 * "ex" API implementation
 * http://lua-users.org/wiki/ExtensionProposal
 * Copyright 2007 Mark Edgar < medgar at student gc maricopa edu >
 */
#include <windows.h>
#include <stdlib.h>
#include "dirent.h"

struct DIR_tag {
  int first;
  HANDLE hf;
  WIN32_FIND_DATA fd;
};

static char *mkpattern(const char *name)
{
  static const char suffix[] = "\\*";
  size_t len = strlen(name);
  char *pattern = malloc(len + sizeof suffix);
  if (pattern)
    strcpy((char*)memcpy(pattern, name, len) + len, suffix);
  return pattern;
}

DIR *opendir(const char *name)
{
  DIR *pi = malloc(sizeof *pi);
  char *pattern = mkpattern(name);
  if (!pi || !pattern
      || INVALID_HANDLE_VALUE == (pi->hf = FindFirstFile(pattern, &pi->fd))) {
    free(pi);
    pi = 0;
  }
  else {
    pi->first = 1;
  }
  free(pattern);
  return pi;
}

static int isdotfile(const char *name)
{
  return name[0] == '.' && (name[1] == 0
    || (name[1] == '.' && name[2] == 0));
}

const WIN32_FIND_DATA *readdir(DIR *pi)
{
  switch (pi->first) do {
  default:
    if (!FindNextFile(pi->hf, &pi->fd)) {
      FindClose(pi->hf);
      pi->hf = INVALID_HANDLE_VALUE;
      return 0;
    }
  case 1: pi->first = 0;
  } while (isdotfile(pi->fd.cFileName));
  return &pi->fd;
}

void closedir(DIR *pi)
{
  if (pi->hf != INVALID_HANDLE_VALUE) {
      FindClose(pi->hf);
      pi->hf = INVALID_HANDLE_VALUE;
  }
  free(pi);
}
