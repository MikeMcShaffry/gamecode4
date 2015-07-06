#ifndef DISKFILE_H
#define DISKFILE_H

#include "File.h"
#include <time.h>

namespace Misc {

/**
**/
class DiskFile : public File
{
public:
    DiskFile();
    virtual ~DiskFile();

	virtual bool Open(const char* fileName, UINT openFlags = File::MODE_READONLY);
    virtual void Close();

	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG GetLength() const;
	virtual void SetLength(ULONGLONG dwNewLen);

	virtual LONGLONG Seek(LONGLONG offset, SeekFlags seekFlags = SEEKFLAG_BEGIN);
    virtual ULONGLONG Read(void* buffer, ULONGLONG count);
    virtual ULONGLONG Write(const void* buffer, ULONGLONG count);

	time_t GetLastWriteTime();
	void SetLastWriteTime(time_t lastWriteTime);

private:
    char* m_fileName;
#if defined(WIN32)
    void* m_fileHandle;
#else
	int m_fileHandle;
#endif
};

} // namespace Misc

#endif // DISKFILE_H
