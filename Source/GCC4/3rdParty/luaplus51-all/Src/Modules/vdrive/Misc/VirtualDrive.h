///////////////////////////////////////////////////////////////////////////////
// $Workfile: VirtualDrive.h $
// $Archive: /WorkspaceWhiz/Src/Shared/VirtualDrive.h $
// $Date: 2003/01/05 $ $Revision: #8 $ $Author: Joshua $
///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2010 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as due credit is given and
// this header is left intact.
//
// This particular version comes from the MIT licensed LuaPlus source code
// distribution.
///////////////////////////////////////////////////////////////////////////////
#ifndef VIRTUALDRIVE_H
#define VIRTUALDRIVE_H

#include "Misc.h"
#include "zlib.h"
#include <time.h>
#include "List.h"
#include "HeapString.h"
#include "Map.h"
#include <stdlib.h>
#include <ctype.h>
#define VIRTUALDRIVE_ENCRYPTION 1
#if VIRTUALDRIVE_ENCRYPTION
#include "aes/fileenc.h"
#include "aes/prng.h"
#endif // VIRTUALDRIVE_ENCRYPTION
#include "File.h"

namespace Misc {

class VirtualDrive;

class VirtualFileHandle
{
public:
    VirtualFileHandle();
    ~VirtualFileHandle();

    VirtualDrive* GetParentDrive() const;
	bool IsValid() const;

private:
	class Detail;
	Detail* detail;

    VirtualFileHandle* nextOpenFile;
    VirtualFileHandle* prevOpenFile;
    VirtualDrive* parentDrive;

	friend class VirtualDrive;
};


/**
	A virtual drive is housed within a physical disk file.  The disk file
	contains one or more files, known as virtual files, and a directory
	describing all the files.  Similar in concept to WAD files.

	@see VirtualFile
**/
class VirtualDrive
{
public:
	enum {  INVALID_FILE_ENTRY = (size_t)-1  };
    enum {  UNCOMPRESSED = 0, COMPRESSED = 8  };
    enum {
		SUPPORT_MD5 = 0x00000001,
		EXTRA_DIRECTORY_AT_BEGINNING = 0x00000002,
	};

	/**
		Represents a file entry within the virtual drive's directory.
	**/
	class FileEntry
	{
	public:
		time_t GetTimeStamp() const			        {  return m_fileTime;  }
        DWORD GetOffset() const                     {  return m_offset;  }
        DWORD GetCompressedSize() const             {  return m_compressedSize;  }
        DWORD GetUncompressedSize() const           {  return m_uncompressedSize;  }
		DWORD GetCRC() const						{  return m_crc;  }
        DWORD GetCompressionMethod() const          {  return m_compressionMethod;  }
		const char* GetFileName() const		        {  return m_fileName;  }

		void SetTimeStamp(time_t fileTime)	        {  m_fileTime = fileTime;  m_parentDrive->m_changed = true;  }
		void SetCRC(DWORD crc)						{  m_crc = crc;  m_parentDrive->m_changed = true;  }

		unsigned char* GetMD5() const				{  return (unsigned char*)&m_md5;  }
		void SetMD5(unsigned char* md5)				{  for (int i = 0; i < 16; ++i) m_md5[i] = md5[i];  m_parentDrive->m_changed = true;  }

	protected:
		time_t          m_fileTime;
		DWORD			m_offset;
		DWORD			m_compressedSize;
        DWORD           m_uncompressedSize;
		DWORD			m_crc;
		unsigned char	m_md5[16];
        DWORD           m_compressionMethod;

		VirtualDrive*	m_parentDrive;
        char            m_fileName[1];

		friend class VirtualDrive;
	};

    struct FileOrderInfo
    {
		FileOrderInfo()
			: compressionMethod(COMPRESSED)
			, fileTime(0)
			, size(0)
			, crc(0)
			, lastWriteTime(0)
			, needUpdate(false)
		{
			memset(md5, 0, sizeof(md5));
		}

		FileOrderInfo(const char* _entryName, const char* _srcPath, UINT _compressionMethod = VirtualDrive::COMPRESSED)
			: entryName(_entryName)
			, srcPath(_srcPath)
			, compressionMethod(_compressionMethod)
			, fileTime(0)
			, size(0)
			, crc(0)
			, lastWriteTime(0)
			, needUpdate(false)
		{
			memset(md5, 0, sizeof(md5));
		}

        HeapString entryName;
        HeapString srcPath;
		UINT compressionMethod;
		time_t fileTime;
		size_t size;
		DWORD crc;
		unsigned char md5[16];

	protected:
		friend class VirtualDrive;

		time_t lastWriteTime;
		bool needUpdate;

		bool operator<(const FileOrderInfo& info) const
		{
			return this->entryName < info.entryName;
		}
	};
    typedef List<FileOrderInfo> FileOrderList;

	///////////////////////////////////////////////////////////////////////////
	VirtualDrive();
	~VirtualDrive();

	bool Create(const char* filename, const char* defaultPassword = NULL, DWORD flags = SUPPORT_MD5);
	bool Create(File& parentFile, const char* fileName, const char* defaultPassword = NULL, DWORD flags = SUPPORT_MD5);
	bool Open(const char* filename, bool readOnly = true, const char* defaultPassword = NULL, DWORD flags = SUPPORT_MD5);
	bool Open(File& parentFile, const char* fileName, bool readOnly = true, const char* defaultPassword = NULL, DWORD flags = SUPPORT_MD5);
	bool Close(void);

	void UpdateMD5s();

	struct FlushOptions
	{
		FlushOptions()
			: fileOrder(NULL)
		{
		}

		FileOrderList* fileOrder;
	};

	bool Flush(const FlushOptions* flushOptions = NULL);

	bool IsReadOnly() const					{  return m_readOnly;  }
	bool IsOpened() const					{  return m_parentFile != NULL;  }

	bool FileCreate(const char* filename, VirtualFileHandle& fileHandle, UINT compressionMethod = COMPRESSED, const time_t* fileTime = NULL);
	bool FileOpen(const char* filename, VirtualFileHandle& fileHandle);
	bool FileOpenIndex(size_t index, VirtualFileHandle& fileHandle);
	bool FileClose(VirtualFileHandle& fileHandle);
	void FileCloseAll();

    const char* FileGetFileName(VirtualFileHandle& fileHandle);
	ULONGLONG FileGetPosition(VirtualFileHandle& fileHandle);
	void FileSetLength(VirtualFileHandle& fileHandle, ULONGLONG newLength);
	ULONGLONG FileGetLength(VirtualFileHandle& fileHandle);

	LONGLONG FileSeek(VirtualFileHandle& fileHandle, LONGLONG offset, File::SeekFlags seekFlags = File::SEEKFLAG_BEGIN);
	ULONGLONG FileRead(VirtualFileHandle& fileHandle, void* buffer, ULONGLONG count);
    ULONGLONG FileWrite(VirtualFileHandle& fileHandle, const void* buffer, ULONGLONG count);

	bool FileErase(const char* filename);
	bool FileRename(const char* oldName, const char* newName);

	bool FileCopy(VirtualFileHandle& srcFileHandle, const char* destFilename = NULL, const time_t* overrideFileTime = NULL);
	bool FileCopy(File& srcFile, const char* destFilename, UINT compressionMethod = COMPRESSED, const time_t* fileTime = NULL);
	bool FileCopy(const char* srcFileName, const char* destFilename, UINT compressionMethod = COMPRESSED, const time_t* fileTime = NULL);

	bool BufferCopy(const void* buffer, ULONGLONG size, VirtualFileHandle& destFile);
	bool BufferCopy(const void* buffer, ULONGLONG size, const char* destFilename, UINT compressionMethod = COMPRESSED, const time_t* fileTime = NULL);

	struct PackOptions
	{
		PackOptions()
			: fileOrder(NULL)
			, setNeedsPack(false)
		{
		}

		FileOrderList* fileOrder;
		bool setNeedsPack;
	};

	bool NeedsPack(PackOptions* packOptions = NULL);
    bool Pack(PackOptions* packOptions = NULL);

	typedef int (*fnRetrieveChecksum)(const char* sourcePath, DWORD* crc, unsigned char* md5, void* userData);

	enum FileListStatus
	{
		UPDATING_ARCHIVE,		// This archive needs to be updated.
		DELETING_ENTRY,
		DIRECT_COPY_FROM_ANOTHER_ARCHIVE,
		COPYING_ENTRY_FROM_CACHE,
		COPYING_ENTRY_TO_CACHE,
		UPDATING_ENTRY,
		PACKING_ARCHIVE,
	};
	typedef void (*fnStatusUpdate)(FileListStatus status, const char* text, void* userData);

	struct ProcessFileListOptions
	{
		ProcessFileListOptions()
			: checkOnly(false)
			, fileCacheSizeThreshold(0)
			, retrieveChecksumCallback(NULL)
			, retrieveChecksumUserData(NULL)
			, statusUpdateCallback(NULL)
			, statusUpdateUserData(NULL)
		{
		}

		bool checkOnly;

		HeapString fileCache;
		size_t fileCacheSizeThreshold;				// Any files of this size or larger will be checked in the file cache.

		fnRetrieveChecksum retrieveChecksumCallback;
		void* retrieveChecksumUserData;

		fnStatusUpdate statusUpdateCallback;
		void* statusUpdateUserData;
	};

    bool ProcessFileList(const char* fileListFileName, ProcessFileListOptions* options = 0);
    bool ProcessFileList(FileOrderList& fileOrder, ProcessFileListOptions* options = 0);

	File* GetParentFile() const;
	const char* GetFileName() const;

	size_t GetFileEntryCount(void) const;
	VirtualDrive::FileEntry* GetFileEntry(size_t entry);
	VirtualDrive::FileEntry* FindFileEntry(const char* filename);

	size_t FindFileEntryIndex(const char* filename);
	int	GetNumberOfFilesOpen(void) const;

	static time_t AdjustTime_t(time_t timeToAdjust);

	HeapString errorString;

	HeapString fileCache;

private:
	void FileCloseInternal(VirtualFileHandle& fileHandle);
	void _WriteDirectory(LONGLONG dirOffset, LONGLONG dirHeaderOffset);

	DWORD	m_flags;

	size_t  m_fileEntryCount;
	size_t  m_fileEntryMaxCount;
    ULONGLONG m_dirOffset;
	ULONGLONG m_dirSize;			// Directory size in bytes.
	UINT    m_needsPack:1;
	UINT	m_readOnly:1;
	UINT	m_unused:30;

    char* m_fileName;
    size_t* m_fileEntryOffsets;
    BYTE* m_fileEntries;
	size_t m_fileEntriesSizeBytes;
	size_t m_fileEntriesMaxSizeBytes;

	File* m_parentFile;
	bool m_ownParentFile;

    bool m_changed;								//!< Whether the file needs to be flushed.
	VirtualFileHandle* m_curWriteFile;				//!< The current file being written to.

	VirtualFileHandle* m_headOpenFile;

	class PtrString
	{
	public:
		PtrString() : m_drive(NULL), m_index((size_t)-1), m_str(NULL) {}
		PtrString(VirtualDrive* drive, size_t index) : m_drive(drive), m_index(index), m_str(NULL) {}
		PtrString(const char* str) : m_drive(NULL), m_index((size_t)-1), m_str(str) {}

		VirtualDrive* m_drive;
		size_t m_index;
		const char* m_str;
	};

	class PtrStringTypeTraits : public DefaultTypeTraits<PtrString>
	{
	public:
		static size_t Hash(const PtrString& str)
		{
			const char* ptr = str.m_str ? str.m_str : str.m_drive->GetFileEntry(str.m_index)->GetFileName();
			size_t l = strlen(ptr);
			size_t h = l;
			size_t step = (l >> 5) + 1;
			for (size_t l1 = l; l1 >= step; l1 -= step)
				h = h ^ ((h<<5)+(h>>2)+(unsigned char)(tolower(ptr[l1-1])));

			return h;
		}

		static bool CompareElements(const PtrString& str1, const PtrString& str2)
		{
			const char* ptr1 = str1.m_str ? str1.m_str : str1.m_drive->GetFileEntry(str1.m_index)->GetFileName();
			const char* ptr2 = str2.m_str ? str2.m_str : str2.m_drive->GetFileEntry(str2.m_index)->GetFileName();
#if defined(_MSC_VER)
			return _stricmp(ptr1, ptr2) == 0;
#else
			return strcasecmp(ptr1, ptr2) == 0;
#endif
		}
	};

	typedef Map<PtrString, size_t, PtrStringTypeTraits> FileNameMap;
	FileNameMap fileNameMap;

#if VIRTUALDRIVE_ENCRYPTION
	HeapString defaultPassword;
	fcrypt_ctx defaultzcx[1];
#endif // VIRTUALDRIVE_ENCRYPTION

	bool m_swap;

	friend class VirtualDriveManager;
	friend class VirtualFile;
	friend class FileEntry;
};


/**
	@return Returns the pointer to the File-derived object used for all virtual
		drive file operations.
**/
inline File* VirtualDrive::GetParentFile() const
{
	return m_parentFile;
}


/**
	@return Retrieves the filename of the virtual drive.
**/
inline const char* VirtualDrive::GetFileName() const
{
	return m_fileName;
}


/**
	@return Returns the number of file entries in the virtual drive.
**/
inline size_t VirtualDrive::GetFileEntryCount(void) const
{
	return m_fileEntryCount;
}

} // namespace Misc

#endif // VIRTUALDRIVE_H
