///////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipEntryFile.cpp $
// $Archive: /WorkspaceWhiz/Src/Shared/ZipEntryFile.cpp $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2003 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as this header is left intact.
///////////////////////////////////////////////////////////////////////////////
#include "Misc_InternalPch.h"
#include "ZipEntryFile.h"
#include "ZipArchive.h"
#include <assert.h>

namespace Misc {

ZipEntryFile::ZipEntryFile()
{
}



ZipEntryFile::~ZipEntryFile()
{
	Close();
}


bool ZipEntryFile::Create(ZipArchive& archive, const char* fileName, int compressionMethod, int compressionLevel, const time_t* fileTime)
{
    return archive.FileCreate(fileName, m_fileHandle, compressionMethod, compressionLevel, fileTime);
}


bool ZipEntryFile::Open(ZipArchive& archive, const char* fileName)
{
    return archive.FileOpen(fileName, m_fileHandle);
}


ULONGLONG ZipEntryFile::GetPosition() const
{
    assert(m_fileHandle.GetParentArchive());
    return m_fileHandle.GetParentArchive()->FileGetPosition(const_cast<ZipEntryFileHandle&>(m_fileHandle));
}



void ZipEntryFile::SetLength(ULONGLONG newSize)
{
    assert(m_fileHandle.GetParentArchive());
    m_fileHandle.GetParentArchive()->FileSetLength(m_fileHandle, (UINT)newSize);
}

    
ULONGLONG ZipEntryFile::GetLength(void) const
{
    assert(m_fileHandle.GetParentArchive());
    return m_fileHandle.GetParentArchive()->FileGetLength(const_cast<ZipEntryFileHandle&>(m_fileHandle));
}


LONGLONG ZipEntryFile::Seek(LONGLONG offset, SeekFlags seekFlags)
{
	assert(m_fileHandle.GetParentArchive());
	return m_fileHandle.GetParentArchive()->FileSeek(m_fileHandle, offset, seekFlags);
}


ULONGLONG ZipEntryFile::Read(void* buffer, ULONGLONG count)
{
    assert(m_fileHandle.GetParentArchive());
    return m_fileHandle.GetParentArchive()->FileRead(m_fileHandle, buffer, count);
}


ULONGLONG ZipEntryFile::Write(const void* buffer, ULONGLONG count)
{
    assert(m_fileHandle.GetParentArchive());
    return m_fileHandle.GetParentArchive()->FileWrite(m_fileHandle, buffer, count);
}


void ZipEntryFile::Close()
{
	if (m_fileHandle.GetParentArchive())
	    m_fileHandle.GetParentArchive()->FileClose(m_fileHandle);
}

} // namespace Misc
