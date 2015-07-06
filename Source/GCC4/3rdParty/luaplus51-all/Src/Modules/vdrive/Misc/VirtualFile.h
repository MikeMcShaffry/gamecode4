#ifndef MISC_VIRTUALFILE_H
#define MISC_VIRTUALFILE_H

///////////////////////////////////////////////////////////////////////////////
// $Workfile: VirtualFile.h $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2003 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as due credit is given and
// this header is left intact.
///////////////////////////////////////////////////////////////////////////////
#include "VirtualDrive.h"
#include "File.h"

namespace Misc {

/**
	VirtualFile is a File-derived class for providing access to a virtual file
	within a virtual drive.
**/
class VirtualFile : public File
{
public:
    VirtualFile();
	virtual ~VirtualFile();

    bool Create(VirtualDrive& drive, const char* filename, UINT compressionMethod = VirtualDrive::COMPRESSED, const time_t* fileTime = NULL);
	bool Open(VirtualDrive& drive, const char* filename);

    virtual ULONGLONG GetPosition() const;
	virtual void SetLength(ULONGLONG dwNewLen);
	virtual ULONGLONG GetLength() const;

    virtual bool Open(const char* fileName, UINT openFlags)         {  (void)fileName, (void)openFlags;  return false;  }
	virtual void Close();

	virtual LONGLONG Seek(LONGLONG offset, SeekFlags seekFlags = SEEKFLAG_BEGIN);
	virtual ULONGLONG Read(void* buffer, ULONGLONG count);
    virtual ULONGLONG Write(const void* buffer, ULONGLONG count);

    const char* GetFileName() const;

private:
    VirtualFileHandle m_fileHandle;

    friend class VirtualDrive;
};

} // namespace Misc

#endif //MISC_VIRTUALFILE_H
