#ifndef PATH_H
#define PATH_H

extern int luaopen_os_path(lua_State *L);
extern int path_destroy(const char* inDirName);
extern int ex_access(lua_State *L);
extern int ex_mkdir(lua_State *L);

#endif /* PATH_H */
