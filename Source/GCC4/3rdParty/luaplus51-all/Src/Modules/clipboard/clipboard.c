#include <windows.h>

#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

static int l_gettext(lua_State* L) {
	if (OpenClipboard(NULL)) {
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData) {
			char* data = (char*)GlobalLock(hData);
			lua_pushstring(L, data);

			GlobalUnlock(hData);
		} else {
			lua_pushstring(L, "");
		}

		CloseClipboard();
	} else {
		lua_pushstring(L, "");
	}

	return 1;
}


static int l_puttext(lua_State* L) {
	const char* str = luaL_checkstring(L, 1);

	if (OpenClipboard(NULL)) {
		size_t len;
		HANDLE hGlobalMemory;

		EmptyClipboard();

		len = strlen(str) + 1;
        hGlobalMemory = GlobalAlloc(GHND, len);
        if (hGlobalMemory) {
            LPSTR lpGlobalMemory = (LPSTR)GlobalLock(hGlobalMemory);

            memcpy(lpGlobalMemory, str, len);

            GlobalUnlock(hGlobalMemory);
        }

        SetClipboardData(CF_TEXT, hGlobalMemory);

		CloseClipboard();
	}

	return 0;
}


static const struct luaL_reg clipboard_funcs[] = {
	{ "gettext", l_gettext },
	{ "puttext", l_puttext },
	{ NULL, NULL }
};


LUALIB_API int luaopen_clipboard(lua_State* L) {
	luaL_register(L, "clipboard", clipboard_funcs);
	return 1;
}


