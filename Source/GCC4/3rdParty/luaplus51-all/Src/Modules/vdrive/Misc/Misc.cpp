#include "Misc_InternalPch.h"

#if defined(WIN32)
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#elif defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <unistd.h>
#endif // WIN32

#include <stdio.h>
#include <assert.h>
#include "Map.h"
#include "AnsiString.h"
#include <time.h>

namespace Misc {

bool gInAssert = false;	

DWORD GetMilliseconds()
{
#if defined(WIN32)
	return ::timeGetTime();
#endif
#if defined(PLATFORM_MAC)
	Duration duration = AbsoluteToDuration(UpTime());
	return duration < 0 ? duration / -1000 : duration;
#endif // WIN32
}


void SleepMilliseconds(unsigned int milliseconds)
{
#if defined(WIN32)
	::Sleep(milliseconds);
#elif defined(PLATFORM_MAC)
	usleep(milliseconds * 1000);
#endif
}
#if defined(WIN32)

bool CheckFor98Mill()
{
	static bool needOsCheck = true;
	static bool is98Mill = false;

	if (needOsCheck)
	{
		bool invalid = false;
		OSVERSIONINFOEXA osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
			if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
				return false;
		}

		needOsCheck = false;
		is98Mill = osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS; // let's check Win95, 98, *AND* ME.
	}

	return is98Mill;
}

bool CheckForVista()
{
	static bool needOsCheck = true;
	static bool isVista = false;

	if (needOsCheck)
	{
		bool invalid = false;
		OSVERSIONINFOEXA osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXA));

		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
		if( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOA);
			if ( GetVersionExA((LPOSVERSIONINFOA)&osvi) == 0)
				return false;
		}

		needOsCheck = false;
		isVista = osvi.dwMajorVersion >= 6;
	}

	return isVista;
}

bool CheckForTabletPC()
{
	static bool needsCheck = true;
	static bool isTabletPC = false;

	if (needsCheck)
	{
		isTabletPC = GetSystemMetrics(86) != 0; // check for tablet pc
		needsCheck = false;
	}

	return isTabletPC;
}


#endif // WIN32

} // namespace Misc
