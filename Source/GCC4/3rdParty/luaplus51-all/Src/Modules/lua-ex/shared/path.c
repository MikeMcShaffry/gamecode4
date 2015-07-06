#include <lua.h>
#include <lauxlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <io.h>
#include <malloc.h>
#include <direct.h>

#include "pusherror.h"
#define push_error(L) windows_pushlasterror(L)
#else
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern int push_error(lua_State *L);
#endif /* WIN32 */

/*
 * "os.path" API implementation
 */
int path_create(const char* inPath)
{
#if defined(WIN32)
  char path[MAX_PATH];
#else
  char path[FILENAME_MAX];
#endif
  char* pathPtr = path;
  char ch;

  if (*inPath == '/'  ||  *inPath == '\\') {
    *pathPtr++ = *inPath;
    inPath++;			// Skip the initial /
#if defined(WIN32)
    if (*inPath == '/'  ||  *inPath == '\\') {
      // UNC share
      *pathPtr++ = *inPath;
      inPath++;			// Skip the initial /
      // Copy the machine name.
      while ( (ch = *inPath++) ) {
        *pathPtr++ = ch;
        if (ch == '/'  ||  ch == '\\')
          break;
      }
      // Copy the share name.
      while ( (ch = *inPath++) ) {
        *pathPtr++ = ch;
        if (ch == '/'  ||  ch == '\\')
          break;
      }
    }
#endif
  }

  while ( (ch = *inPath++) ) {
    if (ch == '/'  ||  ch == '\\') {
      *pathPtr = 0;
#if defined(WIN32)
      {
        char* colonPtr;
        int isDriveLetter;

        /* Create the directory if it's not a drive letter. */
        colonPtr = pathPtr - 1;
        isDriveLetter = colonPtr == (path + 1)  &&  *colonPtr == ':';
        if (!isDriveLetter) {
          if (!CreateDirectory(path, NULL)  &&  GetLastError() != ERROR_ALREADY_EXISTS)
            return 0;
        }
        *pathPtr++ = '\\';
      }
#else
      if (mkdir(path, 0777)  &&  errno != EEXIST)
        return 0;
      *pathPtr++ = '/';
#endif
    } else
      *pathPtr++ = ch;
  }

  return 1;
}


int path_destroy(const char* inDirName)
{
#if defined(WIN32)
  char dirName[MAX_PATH];
  char* dirNamePtr = dirName;
  char ch;
  WIN32_FIND_DATA fd;
  HANDLE handle;

  if (*inDirName == 0)
    return 1;

  while (ch = *inDirName++) {
    if (ch == '/'  ||  ch == '\\')
      *dirNamePtr++ = '\\';
    else
      *dirNamePtr++ = ch;
  }
  if (dirNamePtr[-1] != '\\')
    *dirNamePtr++ = '\\';
  *dirNamePtr = 0;

  strcpy(dirNamePtr, "*.*");

  handle = FindFirstFile(dirName, &fd);
  if (handle == INVALID_HANDLE_VALUE)
    return 0;

  *dirNamePtr = 0;

  do {
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      int skipDir = fd.cFileName[0] == '.'  &&
        (fd.cFileName[1] == 0  ||  (fd.cFileName[1] == '.'  &&  fd.cFileName[2] == 0));
      if (!skipDir) {
        strcpy(dirNamePtr, fd.cFileName);
        strcat(dirNamePtr, "\\");
        if (!path_destroy(dirName))
          return 0;
      }
    } else {
      strcpy(dirNamePtr, fd.cFileName);
      SetFileAttributes(dirName, FILE_ATTRIBUTE_ARCHIVE);
      if (!DeleteFile(dirName)) {
        FindClose(handle);
        return 0;
      }
    }
  } while (FindNextFile(handle, &fd));

  FindClose(handle);

  *dirNamePtr = 0;
  if (!RemoveDirectory(dirName))
    return 0;
#else
  char dirName[FILENAME_MAX];
  char* dirNamePtr;
  char ch;

  if (*inDirName == 0)
    return 1;

  dirNamePtr = dirName;

  while (ch = *inDirName++) {
    if (ch == '/'  ||  ch == '\\')
      *dirNamePtr++ = '/';
    else
      *dirNamePtr++ = ch;
  }
  if (dirNamePtr[-1] != '/')
    *dirNamePtr++ = '/';
  *dirNamePtr = 0;

  DIR* dirp = opendir(dirName);
  if (!dirp)
    return 0;

  struct dirent* dp;
  while ((dp = readdir(dirp)) != NULL) {
    struct stat attr;
    strcpy(dirNamePtr, dp->d_name);
	if (stat(dirName, &attr) != -1  &&  (attr.st_mode & S_IFDIR)) {
      int skipDir = dp->d_name[0] == '.'  &&  (dp->d_name[1] == 0  ||  (dp->d_name[1] == '.'  &&  dp->d_name[2] == 0));
      if (!skipDir) {
        strcat(dirNamePtr, "\\");
        if (!path_destroy(dirName))
          return 0;
      }
    } else {
      //			::SetFileAttributes(dirName, FILE_ATTRIBUTE_ARCHIVE);
      //			if (!::DeleteFile(dirName))
      if (unlink(dirName) == -1) {
        closedir(dirp);
        return 0;
      }
    }
  }

  closedir(dirp);

  *dirNamePtr = 0;
  if (rmdir(dirName) == -1)
    return 0;
#endif

  return 1;
}


/* pathname access */
int ex_access(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  const char *type = luaL_optstring(L, 2, "");

  int amode = 0;
  if (strcmp(type, "w") == 0)
    amode = 2;
  else if (strcmp(type, "r") == 0)
    amode = 4;
  else if (strcmp(type, "rw") == 0)
    amode = 6;

  lua_pushboolean(L, access(pathname, amode) == 0);
  return 1;
}


/* pathname -- true/nil error */
int ex_mkdir(lua_State *L)
{
  const char *pathname = luaL_checkstring(L, 1);
  if (!path_create(pathname))
    return push_error(L);
  lua_pushboolean(L, 1);
  return 1;
}


static int path_append_helper(lua_State *L, int stackStart, int stackTop) {
  int concatBottom = stackTop;
  int newTop;
  int i;
  for (i = stackStart; i <= stackTop; ++i) {
    const char *path = luaL_checkstring(L, i);
    if (path[0] > 0) {
      if (*path == '/'  ||  *path == '\\'  ||  (path[0]  &&  path[1] == ':'))
        concatBottom = lua_gettop(L);

      lua_checkstack(L, 2);
      lua_pushvalue(L, i);
      lua_pushliteral(L, "/");
    }
  }

  newTop = lua_gettop(L);
  if (newTop - concatBottom > 0) {
    lua_remove(L, lua_gettop(L));
    --newTop;
  }
  lua_concat(L, newTop - concatBottom);
  return 1;
}


static int path_simplify_helper(lua_State *L, int index) {
  const char *path = luaL_checkstring(L, index);
  const char *pathEnd = path + strlen(path);
  char *file;
  char *filestart, *fileorg;

  filestart = fileorg = malloc(pathEnd - path + 1);
  memcpy(fileorg, path, pathEnd - path);
  fileorg[pathEnd - path] = 0;

  {
    char *ptr = filestart;
    char *endptr = filestart + (pathEnd - path);
    file = filestart;
    while ( ptr != endptr ) {
      // Skip './'
      if ( *ptr == '.' ) {
        if ( ptr[1] == 0  ||  ptr[1] == '/'  ||  ptr[1] == '\\' ) {
          int add = ptr[1] ? 1 : 0;
          ptr += 1 + add;
          if ( file == filestart ) {
            file += 1 + add;
            filestart += 1 + add;
          }
        } else if ( ptr[1] == '.'  &&  ( ptr[2] == 0  ||  ptr[2] == '/'  ||  ptr[2] == '\\' ) ) {
          // Go up a subdirectory.
          int add = ptr[2] ? 1 : 0;
          ptr += 2 + add;
          if ( file != filestart ) {
	    file--;
	    file -= (*file == '/' ? 1 : 0);
	    if ( file - filestart == 1  &&  *file == ':' )
              file += 2;
            else {
              while ( file >= filestart  &&  ( *file != '/'  &&  *file != '\\' ) )
                file--;
              file++;
            }
          } else {
            file += 2 + add;
//            filestart += 2 + add;
          }
        } else {
          *file++ = *ptr++;
        }
      } else if ( *ptr == '\\'  ||  *ptr == '/' ) {
        if ( file > filestart  &&  ( file[-1] == '/'  ||  file[-1] == '\\' ) ) {
          ptr++;
        } else {
#if defined(WIN32)
          /* is it a unc path? */
          if ( file == filestart  &&  (file[0] == '\\'  ||  file[0] == '/')  &&  (file[1] == '\\'  ||  file[1] == '/')) {
            file += 2;
            ptr += 2;
          } else 
#endif /* WIN32 */
		  {
            *file++ = '/';
            ptr++;
          }
        }
      } else {
        *file++ = *ptr++;
      }
    }
  }
  *file = 0;
//	file--;
//	if ( *file == '/' )
//    *file = 0;

  lua_pushstring(L, filestart);
  free(fileorg);
  return 1;
}


/**
	\internal
	\author Jack Handy

	Borrowed from http://www.codeproject.com/string/wildcmp.asp.
	Modified by Joshua Jensen.
**/
int WildMatch( const char* pattern, const char *string, int caseSensitive )
{
  const char* mp;
  const char* cp;

  // Handle all the letters of the pattern and the string.
  while ( *string != 0  &&  *pattern != '*' ) {
    if ( *pattern != '?' ) {
      if ( caseSensitive ) {
        if ( *pattern != *string )
          return 0;
      } else {
        if ( toupper( *pattern ) != toupper( *string ) )
          return 0;
      }
    }

    pattern++;
    string++;
  }

  mp = NULL;
  cp = NULL;

  while ( *string != 0 ) {
    if (*pattern == '*') {
      // It's a match if the wildcard is at the end.
      if ( *++pattern == 0 )
        return 1;

      mp = pattern;
      cp = string + 1;
    } else {
      if ( caseSensitive ) {
        if ( *pattern == *string  ||  *pattern == '?' ) {
          pattern++;
          string++;
        } else {
          pattern = mp;
          string = cp++;
        }
      } else {
        if ( toupper( *pattern ) == toupper( *string )  ||  *pattern == '?' ) {
          pattern++;
          string++;
        } else {
          pattern = mp;
          string = cp++;
        }
      }
    }
  }

  // Collapse remaining wildcards.
  while ( *pattern == '*' )
    pattern++;

  return !*pattern;
}


static int ex_path_add_slash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);
  if (path[0] == 0) {
    lua_pushstring(L, "/");
    return 1;
  }
  lua_pushvalue(L, 1);
  if (pathEnd[-1] != '\\'  &&  pathEnd[-1] != '/') {
    lua_pushliteral(L, "/");
    lua_concat(L, 2);
    return 1;
  }
  return 1;
}


static int ex_path_add_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *ext = luaL_checkstring(L, 2);
  (void)path; /* avoid warning about unused parameter */
  lua_pushvalue(L, 1);
  if (*ext != '.')
    lua_pushliteral(L, ".");
  lua_pushvalue(L, 2);
  lua_concat(L, 2 + (*ext == '.' ? 0 : 1));
  return 1;
}


static int ex_path_append(lua_State *L) {
  return path_append_helper(L, 1, lua_gettop(L));
}


static int ex_path_combine(lua_State *L) {
  ex_path_append(L);
  return path_simplify_helper(L, -1);
}


static int ex_path_escape(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *ptr = path;

#if defined(WIN32)
  while (*ptr  &&  *ptr != ' '  &&  *ptr != '/')
    ++ptr;

  if (*ptr == ' '  ||  *ptr == '/') {
    lua_pushliteral(L, "\"");
    lua_pushvalue(L, 1);
    lua_pushliteral(L, "\"");
    lua_concat(L, 3);
  } else
    lua_pushvalue(L, 1);
#else
  luaL_Buffer b;
  luaL_buffinit(L, &b);

  while (*ptr) {
    if (*ptr == ' '  ||  *ptr == '\\')
      luaL_addchar(&b, '\\');
    luaL_addchar(&b, *ptr);
    ++ptr;
  }

  luaL_pushresult(&b);
#endif
  return 1;
}


static int ex_path_exists(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  lua_pushboolean(L, access(path, 0) != -1);
  return 1;
}


static int ex_path_find_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '.'  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  if (*(pathEnd - 1) == '.')
    lua_pushinteger(L, pathEnd - path);
  else
    lua_pushnil(L);

  return 1;
}


static int ex_path_find_filename(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushinteger(L, pathEnd - path + 1);
  return 1;
}


static int ex_path_get_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '.'  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  if (*(pathEnd - 1) == '.')
    lua_pushstring(L, pathEnd - 1);
  else
    lua_pushliteral(L, "");

  return 1;
}


static int ex_path_get_filename(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushstring(L, pathEnd);
  return 1;
}


static int ex_path_is_directory(lua_State *L) {
  char *trimmedPath;
  struct stat sbuff;
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  (*(pathEnd - 1) == '/'  ||  *(pathEnd - 1) == '\\'))
    --pathEnd;

  trimmedPath = malloc(pathEnd - path + 1);
  memcpy(trimmedPath, path, pathEnd - path);
  trimmedPath[pathEnd - path] = 0;

  if (trimmedPath[0]  &&  stat(trimmedPath, &sbuff) == 0)
#if defined(WIN32)
    lua_pushboolean(L, (sbuff.st_mode & _S_IFDIR) > 0);
#else
    lua_pushboolean(L, S_ISDIR(sbuff.st_mode));
#endif
  else
    lua_pushboolean(L, 0);

  free(trimmedPath);

  return 1;
}


static int ex_path_is_file(lua_State *L) {
  struct stat sbuff;
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);
  if (*(pathEnd - 1) == '/'  ||  *(pathEnd - 1) == '\\')
    lua_pushnil(L);
  else if (path[0]  &&  stat(path, &sbuff) == 0)
#if defined(WIN32)
    lua_pushboolean(L, (sbuff.st_mode & _S_IFDIR) == 0);
#else
    lua_pushboolean(L, !S_ISDIR(sbuff.st_mode));
#endif
  else
    lua_pushnil(L);
  return 1;
}


static int ex_path_is_relative(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  if (*path == 0)
    lua_pushboolean(L, 1);
  else if (*path == '/'  ||  *path == '\\')
    lua_pushboolean(L, 0);
  else if (*(path + 1) == ':')
    lua_pushboolean(L, 0);
  else
    lua_pushboolean(L, 1);
  return 1;
}


static int ex_path_is_root(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  if (*path == 0)
    lua_pushboolean(L, 0);
  else if (*path == '/'  ||  *path == '\\')
    lua_pushboolean(L, 1);
  else if (*(path + 1) == ':')
    lua_pushboolean(L, 1);
  else
    lua_pushboolean(L, 0);
  return 1;
}


static int ex_path_is_unc(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  if (*path == 0)
    lua_pushboolean(L, 0);
  else if (strlen(path) < 3)
    lua_pushboolean(L, 0);
  else if ((path[0] == '/'  ||  path[0] == '\\')  &&  (path[1] == '/'  ||  path[1] == '\\'))
    lua_pushboolean(L, 1);
  else
    lua_pushboolean(L, 0);

  return 1;
}


static int ex_path_is_writable(lua_State *L) {
  struct stat sbuff;
  const char *path = luaL_checkstring(L, 1);
  if (path[0]  &&  stat(path, &sbuff) == 0)
#if defined(WIN32)
    lua_pushboolean(L, (sbuff.st_mode & _S_IWRITE) == _S_IWRITE);
#else
    lua_pushboolean(L, (sbuff.st_mode & S_IWRITE) == S_IWRITE);
#endif
  else
    lua_pushboolean(L, 0);
  return 1;
}


static int ex_path_match(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *spec = luaL_checkstring(L, 2);
  int caseSensitive = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : 0;

  lua_pushboolean(L, WildMatch(spec, path, caseSensitive));
  return 1;
}


static int ex_path_make_absolute(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *cwd = getcwd(NULL, 0);
  int start = lua_gettop(L);
  (void)path; /* avoid warning about unused parameter */

  lua_pushstring(L, cwd);
  lua_pushvalue(L, 1);
  path_append_helper(L, start, start + 2);
  return path_simplify_helper(L, -1);
}


static int ex_path_make_backslash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  char *newPath = malloc(pathEnd - path + 1);
  char *ptr = newPath;
  while (*path) {
    if (*path == '/') {
      *ptr++ = '\\';
      ++path;
    } else
      *ptr++ = *path++;
  }
  *ptr = 0;

  lua_pushstring(L, newPath);
  free(newPath);
  return 1;
}


static int ex_path_make_slash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  char *newPath = malloc(pathEnd - path + 1);
  char *ptr = newPath;
  while (*path) {
    if (*path == '\\') {
      *ptr++ = '/';
      ++path;
    } else
      *ptr++ = *path++;
  }
  *ptr = 0;

  lua_pushstring(L, newPath);
  free(newPath);
  return 1;
}


static int ex_path_make_writable(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

#if defined(WIN32)
  const char *pathname = luaL_checkstring(L, 1);
  int pmode = _S_IWRITE;
  if (-1 == _chmod(pathname, pmode))
    return push_error(L);
  lua_pushboolean(L, 1);
#else
  const char *pathname = luaL_checkstring(L, 1);
  int pmode = 0666;
  if (-1 == chmod(pathname, pmode))
    return push_error(L);
  lua_pushboolean(L, 1);
#endif
  return 1;
}


static int ex_path_replace_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);
  const char *ext = luaL_checkstring(L, 2);

  /* Search for extension */
  const char *ptr = pathEnd - 1;
  while (*ptr  &&  *ptr != '.')
    --ptr;
  if (*ptr == '.')
    pathEnd = ptr;

  lua_pushlstring(L, path, pathEnd - path);
  if (*ext != '.')
    lua_pushliteral(L, ".");
  lua_pushvalue(L, 2);
  lua_concat(L, 2 + (*ext == '.' ? 0 : 1));
  return 1;
}


static int ex_path_remove_directory(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  if (path == pathEnd)
    lua_pushstring(L, path);
  else
    lua_pushstring(L, pathEnd);
  return 1;
}


static int ex_path_remove_extension(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  /* Search for extension */
  const char *ptr = pathEnd - 1;
  while (*ptr  &&  *ptr != '.')
    --ptr;
  if (*ptr == '.')
    pathEnd = ptr;

  lua_pushlstring(L, path, pathEnd - path);
  return 1;
}


static int ex_path_remove_filename(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushlstring(L, path, pathEnd - path);
  return 1;
}


static int ex_path_remove_slash(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  (*(pathEnd - 1) == '\\'  ||  *(pathEnd - 1) == '/'))
    --pathEnd;

  lua_pushlstring(L, path, pathEnd - path);
  return 1;
}


static int ex_path_simplify(lua_State *L) {
  return path_simplify_helper(L, 1);
}


static int ex_path_split(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  const char *pathEnd = path + strlen(path);

  while (pathEnd > path  &&  *(pathEnd - 1) != '\\'  &&  *(pathEnd - 1) != '/'  &&  *(pathEnd - 1) != ':')
    --pathEnd;

  lua_pushlstring(L, path, pathEnd - path);
  lua_pushstring(L, pathEnd);
  return 2;
}


static int ex_path_trim(lua_State *L) {
  const char *pathStart = luaL_checkstring(L, 1);
  const char *pathEnd;

  /* trim the front */
  while (*pathStart == ' '  ||  *pathStart == '\t')
    ++pathStart;

  /* trim the end */
  pathEnd = pathStart + strlen(pathStart);
  while (pathEnd - 1 > pathStart  &&  (*(pathEnd - 1) == ' '  ||  *(pathEnd - 1) == '\t'))
    --pathEnd;

  lua_pushlstring(L, pathStart, pathEnd - pathStart);
  return 1;
}


static int ex_path_unescape(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

#if defined(WIN32)
  if (path[0] == '"') {
    const char* lastquote = path + 1;
    while (*lastquote  &&  *lastquote != '"')
      ++lastquote;
	  lua_pushlstring(L, path + 1, lastquote - (path + 1));
  } else {
    lua_pushvalue(L, 1);
  }
#else
  const char *ptr = path;
  luaL_Buffer b;
  luaL_buffinit(L, &b);

  while (*ptr) {
    if (*ptr != '\\')
      luaL_addchar(&b, *ptr);
    ++ptr;
  }

  luaL_pushresult(&b);
#endif
  return 1;
}


int luaopen_os_path(lua_State *L) {
  const luaL_reg ex_os_pathlib[] = {
    /* environment */
    {"add_slash",         ex_path_add_slash},
    {"add_extension",     ex_path_add_extension},
    {"append",            ex_path_append},
    {"combine",           ex_path_combine},
    {"escape",            ex_path_escape},
    {"exists",            ex_path_exists},
    {"find_extension",    ex_path_find_extension},
    {"find_filename",     ex_path_find_filename},
    {"get_extension",     ex_path_get_extension},
    {"get_filename",      ex_path_get_filename},
    {"is_directory",      ex_path_is_directory},
    {"is_file",           ex_path_is_file},
    {"is_relative",       ex_path_is_relative},
    {"is_root",           ex_path_is_root},
    {"is_unc",            ex_path_is_unc},
    {"is_writable",       ex_path_is_writable},
    {"make_absolute",     ex_path_make_absolute},
    {"make_backslash",    ex_path_make_backslash},
    {"make_slash",        ex_path_make_slash},
    {"make_writable",     ex_path_make_writable},
    {"match",             ex_path_match},
    {"remove_directory",  ex_path_remove_directory},
    {"remove_extension",  ex_path_remove_extension},
    {"remove_filename",   ex_path_remove_filename},
    {"remove_slash",      ex_path_remove_slash},
    {"replace_extension", ex_path_replace_extension},
    {"simplify",          ex_path_simplify},
    {"split",             ex_path_split},
    {"trim",              ex_path_trim},
    {"unescape",          ex_path_unescape},
    {0,0} };

  luaL_openlib(L, "os.path", ex_os_pathlib, 0);
  return 1;
}
