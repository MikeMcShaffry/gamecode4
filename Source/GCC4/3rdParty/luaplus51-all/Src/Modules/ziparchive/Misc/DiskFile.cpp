#include "Misc_InternalPch.h"
#include "DiskFile.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#undef GetClassInfo
#else
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#endif

namespace Misc {

DiskFile::DiskFile()
	: m_fileName(NULL)
#if defined(PLATFORM_WINDOWS)
    , m_fileHandle(INVALID_HANDLE_VALUE)
#else
	, m_fileHandle(-1)
#endif
{
}


DiskFile::~DiskFile()
{
	Close();
}


ULONGLONG DiskFile::GetPosition() const
{
#if defined(PLATFORM_WINDOWS)
	LARGE_INTEGER liPos;
	liPos.QuadPart = 0;
	liPos.LowPart = ::SetFilePointer(m_fileHandle, liPos.LowPart, &liPos.HighPart, FILE_CURRENT);
	if (liPos.LowPart == (DWORD)-1)
		return (ULONGLONG)-1;

	return liPos.QuadPart;
#else
	return lseek(m_fileHandle, 0, SEEK_CUR);
#endif
}


void DiskFile::SetLength(ULONGLONG newSize)
{
#if defined(PLATFORM_WINDOWS)
	Seek(newSize, SEEKFLAG_BEGIN);
    ::SetEndOfFile(m_fileHandle);
#else
	ftruncate(m_fileHandle, newSize);
#endif
}


ULONGLONG DiskFile::GetLength(void) const
{
#if defined(PLATFORM_WINDOWS)
	ULARGE_INTEGER fileSize;
	fileSize.LowPart = ::GetFileSize(m_fileHandle, &fileSize.HighPart);
	return fileSize.QuadPart;
#else
	off_t originalOffset = lseek(m_fileHandle, 0, SEEK_CUR);
	off_t length = lseek(m_fileHandle, 0, SEEK_END);
	lseek(m_fileHandle, originalOffset, SEEK_SET);
	return length;
#endif
}


bool DiskFile::Open(const char* fileName, UINT openFlags)
{
    Close();

    delete[] m_fileName;
    size_t fileNameLen = strlen(fileName);
	m_fileName = new char[fileNameLen + 1];
    strncpy(m_fileName, fileName, fileNameLen + 1);

#if defined(PLATFORM_WINDOWS)
    DWORD dwDesiredAccess = 0;
    if (openFlags & MODE_READONLY)
        dwDesiredAccess |= GENERIC_READ;
    if (openFlags & MODE_WRITEONLY)
        dwDesiredAccess |= GENERIC_WRITE;

    DWORD dwCreationDisposition = 0;
    if (openFlags & MODE_CREATE)
    {
        if (openFlags & MODE_TRUNCATE)
            dwCreationDisposition |= CREATE_ALWAYS;
        else
            dwCreationDisposition |= OPEN_ALWAYS;
    }
    else
        dwCreationDisposition |= OPEN_EXISTING;

    m_fileHandle = ::CreateFile(fileName, dwDesiredAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 0);
    if (m_fileHandle == INVALID_HANDLE_VALUE)
        return FALSE;
#else
	int flags = 0;
	if ((openFlags & MODE_READWRITE) == MODE_READWRITE)
	{
		flags |= O_RDWR;
	}
	else
	{
		if (openFlags & MODE_READONLY)
			flags |= O_RDONLY;
		if (openFlags & MODE_WRITEONLY)
			flags |= O_WRONLY;
	}

    if (openFlags & MODE_CREATE)
    {
		flags |= O_CREAT;
        if (openFlags & MODE_TRUNCATE)
			flags |= O_TRUNC;
    }

    m_fileHandle = open(fileName, flags, 0666);
    if (m_fileHandle == -1)
        return false;
#endif

    return true;
}


void DiskFile::Close()
{
#if defined(PLATFORM_WINDOWS)
    if (m_fileHandle != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_fileHandle);
        m_fileHandle = INVALID_HANDLE_VALUE;
    }
#else
	if (m_fileHandle != -1)
	{
		close(m_fileHandle);
		m_fileHandle = -1;
	}
#endif

    delete[] m_fileName;
    m_fileName = NULL;
}


LONGLONG DiskFile::Seek(LONGLONG offset, SeekFlags seekFlags)
{
#if defined(PLATFORM_WINDOWS)
	DWORD moveMethod = FILE_BEGIN;
	switch (seekFlags)
	{
		case SEEKFLAG_BEGIN:	moveMethod = FILE_BEGIN;		break;
		case SEEKFLAG_CURRENT:	moveMethod = FILE_CURRENT;		break;
		case SEEKFLAG_END:		moveMethod = FILE_END;			break;
	}

	LARGE_INTEGER liOff;

	liOff.QuadPart = offset;
	liOff.LowPart = ::SetFilePointer((HANDLE)m_fileHandle, liOff.LowPart, &liOff.HighPart, moveMethod);
	if (liOff.LowPart  == (DWORD)-1)
		if (::GetLastError() != NO_ERROR)
			return -1;

	return liOff.QuadPart;
#else
	int moveMethod = SEEK_SET;
	switch (seekFlags)
	{
		case SEEKFLAG_BEGIN:	moveMethod = SEEK_SET;			break;
		case SEEKFLAG_CURRENT:	moveMethod = SEEK_CUR;			break;
		case SEEKFLAG_END:		moveMethod = SEEK_END;			break;
	}

	return lseek(m_fileHandle, offset, moveMethod);
#endif
}


ULONGLONG DiskFile::Read(void* buffer, ULONGLONG count)
{
#if defined(PLATFORM_WINDOWS)
    DWORD bytesRead = 0;
    ::ReadFile(m_fileHandle, buffer, (DWORD)count, &bytesRead, NULL);
    return bytesRead;
#else
	return read(m_fileHandle, buffer, count);
#endif
}


ULONGLONG DiskFile::Write(const void* buffer, ULONGLONG count)
{
#if defined(PLATFORM_WINDOWS)
    DWORD bytesWritten = 0;
    ::WriteFile(m_fileHandle, buffer, (DWORD)count, &bytesWritten, NULL);
    return bytesWritten;
#else
	return write(m_fileHandle, buffer, count);
#endif
}


#if defined(PLATFORM_WINDOWS)
extern time_t ConvertFILETIME_To_time_t(const FILETIME& fileTime);
#endif


time_t DiskFile::GetLastWriteTime()
{
#if defined(PLATFORM_WINDOWS)
	FILETIME ftLastWrite;
	if ( !::GetFileTime(m_fileHandle, NULL, NULL, &ftLastWrite) )
		return 0;

	return ConvertFILETIME_To_time_t(ftLastWrite);
#else
	struct stat sb;
	if (fstat(m_fileHandle, &sb) == -1)
		return 0;
	return sb.st_mtime;
#endif

}


#if defined(PLATFORM_WINDOWS)
BOOL (WINAPI *fnTzSpecificLocalTimeToSystemTime)(LPTIME_ZONE_INFORMATION lpTimeZoneInformation, LPSYSTEMTIME lpLocalTime, LPSYSTEMTIME lpUniversalTime);
#endif // PLATFORM_WINDOWS

void DiskFile::SetLastWriteTime(time_t lastWriteTime)
{
#if defined(PLATFORM_WINDOWS)
#if _MSC_VER  &&  _MSC_VER <= 1300
	assert(0);
#else
	FILETIME localFILETIME;
	LONGLONG ll = Int32x32To64(lastWriteTime, 10000000) + 116444736000000000ULL;
	localFILETIME.dwLowDateTime = (DWORD) ll;
	localFILETIME.dwHighDateTime = (DWORD)(ll >>32);

	SYSTEMTIME localSystemTime;
	FileTimeToSystemTime(&localFILETIME, &localSystemTime);

	SYSTEMTIME universalSystemTime;

	if (CheckFor98Mill())
	{
		CORE_ASSERT(0);
	}
	else
	{
		if (!fnTzSpecificLocalTimeToSystemTime)
		{
			HMODULE aLib = LoadLibraryA("kernel32.dll");
			if (aLib == NULL)
				return;

			*(void**)&fnTzSpecificLocalTimeToSystemTime = (void*)GetProcAddress(aLib, "TzSpecificLocalTimeToSystemTime");
		}
		fnTzSpecificLocalTimeToSystemTime(NULL, &localSystemTime, &universalSystemTime);
	}

	FILETIME lastWriteTimeFILETIME;
	SystemTimeToFileTime(&localSystemTime, &lastWriteTimeFILETIME);

	if ( !::SetFileTime(m_fileHandle, NULL, NULL, &lastWriteTimeFILETIME) )
		return;
#endif
#else
	timeval tv[2];
	tv[0].tv_sec = lastWriteTime;
	tv[0].tv_usec = 0;
	tv[1].tv_sec = lastWriteTime;
	tv[1].tv_usec = 0;
	if (futimes(m_fileHandle, (const timeval*)&tv) != 0)
		return;
#endif

}


} // namespace Misc
