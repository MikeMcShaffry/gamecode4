///////////////////////////////////////////////////////////////////////////////
// This source file is part of the Workspace Whiz source distribution and
// is Copyright 1997-2009 by Joshua C. Jensen.  (http://workspacewhiz.com/)
//
// The code presented in this file may be freely used and modified for all
// non-commercial and commercial purposes so long as due credit is given and
// this header is left intact.
//
// This particular version comes from the MIT licensed LuaPlus source code
// distribution.
///////////////////////////////////////////////////////////////////////////////
#include "Misc_InternalPch.h"
#include "VirtualDrive.h"
#include "DiskFile.h"
#include "VirtualFile.h"
#include <time.h>
#include "zlib.h"
#include <assert.h>
#include <stdio.h>
#if defined(WIN32)
#include <windows.h>
#include <io.h>
#elif defined(__GNUC__)
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#endif
#include "List.h"
#include "Map.h"
#if VIRTUALDRIVE_ENCRYPTION
#include "aes/fileenc.h"
#include "aes/prng.h"
#endif // VIRTUALDRIVE_ENCRYPTION

extern "C"
{
	#define PROTOTYPES 1
	#include "md5/md5global.h"
	#include "md5/md5.h"
}

namespace Misc {

/*
 * "os.path" API implementation
 */
int PathCreate(const char* inPath)
{
#if defined(WIN32)
  char path[MAX_PATH];
#else
  char path[FILENAME_MAX];
#endif
  char* pathPtr = path;
  char ch;

  if (*inPath == '/'  ||  *inPath == '\\') {
    *pathPtr++ = *inPath;
    inPath++;			// Skip the initial /
#if defined(WIN32)
    if (*inPath == '/'  ||  *inPath == '\\') {
      // UNC share
      *pathPtr++ = *inPath;
      inPath++;			// Skip the initial /
      // Copy the machine name.
      while (ch = *inPath++) {
        *pathPtr++ = ch;
        if (ch == '/'  ||  ch == '\\')
          break;
      }
      // Copy the share name.
      while (ch = *inPath++) {
        *pathPtr++ = ch;
        if (ch == '/'  ||  ch == '\\')
          break;
      }
    }
#endif
  }

  while (ch = *inPath++) {
    if (ch == '/'  ||  ch == '\\') {
      char* colonPtr;
      int isDriveLetter;

      *pathPtr = 0;
#if defined(WIN32)
      /* Create the directory if it's not a drive letter. */
      colonPtr = pathPtr - 1;
      isDriveLetter = colonPtr == (path + 1)  &&  *colonPtr == ':';
      if (!isDriveLetter) {
        if (!CreateDirectory(path, NULL)  &&  GetLastError() != ERROR_ALREADY_EXISTS)
          return 0;
      }
      *pathPtr++ = '\\';
#else
      if (mkdir(path, 0777)  &&  errno != EEXIST)
        return 0;
      *pathPtr++ = '/';
#endif
    } else
      *pathPtr++ = ch;
  }

  return 1;
}


char emptyMD5[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define Z_BUFSIZE 16384

#if defined(_MSC_VER)
#include <pshpack1.h>
#endif
#if defined(__GNUC__)  ||  defined(__CWCC__)  ||  defined(__MWERKS__)
#pragma pack(push, 1)
#endif

struct ZipDirHeader
{
	enum
	{
		SIGNATURE = 0x06054b50,
	};
	union
	{
		UINT   signature;
		BYTE   signatureBytes[4];
	};
	WORD   number_disk;
	WORD   number_disk_with_CD;
	WORD   number_entry_this_disk;
	WORD    number_entry_CD;
	UINT   size_central_dir;
	UINT   offset_central_dir;
	WORD    size_comment;
#if defined(__GNUC__)  ||  defined(__CWCC__)  ||  defined(__MWERKS__)
} __attribute__((packed));
#else
};
#endif


struct ZipFileHeader
{
	enum
	{
		SIGNATURE   = 0x02014b50
	};

	UINT signature;
    WORD version;               /* version made by                 2 bytes */
    WORD version_needed;        /* version needed to extract       2 bytes */
    WORD flag;                  /* general purpose bit flag        2 bytes */
    WORD compression_method;    /* compression method              2 bytes */
    UINT dosDate;              /* last mod file date in Dos fmt   4 bytes */
    UINT crc;                  /* crc-32                          4 bytes */
    UINT compressed_size;      /* compressed size                 4 bytes */
    UINT uncompressed_size;    /* uncompressed size               4 bytes */
    WORD size_filename;         /* fileName length                 2 bytes */
    WORD size_file_extra;       /* extra field length              2 bytes */
    WORD size_file_comment;     /* file comment length             2 bytes */

    WORD disk_num_start;        /* disk number start               2 bytes */
    WORD internal_fa;           /* internal file attributes        2 bytes */
    UINT external_fa;          /* external file attributes        4 bytes */

    UINT offset_curfile;       /* file offset                     4 bytes */

	const char* GetName() const		{  return (const char *)(this + 1);  }
	const char* GetExtra() const	{  return GetName() + size_filename;  }
	const char* GetComment() const	{  return GetExtra() + size_file_extra;  }
#if defined(__GNUC__)  ||  defined(__CWCC__)  ||  defined(__MWERKS__)
} __attribute__((packed));
#else
};
#endif


struct ZipLocalHeader
{
	enum
	{
		SIGNATURE = 0x04034b50
	};
	UINT signature;
	WORD version_needed;		/* version needed to extract       2 bytes */
    WORD flag;                  /* general purpose bit flag        2 bytes */
    WORD compression_method;    /* compression method              2 bytes */
    UINT dosDate;              /* last mod file date in Dos fmt   4 bytes */
    UINT crc;                  /* crc-32                          4 bytes */
    UINT compressed_size;      /* compressed size                 4 bytes */
    UINT uncompressed_size;    /* uncompressed size               4 bytes */
    WORD size_filename;         /* fileName length                 2 bytes */
    WORD size_file_extra;       /* extra field length              2 bytes */
#if defined(__GNUC__)  ||  defined(__CWCC__)  ||  defined(__MWERKS__)
} __attribute__((packed));
#else
};
#endif

struct ZipExtraHeader
{
	union
	{
		unsigned short headerId;
		unsigned char headerIdBytes[2];
	};
	unsigned short dataSize;
};

struct FWKCS_MD5
{
	enum
	{
		HEADER_ID_A = 0x4b,
		HEADER_ID_B = 0x46
	};

	union
	{
		unsigned short headerId;
		unsigned char headerIdBytes[2];
	};
	unsigned short dataSize;
	unsigned char preface[3];
	unsigned char md5[16];
#if defined(__GNUC__)  ||  defined(__CWCC__)  ||  defined(__MWERKS__)
} __attribute__((packed));
#else
};
#endif

#if defined(__GNUC__)  ||  defined(__CWCC__)  ||  defined(__MWERKS__)
#pragma pack(pop)
#endif
#if defined(_MSC_VER)
#include <poppack.h>
#endif


static DWORD ConvertTime_tToDosTime(time_t time)
{
    struct tm* ptm = localtime(&time);
	DWORD year = (DWORD)ptm->tm_year;
	if (year > 1980)
		year -= 1980;
	else if (year > 80)
		year -= 80;
	return (DWORD) (((ptm->tm_mday) + (32 * (ptm->tm_mon+1)) + (512 * year)) << 16) |
			((ptm->tm_sec/2) + (32* ptm->tm_min) + (2048 * (DWORD)ptm->tm_hour));
}


static time_t ConvertDosDateToTime_t(DWORD dosDate)
{
	struct tm atm;
    ULONG uDate;
    uDate = (ULONG)(dosDate>>16);
    atm.tm_mday = (UINT)(uDate&0x1f);
    atm.tm_mon =  (UINT)(((uDate&0x1E0)/0x20)-1);
    atm.tm_year = (UINT)(((uDate&0x0FE00)/0x0200)+1980)-1900;

	atm.tm_hour = (UINT) ((dosDate & 0xF800)/0x800);
    atm.tm_min =  (UINT) ((dosDate & 0x7E0)/0x20);
    atm.tm_sec =  (UINT) (2*(dosDate & 0x1f));

	atm.tm_isdst = -1;

	return mktime(&atm);
}


inline char SwapEndian(char val)
{
	return val;
}


inline unsigned char SwapEndian(unsigned char val)
{
	return val;
}


inline short SwapEndian(short val)
{
	return (short)(((unsigned short)val >> 8) | ((unsigned short)val << 8));
}


inline unsigned short SwapEndian(unsigned short val)
{
	return (unsigned short)((val >> 8) | (val << 8));
}


inline unsigned int SwapEndian(unsigned int val)
{
//	return ((val >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | (val << 24));
	BYTE out[4];
	BYTE* in = (BYTE*)&val;
	out[0] = in[3];
	out[1] = in[2];
	out[2] = in[1];
	out[3] = in[0];
	return *(unsigned int*)&out;
}


inline int SwapEndian(int val)
{
	return SwapEndian((unsigned int)val);
//	return (int)(((unsigned int)val >> 24) | (((unsigned int)val & 0x00ff0000) >> 8) | (((unsigned int)val & 0x0000ff00) << 8) | ((unsigned int)val << 24));
}

///////////////////////////////////////////////////////////////////////////////
#if 0
/**
	\internal Used with qsort().
**/
static int MISC_CDECL Compare(VirtualDrive::FileEntry* item1, VirtualDrive::FileEntry* item2)
{
#if defined(WIN32)
	return (stricmp(item1->GetFileName(), item2->GetFileName()));
#else
	return (strcasecmp(item1->GetFileName(), item2->GetFileName()));
#endif
}

#endif


class VirtualFileHandle::Detail
{
public:
	Detail()
		: fileEntryIndex(VirtualDrive::INVALID_FILE_ENTRY)
		, bufferedData(NULL)
		, curUncompressedFilePosition(0)
	{
	}

	size_t fileEntryIndex;
    size_t headerSize;
    z_stream stream;
    BYTE* bufferedData;
	DWORD posInBufferedData;
	ULONGLONG curCompressedFilePosition;
	ULONGLONG curUncompressedFilePosition;
#if VIRTUALDRIVE_ENCRYPTION
	fcrypt_ctx  zcx[1];
#endif // VIRTUALDRIVE_ENCRYPTION
	MD5_CTX md5writecontext;
};


VirtualFileHandle::VirtualFileHandle()
	: nextOpenFile(NULL)
	, prevOpenFile(NULL)
	, parentDrive(NULL)
{
	this->detail = new Detail();
}


VirtualFileHandle::~VirtualFileHandle()
{
	if (this->detail->fileEntryIndex != VirtualDrive::INVALID_FILE_ENTRY  &&  this->parentDrive)
		this->parentDrive->FileClose(*this);
	delete this->detail;
}


VirtualDrive* VirtualFileHandle::GetParentDrive() const
{
	return this->parentDrive;
}


bool VirtualFileHandle::IsValid() const
{
	return this->detail->fileEntryIndex != size_t(-1) /*VirtualDrive::INVALID_FILE_ENTRY*/;
}


///////////////////////////////////////////////////////////////////////////////
/* simple entropy collection function that uses the fast timer      */
/* since we are not using the random pool for generating secret     */
/* keys we don't need to be too worried about the entropy quality   */
#if 0
static int entropy_fun(unsigned char buf[], unsigned int len)
{
#if defined(WIN32)
	unsigned __int64    pentium_tsc[1];
	unsigned int        i;

	QueryPerformanceCounter((LARGE_INTEGER *)pentium_tsc);
	for(i = 0; i < 8 && i < len; ++i)
	buf[i] = ((unsigned char*)pentium_tsc)[i];
	return i;
#elif defined(__GNUC__)
	CORE_ASSERT(0);
	return 0;
#else
	CORE_ASSERT(0);
	return 0;
#endif
}
#endif

/**
	The constructor.
**/
VirtualDrive::VirtualDrive() :
	m_fileEntryCount(0),
	m_fileEntryMaxCount(0),
	m_dirOffset(0xffffffff),
	m_dirSize(0),
	m_needsPack(false),
	m_readOnly(false),
	m_unused(0),
	m_fileName(NULL),
	m_fileEntryOffsets(NULL),
	m_fileEntries(NULL),
	m_fileEntriesSizeBytes(0),
	m_fileEntriesMaxSizeBytes(0),
	m_parentFile(NULL),
	m_ownParentFile(false),
	m_changed(false),
	m_curWriteFile(NULL),
	m_headOpenFile(NULL),
	m_swap(false)
{
//	prng_init(entropy_fun, rng);                /* initialise RNG   */
} // VirtualDrive()


///////////////////////////////////////////////////////////////////////////////
/**
	The destructor automatically closes all open virtual files and the virtual
	drive.
**/
VirtualDrive::~VirtualDrive()
{
	Close();
} // ~VirtualDrive()


#if VIRTUALDRIVE_ENCRYPTION

///////////////////////////////////////////////////////////////////////////////
static void OpenHeaderCryptContext(const HeapString& password, fcrypt_ctx zcx[1])
{
	int passwordLen = (int)password.Length();
	unsigned char encryptionMode = (passwordLen < 32 ? 1 : passwordLen < 48 ? 2 : 3);

	unsigned char salt[16];
//	prng_ctx rng[1];    /* the context for the random number pool   */
//	prng_init(entropy_fun, rng);                /* initialise RNG   */
//	prng_rand(salt, SALT_LENGTH(encryptionMode), rng);    /* and the salt     */

	unsigned int value = 255;
	unsigned int subtractValue = 255 / SALT_LENGTH(encryptionMode);
	for (int i = 0; i < SALT_LENGTH(encryptionMode); ++i)
	{
		salt[i] = value;
		value -= subtractValue;
	}

	unsigned char tmp_buf1[16];
	fcrypt_init(encryptionMode, (unsigned char*)(const char*)password, (unsigned int)passwordLen, salt, tmp_buf1, zcx);
}

#endif

/**
	Creates a new virtual drive on the disk.  The new virtual drive has no
	file entries.

	@param fileName The full path of the virtual drive.
	@return Returns true if the virtual drive was created successfully, false
		otherwise.
**/
bool VirtualDrive::Create(const char* fileName, const char* password, DWORD flags)
{
	Close();

	DiskFile* parentFile = new DiskFile;
    if (!parentFile->Open(fileName, File::MODE_CREATE | File::MODE_TRUNCATE | File::MODE_READWRITE))
	{
		delete parentFile;
		return false;
	}

	bool ret = Create(*parentFile, fileName, password, flags);
	if (!ret)
	{
		delete parentFile;
		m_parentFile = NULL;
	}
	m_ownParentFile = true;
	return ret;
} // Create()


bool VirtualDrive::Create(File& parentFile, const char* fileName, const char* _defaultPassword, DWORD flags)
{
#if !VIRTUALDRIVE_ENCRYPTION
	(void)_defaultPassword;
#endif // VIRTUALDRIVE_ENCRYPTION
	Close();

	m_flags = flags;

	// Save the fileName.
    delete[] m_fileName;
	m_fileName = new char[strlen(fileName) + 1];
    strcpy(m_fileName, fileName);

	// Okay, see if we're big endian.
	union
	{
		UINT   signature;
		BYTE   signatureBytes[4];
	};
	signatureBytes[0] = 0x50;
	signatureBytes[1] = 0x4b;
	signatureBytes[2] = 0x05;
	signatureBytes[3] = 0x06;

	m_swap = false;
	if (signature != ZipDirHeader::SIGNATURE)
	{
		m_swap = true;
	}

#if VIRTUALDRIVE_ENCRYPTION
	// Start the encryption.
	this->defaultPassword = _defaultPassword;
	if (this->defaultPassword.Length() > 0)
	{
		OpenHeaderCryptContext(this->defaultPassword, this->defaultzcx);
	}
#endif // VIRTUALDRIVE_ENCRYPTION

	// Create the virtual drive disk file.
	m_ownParentFile = false;
	m_parentFile = &parentFile;

	m_readOnly = false;

	// Remove all directory entries.
	delete[] m_fileEntries;
    m_fileEntries = NULL;
	m_curWriteFile = NULL;

	// Write the header.
	m_dirOffset = 0;
	m_dirSize = 0;
	m_needsPack = false;
	this->fileNameMap.Clear();
	this->fileNameMap.SetBlockSize(100);
	this->fileNameMap.SetHashTableSize(100);
    m_changed = true;

    Flush();

	// Created successfully!
	return true;
} // Create()


///////////////////////////////////////////////////////////////////////////////
/**
	Opens an existing virtual drive from the disk.

	@param fileName The full path of the virtual drive.
	@return Returns true if the virtual drive was opened successfully, false
		otherwise.
**/
bool VirtualDrive::Open(const char* fileName, bool readOnly, const char* password, DWORD flags)
{
	Close();

	DiskFile* parentFile = new DiskFile();

	if (readOnly)
	{
		if (!parentFile->Open(fileName, File::MODE_READONLY))
		{
			// Couldn't open the file!
			delete parentFile;

			return false;
		}
	}
	else if (!parentFile->Open(fileName, File::MODE_READWRITE))
	{
		if (!parentFile->Open(fileName, File::MODE_READONLY))
		{
			// Couldn't open the file!
			delete parentFile;

			return false;
		}

		readOnly = true;
	}

	bool ret = Open(*parentFile, fileName, readOnly, password, flags);
	if (!ret)
	{
		delete parentFile;
		m_parentFile = NULL;
	}
	m_ownParentFile = true;
	return ret;
}


bool VirtualDrive::Open(File& parentFile, const char* fileName, bool readOnly, const char* _defaultPassword, DWORD flags)
{
#if !VIRTUALDRIVE_ENCRYPTION
	(void)_defaultPassword;
#endif // VIRTUALDRIVE_ENCRYPTION
	Close();

	m_flags = flags;

	// Save the fileName.
	m_fileName = new char[strlen(fileName) + 1];
    strcpy(m_fileName, fileName);

	// Open the file.
	m_ownParentFile = false;
	m_parentFile = &parentFile;
	m_readOnly = readOnly;

#if VIRTUALDRIVE_ENCRYPTION
	this->defaultPassword = _defaultPassword;

	// Initialize decryption, if needed.
	fcrypt_ctx zcx[1];
	if (this->defaultPassword.Length() > 0)
	{
		OpenHeaderCryptContext(this->defaultPassword, this->defaultzcx);
		zcx[0] = this->defaultzcx[0];
	}
#endif // VIRTUALDRIVE_ENCRYPTION

	// See if it is a .zip file.
	ZipDirHeader zipDirHeader;
	m_parentFile->Seek(m_parentFile->GetLength() - sizeof(ZipDirHeader));
	m_parentFile->Read(&zipDirHeader, sizeof(ZipDirHeader));
#if VIRTUALDRIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
	{
		zcx[0] = this->defaultzcx[0];
		fcrypt_decrypt((unsigned char*)&zipDirHeader, sizeof(ZipDirHeader), zcx);
	}
#endif // VIRTUALDRIVE_ENCRYPTION

	// Test the zip signature.
	if (zipDirHeader.signatureBytes[0] != 0x50  ||  zipDirHeader.signatureBytes[1] != 0x4b  ||  zipDirHeader.signatureBytes[2] != 0x05  ||  zipDirHeader.signatureBytes[3] != 0x06)
	{
		return false;
	}

	// Okay, see if it is big endian.
	m_swap = false;
	if (zipDirHeader.signature != ZipDirHeader::SIGNATURE)
	{
		m_swap = true;
	}

	if (m_swap)
	{
		zipDirHeader.signature = SwapEndian(zipDirHeader.signature);
		zipDirHeader.number_disk = SwapEndian(zipDirHeader.number_disk);
		zipDirHeader.number_disk_with_CD = SwapEndian(zipDirHeader.number_disk_with_CD);
		zipDirHeader.number_entry_this_disk = SwapEndian(zipDirHeader.number_entry_this_disk);
		zipDirHeader.number_entry_CD = SwapEndian(zipDirHeader.number_entry_CD);
		zipDirHeader.size_central_dir = SwapEndian(zipDirHeader.size_central_dir);
		zipDirHeader.offset_central_dir = SwapEndian(zipDirHeader.offset_central_dir);
		zipDirHeader.size_comment = SwapEndian(zipDirHeader.size_comment);
	}

	// Allocate the data buffer, and read the whole thing.
	// TODO: Allocate this up top so it doesn't fragment.
	BYTE* zipDirBuffer = new BYTE[zipDirHeader.size_central_dir];
	m_parentFile->Seek(zipDirHeader.offset_central_dir);
	m_parentFile->Read(zipDirBuffer, zipDirHeader.size_central_dir);

#if VIRTUALDRIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
	{
		zcx[0] = this->defaultzcx[0];
		fcrypt_decrypt((unsigned char*)zipDirBuffer, zipDirHeader.size_central_dir, zcx);
	}
#endif // VIRTUALDRIVE_ENCRYPTION

	BYTE* zipDirPtr = zipDirBuffer;

	m_dirOffset = zipDirHeader.offset_central_dir;
	m_dirSize = zipDirHeader.size_central_dir;

	DWORD totalSize = 0;
    m_fileEntryCount = zipDirHeader.number_entry_CD;
	m_fileEntryMaxCount = m_fileEntryCount;
    if (m_fileEntryCount > 0)
        m_fileEntryOffsets = new size_t[m_fileEntryCount];
	this->fileNameMap.SetBlockSize(m_fileEntryCount + 1);
	this->fileNameMap.SetHashTableSize(m_fileEntryCount / 5);
	for (size_t i = 0; i < m_fileEntryCount; ++i)
	{
		ZipFileHeader* zipFileHeader = (ZipFileHeader*)zipDirPtr;

		if (m_swap)
		{
			zipFileHeader->signature = SwapEndian(zipFileHeader->signature);
			zipFileHeader->version = SwapEndian(zipFileHeader->version);
			zipFileHeader->version_needed = SwapEndian(zipFileHeader->version_needed);
			zipFileHeader->flag = SwapEndian(zipFileHeader->flag);
			zipFileHeader->compression_method = SwapEndian(zipFileHeader->compression_method);
			zipFileHeader->dosDate = SwapEndian(zipFileHeader->dosDate);
			zipFileHeader->crc = SwapEndian(zipFileHeader->crc);
			zipFileHeader->compressed_size = SwapEndian(zipFileHeader->compressed_size);
			zipFileHeader->uncompressed_size = SwapEndian(zipFileHeader->uncompressed_size);
			zipFileHeader->size_filename = SwapEndian(zipFileHeader->size_filename);
			zipFileHeader->size_file_extra = SwapEndian(zipFileHeader->size_file_extra);
			zipFileHeader->size_file_comment = SwapEndian(zipFileHeader->size_file_comment);
			zipFileHeader->disk_num_start = SwapEndian(zipFileHeader->disk_num_start);
			zipFileHeader->internal_fa = SwapEndian(zipFileHeader->internal_fa);
			zipFileHeader->external_fa = SwapEndian(zipFileHeader->external_fa);
			zipFileHeader->offset_curfile = SwapEndian(zipFileHeader->offset_curfile);
		}

		if (zipFileHeader->signature != ZipFileHeader::SIGNATURE)
		{
			// Bad zip file.
			delete[] zipDirBuffer;
			Close();
			return false;
		}

        m_fileEntryOffsets[i] = totalSize;
		totalSize += sizeof(FileEntry) + zipFileHeader->size_filename;
		zipDirPtr += sizeof(ZipFileHeader) + zipFileHeader->size_filename + zipFileHeader->size_file_extra + zipFileHeader->size_file_comment;
	}
	zipDirPtr = zipDirBuffer;

	m_fileEntries = new BYTE[totalSize];
    m_fileEntriesSizeBytes = totalSize;
	m_fileEntriesMaxSizeBytes = totalSize;

    BYTE* fileEntryPtr = m_fileEntries;

	for (size_t i = 0; i < zipDirHeader.number_entry_CD; ++i)
	{
		ZipFileHeader* zipFileHeader = (ZipFileHeader*)zipDirPtr;

		FileEntry& fileEntry = *(FileEntry*)fileEntryPtr;
		fileEntry.m_fileTime = ConvertDosDateToTime_t(zipFileHeader->dosDate);
		fileEntry.m_offset = zipFileHeader->offset_curfile;
		fileEntry.m_compressedSize = zipFileHeader->compressed_size;
		fileEntry.m_uncompressedSize = zipFileHeader->uncompressed_size;
		fileEntry.m_crc = zipFileHeader->crc;
		fileEntry.m_compressionMethod = zipFileHeader->compression_method;
        fileEntry.m_parentDrive = this;

		WORD stringLen = zipFileHeader->size_filename;
		memcpy(fileEntry.m_fileName, zipFileHeader->GetName(), stringLen);
        fileEntry.m_fileName[stringLen] = 0;

		this->fileNameMap[PtrString(this, i)] = i;

		fileEntryPtr += sizeof(FileEntry) + stringLen;

		bool setMD5 = false;

		BYTE* extraPtr = (BYTE*)zipFileHeader->GetExtra();
		size_t extraSize = zipFileHeader->size_file_extra;
		while (extraSize)
		{
			ZipExtraHeader* extraHeader = (ZipExtraHeader*)extraPtr;
			if (m_swap)
			{
				extraHeader->dataSize = SwapEndian(extraHeader->dataSize);
			}

			if (extraHeader->headerIdBytes[0] == FWKCS_MD5::HEADER_ID_A  &&  extraHeader->headerIdBytes[1] == FWKCS_MD5::HEADER_ID_B)
			{
				if (m_flags & SUPPORT_MD5)
				{
					memcpy(fileEntry.m_md5, extraPtr + 4 + 3, 16);
					setMD5 = 1;
				}
			}

			extraSize -= 4 + extraHeader->dataSize;
			extraPtr += 4 + extraHeader->dataSize;
		}

		if (!setMD5)
			memset(fileEntry.m_md5, 0, sizeof(fileEntry.m_md5));

		zipDirPtr += sizeof(ZipFileHeader) + zipFileHeader->size_filename + zipFileHeader->size_file_extra + zipFileHeader->size_file_comment;
	}

	delete[] zipDirBuffer;

	this->fileNameMap.SetBlockSize(50);

	// Finish setting it up.
	m_changed = false;
	m_curWriteFile = NULL;

	// Opened successfully!
	return true;
} // Open()


///////////////////////////////////////////////////////////////////////////////
/**
	Closes the virtual drive, flushing any unsaved changed to disk.  All open
	virtual files are closed.

	@return Returns true if successful.
**/
bool VirtualDrive::Close()
{
	if (!IsOpened())
		return true;

	// Flush unsaved changed.
	Flush();

	// Close all open virtual files.
	FileCloseAll();

	// Remove all entries from the directory.
	delete[] m_fileEntries;
    m_fileEntries = NULL;
    m_fileEntriesSizeBytes = 0;
	m_fileEntriesMaxSizeBytes = 0;

    delete[] m_fileEntryOffsets;
    m_fileEntryOffsets = NULL;
    m_fileEntryCount = 0;
	m_fileEntryMaxCount = 0;

	this->fileNameMap.Clear();

	// Clear out the drive's fileName.
	delete[] m_fileName;
    m_fileName = NULL;

#if VIRTUALDRIVE_ENCRYPTION
	// Clear the password.
	this->defaultPassword.Clear();
#endif // VIRTUALDRIVE_ENCRYPTION

	// Destroy the File.
	if (m_ownParentFile)
	{
		delete m_parentFile;
	}
	m_parentFile = NULL;
	m_ownParentFile = false;

    m_dirOffset = 0;
    m_dirSize = 0;

	return true;
} // Close()


///////////////////////////////////////////////////////////////////////////////
void VirtualDrive::UpdateMD5s()
{
	// Test all the directory entries.
	const DWORD FILE_BLOCK_SIZE = 32768;
	BYTE* buffer = NULL;
	for (size_t i = 0; i < m_fileEntryCount; ++i)
	{
		FileEntry& entry = *GetFileEntry(i);
		if (memcmp(entry.GetMD5(), emptyMD5, sizeof(emptyMD5)) == 0)
		{
			if (!buffer)
				buffer = new BYTE[FILE_BLOCK_SIZE];

			VirtualFileHandle fileHandle;
			if (FileOpen(entry.GetFileName(), fileHandle))
			{
				MD5_CTX c;
				MD5Init(&c);

				DWORD bytesToDo = entry.GetUncompressedSize();
				while (bytesToDo > 0)
				{
					DWORD bytesToRead = bytesToDo < FILE_BLOCK_SIZE ? bytesToDo : FILE_BLOCK_SIZE;
					FileRead(fileHandle, buffer, bytesToRead);
					bytesToDo -= bytesToRead;
					MD5Update(&c, (unsigned char*)buffer, bytesToRead);
				}

				FileClose(fileHandle);

				unsigned char digest[16];
				MD5Final(digest, &c);

				entry.SetMD5(digest);
			}
		}
	}

	delete [] buffer;
}


/**
**/
void VirtualDrive::_WriteDirectory(LONGLONG dirOffset, LONGLONG dirHeaderOffset)
{
#if VIRTUALDRIVE_ENCRYPTION
	fcrypt_ctx zcx[1];
	if (this->defaultPassword.Length() > 0)
		zcx[0] = this->defaultzcx[0];
#endif

	// MD5 structure.
	FWKCS_MD5 md5info;
	if (m_flags & SUPPORT_MD5)
	{
		md5info.headerIdBytes[0] = FWKCS_MD5::HEADER_ID_A;
		md5info.headerIdBytes[1] = FWKCS_MD5::HEADER_ID_B;
		md5info.dataSize = 3 + 16;
		md5info.preface[0] = 'M';
		md5info.preface[1] = 'D';
		md5info.preface[2] = '5';
		if (m_swap)
		{
			md5info.dataSize = SwapEndian(md5info.dataSize);
		}
	}

	// Write all the directory entries.
    for (size_t i = 0; i < m_fileEntryCount; ++i)
	{
		FileEntry& entry = *GetFileEntry(i);
		ZipFileHeader zipFileHeader;
		zipFileHeader.signature = ZipFileHeader::SIGNATURE;
		zipFileHeader.version = 20;
		zipFileHeader.version_needed = 20;

		zipFileHeader.flag = 0;
		zipFileHeader.compression_method = (WORD)entry.m_compressionMethod;
		zipFileHeader.dosDate = ConvertTime_tToDosTime(entry.m_fileTime);
//		time_t t = ConvertDosDateToTime_t(zipFileHeader.dosDate);
		zipFileHeader.crc = entry.m_crc;
		zipFileHeader.compressed_size = entry.m_compressedSize;
		zipFileHeader.uncompressed_size = entry.m_uncompressedSize;
		int size_filename = (int)strlen(entry.GetFileName());
		zipFileHeader.size_filename = (WORD)size_filename;
		int size_file_extra = (m_flags & SUPPORT_MD5) ? sizeof(md5info) : 0;
		zipFileHeader.size_file_extra = size_file_extra;
		zipFileHeader.size_file_comment = 0;
		zipFileHeader.disk_num_start = 0;
		zipFileHeader.internal_fa = 0;
		zipFileHeader.external_fa = 0;
		zipFileHeader.offset_curfile = entry.m_offset;

		if (m_swap)
		{
			zipFileHeader.signature = SwapEndian(zipFileHeader.signature);
			zipFileHeader.version = SwapEndian(zipFileHeader.version);
			zipFileHeader.version_needed = SwapEndian(zipFileHeader.version_needed);
			zipFileHeader.flag = SwapEndian(zipFileHeader.flag);
			zipFileHeader.compression_method = SwapEndian(zipFileHeader.compression_method);
			zipFileHeader.dosDate = SwapEndian(zipFileHeader.dosDate);
			zipFileHeader.crc = SwapEndian(zipFileHeader.crc);
			zipFileHeader.compressed_size = SwapEndian(zipFileHeader.compressed_size);
			zipFileHeader.uncompressed_size = SwapEndian(zipFileHeader.uncompressed_size);
			zipFileHeader.size_filename = SwapEndian(zipFileHeader.size_filename);
			zipFileHeader.size_file_extra = SwapEndian(zipFileHeader.size_file_extra);
			zipFileHeader.size_file_comment = SwapEndian(zipFileHeader.size_file_comment);
			zipFileHeader.disk_num_start = SwapEndian(zipFileHeader.disk_num_start);
			zipFileHeader.internal_fa = SwapEndian(zipFileHeader.internal_fa);
			zipFileHeader.external_fa = SwapEndian(zipFileHeader.external_fa);
			zipFileHeader.offset_curfile = SwapEndian(zipFileHeader.offset_curfile);
		}

		m_parentFile->Seek(dirOffset);
#if VIRTUALDRIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
			fcrypt_encrypt((unsigned char*)&zipFileHeader, sizeof(ZipFileHeader), zcx);
#endif
		m_parentFile->Write(&zipFileHeader, sizeof(ZipFileHeader));
#if VIRTUALDRIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
		{
			HeapString tempFileName = entry.GetFileName();
			fcrypt_encrypt((unsigned char*)(const char*)tempFileName, (unsigned int)tempFileName.Length(), zcx);
			m_parentFile->Write((const char*)tempFileName, tempFileName.Length());
		}
		else
#endif // VIRTUALDRIVE_ENCRYPTION
		{
			m_parentFile->Write((const char*)entry.GetFileName(), zipFileHeader.size_filename);
		}

		// Write MD5.
		if (m_flags & SUPPORT_MD5)
		{
			memcpy(md5info.md5, entry.GetMD5(), 16);
			m_parentFile->Write(&md5info, sizeof(md5info));
		}

		dirOffset += sizeof(ZipFileHeader) + size_filename + size_file_extra;
	}

#if VIRTUALDRIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
		zcx[0] = this->defaultzcx[0];
#endif // VIRTUALDRIVE_ENCRYPTION

	ZipDirHeader zipDirHeader;
	zipDirHeader.signature = ZipDirHeader::SIGNATURE;
	zipDirHeader.number_disk = 0;
	zipDirHeader.number_disk_with_CD = 0;
	zipDirHeader.number_entry_this_disk = (WORD)m_fileEntryCount;
	zipDirHeader.number_entry_CD = (WORD)m_fileEntryCount;
	zipDirHeader.size_central_dir = (UINT)m_dirSize;
	zipDirHeader.offset_central_dir = (UINT)m_dirOffset;
	zipDirHeader.size_comment = 0;

	if (m_swap)
	{
		zipDirHeader.signature = SwapEndian(zipDirHeader.signature);
		zipDirHeader.number_disk = SwapEndian(zipDirHeader.number_disk);
		zipDirHeader.number_disk_with_CD = SwapEndian(zipDirHeader.number_disk_with_CD);
		zipDirHeader.number_entry_this_disk = SwapEndian(zipDirHeader.number_entry_this_disk);
		zipDirHeader.number_entry_CD = SwapEndian(zipDirHeader.number_entry_CD);
		zipDirHeader.size_central_dir = SwapEndian(zipDirHeader.size_central_dir);
		zipDirHeader.offset_central_dir = SwapEndian(zipDirHeader.offset_central_dir);
		zipDirHeader.size_comment = SwapEndian(zipDirHeader.size_comment);
	}

#if VIRTUALDRIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
		fcrypt_encrypt((unsigned char*)&zipDirHeader, sizeof(ZipDirHeader), zcx);
#endif // VIRTUALDRIVE_ENCRYPTION

	m_parentFile->Seek(dirHeaderOffset);
	m_parentFile->Write(&zipDirHeader, sizeof(ZipDirHeader));
}


/**
	Sorts the directory entries by fileName and writes it all to disk.
**/
bool VirtualDrive::Flush(const FlushOptions* flushOptions)
{
	if (m_readOnly)
		return false;

	// If there is no parent file, then the drive has never been opened.  Exit.
	if (!m_parentFile)
		return false;

	// If there have been no changes made, exit successfully.
	if (!m_changed)
		return true;

	// Write the directory header id.
	size_t directorySize = 0;
	size_t lowestFileOffset = (size_t)-1;
	for (size_t i = 0; i < m_fileEntryCount; ++i)
	{
		FileEntry& entry = *GetFileEntry(i);
		if (entry.GetOffset() < lowestFileOffset)
			lowestFileOffset = entry.GetOffset();
		int size_filename = (int)strlen(entry.GetFileName());
		int size_file_extra = (m_flags & SUPPORT_MD5) ? sizeof(FWKCS_MD5) : 0;
		directorySize += sizeof(ZipFileHeader) + size_filename + size_file_extra;
	}

	m_dirSize = directorySize;

	_WriteDirectory(m_dirOffset, m_dirOffset + directorySize);
	m_parentFile->SetLength(m_dirOffset + directorySize + sizeof(ZipDirHeader));

	// Reset to no changes.
	m_changed = false;

	if ((m_flags & EXTRA_DIRECTORY_AT_BEGINNING)  &&  m_fileEntryCount > 0)
	{
		if (lowestFileOffset == directorySize + sizeof(ZipDirHeader))
			_WriteDirectory(sizeof(ZipDirHeader), 0);
		else {
			PackOptions packOptions;
			packOptions.fileOrder = flushOptions ? flushOptions->fileOrder : NULL;
			Pack(&packOptions);
		}
	}

	return true;
} // Flush()


///////////////////////////////////////////////////////////////////////////////
/**
	Creates a new virtual file within the virtual drive for writing. Only one
	virtual file may be written to at any one time, because the new file is
	appended to the end of the virtual drive.

	If the new virtual file already exists within the directory entries, the
	old file of the same name is invalidated. The space the old file took
	becomes unused and not retrievable.  To "retrieve" the space, the virtual
	drive must be packed (see Pack()).

	@param fileName The fileName of the new virtual file to create.
	@param fileHandle A reference to the VirtualFileHandle object to be filled in
		with the newly created virtual file's information.
	@param fileTime A CTime object describing the virtual file's creation date
		and time.  The current date and time is used if fileTime is NULL,
	@return Returns true if the virtual file was created successfully, false
		otherwise.
**/
bool VirtualDrive::FileCreate(const char* fileName, VirtualFileHandle& fileHandle,
							  UINT compressionMethod, const time_t* fileTime)
{
	// If the drive is read-only, then exit.
	if (m_readOnly  ||  !m_parentFile)
		return false;

	// If there is a file currently being written to, then it must be closed
	// first.  Abort the file creation.
	if (m_curWriteFile)
		return false;

	// Close it if already opened.
	FileClose(fileHandle);

	// Retrieve the index of the file entry called fileName.
	size_t index = FindFileEntryIndex(fileName);

	// Does the entry exist?
	if (index == INVALID_FILE_ENTRY)
	{
		// No. It needs to be added.  Increase the file entry count by 1.
        index = m_fileEntryCount;

        size_t* origOffsets = m_fileEntryOffsets;
		if (m_fileEntryCount >= m_fileEntryMaxCount)
		{
			m_fileEntryMaxCount += 100;
	        m_fileEntryOffsets = new size_t[m_fileEntryMaxCount + 1];
			if (origOffsets)
			{
	            memcpy(m_fileEntryOffsets, origOffsets, m_fileEntryCount * sizeof(UINT));
				delete[] origOffsets;
			}
		}

		m_fileEntryOffsets[m_fileEntryCount] = m_fileEntriesSizeBytes;
		m_fileEntryCount++;

		size_t stringLen = strlen(fileName);
		BYTE* origBuf = m_fileEntries;
		size_t origSize = m_fileEntriesSizeBytes;

        m_fileEntriesSizeBytes += sizeof(FileEntry) + stringLen;
		if (m_fileEntriesSizeBytes >= m_fileEntriesMaxSizeBytes)
		{
			m_fileEntriesMaxSizeBytes += sizeof(FileEntry) + stringLen + 10 * 1024;
			m_fileEntries = new BYTE[m_fileEntriesMaxSizeBytes];
			if (origSize > 0)
				memcpy(m_fileEntries, origBuf, origSize);
			if (origBuf)
				delete[] origBuf;
		}
        FileEntry* destEntry = (FileEntry*)(m_fileEntries + origSize);
		memcpy(destEntry->m_fileName, fileName, stringLen + 1);

		this->fileNameMap[PtrString(this, m_fileEntryCount - 1)] = index;
	}
	else
	{
		m_needsPack = true;
	}

	// Fill in the new (or existing file entry).
	FileEntry& fileEntry = *GetFileEntry(index);

	// Add the proper file time.
	if (!fileTime)
	{
		fileEntry.m_fileTime = AdjustTime_t(time(NULL));
	}
	else
	{
		fileEntry.m_fileTime = AdjustTime_t(*fileTime);
	}

	fileEntry.m_offset = (UINT)m_dirOffset;
    fileEntry.m_compressedSize = 0;
    fileEntry.m_uncompressedSize = 0;
	fileEntry.m_crc = 0;
	fileEntry.m_compressionMethod = compressionMethod == UNCOMPRESSED ? 0 : Z_DEFLATED;
    fileEntry.m_parentDrive = this;

	// Assign the appropriate information to the virtual file object.
    fileHandle.parentDrive = this;
	fileHandle.detail->fileEntryIndex = index;

    fileHandle.detail->curUncompressedFilePosition = 0;
    fileHandle.detail->curCompressedFilePosition = 0;

	if (compressionMethod == COMPRESSED)
	{
		fileHandle.detail->bufferedData = new BYTE[Z_BUFSIZE];
		fileHandle.detail->posInBufferedData = 0;

		fileHandle.detail->stream.avail_in = 0;
		fileHandle.detail->stream.avail_out = Z_BUFSIZE;
		fileHandle.detail->stream.next_out = fileHandle.detail->bufferedData;
		fileHandle.detail->stream.total_in = 0;
		fileHandle.detail->stream.total_out = 0;
		fileHandle.detail->stream.opaque = 0;
		fileHandle.detail->stream.zalloc = Z_NULL;
		fileHandle.detail->stream.zfree = Z_NULL;

		deflateInit2(&fileHandle.detail->stream, compressionMethod, Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, 0);
	}

	fileHandle.detail->headerSize = sizeof(ZipLocalHeader) + strlen(fileEntry.GetFileName()) + 0;

#if VIRTUALDRIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
	{
		fileHandle.detail->zcx[0] = this->defaultzcx[0];

		fileHandle.detail->curCompressedFilePosition += 0; //SALT_LENGTH(mode) + PWD_VER_LENGTH;
	}
#endif // VIRTUALDRIVE_ENCRYPTION

	if (m_flags & SUPPORT_MD5)
	{
		MD5Init(&fileHandle.detail->md5writecontext);
	}

    // Assign this file to be our current write file.
	m_curWriteFile = &fileHandle;

	// Add this virtual file to the open files list.
	fileHandle.nextOpenFile = m_headOpenFile;
	if (m_headOpenFile)
		m_headOpenFile->prevOpenFile = &fileHandle;
	fileHandle.prevOpenFile = NULL;
	m_headOpenFile = &fileHandle;

	// A change to the directory has occurred.
	m_changed = true;

	return true;
} // FileCreate()


///////////////////////////////////////////////////////////////////////////////
/**
	Opens an existing virtual file within the virtual drive.

	@param fileName The fileName of the existing virtual file to open.
	@param fileHandle A reference to the VirtualFileHandle object to be filled in
		with the newly opened virtual file's information.
	@return Returns true if the virtual file was opened successfully, false
		otherwise.
**/
bool VirtualDrive::FileOpen(const char* fileName, VirtualFileHandle& fileHandle)
{
	if (!m_parentFile)
		return false;

	// Close it if already opened.
	FileClose(fileHandle);

	// Retrieve the index of the file entry called fileName.
	size_t index = FindFileEntryIndex(fileName);
    if (index == INVALID_FILE_ENTRY)
        return false;

    return FileOpenIndex(index, fileHandle);
} // FileOpen()


bool VirtualDrive::FileOpenIndex(size_t index, VirtualFileHandle& fileHandle)
{
	// If the entry doesn't exist, then exit.
	if (index == INVALID_FILE_ENTRY)
		return false;

	if (!m_parentFile)
		return false;

	// Close it if already opened.
	FileClose(fileHandle);

	// Grab the entry.
	FileEntry& fileEntry = *GetFileEntry(index);

	// Assign the internal virtual file information.
    fileHandle.parentDrive = this;
	fileHandle.detail->fileEntryIndex = index;
    fileHandle.detail->curUncompressedFilePosition = 0;

	ZipLocalHeader localHeader;

	m_parentFile->Seek(fileEntry.m_offset);
	m_parentFile->Read(&localHeader, sizeof(ZipLocalHeader));

#if VIRTUALDRIVE_ENCRYPTION
	fcrypt_ctx headerzcx[1];
	if (this->defaultPassword.Length() > 0)
	{
		headerzcx[0] = this->defaultzcx[0];
		fcrypt_decrypt((unsigned char*)&localHeader, sizeof(ZipLocalHeader), headerzcx);
	}
#endif // VIRTUALDRIVE_ENCRYPTION

	if (m_swap)
	{
		localHeader.signature = SwapEndian(localHeader.signature);
		localHeader.version_needed = SwapEndian(localHeader.version_needed);
		localHeader.flag = SwapEndian(localHeader.flag);
		localHeader.compression_method = SwapEndian(localHeader.compression_method);
		localHeader.dosDate = SwapEndian(localHeader.dosDate);
		localHeader.crc = SwapEndian(localHeader.crc);
		localHeader.compressed_size = SwapEndian(localHeader.compressed_size);
		localHeader.uncompressed_size = SwapEndian(localHeader.uncompressed_size);
		localHeader.size_filename = SwapEndian(localHeader.size_filename);
		localHeader.size_file_extra = SwapEndian(localHeader.size_file_extra);
	}

	CORE_ASSERT(localHeader.signature == ZipLocalHeader::SIGNATURE);

	fileHandle.detail->headerSize = sizeof(ZipLocalHeader) + localHeader.size_filename + localHeader.size_file_extra;

    fileHandle.detail->curCompressedFilePosition = 0;

	if (fileEntry.m_compressionMethod != 0)
	{
		fileHandle.detail->stream.avail_in = 0;
	    fileHandle.detail->stream.total_out = 0;

	    fileHandle.detail->stream.opaque = 0;
        fileHandle.detail->bufferedData = new BYTE[Z_BUFSIZE];
        fileHandle.detail->stream.zalloc = Z_NULL;
        fileHandle.detail->stream.zfree = Z_NULL;

		inflateInit2(&fileHandle.detail->stream, -MAX_WBITS);
    }

	fileHandle.detail->headerSize = sizeof(ZipLocalHeader) + strlen(fileEntry.GetFileName()) + 0;

#if VIRTUALDRIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
	{
		fileHandle.detail->zcx[0] = this->defaultzcx[0];

		fileHandle.detail->curCompressedFilePosition += 0; //SALT_LENGTH(fileEntry.m_encryptionMode) + PWD_VER_LENGTH;
	}
#endif // VIRTUALDRIVE_ENCRYPTION

    // Add this virtual file to the open files list.
	fileHandle.nextOpenFile = m_headOpenFile;
	if (m_headOpenFile)
		m_headOpenFile->prevOpenFile = &fileHandle;
	fileHandle.prevOpenFile = NULL;
	m_headOpenFile = &fileHandle;

	return true;
} // FileOpen()


///////////////////////////////////////////////////////////////////////////////
/**
	Closes an open virtual file.  This function should never really need to be
	called directory, as VirtualFileHandle::Close() can be used instead.

	@param fileHandle The VirtualFileHandle object to be closed.
	@return Returns true if the file was closed successfully.
**/
bool VirtualDrive::FileClose(VirtualFileHandle& fileHandle)
{
	if (fileHandle.detail->fileEntryIndex == INVALID_FILE_ENTRY)
		return true;

	// Is the virtual file being closed the current virtual file being written
	// to?
	if (&fileHandle == m_curWriteFile)
	{
        VirtualDrive::FileEntry* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

		if (fileEntry->m_compressionMethod == COMPRESSED)
		{
			int err;
			do
			{
				ULONG uTotalOutBefore;
				if (fileHandle.detail->stream.avail_out == 0)
				{
					m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);
#if VIRTUALDRIVE_ENCRYPTION
					if (this->defaultPassword.Length() > 0)
						fcrypt_encrypt_offset(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // VIRTUALDRIVE_ENCRYPTION
					m_parentFile->Write(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData);
					fileHandle.detail->curCompressedFilePosition += fileHandle.detail->posInBufferedData;
					fileHandle.detail->posInBufferedData = 0;
					fileHandle.detail->stream.avail_out = (UINT)Z_BUFSIZE;
					fileHandle.detail->stream.next_out = fileHandle.detail->bufferedData;
				}
				uTotalOutBefore = fileHandle.detail->stream.total_out;
				err=deflate((::z_stream*)&fileHandle.detail->stream,  Z_FINISH);
				fileHandle.detail->posInBufferedData += (UINT)(fileHandle.detail->stream.total_out - uTotalOutBefore);
			} while (err == Z_OK);

			if (fileHandle.detail->posInBufferedData > 0  &&  (err == Z_OK  ||  err == Z_STREAM_END))
			{
				m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);
#if VIRTUALDRIVE_ENCRYPTION
				if (this->defaultPassword.Length() > 0)
					fcrypt_encrypt_offset(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // VIRTUALDRIVE_ENCRYPTION
				m_parentFile->Write(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData);
				fileHandle.detail->curCompressedFilePosition += fileHandle.detail->posInBufferedData;
			}

			fileEntry->m_uncompressedSize = fileHandle.detail->stream.total_in;
			fileEntry->m_compressedSize = (DWORD)fileHandle.detail->curCompressedFilePosition;  //stream.total_out;
		}
		else
		{
			fileEntry->m_compressedSize = (DWORD)fileHandle.detail->curCompressedFilePosition;
			fileEntry->m_uncompressedSize = (DWORD)fileHandle.detail->curUncompressedFilePosition;
		}

		if (m_flags & SUPPORT_MD5)
		{
			MD5Final(fileEntry->m_md5, &fileHandle.detail->md5writecontext);
		}
	}

	FileCloseInternal(fileHandle);

	return true;
}


void VirtualDrive::FileCloseInternal(VirtualFileHandle& fileHandle)
{
	VirtualDrive::FileEntry* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

	if (&fileHandle == m_curWriteFile)
	{
		// Yes.  Reassign the file header's directory offset.
        m_dirOffset = fileEntry->m_offset + fileHandle.detail->curCompressedFilePosition + fileHandle.detail->headerSize;

		// Write the changed parts of the header.
		ZipLocalHeader localHeader;
		localHeader.signature = ZipLocalHeader::SIGNATURE;
		localHeader.version_needed = 20;

		localHeader.flag = 0;
		localHeader.compression_method = (WORD)fileEntry->m_compressionMethod;
		localHeader.dosDate = ConvertTime_tToDosTime(fileEntry->m_fileTime);
		localHeader.crc = fileEntry->m_crc;
		localHeader.compressed_size = fileEntry->m_compressedSize;
		localHeader.uncompressed_size = fileEntry->m_uncompressedSize;
		localHeader.size_filename = (WORD)strlen(fileEntry->GetFileName());
		localHeader.size_file_extra = 0;

		WORD size_filename = localHeader.size_filename;
		fileHandle.detail->headerSize = sizeof(ZipLocalHeader) + size_filename + localHeader.size_file_extra;

		if (m_swap)
		{
			localHeader.signature = SwapEndian(localHeader.signature);
			localHeader.version_needed = SwapEndian(localHeader.version_needed);
			localHeader.flag = SwapEndian(localHeader.flag);
			localHeader.compression_method = SwapEndian(localHeader.compression_method);
			localHeader.dosDate = SwapEndian(localHeader.dosDate);
			localHeader.crc = SwapEndian(localHeader.crc);
			localHeader.compressed_size = SwapEndian(localHeader.compressed_size);
			localHeader.uncompressed_size = SwapEndian(localHeader.uncompressed_size);
			localHeader.size_filename = SwapEndian(localHeader.size_filename);
			localHeader.size_file_extra = SwapEndian(localHeader.size_file_extra);
		}

		m_parentFile->Seek(fileEntry->m_offset);

#if VIRTUALDRIVE_ENCRYPTION
		fcrypt_ctx headerzcx[1];
		if (this->defaultPassword.Length() > 0)
		{
			headerzcx[0] = this->defaultzcx[0];
			fcrypt_encrypt((unsigned char*)&localHeader, sizeof(ZipLocalHeader), headerzcx);
		}
#endif // VIRTUALDRIVE_ENCRYPTION

		m_parentFile->Write(&localHeader, sizeof(ZipLocalHeader));
#if VIRTUALDRIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
		{
			HeapString tempFileName = fileEntry->GetFileName();
			fcrypt_encrypt((unsigned char*)(const char*)tempFileName, (unsigned int)tempFileName.Length(), headerzcx);
			m_parentFile->Write((const char*)tempFileName, tempFileName.Length());
		}
		else
#endif // VIRTUALDRIVE_ENCRYPTION
		{
			m_parentFile->Write((const char*)fileEntry->GetFileName(), size_filename);
		}

		// Turn off the current write file.
		m_curWriteFile = NULL;

    	if (fileEntry->m_compressionMethod == COMPRESSED)
			deflateEnd(&fileHandle.detail->stream);
	}
    else
    {
    	if (fileEntry->m_compressionMethod == COMPRESSED)
    		inflateEnd(&fileHandle.detail->stream);
    }

    delete[] fileHandle.detail->bufferedData;
    fileHandle.detail->bufferedData = NULL;
	fileHandle.detail->fileEntryIndex = VirtualDrive::INVALID_FILE_ENTRY;

	// Remove the file from the open files list.
	if (&fileHandle == m_headOpenFile)
	{
		m_headOpenFile = fileHandle.nextOpenFile;
	}
	else
	{
		if (fileHandle.prevOpenFile)
			fileHandle.prevOpenFile->nextOpenFile = fileHandle.nextOpenFile;
	}
	if (fileHandle.nextOpenFile)
		fileHandle.nextOpenFile->prevOpenFile = fileHandle.prevOpenFile;
} // FileClose()


///////////////////////////////////////////////////////////////////////////////
/**
	Closes all open virtual files for the current virtual drive.
**/
void VirtualDrive::FileCloseAll()
{
	// Wander the open files list, closing each file.
    VirtualFileHandle* openFile = m_headOpenFile;
    while (openFile)
    {
        VirtualFileHandle* nextOpenFile = openFile->nextOpenFile;
        FileClose(*openFile);
        openFile = nextOpenFile;
    }

	// Remove all files from the open files list.
    m_headOpenFile = NULL;
}


const char* VirtualDrive::FileGetFileName(VirtualFileHandle& fileHandle)
{
    return GetFileEntry(fileHandle.detail->fileEntryIndex)->m_fileName;
}


ULONGLONG VirtualDrive::FileGetPosition(VirtualFileHandle& fileHandle)
{
    return fileHandle.detail->curUncompressedFilePosition;
}


void VirtualDrive::FileSetLength(VirtualFileHandle& fileHandle, ULONGLONG newLength)
{
	if (m_readOnly)
        return;

    assert(0);

    FileEntry* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

    if (newLength <= fileEntry->m_uncompressedSize)
	{
		fileEntry->m_uncompressedSize = (DWORD)newLength;
		m_changed = true;
		m_needsPack = true;
	}
}


ULONGLONG VirtualDrive::FileGetLength(VirtualFileHandle& fileHandle)
{
    FileEntry* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);
	return fileEntry->m_uncompressedSize;
}


LONGLONG VirtualDrive::FileSeek(VirtualFileHandle& fileHandle, LONGLONG offset, File::SeekFlags seekFlags)
{
	FileEntry* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

	if (fileEntry->GetCompressionMethod() == UNCOMPRESSED)
	{
		switch (seekFlags)
		{
			case File::SEEKFLAG_BEGIN:
				if (offset < 0  ||  (DWORD)offset > fileEntry->m_uncompressedSize)
					return -1;
				fileHandle.detail->curCompressedFilePosition = offset;
				fileHandle.detail->curUncompressedFilePosition = offset;
				break;

			case File::SEEKFLAG_CURRENT:
#if VIRTUALDRIVE_ENCRYPTION
				if (this->defaultPassword.Length() > 0)
					CORE_ASSERT(0);
#endif // VIRTUALDRIVE_ENCRYPTION
				if ((offset + (LONG)fileHandle.detail->curUncompressedFilePosition < 0)  ||
					(offset + (LONG)fileHandle.detail->curUncompressedFilePosition > (LONG)fileEntry->m_uncompressedSize))
					return -1;
				fileHandle.detail->curUncompressedFilePosition = fileHandle.detail->curCompressedFilePosition = offset + fileHandle.detail->curUncompressedFilePosition;
				break;

			case File::SEEKFLAG_END:
#if VIRTUALDRIVE_ENCRYPTION
				if (this->defaultPassword.Length() > 0)
					CORE_ASSERT(0);
#endif // VIRTUALDRIVE_ENCRYPTION
				if (offset > 0  ||  (LONG)fileHandle.detail->curUncompressedFilePosition + offset < 0)
					return -1;
				fileHandle.detail->curUncompressedFilePosition = fileHandle.detail->curCompressedFilePosition = fileEntry->m_uncompressedSize - offset;
				break;
		}
	}
	else
	{
		switch (seekFlags)
		{
			case File::SEEKFLAG_BEGIN:
				if (offset == 0)
				{
					fileHandle.detail->curCompressedFilePosition = offset;
					fileHandle.detail->curUncompressedFilePosition = offset;
				}
				else
				{
					CORE_ASSERT(0);
				}
				break;

			default:
				CORE_ASSERT(0);
		}
	}

	return fileHandle.detail->curUncompressedFilePosition;
}


ULONGLONG VirtualDrive::FileRead(VirtualFileHandle& fileHandle, void* buffer, ULONGLONG count)
{
    FileEntry* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

	ULONGLONG endPos = (UINT)fileHandle.detail->curUncompressedFilePosition + count;
	if (endPos > fileEntry->m_uncompressedSize)
		count -= endPos - fileEntry->m_uncompressedSize;

	if (fileEntry->GetCompressionMethod() == UNCOMPRESSED)
	{
		if (fileHandle.detail->curUncompressedFilePosition >= fileEntry->m_uncompressedSize)
			return 0;

		m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);
		count = m_parentFile->Read(buffer, (UINT)count);
#if VIRTUALDRIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
			fcrypt_decrypt_offset((unsigned char*)buffer, (unsigned int)count, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // VIRTUALDRIVE_ENCRYPTION
		fileHandle.detail->curCompressedFilePosition += count;
		fileHandle.detail->curUncompressedFilePosition += count;

		return count;
	}

    z_stream& stream = fileHandle.detail->stream;
	stream.next_out = (BYTE*)buffer;
	stream.avail_out = (UINT)count;

	ULONGLONG rest_read_compressed = fileEntry->m_compressedSize - fileHandle.detail->curCompressedFilePosition;
    ULONGLONG numRead = 0;

	while (stream.avail_out > 0)
	{
		if (stream.avail_in == 0  &&  rest_read_compressed > 0)
		{
			UINT uReadThis = Z_BUFSIZE;
			if (rest_read_compressed < uReadThis)
				uReadThis = (UINT)rest_read_compressed;
			if (uReadThis == 0)
				break;

			m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);
			if (m_parentFile->Read(fileHandle.detail->bufferedData, uReadThis) != uReadThis)
				return numRead;

#if VIRTUALDRIVE_ENCRYPTION
			if (this->defaultPassword.Length() > 0)
				fcrypt_decrypt_offset((unsigned char*)fileHandle.detail->bufferedData, (unsigned int)uReadThis, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // VIRTUALDRIVE_ENCRYPTION

			fileHandle.detail->curCompressedFilePosition += uReadThis;

			rest_read_compressed -= uReadThis;

			stream.next_in = (BYTE*)fileHandle.detail->bufferedData;
			stream.avail_in = (UINT)uReadThis;
		}

		ULONG uTotalOutBefore = stream.total_out;

		int err = inflate(&stream, Z_SYNC_FLUSH);

		size_t bytesInflated = stream.total_out - uTotalOutBefore;
        fileHandle.detail->curUncompressedFilePosition += bytesInflated;
		numRead += bytesInflated;

		if (err != Z_OK)
			break;
	}

	return numRead;
}


ULONGLONG VirtualDrive::FileWrite(VirtualFileHandle& fileHandle, const void* buffer, ULONGLONG count)
{
	if (m_readOnly)
        return 0;

    VirtualDrive::FileEntry* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

#if VIRTUALDRIVE_ENCRYPTION
	if (this->defaultPassword.Length() == 0)
	{
#endif // VIRTUALDRIVE_ENCRYPTION
		fileEntry->m_crc = crc32(fileEntry->m_crc, (BYTE*)buffer, (uInt)count);
		if (m_flags & SUPPORT_MD5)
		{
			MD5Update(&fileHandle.detail->md5writecontext, (unsigned char*)buffer, (unsigned int)count);
		}
#if VIRTUALDRIVE_ENCRYPTION
	}
#endif

	if (fileEntry->m_compressionMethod == UNCOMPRESSED)
	{
		m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);

#if VIRTUALDRIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
		{
			// Operate in 4k buffers.
			const UINT BUFFER_SIZE = 4 * 1024;
			BYTE cryptBuffer[BUFFER_SIZE];
			BYTE* inPtr = (BYTE*)buffer;
			ULONGLONG cryptCount = count;
			ULONGLONG curOffset = fileHandle.detail->curCompressedFilePosition;
			while (cryptCount > 0)
			{
				// Copy the minimum of BUFFER_SIZE or the fileSize.
				UINT readSize = (UINT)(BUFFER_SIZE < cryptCount ? BUFFER_SIZE : cryptCount);
				memcpy(cryptBuffer, inPtr, readSize);
				fcrypt_encrypt_offset(cryptBuffer, readSize, fileHandle.detail->zcx, (unsigned long)curOffset);
				m_parentFile->Write(cryptBuffer, readSize);
				cryptCount -= readSize;
				curOffset += readSize;
				inPtr += readSize;
			}
		}
		else
#endif // VIRTUALDRIVE_ENCRYPTION
		{
			count = m_parentFile->Write(buffer, count);
		}
		fileHandle.detail->curCompressedFilePosition += count;
		fileHandle.detail->curUncompressedFilePosition += count;
		if (fileHandle.detail->curUncompressedFilePosition > fileEntry->m_uncompressedSize)
			fileEntry->m_uncompressedSize = (DWORD)fileHandle.detail->curUncompressedFilePosition;
		return count;
	}

	int err = Z_OK;

    z_stream& stream = fileHandle.detail->stream;
	stream.next_in = (BYTE*)buffer;
	stream.avail_in = (uInt)count;

	while (err == Z_OK  &&  stream.avail_in > 0)
	{
		if (stream.avail_out == 0)
		{
			m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);
#if VIRTUALDRIVE_ENCRYPTION
			if (this->defaultPassword.Length() > 0)
				fcrypt_encrypt_offset(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // VIRTUALDRIVE_ENCRYPTION
			m_parentFile->Write(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData);
			fileHandle.detail->curCompressedFilePosition += fileHandle.detail->posInBufferedData;
			fileHandle.detail->posInBufferedData = 0;
			stream.avail_out = (UINT)Z_BUFSIZE;
			stream.next_out = fileHandle.detail->bufferedData;
		}

		ULONG uTotalOutBefore = stream.total_out;
		err = deflate(&stream, Z_NO_FLUSH);
		fileHandle.detail->posInBufferedData += (UINT)(stream.total_out - uTotalOutBefore);
	}

    fileHandle.detail->curUncompressedFilePosition += count;
    return count;
}


///////////////////////////////////////////////////////////////////////////////
/**
	Erases the virtual file called [fileName].  The space the virtual file
	occupied is now unused and irretrievable, without using the Pack()
	function.

	@param fileName The fileName of the virtual file to be erased.
	@return Returns true if the file was erased or false if an error occured
		(such as the file specified by fileName not existing or being open
		already).
**/
bool VirtualDrive::FileErase(const char* fileName)
{
	if (m_readOnly)
		return false;

	// Retrieve the index of the file entry called fileName.
	size_t index = FindFileEntryIndex(fileName);

	// If the entry doesn't exist, then exit.
	if (index == INVALID_FILE_ENTRY)
		return false;

	// Wander the list of open files, checking if the file is not already open.
    for (VirtualFileHandle* openFile = m_headOpenFile; openFile; openFile = openFile->nextOpenFile)
    {
        FileEntry* fileEntry = GetFileEntry(openFile->detail->fileEntryIndex);
        if (strcmp(fileEntry->GetFileName(), fileName) == 0)
            return false;
    }

	this->fileNameMap.Remove(PtrString(this, index));

    // Before removing from the offsets, first resize our structure.
	if (index < m_fileEntryCount - 1)
	{
		// Reinsert all following entries into the map.
		for (size_t i = index + 1; i < m_fileEntryCount; ++i)
		{
			this->fileNameMap.Remove(PtrString(this, i));
		}

		size_t sizeToRemove = m_fileEntryOffsets[index + 1] - m_fileEntryOffsets[index];
		memcpy(GetFileEntry(index), GetFileEntry(index + 1), m_fileEntriesSizeBytes - m_fileEntryOffsets[index + 1]);
        memcpy(m_fileEntryOffsets + index, m_fileEntryOffsets + index + 1, (m_fileEntryCount - index - 1) * sizeof(UINT));
		m_fileEntryCount--;

		// Reinsert all following entries into the map.
		for (size_t i = index; i < m_fileEntryCount; ++i)
		{
			m_fileEntryOffsets[i] -= sizeToRemove;
			this->fileNameMap[PtrString(this, i)] = i;
		}

		// Fixup pointers.
		VirtualFileHandle* openFile = m_headOpenFile;
		while (openFile)
		{
			if (openFile->detail->fileEntryIndex > index)
			{
				openFile->detail->fileEntryIndex--;
			}
			openFile = openFile->nextOpenFile;
		}
	}
	else
	{
		m_fileEntryCount--;
	}

	// A directory change has occurred.
	m_changed = true;
	m_needsPack = true;

	return true;
} // FileErase()


///////////////////////////////////////////////////////////////////////////////
/**
	Renames a virtual file from [oldFilename] to [newFilename].

	@param oldFilename The fileName of the virtual file to rename.
	@param newFilename The fileName to rename [oldFilename] to.
	@return Returns true if successful, false if an error occurred.
**/
bool VirtualDrive::FileRename(const char* oldFileName, const char* newFileName)
{
	if (m_readOnly)
		return false;

	// Retrieve the index of the file entry called oldFilename.
	size_t index = FindFileEntryIndex(oldFileName);

	// If it exists, then an entry called oldFilename is already there.
	if (index == INVALID_FILE_ENTRY)
		return false;

	// Get the file entry.
	FileEntry& fileEntry = *GetFileEntry(index);  (void)fileEntry;

	// Detach the old buffer.
	size_t origSize = m_fileEntriesSizeBytes;  (void)origSize;
	BYTE* origBuffer = m_fileEntries;  (void)origBuffer;

	// Figure out the new size.
    size_t newFileNameLen = strlen(newFileName);
    size_t oldFileNameLen = strlen(oldFileName);
	int sizeDifference = (int)newFileNameLen - (int)oldFileNameLen;
	size_t newFileEntriesSize = ((int)m_fileEntriesSizeBytes + sizeDifference);
	if (newFileEntriesSize > m_fileEntriesMaxSizeBytes)
	{
		m_fileEntriesMaxSizeBytes += sizeDifference + 10 * 1024;
		BYTE* newFileEntries = new BYTE[m_fileEntriesMaxSizeBytes];
		memcpy(newFileEntries, m_fileEntries, m_fileEntriesSizeBytes);
		delete[] m_fileEntries;
		m_fileEntries = newFileEntries;
	}

	if (index + 1 < m_fileEntryCount  &&  sizeDifference != 0)
		memmove(m_fileEntries + m_fileEntryOffsets[index + 1] + sizeDifference, m_fileEntries + m_fileEntryOffsets[index + 1], m_fileEntriesSizeBytes - m_fileEntryOffsets[index + 1]);

	m_fileEntriesSizeBytes = ((int)m_fileEntriesSizeBytes + sizeDifference);

	BYTE* dest = m_fileEntries + m_fileEntryOffsets[index];
	memcpy(((FileEntry*)dest)->m_fileName, newFileName, newFileNameLen + 1);

	// Adjust all the file entry offsets.
	this->fileNameMap.Remove(PtrString(this, index));
	this->fileNameMap[PtrString(this, index)] = index;

	if (sizeDifference > 0)
	{
		for (size_t i = index + 1; i < m_fileEntryCount; ++i)
		{
			m_fileEntryOffsets[i] += sizeDifference;
		}
	}

	// The directory changed.
	m_needsPack = true;
	m_changed = true;

	return true;
} // FileRename()


///////////////////////////////////////////////////////////////////////////////
/**
	Copies a file from [srcFile] to a virtual file called [destFilename].

	@param srcFile An open File object to copy the data from.
	@param destFilename The fileName of the virtual file to copy the data from
		[srcFile] to.
	@param fileTime A CTime object describing the virtual file's creation date
		and time.  The current date and time is used if fileTime is NULL,
**/
bool VirtualDrive::FileCopy(VirtualFileHandle& srcFileHandle, const char* destFilename, const time_t* overrideFileTime)
{
	if (m_readOnly)
		return false;

	// Operate in 64k buffers.
	const UINT BUFFER_SIZE = 64 * 1024;

    FileEntry* srcFileEntry = srcFileHandle.GetParentDrive()->GetFileEntry(srcFileHandle.detail->fileEntryIndex);

    // Get the source file's size.
	UINT fileSize = srcFileEntry->m_compressedSize;

	// Create the destination virtual file.
	VirtualFileHandle destFileHandle;
	if (!FileCreate(destFilename ? destFilename : srcFileEntry->GetFileName(), destFileHandle, srcFileEntry->m_compressionMethod, overrideFileTime ? overrideFileTime : &srcFileEntry->m_fileTime))
		return false;
    FileEntry* destFileEntry = GetFileEntry(destFileHandle.detail->fileEntryIndex);

	// Allocate the buffer space.
	BYTE* buffer = new BYTE[BUFFER_SIZE];

	// Go to the beginning of the source file.
    LONGLONG srcOffset = srcFileEntry->m_offset + srcFileHandle.detail->headerSize;
    LONGLONG destOffset = destFileEntry->m_offset + destFileHandle.detail->headerSize;

	// Keep copying until there is no more file left to copy.
	while (fileSize > 0)
	{
		// Copy the minimum of BUFFER_SIZE or the fileSize.
        UINT readSize = BUFFER_SIZE < fileSize ? BUFFER_SIZE : fileSize;
		srcFileHandle.parentDrive->m_parentFile->Seek(srcOffset);
        srcFileHandle.parentDrive->m_parentFile->Read(buffer, readSize);
		m_parentFile->Seek(destOffset);
        m_parentFile->Write(buffer, readSize);
		fileSize -= readSize;
        srcOffset += readSize;
        destOffset += readSize;
	}

	destFileEntry->m_crc = srcFileEntry->m_crc;
	memcpy(destFileEntry->m_md5, srcFileEntry->m_md5, sizeof(destFileEntry->m_md5));
	destFileEntry->m_compressedSize = srcFileEntry->m_compressedSize;
	destFileEntry->m_uncompressedSize = srcFileEntry->m_uncompressedSize;

	destFileHandle.detail->curUncompressedFilePosition = destFileEntry->m_uncompressedSize;
    destFileHandle.detail->curCompressedFilePosition = destFileEntry->m_compressedSize;
	destFileHandle.detail->stream.total_in = destFileEntry->m_uncompressedSize;
	destFileHandle.detail->stream.total_out = destFileEntry->m_compressedSize;
	destFileHandle.detail->stream.avail_out = 0;
	destFileHandle.detail->posInBufferedData = 0;

	// Close the destination virtual file.
	FileCloseInternal(destFileHandle);

	// Destroy the buffer.
	delete [] buffer;

	return true;
} // FileCopy()


bool VirtualDrive::FileCopy(File& srcFile, const char* destFilename, UINT compressionMethod, const time_t* fileTime)
{
	if (m_readOnly)
		return false;

	// Operate in 64k buffers.
	const UINT BUFFER_SIZE = 64 * 1024;

	// Get the source file's size.
	UINT fileSize = (UINT)srcFile.GetLength();

	// See if the destination file exists.
	VirtualFileHandle destFileHandle;
	bool opened = false;
	FileEntry* fileEntry = FindFileEntry(destFilename);  (void)fileEntry;

	// Was the file opened yet?
	if (!opened)
	{
		// Create the destination virtual file.
		if (!FileCreate(destFilename, destFileHandle, compressionMethod, fileTime))
			return false;
	}

	// Allocate the buffer space.
	BYTE* buffer = new BYTE[BUFFER_SIZE];

	// Go to the beginning of the source file.
    LONG offset = 0;

	// Keep copying until there is no more file left to copy.
	while (fileSize > 0)
	{
		// Copy the minimum of BUFFER_SIZE or the fileSize.
        UINT readSize = BUFFER_SIZE < fileSize ? BUFFER_SIZE : fileSize;
		srcFile.Seek(offset);
		srcFile.Read(buffer, readSize);
		FileWrite(destFileHandle, buffer, readSize);
		fileSize -= readSize;
        offset += readSize;
	}

	// Close the destination virtual file.
	FileClose(destFileHandle);

	// Destroy the buffer.
	delete [] buffer;

	return true;
} // FileCopy()


/**
**/
bool VirtualDrive::FileCopy(const char* srcFileName, const char* destFileName, UINT compressionMethod, const time_t* inFileTime)
{
	DiskFile file;
    if (!file.Open(srcFileName, File::MODE_READONLY))
		return false;

	time_t fileTime = inFileTime ? *inFileTime : file.GetLastWriteTime();
	return FileCopy(file, destFileName, compressionMethod, &fileTime) != false;
}


/**
**/
bool VirtualDrive::BufferCopy(const void* buffer, ULONGLONG size, VirtualFileHandle& destFile)
{
	HeapString cacheFileName;

	FileEntry* entry = GetFileEntry(destFile.detail->fileEntryIndex);
	time_t fileTime = entry->GetTimeStamp();

	if (!fileCache.IsEmpty())
	{
		if (fileCache[fileCache.Length() - 1] != '/'  &&  fileCache[fileCache.Length() - 1] != '\\')
			fileCache += "/";

		DWORD crc = crc32(0, (const Bytef*)buffer, (uInt)size);

		if (entry)
		{
			if (entry->GetCRC() == crc)
			{
				if (entry->GetTimeStamp() != fileTime)
				{
					entry->SetTimeStamp(fileTime);
				}

				if (memcmp(entry->m_md5, emptyMD5, sizeof(emptyMD5)) == 0)
				{
					// Calculate the MD5 of the buffer.
					unsigned char digest[16];
					MD5_CTX c;
					MD5Init(&c);
					MD5Update(&c, (unsigned char*)buffer, (unsigned int)size);
					MD5Final(digest, &c);

					entry->SetMD5(digest);
				}

				return true;
			}
		}

		// Calculate the MD5 of the buffer.
		unsigned char digest[16];
		MD5_CTX c;
		MD5Init(&c);
		MD5Update(&c, (unsigned char*)buffer, (unsigned int)size);
		MD5Final(digest, &c);

		char hexBuffer[2*16+1];
		int i;
		for (i=0; i<16; i++) sprintf(hexBuffer+2*i,"%02x",digest[i]);
		HeapString hex = hexBuffer;

		cacheFileName = this->fileCache + hex.Sub(0, 3) + "/" + hex;

		if (access(cacheFileName, 0) != -1)
		{
			VirtualDrive cacheDrive;
			if (cacheDrive.Open(cacheFileName))
			{
				VirtualFileHandle cacheFileHandle;
				if (cacheDrive.FileOpen(entry->GetFileName(), cacheFileHandle))
				{
					if (FileCopy(cacheFileHandle, NULL, &fileTime))
					{
						// Just in case.
						entry->SetMD5(digest);
						return true;
					}
				}
			}
		}
	}

	if (FileWrite(destFile, buffer, size) != size)
		return false;

	if (!cacheFileName.IsEmpty())
	{
		PathCreate(cacheFileName);

		VirtualDrive cacheDrive;
		if (cacheDrive.Create(cacheFileName, 0, m_flags))
		{
			VirtualFileHandle srcFileHandle;
			if (FileOpen(entry->GetFileName(), srcFileHandle))
			{
				cacheDrive.FileCopy(srcFileHandle);
			}
		}
	}

	return true;
}


/**
**/
bool VirtualDrive::BufferCopy(const void* buffer, ULONGLONG size, const char* destFilename, UINT compressionMethod, const time_t* inFileTime)
{
	HeapString cacheFileName;

	time_t fileTime = inFileTime ? *inFileTime : AdjustTime_t(time(NULL));

	if (!fileCache.IsEmpty())
	{
		if (fileCache[fileCache.Length() - 1] != '/'  &&  fileCache[fileCache.Length() - 1] != '\\')
			fileCache += "/";

		DWORD crc = crc32(0, (const Bytef*)buffer, (uInt)size);

		FileEntry* entry = FindFileEntry(destFilename);
		if (entry)
		{
			if (entry->GetCRC() == crc)
			{
				if (entry->GetTimeStamp() != fileTime)
				{
					entry->SetTimeStamp(fileTime);
				}

				if (memcmp(entry->m_md5, emptyMD5, sizeof(emptyMD5)) == 0)
				{
					// Calculate the MD5 of the buffer.
					unsigned char digest[16];
					MD5_CTX c;
					MD5Init(&c);
					MD5Update(&c, (unsigned char*)buffer, (unsigned int)size);
					MD5Final(digest, &c);

					entry->SetMD5(digest);
				}

				return true;
			}
		}

		// Calculate the MD5 of the buffer.
		unsigned char digest[16];
		MD5_CTX c;
		MD5Init(&c);
		MD5Update(&c, (unsigned char*)buffer, (unsigned int)size);
		MD5Final(digest, &c);

		char hexBuffer[2*16+1];
		int i;
		for (i=0; i<16; i++) sprintf(hexBuffer+2*i,"%02x",digest[i]);
		HeapString hex = hexBuffer;

		cacheFileName = this->fileCache + hex.Sub(0, 3) + "/" + hex;

		if (access(cacheFileName, 0) != -1)
		{
			VirtualDrive cacheDrive;
			if (cacheDrive.Open(cacheFileName))
			{
				VirtualFileHandle cacheFileHandle;
				if (cacheDrive.FileOpen(destFilename, cacheFileHandle))
				{
					if (FileCopy(cacheFileHandle, NULL, &fileTime))
					{
						// Just in case.
						FileEntry* entry = FindFileEntry(destFilename);
						entry->SetMD5(digest);
						return true;
					}
				}
			}
		}
	}

	VirtualFileHandle fileHandle;
	if (!FileCreate(destFilename, fileHandle, COMPRESSED, &fileTime))
		return false;

	if (FileWrite(fileHandle, buffer, size) != size)
		return false;

	FileClose(fileHandle);

	if (!cacheFileName.IsEmpty())
	{
		PathCreate(cacheFileName);

		VirtualDrive cacheDrive;
		if (cacheDrive.Create(cacheFileName, 0, m_flags))
		{
			VirtualFileHandle srcFileHandle;
			if (FileOpen(destFilename, srcFileHandle))
			{
				cacheDrive.FileCopy(srcFileHandle);
			}
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/**
	Retrieves the directory file entry at [index].

	@param index The index of the file entry, range 0 to GetFileEntryCount() - 1.
	@return Returns the FileEntry object describing the requested file entry or
		NULL if the index is out of range.
**/
VirtualDrive::FileEntry* VirtualDrive::GetFileEntry(size_t index)
{
	// See if the requested index is in range.
	if (index >= (size_t)m_fileEntryCount)
		return NULL;

	// Return the appropriate file entry.
    return (FileEntry*)(m_fileEntries + m_fileEntryOffsets[index]);
}


///////////////////////////////////////////////////////////////////////////////
/**
	Retrieves the FileEntry object of the directory file entry called [fileName].

	@param fileName The non-case sensitive fileName of the file entry to find.
	@return Returns the FileEntry object describing the requested file entry or
		NULL if it isn't found.
**/
VirtualDrive::FileEntry* VirtualDrive::FindFileEntry(const char* fileName)
{
	// Lookup the fileName.
	return GetFileEntry(FindFileEntryIndex(fileName));
}


///////////////////////////////////////////////////////////////////////////////
/**
	Retrieves the index of the directory file entry called [fileName].

	@param fileName The non-case sensitive fileName of the file entry to find.
	@return Returns the index of the found file entry or INVALID_FILE_ENTRY if
		it wasn't found.
**/

size_t VirtualDrive::FindFileEntryIndex(const char* fileName)
{
	// If there are no file entries, it is pointless to perform the rest of the
	// search.
	if (m_fileEntryCount == 0)
		return INVALID_FILE_ENTRY;

	FileNameMap::Node* node = this->fileNameMap.Find(PtrString(fileName));
	if (!node)
		return INVALID_FILE_ENTRY;
	return this->fileNameMap.Value(node);
} // FindFileEntryIndex


///////////////////////////////////////////////////////////////////////////////
/**
**/
bool VirtualDrive::NeedsPack(PackOptions* packOptions)
{
	if (m_readOnly)
		return false;

	if (m_needsPack)
		return true;

	// Test whether the directory is at the beginning of the file already.
	size_t directorySize = 0;
	if (m_flags & EXTRA_DIRECTORY_AT_BEGINNING)
	{
		directorySize = sizeof(ZipDirHeader);
		size_t lowestFileOffset = (size_t)-1;
		for (size_t i = 0; i < m_fileEntryCount; ++i)
		{
			FileEntry& entry = *GetFileEntry(i);
			if (entry.GetOffset() < lowestFileOffset)
				lowestFileOffset = entry.GetOffset();
			int size_filename = (int)strlen(entry.GetFileName());
			int size_file_extra = (m_flags & SUPPORT_MD5) ? sizeof(FWKCS_MD5) : 0;
			directorySize += sizeof(ZipFileHeader) + size_filename + size_file_extra;
		}

		if (lowestFileOffset != directorySize)
		{
			if (packOptions  &&  packOptions->setNeedsPack)
				m_needsPack = true;
			return true;
		}
	}
	else
	{
		size_t lowestFileOffset = (size_t)-1;
		for (size_t i = 0; i < m_fileEntryCount; ++i)
		{
			FileEntry& entry = *GetFileEntry(i);
			if (entry.GetOffset() < lowestFileOffset)
				lowestFileOffset = entry.GetOffset();
		}

		if (lowestFileOffset != 0)
		{
			if (packOptions  &&  packOptions->setNeedsPack)
				m_needsPack = true;
			return true;
		}
	}

	// See if it is already in order.
    if (packOptions  &&  packOptions->fileOrder)
    {
		FileOrderList* fileOrder = packOptions->fileOrder;

		if (fileOrder->Count() != GetFileEntryCount())
		{
			if (packOptions->setNeedsPack)
				m_needsPack = true;
			return true;
		}
		else
		{
			size_t curOffset = directorySize;
			size_t i = 0;
			for (FileOrderList::Node* node = fileOrder->Head(); node; node = fileOrder->Next(node), ++i)
			{
				HeapString& fileOrderFileName = fileOrder->Value(node).entryName;
				FileEntry* fileEntry = GetFileEntry(i);
				if (fileOrderFileName != fileEntry->GetFileName()  ||  fileEntry->GetOffset() != curOffset)
				{
					if (packOptions->setNeedsPack)
						m_needsPack = true;
					return true;
				}

				curOffset += sizeof(ZipLocalHeader) + strlen(fileEntry->GetFileName()) + fileEntry->GetCompressedSize();
			}
		}
	}

	return false;
}


/**
	Packs a virtual drive, removing all unused space.
**/
bool VirtualDrive::Pack(PackOptions* packOptions)
{
	if (m_readOnly)
		return false;

	// Flush.
	Flush();

	m_needsPack = NeedsPack(packOptions);

	// Do we need to pack it?
	if (!m_needsPack)
		return true;

	// Generate a unique temporary name.
    HeapString newFileName = m_fileName;
	newFileName += ".repack";
#if 0
	char* lastSlash1 = strrchr(m_fileName, '\\');
	char* lastSlash2 = strrchr(m_fileName, '/');
	char* lastSlash = lastSlash1 > lastSlash2 ? lastSlash1 : lastSlash2;
	char saveSlash = *lastSlash;
	*lastSlash = 0;
#if defined(_MSC_VER)
	char* asciiTempName = _tempnam(m_fileName, "VDRIVEPACK_");
#else
	char* asciiTempName = tempnam(m_fileName, "VDRIVEPACK_");
#endif
	*lastSlash = saveSlash;
    HeapString newFileName = asciiTempName;
	free(asciiTempName);
#endif

	// Create a new virtual drive.
	VirtualDrive newDrive;
	HeapString oldPassword;
#if VIRTUALDRIVE_ENCRYPTION
	oldPassword = this->defaultPassword;
	newDrive.Create(newFileName, oldPassword, m_flags);
#else
	newDrive.Create(newFileName, 0, m_flags);
#endif // VIRTUALDRIVE_ENCRYPTION
	newDrive.fileNameMap.SetHashTableSize(this->fileNameMap.GetHashTableSize());
	newDrive.fileNameMap.SetBlockSize(this->fileNameMap.Count());
	newDrive.m_fileEntryMaxCount = this->m_fileEntryMaxCount + 1;		// +1 to avoid a realloc.
	newDrive.m_fileEntryOffsets = new size_t[newDrive.m_fileEntryMaxCount];
	newDrive.m_fileEntriesMaxSizeBytes = this->m_fileEntriesMaxSizeBytes + 1;	// +1 to avoid a realloc.
	newDrive.m_fileEntries = new BYTE[newDrive.m_fileEntriesMaxSizeBytes];

	if (m_flags & EXTRA_DIRECTORY_AT_BEGINNING)
	{
		size_t initialOffset = sizeof(ZipDirHeader);

		for (size_t i = 0; i < m_fileEntryCount; ++i)
		{
			FileEntry& entry = *GetFileEntry(i);
			int size_filename = (int)strlen(entry.GetFileName());
			int size_file_extra = (m_flags & SUPPORT_MD5) ? sizeof(FWKCS_MD5) : 0;
			initialOffset += sizeof(ZipFileHeader) + size_filename + size_file_extra;
		}

		newDrive.GetParentFile()->SetLength(initialOffset);
		newDrive.m_dirOffset = initialOffset;
	}

    if (packOptions  &&  packOptions->fileOrder)
    {
		FileOrderList* fileOrder = packOptions->fileOrder;
		for (FileOrderList::Node* node = fileOrder->Head(); node; node = fileOrder->Next(node))
        {
            HeapString& fileName = fileOrder->Value(node).entryName;

	        VirtualFileHandle srcFileHandle;
	        if (FileOpen(fileName, srcFileHandle))
	        {
	            if (!newDrive.FileCopy(srcFileHandle))
                {
                    FileClose(srcFileHandle);
					newDrive.Flush();
                    newDrive.Close();
#if defined(_MSC_VER)
                    _unlink(newFileName);
#else
					unlink(newFileName);
#endif
                    return false;
                }
                FileClose(srcFileHandle);
            }
        }
    }
	else
	{
		// Go through the entire directory.
		for (UINT i = 0; i < m_fileEntryCount; i++)
		{
			// Grab the file entry.
			FileEntry& fileEntry = *GetFileEntry(i);

			// Already processed?
			if (newDrive.FindFileEntry(fileEntry.GetFileName()))
				continue;

			// Open the source file.
			VirtualFileHandle srcFile;
			if (!FileOpen(fileEntry.GetFileName(), srcFile))
			{
				newDrive.Flush();
				newDrive.Close();
#if defined(_MSC_VER)
				_unlink(newFileName);
#else
				unlink(newFileName);
#endif
				return false;
			}

			// Copy the source file to the new virtual drive's destination virtual
			// file.
			if (!newDrive.FileCopy(srcFile))
			{
				newDrive.Flush();
				newDrive.Close();
#if defined(_MSC_VER)
				_unlink(newFileName);
#else
				unlink(newFileName);
#endif
				return false;
			}

			// Close the source file.
			FileClose(srcFile);
		}
	}

	// Close the new virtual drive.
	newDrive.fileNameMap.SetBlockSize(50);
	newDrive.Flush();
	newDrive.Close();

	// Close the current virtual drive.
	HeapString oldDriveName = m_fileName;
	Close();

#if defined(_MSC_VER)
	// Copy the packed drive.
	unlink(oldDriveName);
	if (!::MoveFile(newFileName, oldDriveName))
		::CopyFile(newFileName, oldDriveName, false);

	_unlink(newFileName);
#else
	assert(0);
	unlink(newFileName);
#endif

	this->m_needsPack = false;

	// Open it with the new
	return Open(oldDriveName, false, oldPassword, m_flags);
} // Pack()


/**
**/
bool VirtualDrive::ProcessFileList(const char* fileListFileName, ProcessFileListOptions* options)
{
	// If the drive is read-only, then exit.
	if (m_readOnly)
		return false;

	FileOrderList fileOrder;

    FILE* file = fopen(fileListFileName, "rt");
    if (!file)
        return false;

    while (true)
	{
		const size_t MISC_MAX_PATH = 1000;
        char srcPath[MISC_MAX_PATH];
        if (!fgets(srcPath, MISC_MAX_PATH, file)  ||  srcPath[0] == 0)
            break;
        srcPath[strlen(srcPath) - 1] = 0;

        char entryName[MISC_MAX_PATH];
        if (!fgets(entryName, MISC_MAX_PATH, file)  ||  entryName[0] == 0)
            break;
        entryName[strlen(entryName) - 1] = 0;

        // Change to forward slashes.
        char* ptr = srcPath;
        while (*ptr)
        {
            if (*ptr == '\\')
                *ptr = '/';
            ptr++;
        }

#if defined(WIN32)
		if (_access(srcPath, 0) == 0)
#else
		if (access(srcPath, 0) == 0)
#endif
        {
			FileOrderInfo& fileOrderInfo = fileOrder.Value(fileOrder.InsertAfterTail());
			fileOrderInfo.srcPath = srcPath;
			fileOrderInfo.entryName = entryName;
		}
	}

    fclose(file);

	bool ret = true;
    if (fileOrder.Count() > 0)
	{
		ret = ProcessFileList(fileOrder, options);
	}

    return ret;
}


#if defined(WIN32)

BOOL (WINAPI *fnSystemTimeToTzSpecificLocalTime)(LPTIME_ZONE_INFORMATION lpTimeZone, LPSYSTEMTIME lpUniversalTime, LPSYSTEMTIME lpLocalTime);

time_t ConvertFILETIME_To_time_t(const FILETIME& fileTime)
{
	SYSTEMTIME universalSystemTime;
	FileTimeToSystemTime(&fileTime, &universalSystemTime);

	SYSTEMTIME sysTime;
	if (CheckFor98Mill())
	{
		CORE_ASSERT(0);
	}
	else
	{
		if (!fnSystemTimeToTzSpecificLocalTime)
		{
			HMODULE aLib = LoadLibraryA("kernel32.dll");
			if (aLib == NULL)
				return 0;

			*(void**)&fnSystemTimeToTzSpecificLocalTime = (void*)GetProcAddress(aLib, "SystemTimeToTzSpecificLocalTime");
		}
		fnSystemTimeToTzSpecificLocalTime(NULL, &universalSystemTime, &sysTime);
	}

	// then convert the system time to a time_t (C-runtime local time)
	if (sysTime.wYear < 1900)
	{
		return 0;
	}

	TIME_ZONE_INFORMATION timeZone;
	int dst = GetTimeZoneInformation(&timeZone) == TIME_ZONE_ID_STANDARD;

	struct tm atm;
	atm.tm_sec = sysTime.wSecond & ~1;		// Zip files are only accurate to 2 seconds.
	atm.tm_min = sysTime.wMinute;
	atm.tm_hour = sysTime.wHour;
	atm.tm_mday = sysTime.wDay;
	atm.tm_mon = sysTime.wMonth - 1;        // tm_mon is 0 based
	atm.tm_year = sysTime.wYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = -1;
	time_t t = mktime(&atm);
	return t;
}

#endif


/**
**/
extern "C" DWORD VirtualDrive_GetFileCRC(FILE* file, UINT startOffset, unsigned char md5[16])
{
	const DWORD FILE_BLOCK_SIZE = 64 * 1024;
	BYTE* buffer = new BYTE[FILE_BLOCK_SIZE];
	DWORD fileCRC = 0;
	MD5_CTX c;

	MD5Init(&c);

	fseek(file, 0, SEEK_END);
	UINT fileLen = ftell(file);
    fseek(file, startOffset, SEEK_SET);
	DWORD bytesToDo = fileLen - startOffset;
	while (bytesToDo > 0)
	{
		DWORD bytesToRead = bytesToDo < FILE_BLOCK_SIZE ? bytesToDo : FILE_BLOCK_SIZE;

        fread(buffer, bytesToRead, 1, file);
		bytesToDo -= bytesToRead;
		fileCRC = crc32(fileCRC, buffer, bytesToRead);
		MD5Update(&c, (unsigned char*)buffer, bytesToRead);
	}

    fseek(file, 0, SEEK_SET);

	delete [] buffer;
	MD5Final(md5, &c);

	return fileCRC;
}


bool VirtualDrive::ProcessFileList(VirtualDrive::FileOrderList& fileOrder, ProcessFileListOptions* options)
{
	// If the drive is read-only, then exit.
	if (m_readOnly)
		return false;

#if 0
	typedef Map<HeapString, time_t> AllFileNameMap;
	AllFileNameMap allFileNameMap;
#endif

    typedef Map<HeapString, FileOrderInfo*> FileNameMap;
	FileNameMap fileNameMap;

	HeapString networkCache;
	if (options  &&  options->fileCache.IsNotEmpty())
	{
		networkCache = options->fileCache.TrimRight("/\\") + "/";
	}

	char emptyMD5[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	typedef Map<HeapString, VirtualDrive> DriveMap;
	DriveMap openDrives;

	// The lists of files and directories we need.
	for (FileOrderList::Node* node = fileOrder.Head(); node; node = fileOrder.Next(node))
	{
		FileOrderInfo& fileOrderInfo = fileOrder.Value(node);
		WIN32_FIND_DATA fd;
		int pipePos = fileOrderInfo.srcPath.ReverseFind('|');
		if (pipePos != -1)
		{
			HeapString archiveFileName = fileOrderInfo.srcPath.Sub(0, pipePos);
			HeapString entryName = fileOrderInfo.srcPath.Sub(pipePos + 1);
			HeapString lowerArchiveFileName = archiveFileName.Lower();
			VirtualDrive* openDrive;
			DriveMap::Node* node = openDrives.Find(lowerArchiveFileName);
			if (!node)
			{
				openDrive = &openDrives.Value(openDrives.Insert(lowerArchiveFileName));
				openDrive->Open(archiveFileName, options->checkOnly);
				openDrive->Flush();
			}
			else
			{
				openDrive = &openDrives.Value(node);
			}

			VirtualDrive::FileEntry* openDriveEntry = openDrive->FindFileEntry(entryName);
			if (fileOrderInfo.fileTime != 0  ||  openDriveEntry)
			{
				if (fileOrderInfo.fileTime == 0)
				{
					fileOrderInfo.lastWriteTime = openDriveEntry->GetTimeStamp();
					fileOrderInfo.size = openDriveEntry->GetUncompressedSize();
				}
				else
					fileOrderInfo.lastWriteTime = fileOrderInfo.fileTime;

				if (fileOrderInfo.size == 0)
					fileOrderInfo.size = openDriveEntry->GetUncompressedSize();

				if (fileOrderInfo.crc == 0)
					fileOrderInfo.crc = openDriveEntry->GetCRC();

				if (memcmp(fileOrderInfo.md5, emptyMD5, sizeof(emptyMD5)) == 0)
					memcpy(fileOrderInfo.md5, openDriveEntry->GetMD5(), sizeof(fileOrderInfo.md5));

				fileOrderInfo.needUpdate = true;

				fileNameMap[fileOrderInfo.entryName.Lower()] = &fileOrderInfo;
			}
		}
		else
		{
			if (fileOrderInfo.fileTime != 0  ||  GetFileAttributesEx(fileOrderInfo.srcPath, GetFileExInfoStandard, &fd))
			{
				if (fileOrderInfo.fileTime == 0)
				{
					fileOrderInfo.lastWriteTime = ConvertFILETIME_To_time_t(fd.ftLastWriteTime);
					fileOrderInfo.size = fd.nFileSizeLow;
				}
				else
					fileOrderInfo.lastWriteTime = fileOrderInfo.fileTime;
				fileOrderInfo.needUpdate = true;

				fileNameMap[fileOrderInfo.entryName.Lower()] = &fileOrderInfo;
			}
			else
			{
				this->errorString = "Unable to open file [" + fileOrderInfo.srcPath + "].";
				return false;
			}
		}
	}

	// Calculate new directory size.
	if ((m_flags & EXTRA_DIRECTORY_AT_BEGINNING)  &&  m_fileEntryCount == 0)
	{
		size_t initialOffset = sizeof(ZipDirHeader);
		for (FileOrderList::Node* node = fileOrder.Head(); node; node = fileOrder.Next(node))
		{
			FileOrderInfo* info = &fileOrder.Value(node);
			int size_filename = (int)info->entryName.Length();
			int size_file_extra = (m_flags & SUPPORT_MD5) ? sizeof(FWKCS_MD5) : 0;
			initialOffset += sizeof(ZipFileHeader) + size_filename + size_file_extra;
		}

		GetParentFile()->SetLength(initialOffset);
		m_dirOffset = initialOffset;
	}

	bool filenameShown = false;

	typedef Map<HeapString, bool> FileEntryMap;
	FileEntryMap fileEntryMap;
	for (UINT i = 0; i < GetFileEntryCount(); ++i)
	{
		FileEntry* entry = GetFileEntry(i);
		fileEntryMap[entry->GetFileName()] = true;
	}

	for (FileOrderList::Node* node = fileOrder.Head(); node; node = fileOrder.Next(node))
	{
		FileOrderInfo* info = &fileOrder.Value(node);
		size_t entryIndex = FindFileEntryIndex(info->entryName);
		if (entryIndex == INVALID_FILE_ENTRY)
			continue;
		FileEntry* entry = GetFileEntry(entryIndex);
		fileEntryMap.Remove(entry->GetFileName());

        // Check the time stamps and CRCs.
        if (strcmp(entry->GetFileName(), info->entryName) != 0)
        {
			if (options->checkOnly)
				return true;

			FileRename(entry->GetFileName(), info->entryName);
			entry = GetFileEntry(entryIndex);
		}

        time_t entryTimeStamp = entry->GetTimeStamp();
		struct tm atm1 = *localtime(&entryTimeStamp);
		struct tm atm2 = *localtime(&info->lastWriteTime);

		bool timestampChanged = false;
		info->needUpdate = info->compressionMethod != entry->GetCompressionMethod();
		timestampChanged = atm1.tm_year != atm2.tm_year  ||  atm1.tm_mday != atm2.tm_mday  ||  atm1.tm_mon != atm2.tm_mon;
        if (!timestampChanged)
        {
			int seconds1 = atm1.tm_hour * 3600 + atm1.tm_min * 60 + atm1.tm_sec;
			int seconds2 = atm2.tm_hour * 3600 + atm2.tm_min * 60 + atm2.tm_sec;
            timestampChanged = (abs(seconds2 - seconds1) > 2);
		}
		info->needUpdate |= timestampChanged;
	    info->needUpdate |= info->compressionMethod != entry->GetCompressionMethod();
		if (info->crc != 0)
			info->needUpdate |= info->crc != entry->GetCRC();
		info->needUpdate |= memcmp(entry->GetMD5(), emptyMD5, sizeof(emptyMD5)) == 0;
		if (memcmp(info->md5, emptyMD5, sizeof(emptyMD5)) != 0)
			info->needUpdate |= memcmp(entry->GetMD5(), info->md5, sizeof(info->md5)) != 0;

        if (info->needUpdate)
        {
			if (info->lastWriteTime != info->fileTime  ||  timestampChanged)
			{
				info->fileTime = info->lastWriteTime;

				int found = 0;
				if (options->retrieveChecksumCallback)
				{
					found = options->retrieveChecksumCallback(info->srcPath, &info->crc, (unsigned char*)&info->md5, options->retrieveChecksumUserData);
				}

				if (!found)
				{
					if (options->checkOnly)
						return true;

					FILE* file = fopen(info->srcPath, "rb");
					if (file)
					{
						info->crc = VirtualDrive_GetFileCRC(file, 0, info->md5);
						fclose(file);
					}
				}
			}

		    if (info->compressionMethod == entry->GetCompressionMethod()  &&  info->crc == entry->GetCRC()  &&  memcmp(entry->GetMD5(), emptyMD5, sizeof(emptyMD5)) != 0)
		    {
				if (options->checkOnly)
					return true;

                entry->SetTimeStamp(info->lastWriteTime);

			    info->needUpdate = false;
			    continue;
		    }
        }
	}

	if (options->checkOnly  &&  fileEntryMap.Head())
		return true;
	for (FileEntryMap::Node* node = fileEntryMap.Head(); node; node = fileEntryMap.Next(node))
	{
		if (!filenameShown)
		{
			if (options->statusUpdateCallback)
				options->statusUpdateCallback(UPDATING_ARCHIVE, m_fileName, options->statusUpdateUserData);
//				printf("-> %s", m_fileName);
			filenameShown = true;
		}
		if (options->statusUpdateCallback)
			options->statusUpdateCallback(DELETING_ENTRY, fileEntryMap.Key(node), options->statusUpdateUserData);
//			printf("\tdel: %s\n", entry->GetFileName());
		FileErase(fileEntryMap.Key(node));
        m_needsPack = true;
	}

	size_t index = 0;
	for (FileOrderList::Node* node = fileOrder.Head(); node; node = fileOrder.Next(node), ++index)
	{
		if (index >= GetFileEntryCount())
		{
			// The rest of the files will be appended at the end.
			break;
		}

		FileOrderInfo& info = fileOrder.Value(node);
		FileEntry* entry = GetFileEntry(index);
		if (info.entryName.CompareNoCase(entry->GetFileName()) != 0)
		{
			if (options->checkOnly)
				return true;

			m_needsPack = true;
            break;
		}
	}

	for (FileOrderList::Node* node = fileOrder.Head(); node; node = fileOrder.Next(node), ++index)
	{
		FileOrderInfo& info = fileOrder.Value(node);

		if (!info.needUpdate)
			continue;

		if (options->checkOnly)
			return true;

		HeapString cacheFileName;
		int pipePos = info.srcPath.ReverseFind('|');
		if (pipePos != -1)
		{
			HeapString archiveFileName = info.srcPath.Sub(0, pipePos);
			HeapString entryName = info.srcPath.Sub(pipePos + 1);
			HeapString lowerArchiveFileName = archiveFileName.Lower();
			VirtualDrive* cacheDrive;
			DriveMap::Node* node = openDrives.Find(lowerArchiveFileName);
			if (node)
			{
				cacheDrive = &openDrives.Value(node);

				FileEntry* cacheFileEntry = cacheDrive->FindFileEntry( entryName );
				if (cacheFileEntry->GetCompressionMethod() == info.compressionMethod)
				{
					VirtualFileHandle cacheFileHandle;
					if (cacheDrive->FileOpen(entryName, cacheFileHandle))
					{
						if (!filenameShown)
						{
							if (options->statusUpdateCallback)
								options->statusUpdateCallback(UPDATING_ARCHIVE, m_fileName, options->statusUpdateUserData);
							filenameShown = true;
						}
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(DIRECT_COPY_FROM_ANOTHER_ARCHIVE, info.entryName, options->statusUpdateUserData);

						if (FileCopy(cacheFileHandle, info.entryName, &info.lastWriteTime))
						{
//BROKEN						FileRename(entryName, info.entryName);
							continue;
						}
					}
				}
				else
				{
					VirtualFile cacheFile;
					if (cacheFile.Open(*cacheDrive, entryName))
					{
						if (!filenameShown)
						{
							if (options->statusUpdateCallback)
								options->statusUpdateCallback(UPDATING_ARCHIVE, m_fileName, options->statusUpdateUserData);
							filenameShown = true;
						}
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(UPDATING_ENTRY, info.entryName, options->statusUpdateUserData);

						if (FileCopy(cacheFile, info.entryName, info.compressionMethod, &info.lastWriteTime))
						{
//BROKEN						FileRename(entryName, info.entryName);
							continue;
						}
					}
				}
			}
		}

		else if (info.compressionMethod != UNCOMPRESSED  &&  networkCache.IsNotEmpty()  &&  (info.size == 0  ||  info.size >= options->fileCacheSizeThreshold))
		{
			if (info.lastWriteTime != info.fileTime  ||  info.crc == 0  ||  memcmp(info.md5, emptyMD5, sizeof(emptyMD5)) == 0)
			{
				info.fileTime = info.lastWriteTime;
				int found = 0;
				if (options->retrieveChecksumCallback)
				{
					found = options->retrieveChecksumCallback(info.srcPath, &info.crc, (unsigned char*)&info.md5, options->retrieveChecksumUserData);
				}

				if (!found)
				{
					FILE* file = fopen(info.srcPath, "rb");
					if (file)
					{
						info.crc = VirtualDrive_GetFileCRC(file, 0, info.md5);
						fclose(file);
					}
				}
			}

			char hexBuffer[2*16+1];
			int i;
			for (i=0; i<16; i++) sprintf(hexBuffer+2*i,"%02x",info.md5[i]);
			HeapString hex = hexBuffer;

			cacheFileName = networkCache + hex.Sub(0, 3) + "/" + hex;

			if (access(cacheFileName, 0) != -1)
			{
				VirtualDrive cacheDrive;
				if (cacheDrive.Open(cacheFileName))
				{
					VirtualFileHandle cacheFileHandle;
					if (cacheDrive.FileOpen(info.entryName, cacheFileHandle))
					{
						if (!filenameShown)
						{
							if (options->statusUpdateCallback)
								options->statusUpdateCallback(UPDATING_ARCHIVE, m_fileName, options->statusUpdateUserData);
							filenameShown = true;
						}
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(COPYING_ENTRY_FROM_CACHE, info.entryName, options->statusUpdateUserData);
						if (FileCopy(cacheFileHandle, NULL, &info.lastWriteTime))
							continue;
					}
				}
			}
		}

		if (!filenameShown)
		{
			if (options->statusUpdateCallback)
				options->statusUpdateCallback(UPDATING_ARCHIVE, m_fileName, options->statusUpdateUserData);
			filenameShown = true;
		}
		if (options->statusUpdateCallback)
			options->statusUpdateCallback(UPDATING_ENTRY, info.entryName, options->statusUpdateUserData);

		// Create the data file.
		DiskFile diskFile;
		diskFile.Open(info.srcPath);
        FileCopy(diskFile, info.entryName, info.compressionMethod, &info.lastWriteTime);

		if (cacheFileName.IsNotEmpty())
		{
			PathCreate(cacheFileName);

			VirtualDrive cacheDrive;
			if (cacheDrive.Create(cacheFileName, 0, m_flags))
			{
				VirtualFileHandle srcFileHandle;
				if (FileOpen(info.entryName, srcFileHandle))
				{
					cacheDrive.FileCopy(srcFileHandle);
					if (!filenameShown)
					{
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(UPDATING_ARCHIVE, m_fileName, options->statusUpdateUserData);
						filenameShown = true;
					}
					if (options->statusUpdateCallback)
						options->statusUpdateCallback(COPYING_ENTRY_TO_CACHE, info.entryName, options->statusUpdateUserData);
				}
			}
		}
	}

	if (options->checkOnly)
		return false;

	FlushOptions flushOptions;
	flushOptions.fileOrder = &fileOrder;
	Flush(&flushOptions);

	PackOptions packOptions;
	packOptions.fileOrder = &fileOrder;
	if (NeedsPack(&packOptions))
	{
		if (!filenameShown)
		{
			if (options->statusUpdateCallback)
				options->statusUpdateCallback(UPDATING_ARCHIVE, m_fileName, options->statusUpdateUserData);
			filenameShown = true;
		}
		if (options->statusUpdateCallback)
			options->statusUpdateCallback(PACKING_ARCHIVE, m_fileName, options->statusUpdateUserData);
		Pack(&packOptions);
	}

	return true;
}


time_t VirtualDrive::AdjustTime_t(time_t timeToAdjust)
{
	DWORD convertedTime = ConvertTime_tToDosTime(timeToAdjust);
	return ConvertDosDateToTime_t(convertedTime);
}


} // namespace Misc
