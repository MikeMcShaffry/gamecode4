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
#include "ZipArchive.h"
#include "DiskFile.h"
#include "ZipEntryFile.h"
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
#if defined(macintosh)  ||  defined(__APPLE__)
#include <copyfile.h>
#endif
#include "List.h"
#include "Map.h"
#if ZIPARCHIVE_ENCRYPTION
#include "aes/fileenc.h"
#include "aes/prng.h"
#endif // ZIPARCHIVE_ENCRYPTION

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
void ZipEntryInfo::SetTimeStamp(time_t fileTime) {
	if (m_fileTime != fileTime)
	{
		m_fileTime = fileTime;
		m_parentDrive->m_changed = true;
	}
}


void ZipEntryInfo::SetCRC(DWORD crc) {
	if (m_crc != crc)
	{
		m_crc = crc;
		m_parentDrive->m_changed = true;
	}
}


void ZipEntryInfo::SetMD5(unsigned char* md5) {
	if (memcmp(m_md5, md5, 16) != 0)
	{
		for (int i = 0; i < 16; ++i) m_md5[i] = md5[i];
		m_parentDrive->m_changed = true;
	}
}


///////////////////////////////////////////////////////////////////////////////
class ZipEntryFileHandle::Detail
{
public:
	Detail()
		: fileEntryIndex(ZipArchive::INVALID_FILE_ENTRY)
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
#if ZIPARCHIVE_ENCRYPTION
	fcrypt_ctx  zcx[1];
#endif // ZIPARCHIVE_ENCRYPTION
	MD5_CTX md5writecontext;
};


ZipEntryFileHandle::ZipEntryFileHandle()
	: nextOpenFile(NULL)
	, prevOpenFile(NULL)
	, parentDrive(NULL)
{
	this->detail = new Detail();
}


ZipEntryFileHandle::~ZipEntryFileHandle()
{
	if (this->detail->fileEntryIndex != ZipArchive::INVALID_FILE_ENTRY  &&  this->parentDrive)
		this->parentDrive->FileClose(*this);
	delete this->detail;
}


ZipArchive* ZipEntryFileHandle::GetParentArchive() const
{
	return this->parentDrive;
}


bool ZipEntryFileHandle::IsValid() const
{
	return this->detail->fileEntryIndex != size_t(-1) /*ZipArchive::INVALID_FILE_ENTRY*/;
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
ZipArchive::ZipArchive() :
	m_fileEntryCount(0),
	m_fileEntryMaxCount(0),
	m_dirOffset(0xffffffff),
	m_dirSize(0),
	m_needsPack(false),
	m_readOnly(false),
	m_unused(0),
	m_filename(NULL),
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
} // ZipArchive()


///////////////////////////////////////////////////////////////////////////////
/**
	The destructor automatically closes all open virtual files and the virtual
	drive.
**/
ZipArchive::~ZipArchive()
{
	Close();
} // ~ZipArchive()


#if ZIPARCHIVE_ENCRYPTION

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
	Creates a new zip archive on the disk.  The new zip archive has no
	file entries.

	@param fileName The full path of the zip archive.
	@return Returns true if the zip archive was created successfully, false
		otherwise.
**/
bool ZipArchive::Create(const char* fileName, DWORD flags, const char* _defaultPassword)
{
	Close();

	DiskFile* parentFile = new DiskFile;
    if (!parentFile->Open(fileName, File::MODE_CREATE | File::MODE_TRUNCATE | File::MODE_READWRITE)) {
		delete parentFile;
		return false;
	}

	bool ret = Create(*parentFile, fileName, flags, _defaultPassword);
	if (!ret) {
		delete parentFile;
		m_parentFile = NULL;
	}
	m_ownParentFile = true;
	return ret;
} // Create()


bool ZipArchive::Create(File& parentFile, const char* filename, DWORD flags, const char* _defaultPassword)
{
#if !ZIPARCHIVE_ENCRYPTION
	(void)_defaultPassword;
#endif // ZIPARCHIVE_ENCRYPTION
	Close();

	m_flags = flags;

	// Save the fileName.
    delete[] m_filename;
	m_filename = new char[strlen(filename) + 1];
    strcpy(m_filename, filename);

	// Okay, see if we're big endian.
	union {
		UINT   signature;
		BYTE   signatureBytes[4];
	};
	signatureBytes[0] = 0x50;
	signatureBytes[1] = 0x4b;
	signatureBytes[2] = 0x05;
	signatureBytes[3] = 0x06;

	m_swap = signature != ZipDirHeader::SIGNATURE;

#if ZIPARCHIVE_ENCRYPTION
	// Start the encryption.
	this->defaultPassword = _defaultPassword;
	if (this->defaultPassword.Length() > 0) {
		OpenHeaderCryptContext(this->defaultPassword, this->defaultzcx);
	}
#endif // ZIPARCHIVE_ENCRYPTION

	// Create the zip archive disk file.
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
	Opens an existing zip archive from the disk.

	@param fileName The full path of the zip archive.
	@return Returns true if the zip archive was opened successfully, false
		otherwise.
**/
bool ZipArchive::Open(const char* fileName, bool readOnly, DWORD flags, const char* _defaultPassword)
{
	Close();

	DiskFile* parentFile = new DiskFile();

	if (readOnly)
	{
		if (!parentFile->Open(fileName, File::MODE_READONLY)) {
			// Couldn't open the file!
			delete parentFile;

			return false;
		}
	}
	else if (!parentFile->Open(fileName, File::MODE_READWRITE)) {
		if (!parentFile->Open(fileName, File::MODE_READONLY)) {
			// Couldn't open the file!
			delete parentFile;

			return Create(fileName, flags, _defaultPassword);
		}

		readOnly = true;
	}

	bool ret = Open(*parentFile, fileName, readOnly, flags, _defaultPassword);
	if (!ret) {
		delete parentFile;
		m_parentFile = NULL;
	}
	m_ownParentFile = true;
	return ret;
}


bool ZipArchive::Open(File& parentFile, const char* filename, bool readOnly, DWORD flags, const char* _defaultPassword)
{
#if !ZIPARCHIVE_ENCRYPTION
	(void)_defaultPassword;
#endif // ZIPARCHIVE_ENCRYPTION

	// This object might already be in use.  Close it.
	Close();

	m_flags = flags;

	// Save the fileName.
	m_filename = new char[strlen(filename) + 1];
    strcpy(m_filename, filename);

	// Open the file.
	m_ownParentFile = false;
	m_parentFile = &parentFile;
	m_readOnly = readOnly;

#if ZIPARCHIVE_ENCRYPTION
	this->defaultPassword = _defaultPassword;

	// Initialize decryption, if needed.
	fcrypt_ctx zcx[1];
	if (this->defaultPassword.Length() > 0) {
		OpenHeaderCryptContext(this->defaultPassword, this->defaultzcx);
		zcx[0] = this->defaultzcx[0];
	}
#endif // ZIPARCHIVE_ENCRYPTION

	// See if it is a .zip file.
	ZipDirHeader zipDirHeader;
	m_parentFile->Seek(m_parentFile->GetLength() - sizeof(ZipDirHeader));
	m_parentFile->Read(&zipDirHeader, sizeof(ZipDirHeader));
#if ZIPARCHIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0) {
		zcx[0] = this->defaultzcx[0];
		fcrypt_decrypt((unsigned char*)&zipDirHeader, sizeof(ZipDirHeader), zcx);
	}
#endif // ZIPARCHIVE_ENCRYPTION

	// Test the zip signature.
	if (zipDirHeader.signatureBytes[0] != 0x50  ||  zipDirHeader.signatureBytes[1] != 0x4b  ||  zipDirHeader.signatureBytes[2] != 0x05  ||  zipDirHeader.signatureBytes[3] != 0x06) {
		return false;
	}

	// Okay, see if it is big endian.
	m_swap = zipDirHeader.signature != ZipDirHeader::SIGNATURE;
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

#if ZIPARCHIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0) {
		zcx[0] = this->defaultzcx[0];
		fcrypt_decrypt((unsigned char*)zipDirBuffer, zipDirHeader.size_central_dir, zcx);
	}
#endif // ZIPARCHIVE_ENCRYPTION

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

	size_t i;
	for (i = 0; i < m_fileEntryCount; ++i) {
		ZipFileHeader* zipFileHeader = (ZipFileHeader*)zipDirPtr;

		if (m_swap) {
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

		if (zipFileHeader->signature != ZipFileHeader::SIGNATURE) {
			// Bad zip file.
			delete[] zipDirBuffer;
			Close();
			return false;
		}

        m_fileEntryOffsets[i] = totalSize;
		totalSize += sizeof(ZipEntryInfo) + zipFileHeader->size_filename;
		zipDirPtr += sizeof(ZipFileHeader) + zipFileHeader->size_filename + zipFileHeader->size_file_extra + zipFileHeader->size_file_comment;
	}
	zipDirPtr = zipDirBuffer;

	m_fileEntries = new BYTE[totalSize];
    m_fileEntriesSizeBytes = totalSize;
	m_fileEntriesMaxSizeBytes = totalSize;

    BYTE* fileEntryPtr = m_fileEntries;

	for (i = 0; i < zipDirHeader.number_entry_CD; ++i) {
		ZipFileHeader* zipFileHeader = (ZipFileHeader*)zipDirPtr;

		ZipEntryInfo& fileEntry = *(ZipEntryInfo*)fileEntryPtr;
		fileEntry.m_fileTime = ConvertDosDateToTime_t(zipFileHeader->dosDate);
		fileEntry.m_offset = zipFileHeader->offset_curfile;
		fileEntry.m_compressedSize = zipFileHeader->compressed_size;
		fileEntry.m_uncompressedSize = zipFileHeader->uncompressed_size;
		fileEntry.m_crc = zipFileHeader->crc;
		fileEntry.m_compressionMethod = zipFileHeader->compression_method;
        fileEntry.m_parentDrive = this;

		WORD stringLen = zipFileHeader->size_filename;
		memcpy(fileEntry.m_filename, zipFileHeader->GetName(), stringLen);
        fileEntry.m_filename[stringLen] = 0;

		this->fileNameMap[PtrString(this, i)] = i;

		fileEntryPtr += sizeof(ZipEntryInfo) + stringLen;

		bool setMD5 = false;

		BYTE* extraPtr = (BYTE*)zipFileHeader->GetExtra();
		size_t extraSize = zipFileHeader->size_file_extra;
		while (extraSize) {
			ZipExtraHeader* extraHeader = (ZipExtraHeader*)extraPtr;
			if (m_swap) {
				extraHeader->dataSize = SwapEndian(extraHeader->dataSize);
			}

			if (extraHeader->headerIdBytes[0] == FWKCS_MD5::HEADER_ID_A  &&  extraHeader->headerIdBytes[1] == FWKCS_MD5::HEADER_ID_B) {
				if (m_flags & SUPPORT_MD5) {
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
	Closes the zip archive, flushing any unsaved changed to disk.  All open
	virtual files are closed.

	@return Returns true if successful.
**/
bool ZipArchive::Close()
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
	delete[] m_filename;
    m_filename = NULL;

#if ZIPARCHIVE_ENCRYPTION
	// Clear the password.
	this->defaultPassword.Clear();
#endif // ZIPARCHIVE_ENCRYPTION

	// Destroy the File.
	if (m_ownParentFile) {
		delete m_parentFile;
	}
	m_parentFile = NULL;
	m_ownParentFile = false;

    m_dirOffset = 0;
    m_dirSize = 0;

	return true;
} // Close()


///////////////////////////////////////////////////////////////////////////////
void ZipArchive::UpdateMD5s()
{
	if (!IsOpened())
		return;

	// Test all the directory entries.
	const DWORD FILE_BLOCK_SIZE = 32768;
	BYTE* buffer = NULL;
	for (size_t i = 0; i < m_fileEntryCount; ++i)
	{
		ZipEntryInfo& entry = *GetFileEntry(i);
		if (memcmp(entry.GetMD5(), emptyMD5, sizeof(emptyMD5)) == 0)
		{
			if (!buffer)
				buffer = new BYTE[FILE_BLOCK_SIZE];

			ZipEntryFileHandle fileHandle;
			if (FileOpen(entry.GetFilename(), fileHandle))
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
void ZipArchive::_WriteDirectory(LONGLONG dirOffset, LONGLONG dirHeaderOffset)
{
#if ZIPARCHIVE_ENCRYPTION
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
		ZipEntryInfo& entry = *GetFileEntry(i);
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
		int size_filename = (int)strlen(entry.GetFilename());
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
#if ZIPARCHIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
			fcrypt_encrypt((unsigned char*)&zipFileHeader, sizeof(ZipFileHeader), zcx);
#endif
		m_parentFile->Write(&zipFileHeader, sizeof(ZipFileHeader));
#if ZIPARCHIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
		{
			HeapString tempFileName = entry.GetFilename();
			fcrypt_encrypt((unsigned char*)(const char*)tempFileName, (unsigned int)tempFileName.Length(), zcx);
			m_parentFile->Write((const char*)tempFileName, tempFileName.Length());
		}
		else
#endif // ZIPARCHIVE_ENCRYPTION
		{
			m_parentFile->Write((const char*)entry.GetFilename(), zipFileHeader.size_filename);
		}

		// Write MD5.
		if (m_flags & SUPPORT_MD5)
		{
			memcpy(md5info.md5, entry.GetMD5(), 16);
#if ZIPARCHIVE_ENCRYPTION
			if (this->defaultPassword.Length() > 0) {
				FWKCS_MD5 encryptedmd5info = md5info;
				fcrypt_encrypt((unsigned char*)&encryptedmd5info, (unsigned int)sizeof(FWKCS_MD5), zcx);
				m_parentFile->Write(&encryptedmd5info, sizeof(encryptedmd5info));
			} else
#endif // ZIPARCHIVE_ENCRYPTION
			{
				m_parentFile->Write(&md5info, sizeof(md5info));
			}
		}

		dirOffset += sizeof(ZipFileHeader) + size_filename + size_file_extra;
	}

#if ZIPARCHIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
		zcx[0] = this->defaultzcx[0];
#endif // ZIPARCHIVE_ENCRYPTION

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

#if ZIPARCHIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
		fcrypt_encrypt((unsigned char*)&zipDirHeader, sizeof(ZipDirHeader), zcx);
#endif // ZIPARCHIVE_ENCRYPTION

	m_parentFile->Seek(dirHeaderOffset);
	m_parentFile->Write(&zipDirHeader, sizeof(ZipDirHeader));
}


/**
	Sorts the directory entries by fileName and writes it all to disk.
**/
bool ZipArchive::Flush(const FlushOptions* flushOptions)
{
	if (!IsOpened())
		return false;

	if (m_readOnly)
		return false;

	// If there have been no changes made, exit successfully.
	if (!m_changed)
		return true;

	// Write the directory header id.
	size_t directorySize = 0;
	size_t lowestFileOffset = (size_t)-1;
	for (size_t i = 0; i < m_fileEntryCount; ++i)
	{
		ZipEntryInfo& entry = *GetFileEntry(i);
		if (entry.GetOffset() < lowestFileOffset)
			lowestFileOffset = entry.GetOffset();
		int size_filename = (int)strlen(entry.GetFilename());
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
			packOptions.fileOrderList = flushOptions ? flushOptions->fileOrderList : NULL;
			Pack(&packOptions);
		}
	}

	return true;
} // Flush()


///////////////////////////////////////////////////////////////////////////////
/**
	Creates a new file entry within the zip archive for writing. Only one
	file entry may be written to at any one time, because the new file is
	appended to the end of the zip archive.

	If the new file entry already exists within the directory entries, the
	old file of the same name is invalidated.  The space the old file took
	becomes unused and not retrievable.  To "retrieve" the space, the zip
	archive must be packed (see Pack()).

	@param fileName The fileName of the new file entry to create.
	@param fileHandle A reference to the ZipEntryFileHandle object to be filled in
		with the newly created file entry's information.
	@param fileTime A time_t object describing the file entry's creation date
		and time.  The current date and time is used if fileTime is NULL.
	@return Returns true if the file entry was created successfully, false
		otherwise.
**/
bool ZipArchive::FileCreate(const char* fileName, ZipEntryFileHandle& fileHandle,
							  int compressionMethod, int compressionLevel, const time_t* fileTime) {
	if (!IsOpened())
		return false;

	// If the drive is read-only, then exit.
	if (m_readOnly)
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
	if (index == INVALID_FILE_ENTRY) {
		// No. It needs to be added.  Increase the file entry count by 1.
        index = m_fileEntryCount;

        size_t* origOffsets = m_fileEntryOffsets;
		if (m_fileEntryCount >= m_fileEntryMaxCount) {
			m_fileEntryMaxCount += 100;
	        m_fileEntryOffsets = new size_t[m_fileEntryMaxCount + 1];
			if (origOffsets) {
	            memcpy(m_fileEntryOffsets, origOffsets, m_fileEntryCount * sizeof(UINT));
				delete[] origOffsets;
			}
		}

		m_fileEntryOffsets[m_fileEntryCount] = m_fileEntriesSizeBytes;
		m_fileEntryCount++;

		size_t stringLen = strlen(fileName);
		BYTE* origBuf = m_fileEntries;
		size_t origSize = m_fileEntriesSizeBytes;

        m_fileEntriesSizeBytes += sizeof(ZipEntryInfo) + stringLen;
		if (m_fileEntriesSizeBytes >= m_fileEntriesMaxSizeBytes) {
			m_fileEntriesMaxSizeBytes += sizeof(ZipEntryInfo) + stringLen + 10 * 1024;
			m_fileEntries = new BYTE[m_fileEntriesMaxSizeBytes];
			if (origSize > 0)
				memcpy(m_fileEntries, origBuf, origSize);
			if (origBuf)
				delete[] origBuf;
		}
        ZipEntryInfo* destEntry = (ZipEntryInfo*)(m_fileEntries + origSize);
		memcpy(destEntry->m_filename, fileName, stringLen + 1);

		this->fileNameMap[PtrString(this, m_fileEntryCount - 1)] = index;
	} else {
		m_needsPack = true;
	}

	// Fill in the new (or existing file entry).
	ZipEntryInfo& fileEntry = *GetFileEntry(index);

	// Add the proper file time.
	if (!fileTime) {
		fileEntry.m_fileTime = AdjustTime_t(time(NULL));
	} else {
		fileEntry.m_fileTime = AdjustTime_t(*fileTime);
	}

	fileEntry.m_offset = (UINT)m_dirOffset;
    fileEntry.m_compressedSize = 0;
    fileEntry.m_uncompressedSize = 0;
	fileEntry.m_crc = 0;
	fileEntry.m_compressionMethod = compressionMethod;
    fileEntry.m_parentDrive = this;

	// Assign the appropriate information to the file entry object.
    fileHandle.parentDrive = this;
	fileHandle.detail->fileEntryIndex = index;

    fileHandle.detail->curUncompressedFilePosition = 0;
    fileHandle.detail->curCompressedFilePosition = 0;

	if (compressionMethod == DEFLATED) {
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

		deflateInit2(&fileHandle.detail->stream, compressionLevel, compressionMethod == UNCOMPRESSED ? 0 : Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, 0);
	}

	fileHandle.detail->headerSize = sizeof(ZipLocalHeader) + strlen(fileEntry.GetFilename()) + 0;

#if ZIPARCHIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0) {
		fileHandle.detail->zcx[0] = this->defaultzcx[0];

		fileHandle.detail->curCompressedFilePosition += 0; //SALT_LENGTH(mode) + PWD_VER_LENGTH;
	}
#endif // ZIPARCHIVE_ENCRYPTION

	if (m_flags & SUPPORT_MD5) {
		MD5Init(&fileHandle.detail->md5writecontext);
	}

    // Assign this file to be our current write file.
	m_curWriteFile = &fileHandle;

	// Add this file entry to the open files list.
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
	Opens an existing file entry within the zip archive.

	@param fileName The fileName of the existing file entry to open.
	@param fileHandle A reference to the ZipEntryFileHandle object to be filled in
		with the newly opened file entry's information.
	@return Returns true if the file entry was opened successfully, false
		otherwise.
**/
bool ZipArchive::FileOpen(const char* fileName, ZipEntryFileHandle& fileHandle)
{
	if (!IsOpened())
		return false;

	// Close it if already opened.
	FileClose(fileHandle);

	// Retrieve the index of the file entry called fileName.
	size_t index = FindFileEntryIndex(fileName);
    if (index == INVALID_FILE_ENTRY)
        return false;

    return FileOpenIndex(index, fileHandle);
} // FileOpen()


/**
**/
bool ZipArchive::FileOpenIndex(size_t index, ZipEntryFileHandle& fileHandle)
{
	if (!IsOpened())
		return false;

	// If the entry doesn't exist, then exit.
	if (index == INVALID_FILE_ENTRY)
		return false;

	// Close it if already opened.
	FileClose(fileHandle);

	// Grab the entry.
	ZipEntryInfo& fileEntry = *GetFileEntry(index);

	// Assign the internal file entry information.
    fileHandle.parentDrive = this;
	fileHandle.detail->fileEntryIndex = index;
    fileHandle.detail->curUncompressedFilePosition = 0;

	ZipLocalHeader localHeader;

	m_parentFile->Seek(fileEntry.m_offset);
	m_parentFile->Read(&localHeader, sizeof(ZipLocalHeader));

#if ZIPARCHIVE_ENCRYPTION
	fcrypt_ctx headerzcx[1];
	if (this->defaultPassword.Length() > 0)
	{
		headerzcx[0] = this->defaultzcx[0];
		fcrypt_decrypt((unsigned char*)&localHeader, sizeof(ZipLocalHeader), headerzcx);
	}
#endif // ZIPARCHIVE_ENCRYPTION

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

#if ZIPARCHIVE_ENCRYPTION
	if (this->defaultPassword.Length() > 0)
	{
		fileHandle.detail->zcx[0] = this->defaultzcx[0];

		fileHandle.detail->curCompressedFilePosition += 0; //SALT_LENGTH(fileEntry.m_encryptionMode) + PWD_VER_LENGTH;
	}
#endif // ZIPARCHIVE_ENCRYPTION

    // Add this file entry to the open files list.
	fileHandle.nextOpenFile = m_headOpenFile;
	if (m_headOpenFile)
		m_headOpenFile->prevOpenFile = &fileHandle;
	fileHandle.prevOpenFile = NULL;
	m_headOpenFile = &fileHandle;

	return true;
} // FileOpen()


///////////////////////////////////////////////////////////////////////////////
/**
	Closes an open file entry.  This function should never really need to be
	called directory, as ZipEntryFileHandle::Close() can be used instead.

	@param fileHandle The ZipEntryFileHandle object to be closed.
	@return Returns true if the file was closed successfully.
**/
bool ZipArchive::FileClose(ZipEntryFileHandle& fileHandle)
{
	if (!IsOpened())
		return true;

	if (fileHandle.detail->fileEntryIndex == INVALID_FILE_ENTRY)
		return true;

	// Is the file entry being closed the current file entry being written
	// to?
	if (&fileHandle == m_curWriteFile)
	{
        ZipEntryInfo* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

		if (fileEntry->m_compressionMethod == DEFLATED)
		{
			int err;
			do
			{
				ULONG uTotalOutBefore;
				if (fileHandle.detail->stream.avail_out == 0)
				{
					m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);
#if ZIPARCHIVE_ENCRYPTION
					if (this->defaultPassword.Length() > 0)
						fcrypt_encrypt_offset(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // ZIPARCHIVE_ENCRYPTION
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
#if ZIPARCHIVE_ENCRYPTION
				if (this->defaultPassword.Length() > 0)
					fcrypt_encrypt_offset(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // ZIPARCHIVE_ENCRYPTION
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


void ZipArchive::FileCloseInternal(ZipEntryFileHandle& fileHandle)
{
	ZipEntryInfo* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

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
		localHeader.size_filename = (WORD)strlen(fileEntry->GetFilename());
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

#if ZIPARCHIVE_ENCRYPTION
		fcrypt_ctx headerzcx[1];
		if (this->defaultPassword.Length() > 0)
		{
			headerzcx[0] = this->defaultzcx[0];
			fcrypt_encrypt((unsigned char*)&localHeader, sizeof(ZipLocalHeader), headerzcx);
		}
#endif // ZIPARCHIVE_ENCRYPTION

		m_parentFile->Write(&localHeader, sizeof(ZipLocalHeader));
#if ZIPARCHIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
		{
			HeapString tempFileName = fileEntry->GetFilename();
			fcrypt_encrypt((unsigned char*)(const char*)tempFileName, (unsigned int)tempFileName.Length(), headerzcx);
			m_parentFile->Write((const char*)tempFileName, tempFileName.Length());
		}
		else
#endif // ZIPARCHIVE_ENCRYPTION
		{
			m_parentFile->Write((const char*)fileEntry->GetFilename(), size_filename);
		}

		// Turn off the current write file.
		m_curWriteFile = NULL;

    	if (fileEntry->m_compressionMethod == DEFLATED)
			deflateEnd(&fileHandle.detail->stream);
	}
    else
    {
    	if (fileEntry->m_compressionMethod == DEFLATED)
    		inflateEnd(&fileHandle.detail->stream);
    }

    delete[] fileHandle.detail->bufferedData;
    fileHandle.detail->bufferedData = NULL;
	fileHandle.detail->fileEntryIndex = ZipArchive::INVALID_FILE_ENTRY;

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
	Closes all open virtual files for the current zip archive.
**/
void ZipArchive::FileCloseAll()
{
	if (!IsOpened())
		return;

	// Wander the open files list, closing each file.
    ZipEntryFileHandle* openFile = m_headOpenFile;
    while (openFile)
    {
        ZipEntryFileHandle* nextOpenFile = openFile->nextOpenFile;
        FileClose(*openFile);
        openFile = nextOpenFile;
    }

	// Remove all files from the open files list.
    m_headOpenFile = NULL;
}


const char* ZipArchive::FileGetFileName(ZipEntryFileHandle& fileHandle)
{
    return GetFileEntry(fileHandle.detail->fileEntryIndex)->m_filename;
}


ULONGLONG ZipArchive::FileGetPosition(ZipEntryFileHandle& fileHandle)
{
    return fileHandle.detail->curUncompressedFilePosition;
}


void ZipArchive::FileSetLength(ZipEntryFileHandle& fileHandle, ULONGLONG newLength)
{
	if (m_readOnly)
        return;

    assert(0);

    ZipEntryInfo* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

    if (newLength <= fileEntry->m_uncompressedSize)
	{
		fileEntry->m_uncompressedSize = (DWORD)newLength;
		m_changed = true;
		m_needsPack = true;
	}
}


ULONGLONG ZipArchive::FileGetLength(ZipEntryFileHandle& fileHandle)
{
    ZipEntryInfo* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);
	return fileEntry->m_uncompressedSize;
}


LONGLONG ZipArchive::FileSeek(ZipEntryFileHandle& fileHandle, LONGLONG offset, File::SeekFlags seekFlags)
{
	ZipEntryInfo* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

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
#if ZIPARCHIVE_ENCRYPTION
				if (this->defaultPassword.Length() > 0)
					CORE_ASSERT(0);
#endif // ZIPARCHIVE_ENCRYPTION
				if ((offset + (LONG)fileHandle.detail->curUncompressedFilePosition < 0)  ||
					(offset + (LONG)fileHandle.detail->curUncompressedFilePosition > (LONG)fileEntry->m_uncompressedSize))
					return -1;
				fileHandle.detail->curUncompressedFilePosition = fileHandle.detail->curCompressedFilePosition = offset + fileHandle.detail->curUncompressedFilePosition;
				break;

			case File::SEEKFLAG_END:
#if ZIPARCHIVE_ENCRYPTION
				if (this->defaultPassword.Length() > 0)
					CORE_ASSERT(0);
#endif // ZIPARCHIVE_ENCRYPTION
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


ULONGLONG ZipArchive::FileRead(ZipEntryFileHandle& fileHandle, void* buffer, ULONGLONG count)
{
    ZipEntryInfo* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

	ULONGLONG endPos = (UINT)fileHandle.detail->curUncompressedFilePosition + count;
	if (endPos > fileEntry->m_uncompressedSize)
		count -= endPos - fileEntry->m_uncompressedSize;

	if (fileEntry->GetCompressionMethod() == UNCOMPRESSED)
	{
		if (fileHandle.detail->curUncompressedFilePosition >= fileEntry->m_uncompressedSize)
			return 0;

		m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);
		count = m_parentFile->Read(buffer, (UINT)count);
#if ZIPARCHIVE_ENCRYPTION
		if (this->defaultPassword.Length() > 0)
			fcrypt_decrypt_offset((unsigned char*)buffer, (unsigned int)count, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // ZIPARCHIVE_ENCRYPTION
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

#if ZIPARCHIVE_ENCRYPTION
			if (this->defaultPassword.Length() > 0)
				fcrypt_decrypt_offset((unsigned char*)fileHandle.detail->bufferedData, (unsigned int)uReadThis, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // ZIPARCHIVE_ENCRYPTION

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


ULONGLONG ZipArchive::FileWrite(ZipEntryFileHandle& fileHandle, const void* buffer, ULONGLONG count)
{
	if (m_readOnly)
        return 0;

    ZipEntryInfo* fileEntry = GetFileEntry(fileHandle.detail->fileEntryIndex);

#if ZIPARCHIVE_ENCRYPTION
	if (this->defaultPassword.Length() == 0)
	{
#endif // ZIPARCHIVE_ENCRYPTION
		fileEntry->m_crc = crc32(fileEntry->m_crc, (BYTE*)buffer, (uInt)count);
		if (m_flags & SUPPORT_MD5)
		{
			MD5Update(&fileHandle.detail->md5writecontext, (unsigned char*)buffer, (unsigned int)count);
		}
#if ZIPARCHIVE_ENCRYPTION
	}
#endif

	if (fileEntry->m_compressionMethod == UNCOMPRESSED)
	{
		m_parentFile->Seek(fileHandle.detail->curCompressedFilePosition + fileEntry->m_offset + fileHandle.detail->headerSize);

#if ZIPARCHIVE_ENCRYPTION
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
#endif // ZIPARCHIVE_ENCRYPTION
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
#if ZIPARCHIVE_ENCRYPTION
			if (this->defaultPassword.Length() > 0)
				fcrypt_encrypt_offset(fileHandle.detail->bufferedData, fileHandle.detail->posInBufferedData, fileHandle.detail->zcx, (unsigned long)fileHandle.detail->curCompressedFilePosition);
#endif // ZIPARCHIVE_ENCRYPTION
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
	Erases the file entry called [fileName].  The space the file entry
	occupied is now unused and irretrievable, without using the Pack()
	function.

	@param fileName The fileName of the file entry to be erased.
	@return Returns true if the file was erased or false if an error occured
		(such as the file specified by fileName not existing or being open
		already).
**/
bool ZipArchive::FileErase(const char* fileName)
{
	if (m_readOnly)
		return false;

	// Retrieve the index of the file entry called fileName.
	size_t index = FindFileEntryIndex(fileName);

	// If the entry doesn't exist, then exit.
	if (index == INVALID_FILE_ENTRY)
		return false;

	// Wander the list of open files, checking if the file is not already open.
    for (ZipEntryFileHandle* openFile = m_headOpenFile; openFile; openFile = openFile->nextOpenFile)
    {
        ZipEntryInfo* fileEntry = GetFileEntry(openFile->detail->fileEntryIndex);
        if (strcmp(fileEntry->GetFilename(), fileName) == 0)
            return false;
    }

	this->fileNameMap.Remove(PtrString(this, index));

    // Before removing from the offsets, first resize our structure.
	if (index < m_fileEntryCount - 1)
	{
		// Reinsert all following entries into the map.
		size_t i;
		for (i = index + 1; i < m_fileEntryCount; ++i)
		{
			this->fileNameMap.Remove(PtrString(this, i));
		}

		size_t sizeToRemove = m_fileEntryOffsets[index + 1] - m_fileEntryOffsets[index];
		memcpy(GetFileEntry(index), GetFileEntry(index + 1), m_fileEntriesSizeBytes - m_fileEntryOffsets[index + 1]);
        memcpy(m_fileEntryOffsets + index, m_fileEntryOffsets + index + 1, (m_fileEntryCount - index - 1) * sizeof(UINT));
		m_fileEntryCount--;

		// Reinsert all following entries into the map.
		for (i = index; i < m_fileEntryCount; ++i)
		{
			m_fileEntryOffsets[i] -= sizeToRemove;
			this->fileNameMap[PtrString(this, i)] = i;
		}

		// Fixup pointers.
		ZipEntryFileHandle* openFile = m_headOpenFile;
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
	Renames a file entry from [oldFilename] to [newFilename].

	@param oldFilename The fileName of the file entry to rename.
	@param newFilename The fileName to rename [oldFilename] to.
	@return Returns true if successful, false if an error occurred.
**/
bool ZipArchive::FileRename(const char* oldFileName, const char* newFileName)
{
	if (m_readOnly)
		return false;

	// Retrieve the index of the file entry called oldFilename.
	size_t index = FindFileEntryIndex(oldFileName);

	// If it exists, then an entry called oldFilename is already there.
	if (index == INVALID_FILE_ENTRY)
		return false;

	// Get the file entry.
	ZipEntryInfo& fileEntry = *GetFileEntry(index);  (void)fileEntry;

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
	memcpy(((ZipEntryInfo*)dest)->m_filename, newFileName, newFileNameLen + 1);

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
	Copies a file from [srcFile] to a file entry called [destFilename].

	@param srcFile An open File object to copy the data from.
	@param destFilename The fileName of the file entry to copy the data from
		[srcFile] to.
	@param fileTime A CTime object describing the file entry's creation date
		and time.  The current date and time is used if fileTime is NULL,
**/
bool ZipArchive::FileCopy(ZipEntryFileHandle& srcFileHandle, const char* destFilename, const time_t* overrideFileTime)
{
	if (m_readOnly)
		return false;

	// Operate in 64k buffers.
	const UINT BUFFER_SIZE = 64 * 1024;

    ZipEntryInfo* srcFileEntry = srcFileHandle.GetParentArchive()->GetFileEntry(srcFileHandle.detail->fileEntryIndex);

    // Get the source file's size.
	UINT fileSize = srcFileEntry->m_compressedSize;

	// Create the destination file entry.
	ZipEntryFileHandle destFileHandle;
	if (!FileCreate(destFilename ? destFilename : srcFileEntry->GetFilename(), destFileHandle, srcFileEntry->m_compressionMethod, Z_DEFAULT_COMPRESSION, overrideFileTime ? overrideFileTime : &srcFileEntry->m_fileTime))
		return false;
    ZipEntryInfo* destFileEntry = GetFileEntry(destFileHandle.detail->fileEntryIndex);

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
		if (srcFileHandle.parentDrive->m_parentFile->Seek(srcOffset) != srcOffset) {
			delete[] buffer;
			return false;
		}
		if (srcFileHandle.parentDrive->m_parentFile->Read(buffer, readSize) != readSize) {
			delete[] buffer;
			return false;
		}
		if (m_parentFile->Seek(destOffset) != destOffset) {
			delete[] buffer;
			return false;
		}
		if (m_parentFile->Write(buffer, readSize) != readSize) {
			delete[] buffer;
			return false;
		}
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

	// Close the destination file entry.
	FileCloseInternal(destFileHandle);

	// Destroy the buffer.
	delete [] buffer;

	return true;
} // FileCopy()


bool ZipArchive::FileCopy(File& srcFile, const char* destFilename, int compressionMethod, int compressionLevel, const time_t* fileTime)
{
	if (m_readOnly)
		return false;

	// Create the destination file entry.
	ZipEntryFileHandle destFileHandle;
	if (!FileCreate(destFilename, destFileHandle, compressionMethod, compressionLevel, fileTime))
		return false;

	// Operate in 64k buffers.
	const UINT BUFFER_SIZE = 64 * 1024;

	// Allocate the buffer space.
	BYTE* buffer = new BYTE[BUFFER_SIZE];

	// Get the source file's size.
	UINT fileSize = (UINT)(srcFile.GetLength() - srcFile.GetPosition());

	// Keep copying until there is no more file left to copy.
	bool ret = true;
	while (fileSize > 0)
	{
		// Copy the minimum of BUFFER_SIZE or the fileSize.
        UINT readSize = BUFFER_SIZE < fileSize ? BUFFER_SIZE : fileSize;
		if (srcFile.Read(buffer, readSize) != readSize) {
			ret = false;
			break;
		}
		if (FileWrite(destFileHandle, buffer, readSize) != readSize) {
			ret = false;
			break;
		}
		fileSize -= readSize;
	}

	// Close the destination file entry.
	FileClose(destFileHandle);

	// Destroy the buffer.
	delete [] buffer;

	return ret;
} // FileCopy()


/**
**/
bool ZipArchive::FileCopy(const char* srcFileName, const char* destFileName, int compressionMethod, int compressionLevel, const time_t* inFileTime)
{
	DiskFile file;
    if (!file.Open(srcFileName, File::MODE_READONLY))
		return false;

	time_t fileTime = inFileTime ? *inFileTime : file.GetLastWriteTime();
	return FileCopy(file, destFileName, compressionMethod, compressionLevel, &fileTime) != false;
}


/**
**/
bool ZipArchive::BufferCopy(const void* buffer, ULONGLONG size, ZipEntryFileHandle& destFile)
{
	HeapString cacheFileName;

	ZipEntryInfo* entry = GetFileEntry(destFile.detail->fileEntryIndex);
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
			ZipArchive cacheDrive;
			if (cacheDrive.Open(cacheFileName))
			{
				ZipEntryFileHandle cacheFileHandle;
				if (cacheDrive.FileOpen(entry->GetFilename(), cacheFileHandle))
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

		ZipArchive cacheDrive;
		if (cacheDrive.Create(cacheFileName, m_flags))
		{
			ZipEntryFileHandle srcFileHandle;
			if (FileOpen(entry->GetFilename(), srcFileHandle))
			{
				cacheDrive.FileCopy(srcFileHandle);
			}
		}
	}

	return true;
}


/**
**/
bool ZipArchive::BufferCopy(const void* buffer, ULONGLONG size, const char* destFilename, int compressionMethod, int compressionLevel, const time_t* inFileTime)
{
	HeapString cacheFileName;

	time_t fileTime = inFileTime ? *inFileTime : AdjustTime_t(time(NULL));

	if (!fileCache.IsEmpty())
	{
		if (fileCache[fileCache.Length() - 1] != '/'  &&  fileCache[fileCache.Length() - 1] != '\\')
			fileCache += "/";

		DWORD crc = crc32(0, (const Bytef*)buffer, (uInt)size);

		ZipEntryInfo* entry = FindFileEntry(destFilename);
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
			ZipArchive cacheDrive;
			if (cacheDrive.Open(cacheFileName))
			{
				ZipEntryFileHandle cacheFileHandle;
				if (cacheDrive.FileOpen(destFilename, cacheFileHandle))
				{
					if (FileCopy(cacheFileHandle, NULL, &fileTime))
					{
						// Just in case.
						ZipEntryInfo* entry = FindFileEntry(destFilename);
						entry->SetMD5(digest);
						return true;
					}
				}
			}
		}
	}

	ZipEntryFileHandle fileHandle;
	if (!FileCreate(destFilename, fileHandle, DEFLATED, compressionLevel, &fileTime))
		return false;

	if (FileWrite(fileHandle, buffer, size) != size)
		return false;

	FileClose(fileHandle);

	if (!cacheFileName.IsEmpty())
	{
		PathCreate(cacheFileName);

		ZipArchive cacheDrive;
		if (cacheDrive.Create(cacheFileName, m_flags))
		{
			ZipEntryFileHandle srcFileHandle;
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
	@return Returns the ZipEntryInfo object describing the requested file entry or
		NULL if the index is out of range.
**/
ZipEntryInfo* ZipArchive::GetFileEntry(size_t index)
{
	// See if the requested index is in range.
	if (index >= (size_t)m_fileEntryCount)
		return NULL;

	// Return the appropriate file entry.
    return (ZipEntryInfo*)(m_fileEntries + m_fileEntryOffsets[index]);
}


///////////////////////////////////////////////////////////////////////////////
/**
	Retrieves the ZipEntryInfo object of the directory file entry called [fileName].

	@param fileName The non-case sensitive fileName of the file entry to find.
	@return Returns the ZipEntryInfo object describing the requested file entry or
		NULL if it isn't found.
**/
ZipEntryInfo* ZipArchive::FindFileEntry(const char* fileName)
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

size_t ZipArchive::FindFileEntryIndex(const char* fileName)
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
bool ZipArchive::NeedsPack(PackOptions* packOptions)
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
			ZipEntryInfo& entry = *GetFileEntry(i);
			if (entry.GetOffset() < lowestFileOffset)
				lowestFileOffset = entry.GetOffset();
			int size_filename = (int)strlen(entry.GetFilename());
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
			ZipEntryInfo& entry = *GetFileEntry(i);
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
    if (packOptions  &&  packOptions->fileOrderList) {
		FileOrderList* fileOrderList = packOptions->fileOrderList;

		if (fileOrderList->Count() != GetFileEntryCount()) {
			if (packOptions->setNeedsPack)
				m_needsPack = true;
			return true;
		} else {
			size_t curOffset = directorySize;
			size_t i = 0;
			for (FileOrderList::Node* node = fileOrderList->Head(); node; node = fileOrderList->Next(node), ++i) {
				HeapString& fileOrderFileName = fileOrderList->Value(node).entryName;
				ZipEntryInfo* fileEntry = GetFileEntry(i);
				if (fileOrderFileName != fileEntry->GetFilename()  ||  fileEntry->GetOffset() != curOffset) {
					if (packOptions->setNeedsPack)
						m_needsPack = true;
					return true;
				}

				curOffset += sizeof(ZipLocalHeader) + strlen(fileEntry->GetFilename()) + fileEntry->GetCompressedSize();
			}
		}
	} else {
		size_t curOffset = directorySize;
		for (size_t i = 0; i < GetFileEntryCount(); ++i) {
			ZipEntryInfo* fileEntry = GetFileEntry(i);
			if (fileEntry->GetOffset() != curOffset) {
				if (packOptions->setNeedsPack)
					m_needsPack = true;
				return true;
			}

			curOffset += sizeof(ZipLocalHeader) + strlen(fileEntry->GetFilename()) + fileEntry->GetCompressedSize();
		}
	}

	return false;
}


/**
	Packs a zip archive, removing all unused space.
**/
bool ZipArchive::Pack(PackOptions* packOptions)
{
	if (!IsOpened())
		return false;

	if (m_readOnly)
		return false;

	// Flush.
	Flush();

	m_needsPack = NeedsPack(packOptions);

	// Do we need to pack it?
	if (!m_needsPack)
		return true;

	// Generate a unique temporary name.
    HeapString newFileName = m_filename;
	newFileName += ".repack";

	// Create a new zip archive.
	ZipArchive newDrive;
	HeapString oldPassword;
#if ZIPARCHIVE_ENCRYPTION
	oldPassword = this->defaultPassword;
	newDrive.Create(newFileName, m_flags, oldPassword);
#else
	newDrive.Create(newFileName, m_flags);
#endif // ZIPARCHIVE_ENCRYPTION
	newDrive.fileNameMap.SetHashTableSize(this->fileNameMap.GetHashTableSize());
	newDrive.fileNameMap.SetBlockSize(this->fileNameMap.Count());
	newDrive.m_fileEntryMaxCount = this->m_fileEntryMaxCount + 1;		// +1 to avoid a realloc.
	newDrive.m_fileEntryOffsets = new size_t[newDrive.m_fileEntryMaxCount];
	newDrive.m_fileEntriesMaxSizeBytes = this->m_fileEntriesMaxSizeBytes + 1;	// +1 to avoid a realloc.
	newDrive.m_fileEntries = new BYTE[newDrive.m_fileEntriesMaxSizeBytes];

	if (m_flags & EXTRA_DIRECTORY_AT_BEGINNING) {
		size_t initialOffset = sizeof(ZipDirHeader);

		for (size_t i = 0; i < m_fileEntryCount; ++i) {
			ZipEntryInfo& entry = *GetFileEntry(i);
			int size_filename = (int)strlen(entry.GetFilename());
			int size_file_extra = (m_flags & SUPPORT_MD5) ? sizeof(FWKCS_MD5) : 0;
			initialOffset += sizeof(ZipFileHeader) + size_filename + size_file_extra;
		}

		newDrive.GetParentFile()->SetLength(initialOffset);
		newDrive.m_dirOffset = initialOffset;
	}

    if (packOptions  &&  packOptions->fileOrderList) {
		FileOrderList* fileOrderList = packOptions->fileOrderList;
		for (FileOrderList::Node* node = fileOrderList->Head(); node; node = fileOrderList->Next(node)) {
            HeapString& fileName = fileOrderList->Value(node).entryName;

	        ZipEntryFileHandle srcFileHandle;
	        if (FileOpen(fileName, srcFileHandle)) {
	            if (!newDrive.FileCopy(srcFileHandle)) {
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
		for (UINT i = 0; i < m_fileEntryCount; i++) {
			// Grab the file entry.
			ZipEntryInfo& fileEntry = *GetFileEntry(i);

			// Already processed?
			if (newDrive.FindFileEntry(fileEntry.GetFilename()))
				continue;

			// Open the source file.
			ZipEntryFileHandle srcFile;
			if (!FileOpen(fileEntry.GetFilename(), srcFile)) {
				newDrive.Flush();
				newDrive.Close();
#if defined(_MSC_VER)
				_unlink(newFileName);
#else
				unlink(newFileName);
#endif
				return false;
			}

			// Copy the source file to the new zip archive's destination virtual
			// file.
			if (!newDrive.FileCopy(srcFile)) {
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

	// Close the new zip archive.
	newDrive.fileNameMap.SetBlockSize(50);
	newDrive.Flush();
	newDrive.Close();

	// Close the current zip archive.
	HeapString oldArchiveFileName = m_filename;
	Close();

#if defined(_MSC_VER)
	// Copy the packed drive.
	unlink(oldArchiveFileName);
	if (!::MoveFile(newFileName, oldArchiveFileName))
		::CopyFile(newFileName, oldArchiveFileName, false);

	_unlink(newFileName);
#else
	assert(0);
	unlink(newFileName);
#endif

	this->m_needsPack = false;

	// Open it with the new
	return Open(oldArchiveFileName, false, m_flags, oldPassword);
} // Pack()


#if defined(WIN32)

BOOL (WINAPI *fnSystemTimeToTzSpecificLocalTime)(LPTIME_ZONE_INFORMATION lpTimeZone, LPSYSTEMTIME lpUniversalTime, LPSYSTEMTIME lpLocalTime);

time_t ConvertFILETIME_To_time_t(const FILETIME& fileTime)
{
	SYSTEMTIME universalSystemTime;
	FileTimeToSystemTime(&fileTime, &universalSystemTime);

	SYSTEMTIME sysTime;
	if (CheckFor98Mill()) {
		CORE_ASSERT(0);
	} else {
		if (!fnSystemTimeToTzSpecificLocalTime) {
			HMODULE aLib = LoadLibraryA("kernel32.dll");
			if (aLib == NULL)
				return 0;

			*(void**)&fnSystemTimeToTzSpecificLocalTime = (void*)GetProcAddress(aLib, "SystemTimeToTzSpecificLocalTime");
		}
		fnSystemTimeToTzSpecificLocalTime(NULL, &universalSystemTime, &sysTime);
	}

	// then convert the system time to a time_t (C-runtime local time)
	if (sysTime.wYear < 1900) {
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
extern "C" DWORD ZipArchive_GetFileCRC(FILE* file, UINT startOffset, unsigned char md5[16])
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
	while (bytesToDo > 0) {
		DWORD bytesToRead = bytesToDo < FILE_BLOCK_SIZE ? bytesToDo : FILE_BLOCK_SIZE;

		size_t bytesRead = fread(buffer, bytesToRead, 1, file);
		bytesToDo -= bytesToRead;
		fileCRC = crc32(fileCRC, buffer, bytesToRead);
		MD5Update(&c, (unsigned char*)buffer, bytesToRead);
	}

    fseek(file, 0, SEEK_SET);

	delete [] buffer;
	MD5Final(md5, &c);

	return fileCRC;
}


typedef Map<HeapString, ZipArchive> PFL_OpenZipArchiveMap;

static ZipArchive* PFL_OpenArchive(const HeapString& archiveFileName, PFL_OpenZipArchiveMap& openArchives, DWORD flags) {
	HeapString lowerArchiveFileName = archiveFileName.Lower();
	ZipArchive* openArchive;
	PFL_OpenZipArchiveMap::Node* node = openArchives.Find(lowerArchiveFileName);
	if (!node) {
		openArchive = &openArchives.Value(openArchives.Insert(lowerArchiveFileName));
		if (!openArchive->Open(archiveFileName, true, flags))
			openArchive->Close();
	} else {
		openArchive = &openArchives.Value(node);
	}

	return openArchive;
}


bool ZipArchive::ProcessFileList(ZipArchive::FileOrderList& fileOrderList, ProcessFileListOptions* options)
{
	if (!IsOpened())
		return true;

	// If the drive is read-only, then exit.
	if (m_readOnly)
		return false;

	bool needsPack = m_needsPack;

	// Quick lookups of the filename to the file order structure.
    typedef Map<HeapString, FileOrderInfo*> FileNameMap;
	FileNameMap fileNameMap;

	// Use a network cache if the options specified it.
	HeapString networkCache;
	if (options  &&  options->fileCache.IsNotEmpty())
	{
		networkCache = options->fileCache.TrimRight("/\\") + "/";
	}

	// Use this empty MD5 array to determine if an MD5 is actually specified.
	char emptyMD5[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	// A map of open archives we're copying from.
	PFL_OpenZipArchiveMap openArchives;

	// The lists of files and directories we need.
	FileOrderList::Node* fileOrderListNode;
	for (fileOrderListNode = fileOrderList.Head(); fileOrderListNode; fileOrderListNode = fileOrderList.Next(fileOrderListNode)) {
		FileOrderInfo& fileOrderInfo = fileOrderList.Value(fileOrderListNode);

		HeapString lowerEntryName = fileOrderInfo.entryName.Lower();
		FileNameMap::Node* fileNameNode = fileNameMap.Find(lowerEntryName);
		if (fileNameNode) {
			continue;
		}
		fileOrderInfo.used = true;
		fileNameMap[lowerEntryName] = &fileOrderInfo;

		// If a file is supposed to be transferred from another zip archive, it is identified
		// with a pipe symbol.  The zip archive path precedes the pipe symbol.  The zip entry
		// filename follows the pipe.
		int pipePos = fileOrderInfo.srcPath.ReverseFind('|');
		if (pipePos != -1) {
			HeapString archiveFileName = fileOrderInfo.srcPath.Sub(0, pipePos);
			HeapString entryName = fileOrderInfo.srcPath.Sub(pipePos + 1);
			ZipEntryInfo* openArchiveFileEntry = NULL;

			if (fileOrderInfo.fileTime == 0  ||  fileOrderInfo.size == 0  ||  fileOrderInfo.crc == 0  ||
					memcmp(fileOrderInfo.md5, emptyMD5, sizeof(emptyMD5)) == 0) {
				ZipArchive* openArchive = PFL_OpenArchive(archiveFileName, openArchives, m_flags);
				openArchiveFileEntry = openArchive->FindFileEntry(entryName);
			}

			if (fileOrderInfo.fileTime == 0) {
				if (openArchiveFileEntry) {
					fileOrderInfo.lastWriteTime = openArchiveFileEntry->GetTimeStamp();
					fileOrderInfo.size = openArchiveFileEntry->GetUncompressedSize();
				}
			} else
				fileOrderInfo.lastWriteTime = fileOrderInfo.fileTime;

			if (fileOrderInfo.size == 0) {
				if (openArchiveFileEntry) {
					fileOrderInfo.size = openArchiveFileEntry->GetUncompressedSize();
				}
			}

			if (fileOrderInfo.crc == 0) {
				if (openArchiveFileEntry) {
					fileOrderInfo.crc = openArchiveFileEntry->GetCRC();
				}
			}

			if (memcmp(fileOrderInfo.md5, emptyMD5, sizeof(emptyMD5)) == 0) {
				if (openArchiveFileEntry) {
					memcpy(fileOrderInfo.md5, openArchiveFileEntry->GetMD5(), sizeof(fileOrderInfo.md5));
				}
			}

			fileOrderInfo.needUpdate = true;

			fileNameMap[fileOrderInfo.entryName.Lower()] = &fileOrderInfo;
		} else {
#ifdef WIN32
			WIN32_FIND_DATA fd;

			// If the file time was already provided by the file order entry, then assign it
			// to tentatively need updating.
			if (fileOrderInfo.fileTime != 0) {
				fileOrderInfo.lastWriteTime = fileOrderInfo.fileTime;
				fileOrderInfo.needUpdate = true;
			}
			// Otherwise, grab the information from the file on disk and assign it to tentatively
			// need updating.
			else if (GetFileAttributesEx(fileOrderInfo.srcPath, GetFileExInfoStandard, &fd)) {
				fileOrderInfo.lastWriteTime = ConvertFILETIME_To_time_t(fd.ftLastWriteTime);
				fileOrderInfo.size = fd.nFileSizeLow;
				fileOrderInfo.needUpdate = true;
				fileNameMap[fileOrderInfo.entryName.Lower()] = &fileOrderInfo;
			}
			// If we get here, the file doesn't exist.  Fail.
			else {
				this->errorString = "Unable to open file [" + fileOrderInfo.srcPath + "].";
				return false;
			}
#else
			struct stat sb;

			// If the file time was already provided by the file order entry, then assign it
			// to tentatively need updating.
			if (fileOrderInfo.fileTime != 0) {
				fileOrderInfo.lastWriteTime = fileOrderInfo.fileTime;
				fileOrderInfo.needUpdate = true;
			}
			// Otherwise, grab the information from the file on disk and assign it to tentatively
			// need updating.
			else if (stat(fileOrderInfo.srcPath, &sb) != -1) {
				fileOrderInfo.lastWriteTime = sb.st_mtime;
				fileOrderInfo.size = sb.st_size;
				fileOrderInfo.needUpdate = true;
				fileNameMap[fileOrderInfo.entryName.Lower()] = &fileOrderInfo;
			}
			// If we get here, the file doesn't exist.  Fail.
			else {
				this->errorString = "Unable to open file [" + fileOrderInfo.srcPath + "].";
				return false;
			}
#endif // WIN32
		}
	}

	// Calculate new directory size.
	size_t initialOffset = 0;
	if (m_flags & EXTRA_DIRECTORY_AT_BEGINNING) {
		initialOffset = sizeof(ZipDirHeader);
		for (FileOrderList::Node* node = fileOrderList.Head(); node; node = fileOrderList.Next(node))
		{
			FileOrderInfo* info = &fileOrderList.Value(node);
			if (!info->used)
				continue;
			int size_filename = (int)info->entryName.Length();
			int size_file_extra = (m_flags & SUPPORT_MD5) ? sizeof(FWKCS_MD5) : 0;
			initialOffset += sizeof(ZipFileHeader) + size_filename + size_file_extra;
		}

		size_t lowestFileOffset = (size_t)-1;
		for (size_t i = 0; i < m_fileEntryCount; ++i)
		{
			ZipEntryInfo& entry = *GetFileEntry(i);
			if (entry.GetOffset() < lowestFileOffset)
				lowestFileOffset = entry.GetOffset();
		}

		if (m_fileEntryCount == 0) {
			// Set the start of the .zip file contents to be after the directory
			// at the beginning.
			GetParentFile()->SetLength(initialOffset);
			m_dirOffset = initialOffset;
		} else if (lowestFileOffset != initialOffset) {
			if (options->checkOnly)
				return true;

			needsPack = true;
		}
	}

	bool filenameShown = false;

	// Build a map of the filenames already in the zip, so we can determine which
	// ones need to be erased when the file list is processed.
	typedef Map<HeapString, bool> ArchiveFileEntryMap;
	ArchiveFileEntryMap archiveFileEntryMap;
	for (UINT i = 0; i < GetFileEntryCount(); ++i) {
		ZipEntryInfo* entry = GetFileEntry(i);
		archiveFileEntryMap[entry->GetFilename()] = true;
	}

	// This is used as a quick check to determine if the files currently in the zip archive are
	// already in order.
	size_t index = 0;

	// Go through all the file entries specified by the user's fileOrderList table.
	bool needsUpdate = false;
	bool someDontNeedUpdate = false;
	bool orderChanged = false;

	for (fileOrderListNode = fileOrderList.Head(); fileOrderListNode; fileOrderListNode = fileOrderList.Next(fileOrderListNode), ++index)
	{
		FileOrderInfo* info = &fileOrderList.Value(fileOrderListNode);
		if (!info->used) {
			--index;
			continue;
		}

		// Look up the entry in the open zip archive.
		size_t entryIndex = FindFileEntryIndex(info->entryName);

		// If the current index exceeds the number of files currently in the zip archive,
		// all files in the zip and file order list are in order.  However, if we're
		// below the file entry count and the index doesn't match the entry index, then
		// we're going to need to pack this archive.
		if (!needsPack  &&  index < GetFileEntryCount()  &&  index != entryIndex) {
			// The rest of the files will be appended at the end.
			if (options->checkOnly)
				return true;

			needsPack = true;
			orderChanged = true;
		}

		// If the entry wasn't found, no further comparisons are necessary.  Just
		// look for the next file order filename.
		if (entryIndex == INVALID_FILE_ENTRY) {
			needsUpdate = true;
			continue;
		}

		// Grab the zip file entry.
		ZipEntryInfo* entry = GetFileEntry(entryIndex);
		archiveFileEntryMap.Remove(entry->GetFilename());

        // Check the time stamps and CRCs.
        if (strcmp(entry->GetFilename(), info->entryName) != 0)
        {
			if (options->checkOnly)
				return true;

			FileRename(entry->GetFilename(), info->entryName);
			entry = GetFileEntry(entryIndex);
		}

		info->needUpdate = false;

		// Test whether the timestamp of the file order list entry and the zip file entry are different.
        time_t entryTimeStamp = entry->GetTimeStamp();
		struct tm atm1 = *localtime(&entryTimeStamp);
		struct tm atm2 = *localtime(&info->lastWriteTime);

		bool timestampChanged = atm1.tm_year != atm2.tm_year  ||  atm1.tm_mday != atm2.tm_mday  ||  atm1.tm_mon != atm2.tm_mon;
        if (!timestampChanged)
        {
			int seconds1 = atm1.tm_hour * 3600 + atm1.tm_min * 60 + atm1.tm_sec;
			int seconds2 = atm2.tm_hour * 3600 + atm2.tm_min * 60 + atm2.tm_sec;
            timestampChanged = (abs(seconds2 - seconds1) > 2);
		}
		info->needUpdate |= timestampChanged;

		// If the file size changed, it is a dead giveaway the files are different.
		info->needUpdate |= (info->size != entry->GetUncompressedSize());

		// If the compression method is different (generally uncompressed->compressed or vice versa),
		// the file needs to be re-transferred into the zip.
	    info->needUpdate |= (info->compressionMethod != entry->GetCompressionMethod());

		// If a CRC was provided, test for differences.
		if (info->crc != 0)
			info->needUpdate |= (info->crc != entry->GetCRC());

		// If we're expected to have MD5s and the MD5 hasn't been set or doesn't match, update the file.
		if (m_flags & SUPPORT_MD5) {
			info->needUpdate |= (memcmp(entry->GetMD5(), emptyMD5, sizeof(emptyMD5)) == 0);
			if (memcmp(info->md5, emptyMD5, sizeof(emptyMD5)) != 0)
				info->needUpdate |= (memcmp(entry->GetMD5(), info->md5, sizeof(info->md5)) != 0);
		}

        // After all the checks, do we need to update the file?
		if (info->needUpdate  &&  info->size == entry->GetUncompressedSize()) {
			// If the timestamp is different, assign the correct one.
			if (info->lastWriteTime != info->fileTime  ||  timestampChanged) {
				info->fileTime = info->lastWriteTime;

				// Did the caller provide a callback to retrieve the checksum?  If so, call it.
				int found = 0;
				if (options->retrieveChecksumCallback) {
					found = options->retrieveChecksumCallback(info->srcPath, &info->crc, (unsigned char*)&info->md5, options->retrieveChecksumUserData);
				}

				// We still don't have a checksum.  Calculate it ourselves.
				if (!found) {
					// If we're only checking for changes, bail at this point.
					if (options->checkOnly)
						return true;

					FILE* file = fopen(info->srcPath, "rb");
					if (file) {
						info->crc = ZipArchive_GetFileCRC(file, 0, info->md5);
						fclose(file);
					}
				}
			}

			// Now that we have the extra information about the CRC and potential MD5,
			// see if there is anything still needing to be done.
			bool different = false;
			different |= (info->compressionMethod != entry->GetCompressionMethod());
			different |= (info->crc != entry->GetCRC());
			if (m_flags & SUPPORT_MD5) {
				if (memcmp(info->md5, emptyMD5, sizeof(emptyMD5)) != 0)
					different |= (memcmp(entry->GetMD5(), info->md5, sizeof(info->md5)) != 0);
			}
		    if (!different)
		    {
				if (options->checkOnly)
					return true;

                entry->SetTimeStamp(info->lastWriteTime);

			    info->needUpdate = false;
		    }
        }

		needsUpdate |= info->needUpdate;
		if (!info->needUpdate)
			someDontNeedUpdate = true;
	}

	// If we're just checking to see whether the file order list will do anything to the
	// archive AND there are files leftover in the archiveFileEntryMap, then we'll definitely
	// be erasing some files below.  Return true to the caller.
	if (options->checkOnly  &&  archiveFileEntryMap.Head())
		return true;

	// If there is anything left in the archiveFileEntryMap, that means the file order didn't
	// specify the file, and it needs to be removed.
	for (ArchiveFileEntryMap::Node* archiveFileEntryNode = archiveFileEntryMap.Head(); archiveFileEntryNode; archiveFileEntryNode = archiveFileEntryMap.Next(archiveFileEntryNode)) {
		// Inform the caller of the update.
		if (!filenameShown) {
			if (options->statusUpdateCallback)
				options->statusUpdateCallback(UPDATING_ARCHIVE, m_filename, options->statusUpdateUserData);
			filenameShown = true;
		}
		if (options->statusUpdateCallback)
			options->statusUpdateCallback(DELETING_ENTRY, archiveFileEntryMap.Key(archiveFileEntryNode), options->statusUpdateUserData);

		if (!options->requiresPack) {
			// Erase the file.
			FileErase(archiveFileEntryMap.Key(archiveFileEntryNode));
			needsPack = true;
		}
	}

	needsUpdate |= needsPack;

	ZipArchive* activeArchive = this;

	// If the file order list has a different order than what is currently in the zip
	// or if the options require a pack, build a second zip file to copy everything old
	// update the new files into.
	ZipArchive* newArchive = NULL;
	HeapString newArchiveFileName;
	if (orderChanged  ||  (options->requiresPack  &&  (needsPack  ||  needsUpdate)  &&  m_fileEntryCount > 0)) {
		// Generate a unique temporary name.
		newArchiveFileName = HeapString(m_filename) + ".repack";

		// Create a new zip archive.
		newArchive = new ZipArchive;
#if ZIPARCHIVE_ENCRYPTION
		HeapString oldPassword = this->defaultPassword;
		if (!newArchive->Create(newArchiveFileName, m_flags, oldPassword)) {
			delete newArchive;
#ifdef WIN32
			_unlink(newArchiveFileName);
#else
			unlink(newArchiveFileName);
#endif
			return false;
		}
#else
		if (!newArchive->Create(newArchiveFileName, 0, m_flags)) {
			delete newArchive;
#ifdef WIN32
			_unlink(newArchiveFileName);
#else
			unlink(newArchiveFileName);
#endif
			return false;
		}
#endif // ZIPARCHIVE_ENCRYPTION
		newArchive->fileNameMap.SetBlockSize(fileOrderList.Count());
		newArchive->m_fileEntryMaxCount = fileOrderList.Count() + 1;		// +1 to avoid a realloc.
		newArchive->m_fileEntryOffsets = new size_t[newArchive->m_fileEntryMaxCount];

		if (m_flags & EXTRA_DIRECTORY_AT_BEGINNING) {
			newArchive->GetParentFile()->SetLength(initialOffset);
			newArchive->m_dirOffset = initialOffset;
		}

		activeArchive = newArchive;

		// Inform the caller if we're spending time packing the archive.
		if (someDontNeedUpdate) {
			// Inform the user we are updating the archive.
			if (!filenameShown) {
				if (options->statusUpdateCallback)
					options->statusUpdateCallback(PACKING_ARCHIVE, m_filename, options->statusUpdateUserData);
				filenameShown = true;
			}
		}
	}

	// Iterate each entry in the file order list.
	for (FileOrderList::Node* node = fileOrderList.Head(); needsUpdate  &&  node; node = fileOrderList.Next(node), ++index) {
		FileOrderInfo& info = fileOrderList.Value(node);
		if (!info.used)
			continue;

		if (newArchive) {
			if (!info.needUpdate) {
				if (options->statusUpdateCallback)
					options->statusUpdateCallback(PACKING_COPY, info.entryName, options->statusUpdateUserData);

				ZipEntryFileHandle srcFileHandle;
				if (FileOpen(info.entryName, srcFileHandle)) {
					bool ret = newArchive->FileCopy(srcFileHandle);
					FileClose(srcFileHandle);
					if (!ret)
					{
						delete newArchive;
#if defined(_MSC_VER)
						_unlink(newArchiveFileName);
#else
						unlink(newArchiveFileName);
#endif
						return false;
					}
					continue;
				} else {
					return false;
				}
			}
		}

		// If we determined above there is nothing to update, then go to the next file.
		else if (!info.needUpdate)
			continue;

		// If there was something to update and we're only testing whether a change would occur,
		// inform the caller.
		if (options->checkOnly)
			return true;

		HeapString cacheFileName;

		// Is this file being copied from another zip archive?
		int pipePos = info.srcPath.ReverseFind('|');
		if (pipePos != -1) {
			HeapString archiveFileName = info.srcPath.Sub(0, pipePos);
			HeapString entryName = info.srcPath.Sub(pipePos + 1);
			ZipArchive* cacheDrive = PFL_OpenArchive(archiveFileName, openArchives, m_flags);
			if (cacheDrive) {
				ZipEntryInfo* cacheFileEntry = cacheDrive->FindFileEntry(entryName);
				if (cacheFileEntry->GetCompressionMethod() == info.compressionMethod) {
					ZipEntryFileHandle cacheFileHandle;
					if (cacheDrive->FileOpen(entryName, cacheFileHandle)) {
						if (!filenameShown) {
							if (options->statusUpdateCallback)
								options->statusUpdateCallback(UPDATING_ARCHIVE, m_filename, options->statusUpdateUserData);
							filenameShown = true;
						}
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(DIRECT_COPY_FROM_ANOTHER_ARCHIVE, info.entryName, options->statusUpdateUserData);

						if (activeArchive->FileCopy(cacheFileHandle, info.entryName, &info.lastWriteTime))
						{
//BROKEN						FileRename(entryName, info.entryName);
							continue;
						} else {
							return false;
						}
					}
				}
				else
				{
					ZipEntryFile cacheFile;
					if (cacheFile.Open(*cacheDrive, entryName)) {
						if (!filenameShown) {
							if (options->statusUpdateCallback)
								options->statusUpdateCallback(UPDATING_ARCHIVE, m_filename, options->statusUpdateUserData);
							filenameShown = true;
						}
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(UPDATING_ENTRY, info.entryName, options->statusUpdateUserData);

						if (activeArchive->FileCopy(cacheFile, info.entryName, info.compressionMethod, info.compressionLevel, &info.lastWriteTime)) {
//BROKEN						FileRename(entryName, info.entryName);
							continue;
						} else {
							return false;
						}
					}
				}
			}
		}

		// We're processing a standard file from the disk.  In order to get into this special case
		// processing block, the following criteria must be met:
		//
		// * The file order list wants the file compressed.
		// * There is a network cache.
		// * The size is within the network cache minimum size threshold as provided by the options.
		else if (info.compressionMethod != UNCOMPRESSED  &&  networkCache.IsNotEmpty()  &&
					(info.size == 0  ||  info.size >= options->fileCacheSizeThreshold)) {
			// Do we need a CRC and MD5 update?  If the file order list doesn't provide the CRC
			// or MD5, we have to calculate it.
			if (info.lastWriteTime != info.fileTime  ||  info.crc == 0  ||
						memcmp(info.md5, emptyMD5, sizeof(emptyMD5)) == 0) {
				info.fileTime = info.lastWriteTime;

				// Did the caller provide a callback to retrieve the checksum?  If so, call it.
				int found = 0;
				if (options->retrieveChecksumCallback) {
					found = options->retrieveChecksumCallback(info.srcPath, &info.crc, (unsigned char*)&info.md5, options->retrieveChecksumUserData);
				}

				// We still don't have a checksum.  Calculate it ourselves.
				if (!found) {
					FILE* file = fopen(info.srcPath, "rb");
					if (file) {
						info.crc = ZipArchive_GetFileCRC(file, 0, info.md5);
						fclose(file);
					}
				}
			}

			// Determine if the compressed file is in the provided network cache.
			//
			// First, build up the MD5 string.
			HeapString hex;
			char* hexBuffer = hex.GetBuffer(2 * 16 + 1);
			for (int i = 0; i < 16; ++i) sprintf(hexBuffer + 2 * i, "%02x", info.md5[i]);
			hex.ReleaseBuffer(2 * 16);

			// Now build the cache name in the form: abc/abcdef0123456789987654321fedcba
			cacheFileName = networkCache + hex.Sub(0, 3) + "/" + hex;

			// Does the compressed exist in the network cache?
			if (access(cacheFileName, 0) != -1) {
				// It appears so.  Try and open it.  Compressed network cache entries are
				// stored as single file entry zips.
				ZipArchive cacheDrive;
				if (cacheDrive.Open(cacheFileName)) {
					// The zip archive opened successfully.  Now try the file entry itself.
					ZipEntryFileHandle cacheFileHandle;
					if (cacheDrive.FileOpen(info.entryName, cacheFileHandle)) {
						// So far so good.  Inform the caller we are updating the zip.
						if (!filenameShown) {
							if (options->statusUpdateCallback)
								options->statusUpdateCallback(UPDATING_ARCHIVE, m_filename, options->statusUpdateUserData);
							filenameShown = true;
						}
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(COPYING_ENTRY_FROM_CACHE, info.entryName, options->statusUpdateUserData);

						// Perform a raw file copy from the network cache zip to the current zip.
						if (activeArchive->FileCopy(cacheFileHandle, NULL, &info.lastWriteTime)) {
							// The transfer was successful.  Move on to the next file.
							continue;
						}
						// The transfer failed... move on to the non-network cache version.
					} // if (cacheDrive.FileOpen(info.entryName, cacheFileHandle))
				} // if (cacheDrive.Open(cacheFileName))
			} // if (access(cacheFileName, 0) != -1)
		} // if networkcache

		// Open the file from the disk.
		DiskFile diskFile;
		if (!diskFile.Open(info.srcPath))
			return false;

		// Inform the user we are updating the archive.
		if (!filenameShown) {
			if (options->statusUpdateCallback)
				options->statusUpdateCallback(UPDATING_ARCHIVE, m_filename, options->statusUpdateUserData);
			filenameShown = true;
		}
		if (options->statusUpdateCallback)
			options->statusUpdateCallback(UPDATING_ENTRY, info.entryName, options->statusUpdateUserData);

		// Transfer the file from the disk into the archive compressing as we go per the file order info's specification.
		if (!activeArchive->FileCopy(diskFile, info.entryName, info.compressionMethod, info.compressionLevel, &info.lastWriteTime))
			return false;

		// We're done with the disk file.  Close it.
		diskFile.Close();

		// If we're using a network cache and we just compressed the file, we need to transfer the
		// compressed file into the cache.
		if (cacheFileName.IsNotEmpty()  &&  info.compressionMethod != 0) {
			// Create all directories leading up to the network cache location the compressed file
			// is to be put at.
			PathCreate(cacheFileName);

			// Create a single file zip archive at the network cache location.
			ZipArchive cacheDrive;
			if (cacheDrive.Create(cacheFileName, m_flags)) {
				// Open the zip entry we just compressed.
				ZipEntryFileHandle srcFileHandle;
				if (activeArchive->FileOpen(info.entryName, srcFileHandle)) {
					// Perform a raw transfer of the file straight into the network cache.
					cacheDrive.FileCopy(srcFileHandle);

					// Inform the caller of our update.
					if (!filenameShown) {
						if (options->statusUpdateCallback)
							options->statusUpdateCallback(UPDATING_ARCHIVE, m_filename, options->statusUpdateUserData);
						filenameShown = true;
					}
					if (options->statusUpdateCallback)
						options->statusUpdateCallback(COPYING_ENTRY_TO_CACHE, info.entryName, options->statusUpdateUserData);
				} // if (FileOpen(info.entryName, srcFileHandle))
			} // if (cacheDrive.Create(cacheFileName, 0, m_flags))
		} // if (cacheFileName.IsNotEmpty()  &&  info.compressionMethod != 0)
	} // for (FileOrderList

	if (options->checkOnly)
		return false;

	if (newArchive) {
		// Close the new zip archive.
		newArchive->fileNameMap.SetBlockSize(50);
		newArchive->Flush();
		newArchive->Close();
		delete newArchive;

		// Close the current zip archive.
		HeapString oldArchiveFileName = m_filename;
		HeapString oldPassword = this->defaultPassword;
		m_changed = false;
		m_needsPack = false;
		Close();

#if defined(WIN32)
		// Copy the packed drive.
		unlink(oldArchiveFileName);
		if (!::MoveFile(newArchiveFileName, oldArchiveFileName))
			::CopyFile(newArchiveFileName, oldArchiveFileName, false);

		_unlink(newArchiveFileName);
#elif defined(macintosh)  ||  defined(__APPLE__)
		int ret = copyfile(newArchiveFileName, oldArchiveFileName, NULL, COPYFILE_ALL | COPYFILE_MOVE);
		int err = errno;
		unlink(newArchiveFileName);
#else
		assert(0);
		unlink(newArchiveFileName);
#endif

		// Open it with the new
		if (!Open(oldArchiveFileName, false, m_flags, oldPassword))
			return false;
	} else {
		FlushOptions flushOptions;
		flushOptions.fileOrderList = &fileOrderList;
		Flush(&flushOptions);
/*
		PackOptions packOptions;
		packOptions.fileOrderList = &fileOrderList;
		if (NeedsPack(&packOptions))
		{
			if (!filenameShown)
			{
				if (options->statusUpdateCallback)
					options->statusUpdateCallback(UPDATING_ARCHIVE, m_filename, options->statusUpdateUserData);
				filenameShown = true;
			}
			if (options->statusUpdateCallback)
				options->statusUpdateCallback(PACKING_ARCHIVE, m_filename, options->statusUpdateUserData);
			Pack(&packOptions);
		}
*/
	}

	return true;
}


time_t ZipArchive::AdjustTime_t(time_t timeToAdjust)
{
	DWORD convertedTime = ConvertTime_tToDosTime(timeToAdjust);
	return ConvertDosDateToTime_t(convertedTime);
}


} // namespace Misc
