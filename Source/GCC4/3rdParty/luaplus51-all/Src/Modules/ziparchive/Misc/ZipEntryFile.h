#ifndef MISC_ZIPENTRYFILE_H
#define MISC_ZIPENTRYFILE_H

///////////////////////////////////////////////////////////////////////////////
// $Workfile: ZipEntryFile.h $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2010 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as this header is left intact.
///////////////////////////////////////////////////////////////////////////////
#include "ZipArchive.h"
#include "File.h"

namespace Misc {

/**
	ZipEntryFile is a File-derived class for providing access to a virtual file
	within a zip archive.
**/
class ZipEntryFile : public File
{
public:
    ZipEntryFile();
	virtual ~ZipEntryFile();

    bool Create(ZipArchive& archive, const char* filename, int compressionMethod = ZipArchive::DEFLATED,
			int compressionLevel = Z_DEFAULT_COMPRESSION, const time_t* fileTime = NULL);
	bool Open(ZipArchive& archive, const char* filename);

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
    ZipEntryFileHandle m_fileHandle;

    friend class ZipArchive;
};

} // namespace Misc

#endif //MISC_ZIPENTRYFILE_H
