#include <windows.h>

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID /*lpvReserved*/)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// Get the module name.
		char moduleName[_MAX_PATH];
		moduleName[0] = 0;
		::GetModuleFileName(hinstDLL, (TCHAR*)&moduleName, _MAX_PATH);

		// Get the module path.
		char modulePath[_MAX_PATH];
		strcpy(modulePath, moduleName);
		char* lastSlashPtr = NULL;
		char* ptr = modulePath;
		while (*ptr)
		{
			if (*ptr == '\\')
				lastSlashPtr = ptr + 1;
			ptr++;
		}
		*lastSlashPtr = 0;

		char luaplusPath[_MAX_PATH * 2];
		strcpy(luaplusPath, modulePath);
		strcat(luaplusPath, ";");
		strcat(luaplusPath, modulePath);
		strcat(luaplusPath, "..\\;");

		size_t size = GetEnvironmentVariable("PATH", 0, 0);
		size += strlen(luaplusPath) + 1;
		char* buffer = (char*)malloc(size);
		strcpy(buffer, luaplusPath);

		size = GetEnvironmentVariable("PATH", buffer + strlen(luaplusPath), (DWORD)size - (DWORD)strlen(luaplusPath));
		SetEnvironmentVariable("PATH", buffer);
		free(buffer);

/*
		strcat(luaplusPath, "LuaPlusD_1084.dll");

		HMODULE handle = LoadLibrary(luaplusPath);
		if (!handle)
		{
			strcpy(luaplusPath, modulePath);
			strcat(luaplusPath, "..\\LuaPlusD_1084.dll");
			handle = LoadLibrary(luaplusPath);
		}
		handle = NULL;
*/
	}

	return TRUE;
}
