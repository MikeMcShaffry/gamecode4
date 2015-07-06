/**
	\file fileglob.h
**/
#ifndef __FILEGLOB_H__
#define __FILEGLOB_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32)
	typedef unsigned __int64 fileglob_uint64;
#else
	typedef unsigned long long fileglob_uint64;
#endif

#if !defined(FILEGLOB_BUILD_IMPLEMENTATION)
typedef struct _fileglob fileglob;
typedef void* (*fileglob_Alloc)(void* userData, void* ptr, unsigned int size);
#endif

fileglob* fileglob_Create(const char* inPattern);
fileglob* fileglob_CreateWithAlloc(const char* inPattern, fileglob_Alloc alloc, void* userData);
void fileglob_Destroy(fileglob* self);
void fileglob_AddExclusivePattern(fileglob* self, const char* name);
void fileglob_AddIgnorePattern(fileglob* self, const char* name);

int fileglob_Next(fileglob* self);

const char* fileglob_FileName(fileglob* self);
fileglob_uint64 fileglob_CreationTime(fileglob* self);
fileglob_uint64 fileglob_AccessTime(fileglob* self);
fileglob_uint64 fileglob_WriteTime(fileglob* self);
fileglob_uint64 fileglob_CreationFILETIME(fileglob* self);
fileglob_uint64 fileglob_AccessFILETIME(fileglob* self);
fileglob_uint64 fileglob_WriteFILETIME(fileglob* self);
fileglob_uint64 fileglob_FileSize(fileglob* self);
int fileglob_IsDirectory(fileglob* self);
int fileglob_IsReadOnly(fileglob* self);

int fileglob_WildMatch(const char* pattern, const char *string, int caseSensitive);

#if defined(WIN32)  &&  defined(FILEGLOB_NEED_FILETIME_TO_TIME_T_CONVERSION)
time_t fileglob_ConvertToTime_t(const FILETIME* fileTime);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __FILEGLOB_H__ */
