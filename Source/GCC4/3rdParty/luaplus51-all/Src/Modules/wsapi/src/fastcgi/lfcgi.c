/*
** FastCGI Input/Output library.
**
** $Id: lfcgi.c,v 1.6 2008/04/24 02:58:43 mascarenhas Exp $
*/


#include "fcgi_stdio.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define lfcgiio_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include "lfcgi.h"

/*
** by default, gcc does not get `tmpname'
*/
#ifndef USE_TMPNAME
#ifdef __GNUC__
#define USE_TMPNAME	0
#else
#define USE_TMPNAME	1
#endif
#endif


/*
** by default, posix systems get `popen'
*/
#ifndef USE_POPEN
#ifdef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE >= 2
#define USE_POPEN	1
#endif
#endif
#endif

#ifndef USE_POPEN
#define USE_POPEN	0
#endif




/*
** {======================================================
** FILE Operations
** =======================================================
*/


#if !USE_POPEN
#define pclose(f)    (-1)
#endif


#define FILEHANDLE		"FCGI_FILE*"

#define IO_INPUT		"_input"
#define IO_OUTPUT		"_output"

#ifndef _WIN32
extern char **environ;
#endif

static char **old_env;

static int pushresult (lua_State *L, int i, const char *filename) {
  if (i) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (filename)
      lua_pushfstring(L, "%s: %s", filename, strerror(errno));
    else
      lua_pushfstring(L, "%s", strerror(errno));
    lua_pushnumber(L, errno);
    return 3;
  }
}


static FILE **topfile (lua_State *L, int findex) {
  FILE **f = (FILE **)luaL_checkudata(L, findex, FILEHANDLE);
  if (f == NULL) luaL_argerror(L, findex, "bad file");
  return f;
}


static int io_type (lua_State *L) {
  FILE **f = (FILE **)luaL_checkudata(L, 1, FILEHANDLE);
  if (f == NULL) lua_pushnil(L);
  else if (*f == NULL)
    lua_pushliteral(L, "closed file");
  else
    lua_pushliteral(L, "file");
  return 1;
}


static FILE *tofile (lua_State *L, int findex) {
  FILE **f = topfile(L, findex);
  if (*f == NULL)
    luaL_error(L, "attempt to use a closed file");
  return *f;
}



/*
** When creating file handles, always creates a `closed' file handle
** before opening the actual file; so, if there is a memory error, the
** file is not left opened.
*/
static FILE **newfile (lua_State *L) {
  FILE **pf = (FILE **)lua_newuserdata(L, sizeof(FILE *));
  *pf = NULL;  /* file handle is currently `closed' */
  luaL_getmetatable(L, FILEHANDLE);
  lua_setmetatable(L, -2);
  return pf;
}


/*
** assumes that top of the stack is the `io' library, and next is
** the `io' metatable
*/
static void registerfile (lua_State *L, FILE *f, const char *name,
                                                 const char *impname) {
  lua_pushstring(L, name);
  *newfile(L) = f;
  if (impname) {
    lua_pushstring(L, impname);
    lua_pushvalue(L, -2);
    lua_settable(L, -6);  /* metatable[impname] = file */
  }
  lua_settable(L, -3);  /* io[name] = file */
}


static int aux_close (lua_State *L) {
  FILE *f = tofile(L, 1);
  if (f == stdin || f == stdout || f == stderr)
    return 0;  /* file cannot be closed */
  else {
    int ok = (pclose(f) != -1) || (fclose(f) == 0);
    if (ok)
      *(FILE **)lua_touserdata(L, 1) = NULL;  /* mark file as closed */
    return ok;
  }
}


static int io_close (lua_State *L) {
  if (lua_isnone(L, 1) && lua_type(L, lua_upvalueindex(1)) == LUA_TTABLE) {
    lua_pushstring(L, IO_OUTPUT);
    lua_rawget(L, lua_upvalueindex(1));
  }
  return pushresult(L, aux_close(L), NULL);
}


static int io_gc (lua_State *L) {
  FILE **f = topfile(L, 1);
  if (*f != NULL)  /* ignore closed files */
    aux_close(L);
  return 0;
}


static int io_tostring (lua_State *L) {
  char buff[128];
  FILE **f = topfile(L, 1);
  if (*f == NULL)
    strcpy(buff, "closed");
  else
    sprintf(buff, "%p", lua_touserdata(L, 1));
  lua_pushfstring(L, "file (%s)", buff);
  return 1;
}


static int io_open (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  FILE **pf = newfile(L);
  *pf = fopen(filename, mode);
  return (*pf == NULL) ? pushresult(L, 0, filename) : 1;
}


static int io_popen (lua_State *L) {
#if !USE_POPEN
  luaL_error(L, "`popen' not supported");
  return 0;
#else
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  FILE **pf = newfile(L);
  *pf = popen(filename, mode);
  return (*pf == NULL) ? pushresult(L, 0, filename) : 1;
#endif
}


static int io_tmpfile (lua_State *L) {
  FILE **pf = newfile(L);
  *pf = tmpfile();
  return (*pf == NULL) ? pushresult(L, 0, NULL) : 1;
}


static FILE *getiofile (lua_State *L, const char *name) {
  lua_pushstring(L, name);
  lua_rawget(L, lua_upvalueindex(1));
  return tofile(L, -1);
}


static int g_iofile (lua_State *L, const char *name, const char *mode) {
  if (!lua_isnoneornil(L, 1)) {
    const char *filename = luaL_checkstring(L, 1);
    lua_pushstring(L, name);
    if (filename) {
      FILE **pf = newfile(L);
      *pf = fopen(filename, mode);
      if (*pf == NULL) {
        lua_pushfstring(L, "%s: %s", filename, strerror(errno));
        luaL_argerror(L, 1, lua_tostring(L, -1));
      }
    }
    else {
      tofile(L, 1);  /* check if it is a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_rawset(L, lua_upvalueindex(1));
  }
  /* return current value */
  lua_pushstring(L, name);
  lua_rawget(L, lua_upvalueindex(1));
  return 1;
}


static int io_input (lua_State *L) {
  return g_iofile(L, IO_INPUT, "r");
}


static int io_output (lua_State *L) {
  return g_iofile(L, IO_OUTPUT, "w");
}


static int io_readline (lua_State *L);


static void aux_lines (lua_State *L, int idx, int close_it) {
  lua_pushliteral(L, FILEHANDLE);
  lua_rawget(L, LUA_REGISTRYINDEX);
  lua_pushvalue(L, idx);
  lua_pushboolean(L, close_it);  /* close/not close file when finished */
  lua_pushcclosure(L, io_readline, 3);
}


static int f_lines (lua_State *L) {
  tofile(L, 1);  /* check if it is a valid file handle */
  aux_lines(L, 1, 0);
  return 1;
}


static int io_lines (lua_State *L) {
  if (lua_isnoneornil(L, 1)) {  /* no arguments? */
    lua_pushstring(L, IO_INPUT);
    lua_rawget(L, lua_upvalueindex(1));  /* will iterate over default input */
    return f_lines(L);
  }
  else {
    const char *filename = luaL_checkstring(L, 1);
    FILE **pf = newfile(L);
    *pf = fopen(filename, "r");
    luaL_argcheck(L, *pf, 1,  strerror(errno));
    aux_lines(L, lua_gettop(L), 1);
    return 1;
  }
}


/*
** {======================================================
** READ
** =======================================================
*/


static int read_number (lua_State *L, FILE *f) {
  lua_Number d;
  /*if (fscanf(f, LUA_NUMBER_SCAN, &d) == 1) {*/
  char buf[32], ch;
  char *p = buf;
  for (;;) {
    ch = fgetc (f);
    if ((int)ch == EOF)
      break;
    if (strchr ("0123456789-.", ch))
      break;
    *p++ = ch;
  }
  *p = '\0';
  if (p != buf) {
    d = strtod (buf, NULL);
    lua_pushnumber(L, d);
    return 1;
  }
  else return 0;  /* read fails */
}


static int test_eof (lua_State *L, FILE *f) {
  int c = getc(f);
  ungetc(c, f);
  lua_pushlstring(L, NULL, 0);
  return (c != EOF);
}


static int read_line (lua_State *L, FILE *f) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  for (;;) {
    size_t l;
    char *p = luaL_prepbuffer(&b);
    if (fgets(p, LUAL_BUFFERSIZE, f) == NULL) {  /* eof? */
      luaL_pushresult(&b);  /* close buffer */
      return (lua_strlen(L, -1) > 0);  /* check whether read something */
    }
    l = strlen(p);
    if (p[l-1] != '\n')
      luaL_addsize(&b, l);
    else {
      luaL_addsize(&b, l - 1);  /* do not include `eol' */
      luaL_pushresult(&b);  /* close buffer */
      return 1;  /* read at least an `eol' */
    }
  }
}


static int read_chars (lua_State *L, FILE *f, size_t n) {
  size_t rlen;  /* how much to read */
  size_t nr;  /* number of chars actually read */
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  rlen = LUAL_BUFFERSIZE;  /* try to read that much each time */
  do {
    char *p = luaL_prepbuffer(&b);
    if (rlen > n) rlen = n;  /* cannot read more than asked */
    nr = fread(p, sizeof(char), rlen, f);
    luaL_addsize(&b, nr);
    n -= nr;  /* still have to read `n' chars */
  } while (n > 0 && nr == rlen);  /* until end of count or eof */
  luaL_pushresult(&b);  /* close buffer */
  return (n == 0 || lua_strlen(L, -1) > 0);
}


static int g_read (lua_State *L, FILE *f, int first) {
  int nargs = lua_gettop(L) - 1;
  int success;
  int n;
  if (nargs == 0) {  /* no arguments? */
    success = read_line(L, f);
    n = first+1;  /* to return 1 result */
  }
  else {  /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)lua_tonumber(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      else {
        const char *p = lua_tostring(L, n);
        luaL_argcheck(L, p && p[0] == '*', n, "invalid option");
        switch (p[1]) {
          case 'n':  /* number */
            success = read_number(L, f);
            break;
          case 'l':  /* line */
            success = read_line(L, f);
            break;
          case 'a':  /* file */
            read_chars(L, f, ~((size_t)0));  /* read MAX_SIZE_T chars */
            success = 1; /* always success */
            break;
          case 'w':  /* word */
            return luaL_error(L, "obsolete option `*w' to `read'");
          default:
            return luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  if (!success) {
    lua_pop(L, 1);  /* remove last result */
    lua_pushnil(L);  /* push nil instead */
  }
  return n - first;
}


static int io_read (lua_State *L) {
  return g_read(L, getiofile(L, IO_INPUT), 1);
}


static int f_read (lua_State *L) {
  return g_read(L, tofile(L, 1), 2);
}


static int io_readline (lua_State *L) {
  FILE *f = *(FILE **)lua_touserdata(L, lua_upvalueindex(2));
  if (f == NULL)  /* file is already closed? */
    luaL_error(L, "file is already closed");
  if (read_line(L, f)) return 1;
  else {  /* EOF */
    if (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);
      lua_pushvalue(L, lua_upvalueindex(2));
      aux_close(L);  /* close it */
    }
    return 0;
  }
}

/* }====================================================== */


static int g_write (lua_State *L, FILE *f, int arg) {
  int nargs = lua_gettop(L) - 1;
  int status = 1;
  for (; nargs--; arg++) {
    if (lua_type(L, arg) == LUA_TNUMBER) {
      /* optimization: could be done exactly as for strings */
      status = status &&
          fprintf(f, LUA_NUMBER_FMT, lua_tonumber(L, arg)) > 0;
    }
    else {
      size_t l;
      const char *s = luaL_checklstring(L, arg, &l);
      /*status = status && (fwrite(s, sizeof(char), l, f) == l);*/
      status = status && (fwrite((void *)s, sizeof(char), l, f) == l);
    }
  }
  return pushresult(L, status, NULL);
}


static int io_write (lua_State *L) {
  return g_write(L, getiofile(L, IO_OUTPUT), 1);
}


static int f_write (lua_State *L) {
  return g_write(L, tofile(L, 1), 2);
}


static int f_seek (lua_State *L) {
  static const int mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  static const char *const modenames[] = {"set", "cur", "end", NULL};
  FILE *f = tofile(L, 1);
  int op = luaL_checkoption(L, 2, "cur", modenames);
  long offset = luaL_optlong(L, 3, 0);
  luaL_argcheck(L, op != -1, 2, "invalid mode");
  op = fseek(f, offset, mode[op]);
  if (op)
    return pushresult(L, 0, NULL);  /* error */
  else {
    lua_pushnumber(L, ftell(f));
    return 1;
  }
}


static int io_flush (lua_State *L) {
  return pushresult(L, fflush(getiofile(L, IO_OUTPUT)) == 0, NULL);
}


static int f_flush (lua_State *L) {
  return pushresult(L, fflush(tofile(L, 1)) == 0, NULL);
}

static int lfcgi_accept (lua_State *L) {
	lua_pushnumber( L, FCGI_Accept() );
	return 1;
}

static int lfcgi_getenv (lua_State *L) {
	const char* envVar = luaL_checkstring(L, 1);
	char* val = getenv(envVar);

	if(val != NULL) {
		lua_pushstring(L, val);
	}
	else {
		if (old_env != environ) {
			char **env = environ;
			environ = old_env;
			val = getenv(envVar);
			environ = env;
		}
		if (val != NULL) lua_pushstring(L, val);
		else lua_pushnil(L);
	}
	return 1;
}

static int lfcgi_environ(lua_State *L) {
	char **envp;
	int i=1;
	lua_newtable(L);
    	for ( envp = old_env; *envp != NULL; envp++, i++) {
		lua_pushnumber(L, i);
		lua_pushstring(L, *envp);
		lua_settable(L, -3);
	}
	if (old_env != environ) {
		for ( envp = environ; *envp != NULL; envp++, i++) {
			lua_pushnumber(L, i);
			lua_pushstring(L, *envp);
			lua_settable(L, -3);
		}
	}
	return 1;
}
static int lfcgi_iscgi(lua_State *L) {
  lua_pushboolean(L, old_env == environ);
  return 1;
}

static int lfcgi_getpid(lua_State *L) {
	lua_pushnumber(L, getpid());
	return 1;
}

static const luaL_reg iolib[] = {
  {"input", io_input},
  {"output", io_output},
  {"lines", io_lines},
  {"close", io_close},
  {"flush", io_flush},
  {"open", io_open},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {"accept", lfcgi_accept},
  {"getenv", lfcgi_getenv},
  {"getpid", lfcgi_getpid},
  {"environ", lfcgi_environ},
  {"iscgi", lfcgi_iscgi},
  {NULL, NULL}
};


static const luaL_reg flib[] = {
  {"flush", f_flush},
  {"read", f_read},
  {"lines", f_lines},
  {"seek", f_seek},
  {"write", f_write},
  {"close", io_close},
  {"__gc", io_gc},
  {"__tostring", io_tostring},
  {NULL, NULL}
};


static void createmeta (lua_State *L) {
  luaL_newmetatable(L, FILEHANDLE);  /* create new metatable for file handles */
  /* file methods */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);  /* metatable.__index = metatable */
  luaL_openlib(L, NULL, flib, 0);
}

/* }====================================================== */


LUALIB_API int luaopen_lfcgi (lua_State *L) {
  old_env = environ;
  createmeta(L);
  lua_pushvalue(L, -1);
  luaL_openlib(L, "lfcgi", iolib, 1);
  /* put predefined file handles into `io' table */
  registerfile(L, FCGI_stdin, "stdin", IO_INPUT);
  registerfile(L, FCGI_stdout, "stdout", IO_OUTPUT);
  registerfile(L, FCGI_stderr, "stderr", NULL);
  return 1;
}

