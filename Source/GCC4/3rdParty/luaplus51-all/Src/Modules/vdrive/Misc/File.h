///////////////////////////////////////////////////////////////////////////////
// $Workfile: File.h $
// $Date: 2003/01/05 $ $Revision: #5 $ $Author: Joshua $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2003 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as due credit is given and
// this header is left intact.
///////////////////////////////////////////////////////////////////////////////
#ifndef FILE_H
#define FILE_H

#include "Misc.h"

namespace Misc {

/**
**/
class File
{
public:
    enum Flags
    {
        MODE_CREATE = 0x00000001,
        MODE_TRUNCATE = 0x00000002,
        MODE_READONLY = 0x00000004,
        MODE_WRITEONLY = 0x00000008,
        MODE_READWRITE = MODE_READONLY | MODE_WRITEONLY,
    };

    enum SeekFlags
    {
        SEEKFLAG_BEGIN,
        SEEKFLAG_CURRENT,
        SEEKFLAG_END,
    };

    File()  {};
    virtual ~File()  {};

    virtual bool Open(const char* fileName, UINT openFlags = File::MODE_READONLY) = 0;
    virtual void Close() = 0;

	virtual ULONGLONG GetPosition() const = 0;
	virtual ULONGLONG GetLength() const = 0;
	virtual void SetLength(ULONGLONG newLength) = 0;

	virtual LONGLONG Seek(LONGLONG offset, SeekFlags seekFlags = SEEKFLAG_BEGIN) = 0;
    virtual ULONGLONG Read(void* buffer, ULONGLONG count) = 0;
    virtual ULONGLONG Write(const void* buffer, ULONGLONG count) = 0;
};

} // namespace Misc

#endif //FILE_H
