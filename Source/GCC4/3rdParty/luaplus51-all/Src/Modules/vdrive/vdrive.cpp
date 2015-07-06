///////////////////////////////////////////////////////////////////////////////
// This source file is Copyright 1997-2009 by Joshua C. Jensen.
//
// The vdrive module is licensed under the MIT license.
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
#include "Misc/VirtualDrive.h"
#include "Misc/VirtualFile.h"

#include <time.h>

using namespace Misc;

#define VDRIVEFILEHANDLE_METATABLE "VDriveFileHandleMetaTable"
#define VDRIVE_METATABLE "VDriveMetaTable"

static VirtualDrive* vdrive_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, VDRIVE_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "vdrive.VirtualDrive expected");
  return (VirtualDrive*)ud;
}


static VirtualFileHandle* filehandle_check(lua_State* L, int index) {
  void *ud = luaL_checkudata(L, index, VDRIVEFILEHANDLE_METATABLE);
  luaL_argcheck(L, ud != NULL, index, "vdrive.VirtualFileHandle expected");
  return (VirtualFileHandle*)ud;
}


static int vdrive_gc(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	drive->~VirtualDrive();
	return 0;
}


static int filehandle_gc(lua_State* L) {
    VirtualFileHandle* fileHandle = filehandle_check(L, 1);
	fileHandle->~VirtualFileHandle();
	return 0;
}


int vdrive_create(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* fileName = luaL_checkstring(L, 2);
	const char* defaultPassword = luaL_optstring(L, 3, NULL);
	int flags = (int)luaL_optnumber(L, 4, VirtualDrive::SUPPORT_MD5);
	lua_pushboolean(L, drive->Create(fileName, defaultPassword, flags) != 0);
	return 1;
}


int vdrive_open(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* fileName = luaL_checkstring(L, 2);
	int readOnly = 1;
	const char* defaultPassword = 0;
	int flags;
	if (lua_type(L, 3) == LUA_TBOOLEAN) {
		readOnly = lua_toboolean(L, 3);
	}
	defaultPassword = luaL_optstring(L, 4, NULL);
	flags = (int)luaL_optnumber(L, 5, VirtualDrive::SUPPORT_MD5);
	lua_pushboolean(L, drive->Open(fileName, readOnly != 0, defaultPassword, flags) != 0);
	return 1;
}


int vdrive_close(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	lua_pushboolean(L, drive->Close() != 0);
	return 1;
}


int vdrive_updatemd5s(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	drive->UpdateMD5s();
	return 0;
}


int vdrive_flush(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	lua_pushboolean(L, drive->Flush() != 0);
	return 1;
}


int vdrive_filecreate(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* fileName = luaL_checkstring(L, 2);
	UINT compressionLevel = (UINT)luaL_optnumber(L, 3, 8);
	time_t fileTime = (UINT)luaL_optnumber(L, 4, (lua_Number)time(NULL));

	VirtualFileHandle* fileHandle = (VirtualFileHandle*)lua_newuserdata(L, sizeof(VirtualFileHandle));
	::new(fileHandle) VirtualFileHandle;

	luaL_getmetatable(L, VDRIVEFILEHANDLE_METATABLE);
	lua_setmetatable(L, -2);

	if (!drive->FileCreate(fileName, *fileHandle, compressionLevel, &fileTime))
		return 0;

	return 1;
}


int vdrive_fileopen(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* fileName = NULL;
	int entryIndex = -1;
	if (lua_type(L, 2) == LUA_TSTRING)
		fileName = lua_tostring(L, 2);
	else if (lua_type(L, 2) == LUA_TNUMBER)
		entryIndex = lua_tointeger(L, 2);
	else
		luaL_typerror(L, 2, "string or integer");

	VirtualFileHandle* fileHandle = (VirtualFileHandle*)lua_newuserdata(L, sizeof(VirtualFileHandle));
	::new(fileHandle) VirtualFileHandle;

	luaL_getmetatable(L, VDRIVEFILEHANDLE_METATABLE);
	lua_setmetatable(L, -2);

	if (fileName) {
		if (!drive->FileOpen(fileName, *fileHandle))
			return 0;
	} else {
		if (!drive->FileOpenIndex((size_t)entryIndex, *fileHandle))
			return 0;
	}

	return 1;
}


int vdrive_fileclose(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushboolean(L, fileHandle->GetParentDrive()->FileClose(*fileHandle) != 0);
	return 1;
}


int vdrive_filecloseall(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	drive->FileCloseAll();
	return 0;
}


int vdrive_filegetfilename(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushstring(L, drive->FileGetFileName(*fileHandle));
	return 1;
}


int vdrive_filegetposition(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushnumber(L, (lua_Number)drive->FileGetPosition(*fileHandle));
	return 1;
}


int vdrive_filesetlength(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* fileHandle = filehandle_check(L, 2);
	long len = luaL_checklong(L, 3);
	drive->FileSetLength(*fileHandle, len);
	return 0;
}


int vdrive_filegetlength(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* fileHandle = filehandle_check(L, 2);
	lua_pushnumber(L, (lua_Number)drive->FileGetLength(*fileHandle));
	return 1;
}


int vdrive_fileread(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* fileHandle = filehandle_check(L, 2);
	long len = luaL_optlong(L, 3, 0);
	if (len == 0)
		len = (long)drive->FileGetLength(*fileHandle);

	char* buffer = new char[len];
	UINT nr = (UINT)drive->FileRead(*fileHandle, buffer, len);
	lua_pushlstring(L, buffer, nr);
	delete[] buffer;
	return 1;
}


int vdrive_filewrite(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* fileHandle = filehandle_check(L, 2);
	size_t len;
    const char* buffer = luaL_checklstring(L, 3, &len);
	len = luaL_optlong(L, 4, len);
	lua_pushnumber(L, (lua_Number)drive->FileWrite(*fileHandle, buffer, len));
	return 1;
}


int vdrive_fileerase(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* fileName = luaL_checkstring(L, 2);
	lua_pushboolean(L, drive->FileErase(fileName) != 0);
	return 1;
}


int vdrive_filerename(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* srcFileName = luaL_checkstring(L, 2);
	const char* destFileName = luaL_checkstring(L, 3);
	lua_pushboolean(L, drive->FileRename(srcFileName, destFileName) != 0);
	return 1;
}


int vdrive_filecopy(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	VirtualFileHandle* srcFileHandle = filehandle_check(L, 2);
	lua_pushboolean(L, drive->FileCopy(*srcFileHandle) != 0);
	return 1;
}


int vdrive_fileinsert(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* srcFileName = luaL_checkstring(L, 2);
	const char* destFileName = luaL_checkstring(L, 3);
    int compressionMethod = (UINT)luaL_optnumber(L, 4, VirtualDrive::COMPRESSED);

	DiskFile file;
    if (!file.Open(srcFileName, File::MODE_READONLY)) {
		lua_pushboolean(L, false);
		return 1;
	}

	time_t fileTime = (UINT)luaL_optnumber(L, 5, (lua_Number)file.GetLastWriteTime());

	lua_pushboolean(L, drive->FileCopy(file, destFileName, compressionMethod, &fileTime) != 0);
	return 1;
}


bool Copy(File& srcFile, const char* destFilename)
{
	// Operate in 16k buffers.
	const DWORD BUFFER_SIZE = 16 * 1024;

	DWORD fileSize = (DWORD)srcFile.GetLength();

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
		srcFile.Read(buffer, readSize);
		destFile.Write(buffer, readSize);
		fileSize -= readSize;
	}

	// Close the destination virtual file.
	destFile.Close();

	return true;
}


int vdrive_fileextract(lua_State* L) {
	VirtualDrive* drive = vdrive_check(L, 1);
	const char* srcFileName = luaL_checkstring(L, 2);
	const char* destFileName = luaL_checkstring(L, 3);
	VirtualDrive::FileEntry* entry = drive->FindFileEntry(srcFileName);
	if (!entry) {
		lua_pushboolean(L, false);
		return 1;
	}

	VirtualFile file;
    if (!file.Open(*drive, srcFileName))
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


static bool _vdrive_collectfilelist(lua_State* L, VirtualDrive::FileOrderList& fileOrder, int tableIndex)
{
	if (!lua_istable(L, tableIndex))
		return false;

    int index = 1;
	while (true)
	{
		VirtualDrive::FileOrderInfo info;

		lua_rawgeti(L, tableIndex, index++);						// 1-drive entryName
		if (lua_type(L, -1) == LUA_TTABLE)
		{
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

			lua_getfield(L, -1, "Compressed");
			if (lua_type(L, -1) == LUA_TBOOLEAN)
				info.compressionMethod = lua_toboolean(L, -1) ? VirtualDrive::COMPRESSED : VirtualDrive::UNCOMPRESSED;
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
		}
		else if (lua_type(L, -1) != LUA_TSTRING)
			break;
		else
		{
			info.entryName = lua_tostring(L, -1);

			lua_rawgeti(L, tableIndex, index++);						// 1-drive entryName srcPath
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


int vdrive_pack(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);
    VirtualDrive::FileOrderList fileOrder;

	VirtualDrive::PackOptions options;
	if (lua_istable(L, 2)) {
		lua_getfield(L, 2, "FileOrder");
		if (lua_type(L, -1) == LUA_TSTRING  ||  lua_type(L, -1) == LUA_TTABLE) {
			_vdrive_collectfilelist(L, fileOrder, lua_gettop(L));
			options.fileOrder = &fileOrder;
		}
		lua_pop(L, 1);
	}

	lua_pushboolean(L, drive->Pack(&options) != 0);
	return 1;
}


struct RetrieveChecksumInfo
{
	lua_State* L;
	int callbackRef;
};


static int _vdrive_retrievechecksum(const char* sourcePath, DWORD* crc, unsigned char* md5, void* userData)
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


static void _vdrive_statusupdate(VirtualDrive::FileListStatus status, const char* text, void* userData)
{
	StatusUpdateInfo* info = (StatusUpdateInfo*)userData;
	lua_State* L = info->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, info->callbackRef);
	lua_pushnumber(L, status);
	lua_pushstring(L, text);
	lua_pcall(L, 2, 0, 0);
}


int vdrive_processfilelist(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);

	VirtualDrive::ProcessFileListOptions options;

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
			options.retrieveChecksumCallback = _vdrive_retrievechecksum;

			checksumInfo.L = L;
			checksumInfo.callbackRef = luaL_ref(L, LUA_REGISTRYINDEX);
			options.retrieveChecksumUserData = &checksumInfo;
		}
		else
			lua_pop(L, 1);

		lua_getfield(L, 3, "StatusUpdate");
		if (lua_type(L, -1) == LUA_TFUNCTION)
		{
			options.statusUpdateCallback = _vdrive_statusupdate;

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

    VirtualDrive::FileOrderList fileOrder;
	if (_vdrive_collectfilelist(L, fileOrder, 2))
	{
		bool ret = drive->ProcessFileList(fileOrder, &options);
		luaL_unref(L, LUA_REGISTRYINDEX, checksumInfo.callbackRef);
		luaL_unref(L, LUA_REGISTRYINDEX, statusUpdateInfo.callbackRef);
		lua_pushboolean(L, ret);
		if (!ret)
		{
			lua_pushstring(L, drive->errorString);
			return 2;
		}
        return 1;
	}

    if (lua_isstring(L, 2))
    {
        bool ret = drive->ProcessFileList(lua_tostring(L, 2), &options);
		luaL_unref(L, LUA_REGISTRYINDEX, checksumInfo.callbackRef);
		luaL_unref(L, LUA_REGISTRYINDEX, statusUpdateInfo.callbackRef);
		lua_pushboolean(L, ret);
		if (!ret)
		{
			lua_pushstring(L, drive->errorString);
			return 2;
		}
	    return 1;
    }

    return 0;
}


int vdrive_getfilename(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);
	lua_pushstring(L, drive->GetFileName());
	return 1;
}


int vdrive_fileentrycount(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);
	lua_pushnumber(L, drive->GetFileEntryCount());
	return 1;
}


static void BuildFileEntry(lua_State* L, VirtualDrive::FileEntry* entry)
{
	lua_newtable(L);
	lua_pushstring(L, entry->GetFileName());
	lua_setfield(L, -2, "FileName");

	lua_pushnumber(L, (lua_Number)entry->GetTimeStamp());
	lua_setfield(L, -2, "TimeStamp");

	lua_pushnumber(L, (lua_Number)entry->GetCRC());
	lua_setfield(L, -2, "CRC");

	lua_pushlstring(L, (const char*)entry->GetMD5(), 16);
	lua_setfield(L, -2, "MD5");

	lua_pushnumber(L, (lua_Number)entry->GetOffset());
	lua_setfield(L, -2, "Offset");

	lua_pushnumber(L, (lua_Number)entry->GetUncompressedSize());
	lua_setfield(L, -2, "UncompressedSize");
	lua_pushnumber(L, (lua_Number)entry->GetCompressedSize());
	lua_setfield(L, -2, "CompressedSize");
	lua_pushnumber(L, (lua_Number)entry->GetCompressionMethod());
	lua_setfield(L, -2, "CompressionMethod");
}


int vdrive_fileentry(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);

	int entryIndex = (int)luaL_optlong(L, 2, -1);

	VirtualDrive::FileEntry* entry;

	entry = drive->GetFileEntry(entryIndex - 1);

	if (!entry)
		return 0;

	BuildFileEntry(L, entry);
	return 1;
}


int vdrive_findfileentry(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);

	const char* fileName = luaL_checkstring(L, 2);

	VirtualDrive::FileEntry* entry = drive->FindFileEntry(fileName);

	if (!entry)
		return 0;

	BuildFileEntry(L, entry);
	return 1;
}


int vdrive_fileentryindex(lua_State* L)
{
	VirtualDrive* drive = vdrive_check(L, 1);

	const char* fileName = luaL_checkstring(L, 2);
	size_t index = drive->FindFileEntryIndex(fileName);
	if (index == VirtualDrive::INVALID_FILE_ENTRY)
		return 0;
	lua_pushnumber(L, drive->FindFileEntryIndex(fileName) + 1);
	return 1;
}


int vdrive_new(lua_State* L)
{
	VirtualDrive* drive = (VirtualDrive*)lua_newuserdata(L, sizeof(VirtualDrive));
	::new(drive) VirtualDrive;

	luaL_getmetatable(L, VDRIVE_METATABLE);
	lua_setmetatable(L, -2);

	return 1;
}


int LS_Help(lua_State* L)
{
	char help[] =
"drive = vdrive.new() - Create virtual drive\n"
"time = vdrive.AdjustTime_t(timeToAdjust) - Adjusts a timestamp to zip format\n"
"\n"
"Drive commands:\n"
"\n"
"    opened = drive:create(fileName [, defaultPassword])\n"
"    opened = drive:open(fileName [, readOnly = true [, defaultPassword] ])\n"
"    drive:close()\n"
"    drive:updatemd5s()\n"
"    drive:flush(bool directoryAtBeginning)\n"
"\n"
"    vfile = drive:filecreate(fileName, compressionMethod:0,8{compressed}, time_t fileTime)\n"
"    vfile = drive:fileopen(fileName)\n"
"    drive:filecloseall()\n"
"    bool ret = drive:fileerase(fileName)\n"
"    bool ret = drive:filerename(oldFileName, newFileName)\n"
"    bool ret = drive:fileinsert(srcFileName, destFileName, compressionMethod:0,8{compressed}, time_t fileTime)\n"
"    bool ret = drive:fileextract(srcFileName, destFileName)\n"
"\n"
"    FileOrderTable = {\n"
"        string EntryName,\n"
"        string SourcePath,\n"
"        boolean Compressed,\n"
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
"    drive:pack(PackOptions)\n"
"\n"
"    ProcessFileListOptions = {\n"
"        boolean CheckOnly,\n"
"        boolean DirectoryAtBeginning,\n"
"        string FileCache,\n"
"        integer FileCacheSizeThreshold,\n"
"        crc, md5 = function RetrieveChecksum(sourcePath),\n"
"        function StatusUpdate(FileListStatus, text),\n"
"    }\n"
"    drive:processfilelist(fileNameTable|fileListFileName, ProcessFileListOptions)\n"
"\n"
"    string fileName = drive:getfilename()\n"
"    int entryCount = drive:fileentrycount()\n"
"    FileEntry fileEntry = drive:fileentry(index)\n"
"    FileEntry fileEntry = drive:findfileentry(fileName)\n"
"        FileEntry =\n"
"        {\n"
"            string FileName\n"
"            time_t TimeStamp\n"
"            DWORD CRC\n"
"            string MD5\n"
"            DWORD Offset\n"
"            DWORD Size\n"
"            DWORD UncompressedSize\n"
"            DWORD CompressedSize\n"
"            DWORD CompressionMethod\n"
"        }\n"
"    int entryIndex = drive:fileentryindex(fileName)\n"
"\n"
"File commands:\n"
"\n"
"    string fileName = drive:filegetfilename(vfile)\n"
"    long position = drive:filegetposition(vfile)\n"
"    drive:filesetlength(vfile, newLength)\n"
"    long len = drive:filegetlength(vfile)\n"
"    buffer = drive:fileread(vfile [, optionalSize])\n"
"    drive:filewrite(vfile, buffer [, optionalSize])\n"
"    drive:close(vfile)\n";
"\n"
"FileListStatus Enumerations:\n"
"\n"
"	 vdrive.UPDATING_ARCHIVE\n"
"    vdrive.UPDATING_ARCHIVE\n"
"    vdrive.DELETING_ENTRY\n"
"    vdrive.DIRECT_COPY_FROM_ANOTHER_ARCHIVE\n"
"    vdrive.COPYING_ENTRY_FROM_CACHE\n"
"    vdrive.COPYING_ENTRY_TO_CACHE\n"
"    vdrive.UPDATING_ENTRY\n"
"    vdrive.PACKING_ARCHIVE\n";

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
	lua_pushnumber(L, (lua_Number)VirtualDrive::AdjustTime_t(lua_tointeger(L, 1)));
	return 1;
}


static const struct luaL_reg vdrivefilehandle_funcs[] = {
	{ "__gc",				filehandle_gc },
	{ NULL, NULL },
};


static const struct luaL_reg vdrive_funcs[] = {
	{ "create",				vdrive_create },
	{ "open",				vdrive_open },
	{ "close",				vdrive_close },
	{ "updatemd5s",			vdrive_updatemd5s },
	{ "flush",				vdrive_flush },
	{ "filecreate",			vdrive_filecreate },
	{ "fileopen",			vdrive_fileopen },
	{ "fileclose",			vdrive_fileclose },
	{ "filecloseall",		vdrive_filecloseall },
	{ "_filecopy",			vdrive_filecopy },
	{ "filegetfilename",    vdrive_filegetfilename },
	{ "filegetposition",    vdrive_filegetposition },
	{ "filesetlength",      vdrive_filesetlength },
	{ "filegetlength",      vdrive_filegetlength },
	{ "fileread",           vdrive_fileread },
	{ "filewrite",          vdrive_filewrite },
	{ "fileerase",			vdrive_fileerase },
	{ "filerename",			vdrive_filerename },
	{ "fileinsert",			vdrive_fileinsert },
	{ "fileextract",		vdrive_fileextract },
	{ "pack",				vdrive_pack },
	{ "processfilelist",	vdrive_processfilelist },
	{ "getfilename",		vdrive_getfilename },
	{ "fileentrycount",		vdrive_fileentrycount },
	{ "fileentry",			vdrive_fileentry },
	{ "findfileentry",		vdrive_findfileentry },
	{ "fileentryindex",		vdrive_fileentryindex },

	/* Backward compatibility */
	{ "Create",				vdrive_create },
	{ "Open",				vdrive_open },
	{ "Close",				vdrive_close },
	{ "Flush",				vdrive_flush },
	{ "FileCreate",			vdrive_filecreate },
	{ "FileOpen",			vdrive_fileopen },
	{ "FileClose",			vdrive_fileclose },
	{ "FileCloseAll",		vdrive_filecloseall },
	{ "_FileCopy",			vdrive_filecopy },
	{ "FileGetFileName",    vdrive_filegetfilename },
	{ "FileGetPosition",    vdrive_filegetposition },
	{ "FileSetLength",      vdrive_filesetlength },
	{ "FileGetLength",      vdrive_filegetlength },
	{ "FileRead",           vdrive_fileread },
	{ "FileWrite",          vdrive_filewrite },
	{ "FileErase",			vdrive_fileerase },
	{ "FileRename",			vdrive_filerename },
	{ "FileInsert",			vdrive_fileinsert },
	{ "FileExtract",		vdrive_fileextract },
	{ "Pack",				vdrive_pack },
	{ "ProcessFileList",	vdrive_processfilelist },
	{ "GetFileName",		vdrive_getfilename },
	{ "GetFileEntryCount",	vdrive_fileentrycount },
	{ "GetFileEntry",		vdrive_fileentry },
	{ "FindFileEntry",		vdrive_findfileentry },
	{ "GetFileEntryIndex",	vdrive_fileentryindex },
	{ "__gc",				vdrive_gc },
	{ NULL, NULL },
};


static void vdrive_createmetatables(lua_State* L)
{
	luaL_newmetatable(L, VDRIVEFILEHANDLE_METATABLE);		// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);									// metatable __index metatable
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, vdrivefilehandle_funcs);
	lua_pop(L, 1);

	luaL_newmetatable(L, VDRIVE_METATABLE);					// metatable
	lua_pushliteral(L, "__index");							// metatable __index
	lua_pushvalue(L, -2);									// metatable __index metatable
	lua_rawset(L, -3);										// metatable
	luaL_register(L, NULL, vdrive_funcs);
	lua_pop(L, 1);
}


extern "C" DWORD VirtualDrive_GetFileCRC(FILE* file, UINT startOffset, unsigned char md5[16]);

static int LS_filecrcmd5(lua_State* L)
{
	const char* fileName = luaL_checkstring(L, 1);
	FILE* file = fopen(fileName, "rb");
	unsigned char md5[16];
	DWORD crc = VirtualDrive_GetFileCRC(file, 0, md5);
	fclose(file);
	lua_pushnumber(L, crc);
	lua_pushlstring(L, (const char*)md5, 16);
	return 2;
}


static const struct luaL_reg vdrivelib[] = {
	{ "new",				vdrive_new },
	{ "help",				LS_Help },
	{ "crc32",				LS_crc32 },
	{ "filecrcmd5",			LS_filecrcmd5 },

	/* Backward compatibility */
	{ "VirtualDrive",		vdrive_new },
	{ "AdjustTime_t",		LS_AdjustTime_t },
	{NULL, NULL},
};


extern "C" LUAMODULE_API int luaopen_vdrive(lua_State* L)
{
	luaL_register(L, "vdrive", vdrivelib);

	lua_pushnumber(L, VirtualDrive::UPDATING_ARCHIVE);
	lua_setfield(L, -2, "UPDATING_ARCHIVE");
	lua_pushnumber(L, VirtualDrive::DELETING_ENTRY);
	lua_setfield(L, -2, "DELETING_ENTRY");
	lua_pushnumber(L, VirtualDrive::DIRECT_COPY_FROM_ANOTHER_ARCHIVE);
	lua_setfield(L, -2, "DIRECT_COPY_FROM_ANOTHER_ARCHIVE");
	lua_pushnumber(L, VirtualDrive::COPYING_ENTRY_FROM_CACHE);
	lua_setfield(L, -2, "COPYING_ENTRY_FROM_CACHE");
	lua_pushnumber(L, VirtualDrive::COPYING_ENTRY_TO_CACHE);
	lua_setfield(L, -2, "COPYING_ENTRY_TO_CACHE");
	lua_pushnumber(L, VirtualDrive::UPDATING_ENTRY);
	lua_setfield(L, -2, "UPDATING_ENTRY");
	lua_pushnumber(L, VirtualDrive::PACKING_ARCHIVE);
	lua_setfield(L, -2, "PACKING_ARCHIVE");

	lua_pushnumber(L, VirtualDrive::SUPPORT_MD5);
	lua_setfield(L, -2, "SUPPORT_MD5");
	lua_pushnumber(L, VirtualDrive::EXTRA_DIRECTORY_AT_BEGINNING);
	lua_setfield(L, -2, "EXTRA_DIRECTORY_AT_BEGINNING");

	vdrive_createmetatables(L);
	return 1;
}


