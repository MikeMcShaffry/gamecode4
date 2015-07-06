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
#ifndef MISC_ZIPARCHIVE_H
#define MISC_ZIPARCHIVE_H

#include "Misc.h"
#include "zlib.h"
#include <time.h>
#include "List.h"
#include "HeapString.h"
#include "Map.h"
#include <stdlib.h>
#include <ctype.h>
#define ZIPARCHIVE_ENCRYPTION 1
#if ZIPARCHIVE_ENCRYPTION
#include "aes/fileenc.h"
#include "aes/prng.h"
#endif // ZIPARCHIVE_ENCRYPTION
#include "File.h"

namespace Misc {

//#define ZIPARCHIVE_DEFAULT_MD5 SUPPORT_MD5
#define ZIPARCHIVE_DEFAULT_MD5 0

class ZipArchive;

class ZipEntryFileHandle
{
public:
    ZipEntryFileHandle();
    ~ZipEntryFileHandle();

    ZipArchive* GetParentArchive() const;
	bool IsValid() const;

private:
	class Detail;
	Detail* detail;

    ZipEntryFileHandle* nextOpenFile;
    ZipEntryFileHandle* prevOpenFile;
    ZipArchive* parentDrive;

	friend class ZipArchive;
};


/**
	Represents a file entry within the zip archive's directory.
**/
class ZipEntryInfo
{
public:
	time_t GetTimeStamp() const			        {  return m_fileTime;  }
    DWORD GetOffset() const                     {  return m_offset;  }
    DWORD GetCompressedSize() const             {  return m_compressedSize;  }
    DWORD GetUncompressedSize() const           {  return m_uncompressedSize;  }
	DWORD GetCRC() const						{  return m_crc;  }
    DWORD GetCompressionMethod() const          {  return m_compressionMethod;  }
	const char* GetFilename() const		        {  return m_filename;  }

	unsigned char* GetMD5() const				{  return (unsigned char*)&m_md5;  }

	void SetTimeStamp(time_t fileTime);
	void SetCRC(DWORD crc);
	void SetMD5(unsigned char* md5);

protected:
	time_t          m_fileTime;
	DWORD			m_offset;
	DWORD			m_compressedSize;
    DWORD           m_uncompressedSize;
	DWORD			m_crc;
	unsigned char	m_md5[16];
    DWORD           m_compressionMethod;

	ZipArchive*		m_parentDrive;
    char            m_filename[1];

	friend class ZipArchive;
};


/**
**/
class ZipArchive
{
public:
	enum {  INVALID_FILE_ENTRY = (size_t)-1  };
    enum {  UNCOMPRESSED = 0, DEFLATED = 8  };
    enum {
		SUPPORT_MD5 = 0x00000001,
		EXTRA_DIRECTORY_AT_BEGINNING = 0x00000002,
	};

    struct FileOrderInfo {
		FileOrderInfo()
			: compressionMethod(DEFLATED)
			, compressionLevel(Z_DEFAULT_COMPRESSION)
			, fileTime(0)
			, size(0)
			, crc(0)
			, lastWriteTime(0)
			, needUpdate(false)
		{
			memset(md5, 0, sizeof(md5));
		}

		FileOrderInfo(const char* _entryName, const char* _srcPath, int _compressionMethod = ZipArchive::DEFLATED,
				int _compressionLevel = Z_DEFAULT_COMPRESSION)
			: entryName(_entryName)
			, srcPath(_srcPath)
			, compressionMethod(_compressionMethod)
			, compressionLevel(_compressionLevel)
			, fileTime(0)
			, size(0)
			, crc(0)
			, lastWriteTime(0)
			, needUpdate(false)
			, used(false)
		{
			memset(md5, 0, sizeof(md5));
		}

        HeapString entryName;
        HeapString srcPath;
		int compressionMethod;
		int compressionLevel;
		time_t fileTime;
		size_t size;
		DWORD crc;
		unsigned char md5[16];

	protected:
		friend class ZipArchive;

		time_t lastWriteTime;
		bool needUpdate:1;
		bool used:1;

		bool operator<(const FileOrderInfo& info) const {
			return this->entryName < info.entryName;
		}
	};
    typedef List<FileOrderInfo> FileOrderList;

	///////////////////////////////////////////////////////////////////////////
	ZipArchive();
	~ZipArchive();

	bool Create(const char* filename, DWORD flags = ZIPARCHIVE_DEFAULT_MD5, const char* defaultPassword = NULL);
	bool Create(File& parentFile, const char* fileName, DWORD flags = ZIPARCHIVE_DEFAULT_MD5, const char* defaultPassword = NULL);
	bool Open(const char* filename, bool readOnly = true, DWORD flags = ZIPARCHIVE_DEFAULT_MD5, const char* defaultPassword = NULL);
	bool Open(File& parentFile, const char* fileName, bool readOnly = true, DWORD flags = ZIPARCHIVE_DEFAULT_MD5, const char* defaultPassword = NULL);
	bool Close(void);

	void UpdateMD5s();

	struct FlushOptions
	{
		FlushOptions()
			: fileOrderList(NULL)
		{
		}

		FileOrderList* fileOrderList;
	};

	bool Flush(const FlushOptions* flushOptions = NULL);

	bool IsReadOnly() const					{  return m_readOnly;  }
	bool IsOpened() const					{  return m_parentFile != NULL;  }

	bool FileCreate(const char* filename, ZipEntryFileHandle& fileHandle, int compressionMethod = DEFLATED,
			int compressionLevel = Z_DEFAULT_COMPRESSION, const time_t* fileTime = NULL);
	bool FileOpen(const char* filename, ZipEntryFileHandle& fileHandle);
	bool FileOpenIndex(size_t index, ZipEntryFileHandle& fileHandle);
	bool FileClose(ZipEntryFileHandle& fileHandle);
	void FileCloseAll();

    const char* FileGetFileName(ZipEntryFileHandle& fileHandle);
	ULONGLONG FileGetPosition(ZipEntryFileHandle& fileHandle);
	void FileSetLength(ZipEntryFileHandle& fileHandle, ULONGLONG newLength);
	ULONGLONG FileGetLength(ZipEntryFileHandle& fileHandle);

	LONGLONG FileSeek(ZipEntryFileHandle& fileHandle, LONGLONG offset, File::SeekFlags seekFlags = File::SEEKFLAG_BEGIN);
	ULONGLONG FileRead(ZipEntryFileHandle& fileHandle, void* buffer, ULONGLONG count);
    ULONGLONG FileWrite(ZipEntryFileHandle& fileHandle, const void* buffer, ULONGLONG count);

	bool FileErase(const char* filename);
	bool FileRename(const char* oldName, const char* newName);

	bool FileCopy(ZipEntryFileHandle& srcFileHandle, const char* destFilename = NULL, const time_t* overrideFileTime = NULL);
	bool FileCopy(File& srcFile, const char* destFilename, int compressionMethod = DEFLATED,
			int compressionLevel = Z_DEFAULT_COMPRESSION, const time_t* fileTime = NULL);
	bool FileCopy(const char* srcFileName, const char* destFilename, int compressionMethod = DEFLATED,
			int compressionLevel = Z_DEFAULT_COMPRESSION, const time_t* fileTime = NULL);

	bool BufferCopy(const void* buffer, ULONGLONG size, ZipEntryFileHandle& destFile);
	bool BufferCopy(const void* buffer, ULONGLONG size, const char* destFilename, int compressionMethod = DEFLATED,
			int compressionLevel = Z_DEFAULT_COMPRESSION, const time_t* fileTime = NULL);

	struct PackOptions
	{
		PackOptions()
			: fileOrderList(NULL)
			, setNeedsPack(false)
		{
		}

		FileOrderList* fileOrderList;
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
		PACKING_COPY,
	};
	typedef void (*fnStatusUpdate)(FileListStatus status, const char* text, void* userData);

	struct ProcessFileListOptions
	{
		ProcessFileListOptions()
			: checkOnly(false)
			, requiresPack(true)
			, fileCacheSizeThreshold(0)
			, retrieveChecksumCallback(NULL)
			, retrieveChecksumUserData(NULL)
			, statusUpdateCallback(NULL)
			, statusUpdateUserData(NULL)
		{
		}

		bool checkOnly;
		bool requiresPack;

		HeapString fileCache;
		size_t fileCacheSizeThreshold;				// Any files of this size or larger will be checked in the file cache.

		fnRetrieveChecksum retrieveChecksumCallback;
		void* retrieveChecksumUserData;

		fnStatusUpdate statusUpdateCallback;
		void* statusUpdateUserData;
	};

    bool ProcessFileList(FileOrderList& fileOrder, ProcessFileListOptions* options = 0);

	File* GetParentFile() const;
	const char* GetFilename() const;

	size_t GetFileEntryCount(void) const;
	ZipEntryInfo* GetFileEntry(size_t entry);
	ZipEntryInfo* FindFileEntry(const char* filename);

	size_t FindFileEntryIndex(const char* filename);
	int	GetNumberOfFilesOpen(void) const;

	static time_t AdjustTime_t(time_t timeToAdjust);

	HeapString errorString;

	HeapString fileCache;

private:
	void FileCloseInternal(ZipEntryFileHandle& fileHandle);
	void _WriteDirectory(LONGLONG dirOffset, LONGLONG dirHeaderOffset);

	DWORD	m_flags;

	size_t  m_fileEntryCount;
	size_t  m_fileEntryMaxCount;
    ULONGLONG m_dirOffset;
	ULONGLONG m_dirSize;			// Directory size in bytes.
	UINT    m_needsPack:1;
	UINT	m_readOnly:1;
	UINT	m_changed:1;							//!< Whether the file needs to be flushed.
	UINT	m_swap:1;
	UINT	m_unused:28;

    char* m_filename;
    size_t* m_fileEntryOffsets;
    BYTE* m_fileEntries;
	size_t m_fileEntriesSizeBytes;
	size_t m_fileEntriesMaxSizeBytes;

	File* m_parentFile;
	bool m_ownParentFile;

	ZipEntryFileHandle* m_curWriteFile;				//!< The current file being written to.

	ZipEntryFileHandle* m_headOpenFile;

	class PtrString
	{
	public:
		PtrString() : m_drive(NULL), m_index((size_t)-1), m_str(NULL) {}
		PtrString(ZipArchive* drive, size_t index) : m_drive(drive), m_index(index), m_str(NULL) {}
		PtrString(const char* str) : m_drive(NULL), m_index((size_t)-1), m_str(str) {}

		ZipArchive* m_drive;
		size_t m_index;
		const char* m_str;
	};

	class PtrStringTypeTraits : public DefaultTypeTraits<PtrString>
	{
	public:
		static size_t Hash(const PtrString& str)
		{
			const char* ptr = str.m_str ? str.m_str : str.m_drive->GetFileEntry(str.m_index)->GetFilename();
			size_t l = strlen(ptr);
			size_t h = l;
			size_t step = (l >> 5) + 1;
			for (size_t l1 = l; l1 >= step; l1 -= step)
				h = h ^ ((h<<5)+(h>>2)+(unsigned char)(tolower(ptr[l1-1])));

			return h;
		}

		static bool CompareElements(const PtrString& str1, const PtrString& str2)
		{
			const char* ptr1 = str1.m_str ? str1.m_str : str1.m_drive->GetFileEntry(str1.m_index)->GetFilename();
			const char* ptr2 = str2.m_str ? str2.m_str : str2.m_drive->GetFileEntry(str2.m_index)->GetFilename();
#if defined(_MSC_VER)
			return _stricmp(ptr1, ptr2) == 0;
#else
			return strcasecmp(ptr1, ptr2) == 0;
#endif
		}
	};

	typedef Map<PtrString, size_t, PtrStringTypeTraits> FileNameMap;
	FileNameMap fileNameMap;

#if ZIPARCHIVE_ENCRYPTION
	HeapString defaultPassword;
	fcrypt_ctx defaultzcx[1];
#endif // ZIPARCHIVE_ENCRYPTION

	friend class ZipEntryInfo;
};


/**
	@return Returns the pointer to the File-derived object used for all archive
		file operations.
**/
inline File* ZipArchive::GetParentFile() const
{
	return m_parentFile;
}


/**
	@return Retrieves the filename of the archive.
**/
inline const char* ZipArchive::GetFilename() const
{
	return m_filename;
}


/**
	@return Returns the number of file entries in the archive.
**/
inline size_t ZipArchive::GetFileEntryCount(void) const
{
	return m_fileEntryCount;
}

} // namespace Misc

#endif // MISC_ZIPARCHIVE_H
