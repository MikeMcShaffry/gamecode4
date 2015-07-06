#ifndef MISC_H
#define MISC_H

#include <assert.h>

#if defined(WIN32)
#define PLATFORM_WINDOWS
#define MISC_CDECL __cdecl
#include <new>
#elif defined(macintosh)  ||  defined(__APPLE__)
#define PLATFORM_MAC
#define MISC_CDECL
#include <new>
#elif defined(linux)
#define PLATFORM_LINUX
#define MISC_CDECL
#include <new>
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned long ULONG;

#if defined(PLATFORM_WINDOWS)
typedef signed __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;
#elif defined(PLATFORM_MAC)
typedef signed long long LONGLONG;
typedef unsigned long long ULONGLONG;
#elif defined(PLATFORM_LINUX)
typedef signed long long LONGLONG;
typedef unsigned long long ULONGLONG;
#endif

typedef unsigned short wchar;

#define array_countof(array) (sizeof(array)/sizeof(array[0]))

namespace Misc {

template <typename TYPE>
inline TYPE Min(TYPE num0, TYPE num1)
{
	return (num0 < num1) ? num0 : num1;
}


template <typename TYPE>
inline TYPE Max(TYPE num0, TYPE num1)
{
	return (num0 > num1) ? num0 : num1;
}


template <typename TYPE>
inline TYPE Clamp(TYPE num, TYPE minNum, TYPE maxNum)
{
	return (num < minNum) ? minNum : (num > maxNum) ? maxNum : num;
}

DWORD GetMilliseconds();
void SleepMilliseconds(unsigned int milliseconds);

extern void* gHInstance;
extern bool gInAssert;

#ifdef NDEBUG

#define CORE_ASSERT(exp)		((void)0)

#else

#define CORE_ASSERT(exp)		{ ::Misc::gInAssert = true; assert(exp); ::Misc::gInAssert = false; }

#endif // NDEBUG

class AnsiString;
class Color;
class DirectoryScanner;
class File;
class Image;
class IRCServer;
class VirtualFile;
class VirtualDrive;
class VirtualFileHandle;

class Vector2d;
class Vector2f;
class Vector2i;
class Vector3f;
class Vector3i;
class Vector4f;

class WideString;


bool				CheckFor98Mill();
bool				CheckForVista();
bool				CheckForTabletPC();
bool				AllowAllAccess(const AnsiString& theFileName);

} // namespace Misc

#endif // MISC_H

