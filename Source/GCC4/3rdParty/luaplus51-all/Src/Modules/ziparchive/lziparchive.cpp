///////////////////////////////////////////////////////////////////////////////
// This source file is Copyright 1997-2011 by Joshua C. Jensen.
//
// The lziparchive module is licensed under the MIT license.
///////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#if _MSC_VER  ||  defined(__MINGW32__)
#include <windows.h>
#include <shlwapi.h>
#include <io.h>
#include <malloc.h>
#endif

#include <fcntl.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "Misc/DiskFile.h"
#include "Misc/MemFile.h"
#include "Misc/ZipArchive.h"
#include "Misc/ZipEntryFile.h"

#include <time.h>

#if _MSC_VER  &&  _MSC_VER <= 1300
double ui64ToDouble(unsigned __int64 ui64)
{
  __int64 i64 = (ui64 & 0x7FFFFFFFFFFFFFF);
  double dbl = (double) i64;
  if (ui64 & 0x8000000000000000)
    dbl += (double) 0x8000000000000000;
  return dbl;

}
#else
double ui64ToDouble(unsigned long long ui64)
{
	return (double)ui64;
}
#endif


using namespace Misc;

static void BuildFileEntry(lua_State* L, ZipEntryInfo* entry)
{
	lua_newtable(L);
	lua_pushstring(L, entry->GetFilename());
	lua_setfield(L, -2, "filename");

	lua_pushnumber(L, (lua_Number)entry->GetTimeStamp());
	lua_setfield(L, -2, "timestamp");

	lua_pushnumber(L, (lua_Number)entry->GetCRC());
	lua_setfield(L, -2, "crc");

	lua_pushlstring(L, (const char*)entry->GetMD5(), 16);
	lua_setfield(L, -2, "md5");

	lua_pushnumber(L, (lua_Number)entry->GetOffset());
	lua_setfield(L, -2, "offset");

	lua_pushnumber(L, (lua_Number)entry->GetUncompressedSize());
	lua_setfield(L, -2, "uncompressed_size");
	lua_pushnumber(L, (lua_Number)entry->GetCompressedSize());
	lua_setfield(L, -2, "compressed_size");
	lua_pushnumber(L, (lua_Number)entry->GetCompressionMethod());
	lua_setfield(L, -2, "compression_method");
}


#define ARCHIVEFILEHANDLE_METATABLE "ArchiveFileHandleMetaTable"
#define ZIPARCHIVE_METATABLE "ZipArchiveMetaTable"

static ZipArchive* lziparchive_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, ZIPARCHIVE_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "lziparchive.ZipArchive expected");
  return (ZipArchive*)ud;
}


static ZipEntryFileHandle* filehandle_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, ARCHIVEFILEHANDLE_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "lziparchive.ZipEntryFileHandle expected");
  return (ZipEntryFileHandle*)ud;
}


static int lziparchive_gc(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	archive->~ZipArchive();
	return 0;
}


static int _lziparchive_tostring(lua_State *L) {
	ZipArchive* archive = lziparchive_check(L, 1);

	char buffer[100];

	int top = lua_gettop(L);
	lua_pushstring(L, "[ziparchive object]: filename = \"");
	lua_pushstring(L, archive->GetFilename());
	lua_pushstring(L, "\", ");
	lua_pushstring(L, archive->IsReadOnly() ? "read-only" : "writable");
	sprintf(buffer, ", %u files", archive->GetFileEntryCount());
	lua_pushstring(L, buffer);
	lua_concat(L, lua_gettop(L) - top);
	return 1;
}



static int lziparchive_index(lua_State* L) {
	const char* key;
	ZipArchive* archive = lziparchive_check(L, 1);
	if (lua_type(L, 2) == LUA_TNUMBER) {
		size_t index = lua_tointeger(L, 2);
		luaL_argcheck(L, index >= 1  &&  index <= archive->GetFileEntryCount(), 2, "index not in range of 1..#archive");
		ZipEntryInfo* fileEntry = archive->GetFileEntry(index - 1);
		BuildFileEntry(L, fileEntry);
		return 1;
	}
	key = luaL_checklstring(L, 2, NULL);
	lua_getfield(L, lua_upvalueindex(1), key);
	return 1;
}


static int filehandle_gc(lua_State* L) {
    ZipEntryFileHandle* fileHandle = filehandle_check(L, 1);
	fileHandle->~ZipEntryFileHandle();
	return 0;
}


static bool _lziparchive_openhelper(lua_State* L, ZipArchive* archive, int arg, int numargs) {
	const char* fileName = luaL_checkstring(L, arg++);
	int readOnly = 1;
	const char* defaultPassword = 0;
	int flags = ZipArchive::SUPPORT_MD5;
	const char* mode = "r";
	if (numargs >= 2) {
		mode = luaL_optstring(L, arg++, "r");
		luaL_argcheck(L, mode[1] == 0  &&  (mode[0] == 'r'  ||  mode[0] == 'w'  ||  mode[0] == 'a'), arg - 1, "mode must be 'r', 'w', or 'a'");
		if (numargs >= 3) {
			if (lua_type(L, arg) == LUA_TNUMBER) {
				flags = (int)luaL_optnumber(L, arg++, ZipArchive::SUPPORT_MD5);
			}
			if (numargs >= 4) {
				defaultPassword = luaL_optstring(L, arg++, NULL);
			}
		}
	}

	if (mode[0] == 'r')
		return archive->Open(fileName, true, flags, defaultPassword);
	if (mode[0] == 'a')
		if (archive->Open(fileName, false, flags, defaultPassword))
			return true;
	return archive->Create(fileName, flags, defaultPassword);
}


int lziparchive_open(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	lua_pushboolean(L, _lziparchive_openhelper(L, archive, 2, lua_gettop(L)) != 0);
	return 1;
}


/*
int lziparchive_openfrommemory(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* buffer = luaL_checkstring(L, 2);
	MemFile memFile;
	memFile.Attach((BYTE*)buffer, lua_objlen(L, 2));
	int readOnly = 1;
	const char* defaultPassword = 0;
	int flags;
	if (lua_type(L, 3) == LUA_TBOOLEAN) {
		readOnly = lua_toboolean(L, 3);
	}
	defaultPassword = luaL_optstring(L, 4, NULL);
	flags = (int)luaL_optnumber(L, 5, ZipArchive::SUPPORT_MD5);
	lua_pushboolean(L, archive->Open(memFile, "filename", readOnly != 0, flags, defaultPassword) != 0);
	return 1;
}
*/


int lziparchive_close(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	lua_pushboolean(L, archive->Close() != 0);
	return 1;
}


int lziparchive_updatemd5s(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	archive->UpdateMD5s();
	return 0;
}


int lziparchive_flush(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	lua_pushboolean(L, archive->Flush() != 0);
	return 1;
}


int lziparchive_filecreate(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* fileName = luaL_checkstring(L, 2);
	int compressionMethod = (int)luaL_optnumber(L, 3, 8);
	int compressionLevel = (int)luaL_optnumber(L, 4, Z_DEFAULT_COMPRESSION);
	time_t fileTime = (UINT)luaL_optnumber(L, 5, (lua_Number)time(NULL));

	ZipEntryFileHandle* fileHandle = (ZipEntryFileHandle*)lua_newuserdata(L, sizeof(ZipEntryFileHandle));
	::new(fileHandle) ZipEntryFileHandle;

	luaL_getmetatable(L, ARCHIVEFILEHANDLE_METATABLE);
	lua_setmetatable(L, -2);

	if (!archive->FileCreate(fileName, *fileHandle, compressionMethod, compressionLevel, &fileTime))
		return 0;

	return 1;
}


int lziparchive_fileopen(lua_State* L)
{
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* fileName = NULL;
	int entryIndex = -1;
	if (lua_type(L, 2) == LUA_TSTRING)
		fileName = lua_tostring(L, 2);
	else if (lua_type(L, 2) == LUA_TNUMBER)
		entryIndex = lua_tointeger(L, 2);
	else
		luaL_typerror(L, 2, "string or integer");

	ZipEntryFileHandle* fileHandle = (ZipEntryFileHandle*)lua_newuserdata(L, sizeof(ZipEntryFileHandle));
	::new(fileHandle) ZipEntryFileHandle;

	luaL_getmetatable(L, ARCHIVEFILEHANDLE_METATABLE);
	lua_setmetatable(L, -2);

	if (fileName) {
		if (!archive->FileOpen(fileName, *fileHandle))
			return 0;
	} else {
		if (!archive->FileOpenIndex((size_t)entryIndex, *fileHandle))
			return 0;
	}

	return 1;
}


int lziparchive_fileclose(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushboolean(L, fileHandle->GetParentArchive()->FileClose(*fileHandle) != 0);
	return 1;
}


int lziparchive_filecloseall(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	archive->FileCloseAll();
	return 0;
}


int lziparchive_filegetfilename(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushstring(L, archive->FileGetFileName(*fileHandle));
	return 1;
}


int lziparchive_filegetposition(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushnumber(L, (lua_Number)ui64ToDouble(archive->FileGetPosition(*fileHandle)));
	return 1;
}


int lziparchive_filesetlength(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* fileHandle = filehandle_check(L, 2);
	long len = luaL_checklong(L, 3);
	archive->FileSetLength(*fileHandle, len);
	return 0;
}


int lziparchive_filegetlength(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushnumber(L, (lua_Number)ui64ToDouble(archive->FileGetLength(*fileHandle)));
	return 1;
}


int lziparchive_fileread(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* fileHandle = filehandle_check(L, 2);
	long len = luaL_optlong(L, 3, 0);
	if (len == 0)
		len = (long)archive->FileGetLength(*fileHandle);

	char* buffer = new char[len];
	UINT nr = (UINT)archive->FileRead(*fileHandle, buffer, len);
	lua_pushlstring(L, buffer, nr);
	delete[] buffer;
	return 1;
}


int lziparchive_filewrite(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* fileHandle = filehandle_check(L, 2);
	size_t len;
    const char* buffer = luaL_checklstring(L, 3, &len);
	len = luaL_optlong(L, 4, len);
	lua_pushnumber(L, (lua_Number)ui64ToDouble(archive->FileWrite(*fileHandle, buffer, len)));
	return 1;
}


int lziparchive_fileerase(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* fileName = luaL_checkstring(L, 2);
	lua_pushboolean(L, archive->FileErase(fileName) != 0);
	return 1;
}


int lziparchive_filerename(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* srcFileName = luaL_checkstring(L, 2);
	const char* destFileName = luaL_checkstring(L, 3);
	lua_pushboolean(L, archive->FileRename(srcFileName, destFileName) != 0);
	return 1;
}


int lziparchive_filecopy(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	ZipEntryFileHandle* srcFileHandle = filehandle_check(L, 2);
	lua_pushboolean(L, archive->FileCopy(*srcFileHandle) != 0);
	return 1;
}


int lziparchive_fileinsert(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* srcFileName = luaL_checkstring(L, 2);
	const char* destFileName = luaL_checkstring(L, 3);
    int compressionMethod = (int)luaL_optnumber(L, 4, ZipArchive::DEFLATED);
    int compressionLevel = (int)luaL_optnumber(L, 5, Z_DEFAULT_COMPRESSION);

	DiskFile file;
    if (!file.Open(srcFileName, File::MODE_READONLY)) {
		lua_pushboolean(L, false);
		return 1;
	}

	time_t fileTime = (UINT)luaL_optnumber(L, 5, (lua_Number)file.GetLastWriteTime());

	lua_pushboolean(L, archive->FileCopy(file, destFileName, compressionMethod, compressionLevel, &fileTime) != 0);
	return 1;
}


namespace Misc {
	int PathCreate(const char* inPath);
}


bool Copy(File& srcFile, const char* destFilename)
{
	// Operate in 16k buffers.
	const DWORD BUFFER_SIZE = 16 * 1024;

	DWORD fileSize = (DWORD)srcFile.GetLength();

	if (!Misc::PathCreate(destFilename))
		return false;

	// See if the destination file exists.
	DiskFile destFile;
	if (!destFile.Open(destFilename, File::MODE_CREATE | File::MODE_WRITEONLY))
		return false;

	// Allocate the buffer space.
	BYTE* buffer = (BYTE*)alloca(BUFFER_SIZE);

	// Keep copying until there is no more file left to copy.
	while (fileSize > 0) {
		// Copy the minimum of BUFFER_SIZE or the fileSize.
		DWORD readSize = BUFFER_SIZE < fileSize ? BUFFER_SIZE : fileSize;
		if (srcFile.Read(buffer, readSize) != readSize)
			return false;
		destFile.Write(buffer, readSize);
		fileSize -= readSize;
	}

	// Close the destination virtual file.
	destFile.Close();

	return true;
}


int lziparchive_fileextract(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* srcFileName = luaL_checkstring(L, 2);
	const char* destFileName = luaL_checkstring(L, 3);
	ZipEntryInfo* entry = archive->FindFileEntry(srcFileName);
	if (!entry) {
		lua_pushboolean(L, false);
		return 1;
	}

	ZipEntryFile file;
    if (!file.Open(*archive, srcFileName))
	{
		lua_pushboolean(L, false);
		return 1;
	}

	bool ret = Copy(file, destFileName);

/*	FILETIME createTime;
	FILETIME accessTime;
	FILETIME writeTime;
	HANDLE handle = CreateFile(destFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileTime(handle, &createTime, &accessTime, &writeTime);
	writeTime.dwLowDateTime = args[ 2 ].GetInteger();
	writeTime.dwHighDateTime = args[ 3 ].GetInteger();
	SetFileTime(handle, &createTime, &accessTime, &writeTime);
	CloseHandle(handle);
*/
	lua_pushboolean(L, ret);
	return 1;
}


static bool _lziparchive_collectfilelist(lua_State* L, ZipArchive::FileOrderList& fileOrder, int tableIndex) {
	if (!lua_istable(L, tableIndex))
		return false;

    int index = 1;
	while (true) {
		ZipArchive::FileOrderInfo info;

		lua_rawgeti(L, tableIndex, index++);						// 1-archive entryName
		if (lua_type(L, -1) == LUA_TTABLE) {
			lua_getfield(L, -1, "EntryName");
			if (lua_type(L, -1) != LUA_TSTRING)
				break;
			info.entryName = lua_tostring(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, -1, "SourcePath");
			if (lua_type(L, -1) != LUA_TSTRING)
				break;
			info.srcPath = lua_tostring(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, -1, "CompressionMethod");
			if (lua_type(L, -1) == LUA_TNUMBER)
				info.compressionMethod = (int)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, -1, "CompressionLevel");
			if (lua_type(L, -1) == LUA_TNUMBER)
			{
				info.compressionLevel = (int)lua_tonumber(L, -1);
				if (info.compressionLevel < -1)
					info.compressionLevel = Z_DEFAULT_COMPRESSION;
				else if (info.compressionLevel > Z_BEST_COMPRESSION)
					info.compressionLevel = Z_DEFAULT_COMPRESSION;
			}
			lua_pop(L, 1);

			lua_getfield(L, -1, "Timestamp");
			if (lua_type(L, -1) == LUA_TNUMBER)
				info.fileTime = (time_t)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, -1, "Size");
			if (lua_type(L, -1) == LUA_TNUMBER)
				info.size = (DWORD)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, -1, "CRC");
			if (lua_type(L, -1) == LUA_TNUMBER)
				info.crc = (DWORD)lua_tonumber(L, -1);
			lua_pop(L, 1);

			lua_getfield(L, -1, "MD5");
			if (lua_type(L, -1) == LUA_TSTRING  &&  lua_objlen(L, -1) == 16)
				memcpy(info.md5, lua_tostring(L, -1), 16);
			lua_pop(L, 1);

			lua_pop(L, 1);
		} else if (lua_type(L, -1) != LUA_TSTRING) {
			break;
		} else {
			info.entryName = lua_tostring(L, -1);

			lua_rawgeti(L, tableIndex, index++);						// 1-archive entryName srcPath
			if (lua_type(L, -1) != LUA_TSTRING)
				break;
			info.srcPath = lua_tostring(L, -1);

			info.compressionMethod = 8;
			lua_rawgeti(L, tableIndex, index);
			if (lua_type(L, -1) == LUA_TNUMBER) {
				info.compressionMethod = lua_tointeger(L, -1);
				index++;
			}

			lua_pop(L, 3);
		}

        fileOrder.InsertAfterTail(info);
	}

	return true;
}


int lziparchive_pack(lua_State* L)
{
	ZipArchive* archive = lziparchive_check(L, 1);
    ZipArchive::FileOrderList fileOrderList;

	ZipArchive::PackOptions options;
	if (lua_istable(L, 2)) {
		lua_getfield(L, 2, "FileOrder");
		if (lua_type(L, -1) == LUA_TSTRING  ||  lua_type(L, -1) == LUA_TTABLE) {
			_lziparchive_collectfilelist(L, fileOrderList, lua_gettop(L));
			options.fileOrderList = &fileOrderList;
		}
		lua_pop(L, 1);
	}

	lua_pushboolean(L, archive->Pack(&options) != 0);
	return 1;
}


struct RetrieveChecksumInfo {
	lua_State* L;
	int callbackRef;
};


static int _lziparchive_retrievechecksum(const char* sourcePath, DWORD* crc, unsigned char* md5, void* userData)
{
	RetrieveChecksumInfo* info = (RetrieveChecksumInfo*)userData;
	lua_State* L = info->L;
	int top = lua_gettop(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, info->callbackRef);
	lua_pushstring(L, sourcePath);
	if (lua_pcall(L, 1, LUA_MULTRET, 0) != 0)
	{
		lua_settop(L, top);
		return 0;
	}
	if (lua_type(L, -2) == LUA_TNUMBER  &&  lua_type(L, -1) == LUA_TSTRING  &&  lua_objlen(L, -1) == 16)
	{
		*crc = (DWORD)lua_tonumber(L, -2);
		memcpy(md5, lua_tostring(L, -1), 16);
		lua_settop(L, top);
		return 1;
	}
	lua_settop(L, top);
	return 0;
}


struct StatusUpdateInfo
{
	lua_State* L;
	int callbackRef;
};


static void _lziparchive_statusupdate(ZipArchive::FileListStatus status, const char* text, void* userData)
{
	StatusUpdateInfo* info = (StatusUpdateInfo*)userData;
	lua_State* L = info->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, info->callbackRef);
	lua_pushnumber(L, status);
	lua_pushstring(L, text);
	lua_pcall(L, 2, 0, 0);
}


int lziparchive_processfilelist(lua_State* L)
{
	ZipArchive* archive = lziparchive_check(L, 1);

	ZipArchive::ProcessFileListOptions options;

	RetrieveChecksumInfo checksumInfo;
	checksumInfo.L = NULL;
	checksumInfo.callbackRef = LUA_NOREF;

	StatusUpdateInfo statusUpdateInfo;
	statusUpdateInfo.L = NULL;
	statusUpdateInfo.callbackRef = LUA_NOREF;
	if (lua_istable(L, 3))
	{
		lua_getfield(L, -1, "CheckOnly");
		if (lua_type(L, -1) == LUA_TBOOLEAN)
			options.checkOnly = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		lua_getfield(L, -1, "RequiresPack");
		if (lua_type(L, -1) == LUA_TBOOLEAN)
			options.requiresPack = lua_toboolean(L, -1) != 0;
		lua_pop(L, 1);

		lua_getfield(L, 3, "FileCache");
		if (lua_type(L, -1) == LUA_TSTRING)
			options.fileCache = lua_tostring(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 3, "FileCacheSizeThreshold");
		if (lua_type(L, -1) == LUA_TNUMBER)
			options.fileCacheSizeThreshold = (size_t)lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 3, "RetrieveChecksum");
		if (lua_type(L, -1) == LUA_TFUNCTION)
		{
			options.retrieveChecksumCallback = _lziparchive_retrievechecksum;

			checksumInfo.L = L;
			checksumInfo.callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
			options.retrieveChecksumUserData = &checksumInfo;
		}
		else
			lua_pop(L, 1);

		lua_getfield(L, 3, "StatusUpdate");
		if (lua_type(L, -1) == LUA_TFUNCTION)
		{
			options.statusUpdateCallback = _lziparchive_statusupdate;

			statusUpdateInfo.L = L;
			statusUpdateInfo.callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
			options.statusUpdateUserData = &statusUpdateInfo;
		}
		else
			lua_pop(L, 1);
	}
	else if (lua_isstring(L, 3))
	{
		options.fileCache = lua_tostring(L, 3);
	}

    ZipArchive::FileOrderList fileOrder;
	if (_lziparchive_collectfilelist(L, fileOrder, 2))
	{
		bool ret = archive->ProcessFileList(fileOrder, &options);
		luaL_unref(L, LUA_REGISTRYINDEX, checksumInfo.callbackRef);
		luaL_unref(L, LUA_REGISTRYINDEX, statusUpdateInfo.callbackRef);
		lua_pushboolean(L, ret);
		if (!ret)
		{
			lua_pushstring(L, archive->errorString);
			return 2;
		}
        return 1;
	}

    return 0;
}


int lziparchive_getfilename(lua_State* L)
{
	ZipArchive* archive = lziparchive_check(L, 1);
	lua_pushstring(L, archive->GetFilename());
	return 1;
}


int lziparchive_fileentrycount(lua_State* L)
{
	ZipArchive* archive = lziparchive_check(L, 1);
	lua_pushnumber(L, archive->GetFileEntryCount());
	return 1;
}


int lziparchive_fileentry(lua_State* L)
{
	ZipArchive* archive = lziparchive_check(L, 1);

	int entryIndex = (int)luaL_optlong(L, 2, -1);

	ZipEntryInfo* entry;

	entry = archive->GetFileEntry(entryIndex - 1);

	if (!entry)
		return 0;

	BuildFileEntry(L, entry);
	return 1;
}


int lziparchive_findfileentry(lua_State* L)
{
	ZipArchive* archive = lziparchive_check(L, 1);

	const char* fileName = luaL_checkstring(L, 2);

	ZipEntryInfo* entry = archive->FindFileEntry(fileName);

	if (!entry)
		return 0;

	BuildFileEntry(L, entry);
	return 1;
}


int lziparchive_fileentryindex(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);

	const char* fileName = luaL_checkstring(L, 2);
	size_t index = archive->FindFileEntryIndex(fileName);
	if (index == ZipArchive::INVALID_FILE_ENTRY)
		return 0;
	lua_pushnumber(L, archive->FindFileEntryIndex(fileName) + 1);
	return 1;
}


#define ZIPARCHIVE_FILEENTRY_METATABLE "ZipArchive_FileIteratorMetatable"

struct fileentry_info {
	int zipArchiveRef;
	ZipArchive* archive;
	size_t entryIndex;
};


static int _lziparchive_fileentry_gc(lua_State* L) {
	fileentry_info* info = (fileentry_info*)luaL_checkudata(L, 1, ZIPARCHIVE_FILEENTRY_METATABLE);
	luaL_unref(L, LUA_REGISTRYINDEX, info->zipArchiveRef);
	return 0;
}


static int _lziparchive_buildfileentry(lua_State* L, int archiveIndex, ZipArchive* archive, int fileEntryIndex) {
	struct fileentry_info* info = (struct fileentry_info*)lua_newuserdata(L, sizeof(struct fileentry_info));
	lua_pushvalue(L, archiveIndex);
	info->zipArchiveRef = luaL_ref(L, LUA_REGISTRYINDEX);
	info->archive = archive;
	info->entryIndex = fileEntryIndex;

	luaL_getmetatable(L, ZIPARCHIVE_FILEENTRY_METATABLE);
	lua_setmetatable(L, -2);

	return 0;
}


static fileentry_info* zafe_checkmetatable(lua_State* L, int index) {
	return (fileentry_info*)luaL_checkudata(L, index, ZIPARCHIVE_FILEENTRY_METATABLE);
}


static ZipEntryInfo* zafe_getentry(lua_State* L, fileentry_info* info) {
	ZipEntryInfo* entry = info->archive->GetFileEntry(info->entryIndex);
	if (!entry)
		luaL_error(L, "ziparchive: index %d is invalid", info->entryIndex);
	return entry;
}


static int _zafe_index_compressed_size(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushnumber(L, entry->GetCompressedSize());
	return 1;
}


static int _zafe_index_compression_method(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushnumber(L, entry->GetCompressionMethod());
	return 1;
}


static int _zafe_index_crc(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushnumber(L, entry->GetCRC());
	return 1;
}


static int _zafe_index_filename(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushstring(L, entry->GetFilename());
	return 1;
}


static int _zafe_index_md5(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushlstring(L, (const char*)entry->GetMD5(), 16);
	return 1;
}


static int _zafe_index_offset(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushnumber(L, entry->GetOffset());
	return 1;
}


static int _zafe_index_timestamp(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushnumber(L, (lua_Number)entry->GetTimeStamp());
	return 1;
}


static int _zafe_index_uncompressed_size(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_pushnumber(L, entry->GetUncompressedSize());
	return 1;
}


static int _zafe_index_table(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	lua_newtable(L);
	lua_pushstring(L, entry->GetFilename());
	lua_setfield(L, -2, "filename");

	lua_pushnumber(L, (lua_Number)entry->GetTimeStamp());
	lua_setfield(L, -2, "timestamp");

	lua_pushnumber(L, (lua_Number)entry->GetCRC());
	lua_setfield(L, -2, "crc");

	lua_pushlstring(L, (const char*)entry->GetMD5(), 16);
	lua_setfield(L, -2, "md5");

	lua_pushnumber(L, (lua_Number)entry->GetOffset());
	lua_setfield(L, -2, "offset");

	lua_pushnumber(L, (lua_Number)entry->GetUncompressedSize());
	lua_setfield(L, -2, "uncompressed_size");
	lua_pushnumber(L, (lua_Number)entry->GetCompressedSize());
	lua_setfield(L, -2, "compressed_size");
	lua_pushnumber(L, (lua_Number)entry->GetCompressionMethod());
	lua_setfield(L, -2, "compression_method");

	return 1;
}


static int _zafe_index_contents(lua_State* L, fileentry_info* info, ZipEntryInfo* entry) {
	ZipEntryFileHandle fileHandle;
	if (!info->archive->FileOpenIndex(info->entryIndex, fileHandle)) {
		return 0;
	}
	LONGLONG bufferSize = info->archive->FileGetLength(fileHandle);
	unsigned char* buffer = (unsigned char*)malloc((size_t)bufferSize);
	if (info->archive->FileRead(fileHandle, buffer, bufferSize) != bufferSize) {
		free(buffer);
		info->archive->FileClose(fileHandle);
		return 0;
	}
	info->archive->FileClose(fileHandle);
	lua_pushlstring(L, (const char*)buffer, (size_t)bufferSize);
	free(buffer);
	return 1;
}


typedef int (*_zafe_index_property_func)(lua_State*, fileentry_info*, ZipEntryInfo* entry);
typedef struct _zafe_index_properties_reg {
	const char *name;
	_zafe_index_property_func func;
} _zafe_index_properties_reg;


static const struct _zafe_index_properties_reg _zafe_index_properties[] = {
	{ "compressed_size",		_zafe_index_compressed_size },
	{ "compression_method",		_zafe_index_compression_method },
	{ "crc",					_zafe_index_crc },
	{ "filename",				_zafe_index_filename },
	{ "md5",					_zafe_index_md5 },
	{ "offset",					_zafe_index_offset },
	{ "timestamp",				_zafe_index_timestamp },
	{ "uncompressed_size",		_zafe_index_uncompressed_size },
	{ "table",					_zafe_index_table },
	{ "contents",				_zafe_index_contents },
	{ NULL, NULL },
};


static int _zafe_index(lua_State *L) {
	_zafe_index_property_func function;
	fileentry_info* info = (fileentry_info*)luaL_checkudata(L, 1, ZIPARCHIVE_FILEENTRY_METATABLE);
	const char* key = luaL_checklstring( L, 2, NULL );
	lua_getfield( L, lua_upvalueindex( 1 ), key );
	function = (_zafe_index_property_func)lua_touserdata( L, -1 );
	if ( function ) {
		ZipEntryInfo* entry = info->archive->GetFileEntry(info->entryIndex);
		if (!entry)
			luaL_error(L, "ziparchive: index %d is invalid", info->entryIndex);
		return function( L, info, entry );
	}
    luaL_argerror(L, 2, "improper key");
	return 1;
}


static int _lziparchive_fileentry_tostring(lua_State* L) {
	char buffer[100];

	int top = lua_gettop(L);
	lua_pushstring(L, "[ziparchive.fileentry object]: ");

	fileentry_info* info = (fileentry_info*)luaL_checkudata(L, 1, ZIPARCHIVE_FILEENTRY_METATABLE);
	ZipEntryInfo* entry = info->archive->GetFileEntry(info->entryIndex);
	if (!entry) {
		sprintf(buffer, "index %u is invalid", info->entryIndex);
		lua_pushstring(L, buffer);
	} else {
		lua_pushstring(L, "filename = \"");
		lua_pushstring(L, entry->GetFilename());
		sprintf(buffer, "\", compressed_size = %u", entry->GetCompressedSize());
		lua_pushstring(L, buffer);
		sprintf(buffer, ", compression_method = %u", entry->GetCompressionMethod());
		lua_pushstring(L, buffer);
		sprintf(buffer, ", crc = %u", entry->GetCRC());
		lua_pushstring(L, buffer);
		sprintf(buffer, ", md5 = %u", entry->GetMD5());
		lua_pushstring(L, buffer);
		sprintf(buffer, ", offset = %u", entry->GetOffset());
		lua_pushstring(L, buffer);
		sprintf(buffer, ", timestamp = %u", entry->GetTimeStamp());
		lua_pushstring(L, buffer);
		sprintf(buffer, ", uncompressed_size = %u", entry->GetUncompressedSize());
		lua_pushstring(L, buffer);
	}

	lua_concat(L, lua_gettop(L) - top);
	return 1;
}



static int ziparchive_fileentry_create_metatable(lua_State *L) {
	luaL_newmetatable(L, ZIPARCHIVE_FILEENTRY_METATABLE);	// metatable
	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcfunction(L, _lziparchive_fileentry_gc);		// metatable __gc function
	lua_settable(L, -3);									// metatable

	lua_pushliteral(L, "__tostring");						// metatable __tostring
	lua_pushcfunction(L, _lziparchive_fileentry_tostring);	// metatable __tostring function
	lua_settable(L, -3);									// metatable

	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index table table

	for (const _zafe_index_properties_reg* l = _zafe_index_properties; l->name; l++) {
		lua_pushlightuserdata(L, (void*)l->func);
		lua_setfield(L, -2, l->name);
	}

	lua_pushcclosure( L, _zafe_index, 1 );
	lua_settable(L, -3);

	lua_pop(L, 1);
	return 0;
}


#define ZIPARCHIVE_FILEITERATOR_METATABLE "ZipArchive_FileIteratorMetatable"

struct fileiter_info {
	int zipArchiveRef;
	ZipArchive* archive;
	size_t index;
	char wildcard[1024];
};


/**
	\internal
	\author Jack Handy

	Borrowed from http://www.codeproject.com/string/wildcmp.asp.
	Modified by Joshua Jensen.
**/
int WildMatch( const char* pattern, const char *string, int caseSensitive )
{
  const char* mp;
  const char* cp;

  // Handle all the letters of the pattern and the string.
  while ( *string != 0  &&  *pattern != '*' ) {
    if ( *pattern != '?' ) {
      if ( caseSensitive ) {
        if ( *pattern != *string )
          return 0;
      } else {
        if ( toupper( *pattern ) != toupper( *string ) )
          return 0;
      }
    }

    pattern++;
    string++;
  }

  mp = NULL;
  cp = NULL;

  while ( *string != 0 ) {
    if (*pattern == '*') {
      // It's a match if the wildcard is at the end.
      if ( *++pattern == 0 )
        return 1;

      mp = pattern;
      cp = string + 1;
    } else {
      if ( caseSensitive ) {
        if ( *pattern == *string  ||  *pattern == '?' ) {
          pattern++;
          string++;
        } else {
          pattern = mp;
          string = cp++;
        }
      } else {
        if ( toupper( *pattern ) == toupper( *string )  ||  *pattern == '?' ) {
          pattern++;
          string++;
        } else {
          pattern = mp;
          string = cp++;
        }
      }
    }
  }

  // Collapse remaining wildcards.
  while ( *pattern == '*' )
    pattern++;

  return !*pattern;
}


static int _lziparchive_fileiterator(lua_State* L) {
	fileiter_info* info = (fileiter_info*)luaL_checkudata(L, lua_upvalueindex(1), ZIPARCHIVE_FILEITERATOR_METATABLE);
	while (info->index < info->archive->GetFileEntryCount()) {
		ZipEntryInfo* fileEntry = info->archive->GetFileEntry(info->index);
		++info->index;
		if (info->wildcard[0] == 0) {
			_lziparchive_buildfileentry(L, lua_upvalueindex(1), info->archive, info->index - 1);
			return 1;
		} else {
			if (WildMatch(info->wildcard, fileEntry->GetFilename(), 0)) {
				_lziparchive_buildfileentry(L, lua_upvalueindex(1), info->archive, info->index - 1);
				return 1;
			}
		}
	}
	return 0;
}


static int _lziparchive_fileiterator_gc(lua_State* L) {
	fileiter_info* info = (fileiter_info*)luaL_checkudata(L, 1, ZIPARCHIVE_FILEITERATOR_METATABLE);
	luaL_unref(L, LUA_REGISTRYINDEX, info->zipArchiveRef);
	return 0;
}


static int ziparchive_fileiterator_create_metatable(lua_State *L) {
	luaL_newmetatable(L, ZIPARCHIVE_FILEITERATOR_METATABLE);// metatable
	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcfunction(L, _lziparchive_fileiterator_gc);		// metatable __gc function
	lua_settable(L, -3);									// metatable
/*	lua_pushliteral(L, "__tostring");						// metatable __tostring
	lua_pushcfunction(L, filefind_tostring);				// metatable __tostring function
	lua_settable(L, -3);									// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);										// metatable __index table table
	luaL_register (L, NULL, filefind_index_functions);
	lua_newtable(L);										// metatable __index table table
	luaL_register(L, NULL, filefind_index_properties);
	lua_pushvalue( L, -4 );
	lua_pushcclosure( L, filefind_index, 3 );
	lua_settable(L, -3);*/

	lua_pop(L, 1);
	return 0;
}


static int lziparchive_files(lua_State* L) {
	ZipArchive* archive = lziparchive_check(L, 1);
	const char* wildcard = luaL_optstring(L, 2, "");

	struct fileiter_info* info = (struct fileiter_info*)lua_newuserdata(L, sizeof(struct fileiter_info));
	lua_pushvalue(L, 1);
	info->zipArchiveRef = luaL_ref(L, LUA_REGISTRYINDEX);
	info->archive = archive;
	info->index = 0;
	strcpy(info->wildcard, wildcard);

	luaL_getmetatable(L, ZIPARCHIVE_FILEITERATOR_METATABLE);
	lua_setmetatable(L, -2);

	lua_pushcclosure(L, _lziparchive_fileiterator, 1);
	return 1;
}


int lziparchive_new(lua_State* L)
{
	ZipArchive* archive = (ZipArchive*)lua_newuserdata(L, sizeof(ZipArchive));
	::new(archive) ZipArchive;

	luaL_getmetatable(L, ZIPARCHIVE_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


int lziparchive_new_open(lua_State* L)
{
	ZipArchive* archive = (ZipArchive*)lua_newuserdata(L, sizeof(ZipArchive));
	::new(archive) ZipArchive;
	if (!_lziparchive_openhelper(L, archive, 1, lua_gettop(L) - 1)) {
		archive->~ZipArchive();
		return 0;
	}

	luaL_getmetatable(L, ZIPARCHIVE_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


int LS_Help(lua_State* L)
{
	char help[] =
"archive = ziparchive.new() - Create a zip archive structure\n"
"archive = ziparchive.open(filename [, mode] [, openflags] [, defaultPassword]) - Create/open a zip archive\n"
"\tmode:\t'r' - default, open existing archive read-only\n"
"\t\t'w' - create new archive\n"
"\t\t'a' - open existing archive writable or create new archive\n"
"\topenflags:\tSUPPORT_MD5 - Calculate md5sums for every file\n"
"\t\t\tEXTRA_DIRECTORY_AT_BEGINNING - Write an extra directory structure\n"
"time = ziparchive.AdjustTime_t(timeToAdjust) - Adjusts a timestamp to zip format\n"
"\n"
"Drive commands:\n"
"\n"
"    opened = archive:open(fileName [, flags = 'r' [, defaultPassword] ])\n"
"    archive:close()\n"
"    archive:updatemd5s()\n"
"    archive:flush(bool directoryAtBeginning)\n"
"\n"
"    vfile = archive:filecreate(fileName, CompressionMethod, CompressionLevel, time_t fileTime)\n"
"    vfile = archive:fileopen(fileName)\n"
"    archive:filecloseall()\n"
"    bool ret = archive:fileerase(fileName)\n"
"    bool ret = archive:filerename(oldFileName, newFileName)\n"
"    bool ret = archive:fileinsert(srcFileName, destFileName, CompressionMethod, CompressionLevel, time_t fileTime)\n"
"    bool ret = archive:fileextract(srcFileName, destFileName)\n"
"\n"
"    FileOrderTable = {\n"
"        string EntryName,\n"
"        string SourcePath,\n"
"        CompressionMethod,\n"
"        CompressionLevel,\n"
"        time_t Timestamp,\n"
"        integer Size,\n"
"        uint32_t CRC,\n"
"        string MD5,\n"
"    }\n"
"\n"
"    PackOptions = {\n"
"        boolean DirectoryAtBeginning (default false),\n"
"        FileOrderTable fileOrder,\n"
"    }\n"
"    archive:pack(PackOptions)\n"
"\n"
"    ProcessFileListOptions = {\n"
"        boolean CheckOnly,\n"
"        boolean RequiresPack,\n"
"        string FileCache,\n"
"        integer FileCacheSizeThreshold,\n"
"        crc, md5 = function RetrieveChecksum(sourcePath),\n"
"        function StatusUpdate(FileListStatus, text),\n"
"    }\n"
"    archive:processfilelist(fileNameTable, ProcessFileListOptions)\n"
"\n"
"    string fileName = archive:getfilename()\n"
"    int entryCount = archive:fileentrycount()\n"
"    FileEntryInfo fileEntry = archive:fileentry(index)\n"
"    FileEntryInfo fileEntry = archive:findfileentry(fileName)\n"
"        FileEntryInfo =\n"
"        {\n"
"            string filename\n"
"            time_t timestamp\n"
"            DWORD crc\n"
"            string md5\n"
"            DWORD offset\n"
"            DWORD size\n"
"            DWORD uncompressed_size\n"
"            DWORD compressed_size\n"
"            DWORD compression_method\n"
"        }\n"
"    int entryIndex = archive:fileentryindex(fileName)\n"
"\n"
"Files Iterator:\n"
"    for entry in archive:files() do print(entry) end\n"
"\n"
"File commands:\n"
"\n"
"    string fileName = archive:filegetfilename(vfile)\n"
"    long position = archive:filegetposition(vfile)\n"
"    archive:filesetlength(vfile, newLength)\n"
"    long len = archive:filegetlength(vfile)\n"
"    buffer = archive:fileread(vfile [, optionalSize])\n"
"    archive:filewrite(vfile, buffer [, optionalSize])\n"
"    archive:close(vfile)\n"
"\n"
"CompressionMethod Enumerations:\n"
"\n"
"    ziparchive.UNCOMPRESSED\n"
"    ziparchive.DEFLATED\n"
"\n"
"CompressionLevel Enumerations:\n"
"\n"
"   The compression level may range from 0 (uncompressed) to 9 (best compression).\n"
"\n"
"    ziparchive.NO_COMPRESSION = 0\n"
"    ziparchive.BEST_SPEED = 1\n"
"    ziparchive.BEST_COMPRESSION = 9\n"
"    ziparchive.DEFAULT_COMPRESSION = -1 (defaults to 6)\n"
"\n"
"FileListStatus Enumerations:\n"
"\n"
"    ziparchive.UPDATING_ARCHIVE\n"
"    ziparchive.DELETING_ENTRY\n"
"    ziparchive.DIRECT_COPY_FROM_ANOTHER_ARCHIVE\n"
"    ziparchive.COPYING_ENTRY_FROM_CACHE\n"
"    ziparchive.COPYING_ENTRY_TO_CACHE\n"
"    ziparchive.UPDATING_ENTRY\n"
"    ziparchive.PACKING_ARCHIVE\n";

	lua_pushstring(L, help);
	return 1;
}


int LS_crc32(lua_State* L)
{
	if (lua_type(L, 1) == LUA_TSTRING) {
		lua_pushnumber(L, crc32(0, (BYTE*)lua_tostring(L, 1), lua_objlen(L, 2)));
		return 1;
	}

	if (lua_type(L, 1) == LUA_TNUMBER  &&  lua_type(L, 2) == LUA_TSTRING) {
		lua_pushnumber(L, crc32(lua_tointeger(L, 1), (BYTE*)lua_tostring(L, 2), lua_objlen(L, 2)));
		return 1;
	}

	return 0;
}


int LS_AdjustTime_t(lua_State* L) {
	lua_pushnumber(L, (lua_Number)ZipArchive::AdjustTime_t(lua_tointeger(L, 1)));
	return 1;
}


static const struct luaL_reg lziparchivefilehandle_funcs[] = {
	{ "__gc",				filehandle_gc },
	{ NULL, NULL },
};


static const struct luaL_reg lziparchive_funcs[] = {
	{ "open",				lziparchive_open },
//	{ "openfrommemory",		lziparchive_openfrommemory },
	{ "close",				lziparchive_close },
	{ "updatemd5s",			lziparchive_updatemd5s },
	{ "flush",				lziparchive_flush },
	{ "filecreate",			lziparchive_filecreate },
	{ "fileopen",			lziparchive_fileopen },
	{ "fileclose",			lziparchive_fileclose },
	{ "filecloseall",		lziparchive_filecloseall },
	{ "_filecopy",			lziparchive_filecopy },
	{ "filegetfilename",    lziparchive_filegetfilename },
	{ "filegetposition",    lziparchive_filegetposition },
	{ "filesetlength",      lziparchive_filesetlength },
	{ "filegetlength",      lziparchive_filegetlength },
	{ "fileread",           lziparchive_fileread },
	{ "filewrite",          lziparchive_filewrite },
	{ "fileerase",			lziparchive_fileerase },
	{ "filerename",			lziparchive_filerename },
	{ "fileinsert",			lziparchive_fileinsert },
	{ "fileextract",		lziparchive_fileextract },
	{ "pack",				lziparchive_pack },
	{ "processfilelist",	lziparchive_processfilelist },
	{ "getfilename",		lziparchive_getfilename },
	{ "fileentrycount",		lziparchive_fileentrycount },
	{ "fileentry",			lziparchive_fileentry },
	{ "findfileentry",		lziparchive_findfileentry },
	{ "fileentryindex",		lziparchive_fileentryindex },

	{ "files",				lziparchive_files },

	{ NULL, NULL },
};


static void lziparchive_createmetatables(lua_State* L)
{
	luaL_newmetatable(L, ARCHIVEFILEHANDLE_METATABLE);		// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);									// metatable __index metatable
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, lziparchivefilehandle_funcs);
	lua_pop(L, 1);

	luaL_newmetatable(L, ZIPARCHIVE_METATABLE);				// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_newtable(L);
	luaL_register(L, NULL, lziparchive_funcs);
	lua_pushcclosure( L, lziparchive_index, 1 );
	lua_rawset(L, -3);										// metatable

	lua_pushliteral(L, "__len");							// metatable __len
	lua_pushcclosure( L, lziparchive_fileentrycount, 0 );
	lua_rawset(L, -3);										// metatable

	lua_pushliteral(L, "__tostring");						// metatable __tostring
	lua_pushcfunction(L, _lziparchive_tostring);			// metatable __tostring function
	lua_settable(L, -3);									// metatable

	lua_pushliteral(L, "__gc");								// metatable __gc
	lua_pushcclosure( L, lziparchive_gc, 0 );
	lua_rawset(L, -3);										// metatable

	lua_pop(L, 1);

/*
	luaL_newmetatable(L, ZIPENTRYINFO_METATABLE);			// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);									// metatable __index metatable
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, lziparchive_funcs);
	lua_pop(L, 1);
*/
	ziparchive_fileiterator_create_metatable(L);
	ziparchive_fileentry_create_metatable(L);
}


extern "C" DWORD ZipArchive_GetFileCRC(FILE* file, UINT startOffset, unsigned char md5[16]);

static int LS_filecrcmd5(lua_State* L)
{
	const char* fileName = luaL_checkstring(L, 1);
	FILE* file = fopen(fileName, "rb");
	unsigned char md5[16];
	DWORD crc = ZipArchive_GetFileCRC(file, 0, md5);
	fclose(file);
	lua_pushnumber(L, crc);
	lua_pushlstring(L, (const char*)md5, 16);
	return 2;
}


static const struct luaL_reg lziparchivelib[] = {
	{ "new",				lziparchive_new },
	{ "open",				lziparchive_new_open },
	{ "help",				LS_Help },
	{ "crc32",				LS_crc32 },
	{ "filecrcmd5",			LS_filecrcmd5 },

	{ "AdjustTime_t",		LS_AdjustTime_t },
	{NULL, NULL},
};


extern "C" int luaopen_ziparchive(lua_State* L)
{
	luaL_register(L, "ziparchive", lziparchivelib);

	lua_pushnumber(L, ZipArchive::UNCOMPRESSED);
	lua_setfield(L, -2, "UNCOMPRESSED");
	lua_pushnumber(L, ZipArchive::DEFLATED);
	lua_setfield(L, -2, "DEFLATED");

	lua_pushnumber(L, Z_NO_COMPRESSION);
	lua_setfield(L, -2, "NO_COMPRESSION");
	lua_pushnumber(L, Z_BEST_SPEED);
	lua_setfield(L, -2, "BEST_SPEED");
	lua_pushnumber(L, Z_BEST_COMPRESSION);
	lua_setfield(L, -2, "BEST_COMPRESSION");
	lua_pushnumber(L, Z_DEFAULT_COMPRESSION);
	lua_setfield(L, -2, "DEFAULT_COMPRESSION");

	lua_pushnumber(L, ZipArchive::UPDATING_ARCHIVE);
	lua_setfield(L, -2, "UPDATING_ARCHIVE");
	lua_pushnumber(L, ZipArchive::DELETING_ENTRY);
	lua_setfield(L, -2, "DELETING_ENTRY");
	lua_pushnumber(L, ZipArchive::DIRECT_COPY_FROM_ANOTHER_ARCHIVE);
	lua_setfield(L, -2, "DIRECT_COPY_FROM_ANOTHER_ARCHIVE");
	lua_pushnumber(L, ZipArchive::COPYING_ENTRY_FROM_CACHE);
	lua_setfield(L, -2, "COPYING_ENTRY_FROM_CACHE");
	lua_pushnumber(L, ZipArchive::COPYING_ENTRY_TO_CACHE);
	lua_setfield(L, -2, "COPYING_ENTRY_TO_CACHE");
	lua_pushnumber(L, ZipArchive::UPDATING_ENTRY);
	lua_setfield(L, -2, "UPDATING_ENTRY");
	lua_pushnumber(L, ZipArchive::PACKING_ARCHIVE);
	lua_setfield(L, -2, "PACKING_ARCHIVE");
	lua_pushnumber(L, ZipArchive::PACKING_COPY);
	lua_setfield(L, -2, "PACKING_COPY");

	lua_pushnumber(L, ZipArchive::SUPPORT_MD5);
	lua_setfield(L, -2, "SUPPORT_MD5");
	lua_pushnumber(L, ZipArchive::EXTRA_DIRECTORY_AT_BEGINNING);
	lua_setfield(L, -2, "EXTRA_DIRECTORY_AT_BEGINNING");

	lziparchive_createmetatables(L);
	return 1;
}


