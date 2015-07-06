#ifndef MEMFILE_H
#define MEMFILE_H

#include "File.h"
#include <stdio.h>

namespace Misc {

/**
**/
class MemFile : public File
{
public:
	MemFile();
	virtual ~MemFile();

	void SetGrowAmount(size_t growAmount);

	void Attach(const BYTE* buffer, size_t bufferSize, size_t growAmount = 0, bool ownBuffer = false);
	BYTE* Detach();

	virtual bool Open(const char* fileName, UINT openFlags = File::MODE_READONLY);
    virtual void Close();

	virtual ULONGLONG GetPosition() const;
	virtual ULONGLONG GetLength() const;
	virtual void SetLength(ULONGLONG dwNewLen);

	virtual LONGLONG Seek(LONGLONG offset, SeekFlags seekFlags = SEEKFLAG_BEGIN);
    virtual ULONGLONG Read(void* buffer, ULONGLONG count);
    virtual ULONGLONG Write(const void* buffer, ULONGLONG count);

	BYTE* GetBuffer()								{  return this->buffer;  }

	void GrowBuffer(ULONGLONG newBufferSize);

private:
	size_t growAmount;
	ULONGLONG position;
	ULONGLONG bufferSize;
	ULONGLONG fileSize;
	BYTE* buffer;
	bool ownBuffer;
};

} // namespace Misc

#endif // MEMFILE_H
