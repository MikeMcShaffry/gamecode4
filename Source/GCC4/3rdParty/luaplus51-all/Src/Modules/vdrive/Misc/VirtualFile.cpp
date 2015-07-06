///////////////////////////////////////////////////////////////////////////////
// $Workfile: VirtualFile.cpp $
// $Archive: /WorkspaceWhiz/Src/Shared/VirtualFile.cpp $
// $Date: 2003/01/05 $ $Revision: #6 $ $Author: Joshua $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2003 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as due credit is given and
// this header is left intact.
///////////////////////////////////////////////////////////////////////////////
#include "Misc_InternalPch.h"
#include "VirtualFile.h"
#include "VirtualDrive.h"
#include <assert.h>

namespace Misc {

VirtualFile::VirtualFile()
{
}



VirtualFile::~VirtualFile()
{
	Close();
}


bool VirtualFile::Create(VirtualDrive& drive, const char* fileName, UINT compressionMethod, const time_t* fileTime)
{
    return drive.FileCreate(fileName, m_fileHandle, compressionMethod, fileTime);
}


bool VirtualFile::Open(VirtualDrive& drive, const char* fileName)
{
    return drive.FileOpen(fileName, m_fileHandle);
}


ULONGLONG VirtualFile::GetPosition() const
{
    assert(m_fileHandle.GetParentDrive());
    return m_fileHandle.GetParentDrive()->FileGetPosition(const_cast<VirtualFileHandle&>(m_fileHandle));
}



void VirtualFile::SetLength(ULONGLONG newSize)
{
    assert(m_fileHandle.GetParentDrive());
    m_fileHandle.GetParentDrive()->FileSetLength(m_fileHandle, (UINT)newSize);
}

    
ULONGLONG VirtualFile::GetLength(void) const
{
    assert(m_fileHandle.GetParentDrive());
    return m_fileHandle.GetParentDrive()->FileGetLength(const_cast<VirtualFileHandle&>(m_fileHandle));
}


LONGLONG VirtualFile::Seek(LONGLONG offset, SeekFlags seekFlags)
{
	assert(m_fileHandle.GetParentDrive());
	return m_fileHandle.GetParentDrive()->FileSeek(m_fileHandle, offset, seekFlags);
}


ULONGLONG VirtualFile::Read(void* buffer, ULONGLONG count)
{
    assert(m_fileHandle.GetParentDrive());
    return m_fileHandle.GetParentDrive()->FileRead(m_fileHandle, buffer, count);
}


ULONGLONG VirtualFile::Write(const void* buffer, ULONGLONG count)
{
    assert(m_fileHandle.GetParentDrive());
    return m_fileHandle.GetParentDrive()->FileWrite(m_fileHandle, buffer, count);
}


void VirtualFile::Close()
{
	if (m_fileHandle.GetParentDrive())
	    m_fileHandle.GetParentDrive()->FileClose(m_fileHandle);
}

} // namespace Misc
