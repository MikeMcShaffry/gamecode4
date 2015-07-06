#include "Misc_InternalPch.h"
#include "MemFile.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

namespace Misc {

MemFile::MemFile()
	: growAmount(1024)
	, position(0)
	, bufferSize(0)
	, fileSize(0)
	, buffer(NULL)
	, ownBuffer(true)
{
}


MemFile::~MemFile()
{
	Close();
}


void MemFile::SetGrowAmount(size_t growAmount)
{
	this->growAmount = growAmount;
}

	
void MemFile::Attach(const BYTE* buffer, size_t bufferSize, size_t growAmount, bool ownBuffer)
{
	Close();

	this->growAmount = growAmount;
	this->position = 0;
	this->bufferSize = bufferSize;
	this->fileSize = growAmount == 0 ? bufferSize : 0;
	this->buffer = (BYTE*)buffer;
	this->ownBuffer = ownBuffer;
}


BYTE* MemFile::Detach()
{
	BYTE* saveBuffer = this->buffer;
	this->position = 0;
	this->bufferSize = 0;
	this->fileSize = 0;
	this->buffer = NULL;
	return saveBuffer;
}


ULONGLONG MemFile::GetPosition() const
{
	return this->position;
}


void MemFile::SetLength(ULONGLONG newSize)
{
	if (newSize > this->bufferSize)
		GrowBuffer(newSize);
	else if (newSize < this->bufferSize)
		this->position = newSize;
	this->fileSize = newSize;
}


ULONGLONG MemFile::GetLength(void) const
{
	return this->fileSize;
}


bool MemFile::Open(const char* fileName, UINT openFlags)
{
    Close();
    return true;
}


void MemFile::Close()
{
	this->position = 0;
	this->bufferSize = 0;
	this->fileSize = 0;
	if (this->ownBuffer)
		delete[] this->buffer;
	this->buffer = NULL;
}


LONGLONG MemFile::Seek(LONGLONG offset, SeekFlags seekFlags)
{
	LONGLONG newPosition = this->position;

	switch (seekFlags)
	{
		case SEEKFLAG_BEGIN:	newPosition = offset;					break;
		case SEEKFLAG_CURRENT:	newPosition = this->position + offset;	break;
		case SEEKFLAG_END:		newPosition = this->fileSize + offset;	break;
	}

	if (newPosition < 0)
		return -1;

	this->position = newPosition;
	return this->position;
}


ULONGLONG MemFile::Read(void* buffer, ULONGLONG count)
{
	if (count == 0)
		return 0;

	LONGLONG offset = this->position;
	ULONGLONG readBytes = offset + count > this->fileSize ? (this->fileSize - offset) : count;
	memcpy(buffer, this->buffer + offset, (size_t)readBytes);
	this->position = offset + readBytes;

	return readBytes;
}


ULONGLONG MemFile::Write(const void* buffer, ULONGLONG count)
{
	if (count == 0)
		return 0;

	LONGLONG offset = this->position;
	if (offset + count > this->bufferSize)
	{
		if (this->growAmount > 0)
			GrowBuffer(offset + count);
		else
			count = this->bufferSize - offset;
	}

	memcpy(this->buffer + offset, buffer, (size_t)count);
	this->position = offset + count;
	if (this->position > this->fileSize)
		this->fileSize = this->position;

	return count;
}


void MemFile::GrowBuffer(ULONGLONG newBufferSize)
{
	if (newBufferSize <= this->bufferSize)
		return;

	assert(this->growAmount != 0);

	newBufferSize += this->growAmount - (newBufferSize % this->growAmount);
	BYTE* newBuffer = new BYTE[(unsigned int)newBufferSize];
	if (this->fileSize > 0)
		memcpy(newBuffer, this->buffer, (size_t)this->fileSize);
	delete[] this->buffer;
	this->buffer = newBuffer;
	this->bufferSize = newBufferSize;

	this->ownBuffer = true;
}


} // namespace Misc
