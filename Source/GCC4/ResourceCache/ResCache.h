#pragma once
//========================================================================
// ResCache.h : Defines a simple resource cache.
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================



class ResHandle;
class ResCache;

#include "ZipFile.h"			// needed for ZipContentsMap

//
// class IResourceExtraData		- Chapter 8, page 224 (see notes below)
//
//   This isn't mentioned specifically on that page, but it is a class that can attach extra data to 
//   a particular resource. Best example is storing the length and format of a sound file. 
//   There's a great discussion of this in Chapter 13, "Game Audio"
//
class IResourceExtraData
{
public:
	virtual std::string VToString()=0;
};

//
//  class Resource			- Chapter 8, page 220
//
class Resource
{
public:
	std::string m_name;
	Resource(const std::string &name); 
};

//
//  class ResourceZipFile	- not discussed in the book
//
//    This class implements the IResourceFile interface with a ZipFile.
class ResourceZipFile : public IResourceFile
{
	ZipFile *m_pZipFile;
	std::wstring m_resFileName;

public:
	ResourceZipFile(const std::wstring resFileName ) { m_pZipFile = NULL; m_resFileName=resFileName; }
	virtual ~ResourceZipFile();

	virtual bool VOpen();
	virtual int VGetRawResourceSize(const Resource &r);
	virtual int VGetRawResource(const Resource &r, char *buffer);
	virtual int VGetNumResources() const;
	virtual std::string VGetResourceName(int num) const;
    virtual bool VIsUsingDevelopmentDirectories(void) const { return false; }
};

//
// class DevelopmentResourceZipFile							- not discussed in the book
//
//    This class fakes a ZIP file from a normal directory, and is used in the 
//    editor.
//
class DevelopmentResourceZipFile : public ResourceZipFile
{
public:
	enum Mode
	{
		Development,	// this mode checks the original asset directory for changes - helps during development
		Editor			// this mode only checks the original asset directory - the ZIP file is left unopened.
	};

	Mode m_mode;
	std::wstring m_AssetsDir;
	std::vector<WIN32_FIND_DATA> m_AssetFileInfo;
	ZipContentsMap m_DirectoryContentsMap;

	DevelopmentResourceZipFile(const std::wstring resFileName, const Mode mode);

	virtual bool VOpen();
	virtual int VGetRawResourceSize(const Resource &r);
	virtual int VGetRawResource(const Resource &r, char *buffer);
	virtual int VGetNumResources() const;
	virtual std::string VGetResourceName(int num) const;
    virtual bool VIsUsingDevelopmentDirectories(void) const { return true; }

	int Find(const std::string &path);

protected:
	void ReadAssetsDirectory(std::wstring fileSpec);
};

//
//  class ResHandle			- Chapter 8, page 222
//
class ResHandle
{
	friend class ResCache;

protected:
	Resource m_resource;
	char *m_buffer;	
	unsigned int m_size;
	shared_ptr<IResourceExtraData> m_extra;
	ResCache *m_pResCache;

public:
	ResHandle(Resource & resource, char *buffer, unsigned int size, ResCache *pResCache);

	virtual ~ResHandle();

	const std::string GetName() { return m_resource.m_name; }
	unsigned int Size() const { return m_size; } 
	char *Buffer() const { return m_buffer; }
	char *WritableBuffer() { return m_buffer; }

	shared_ptr<IResourceExtraData> GetExtra() { return m_extra; }
	void SetExtra(shared_ptr<IResourceExtraData> extra) { m_extra = extra; }
};

//
// class DefaultResourceLoader							- Chapter 8, page 225
//
class DefaultResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return true; }
	virtual bool VDiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize) { return rawSize; }
	virtual bool VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle) { return true; }
	virtual std::string VGetPattern() { return "*"; }

};

//
//  class ResCache										- Chapter 8, page 225
//
typedef std::list< shared_ptr <ResHandle > > ResHandleList;					// lru list
typedef std::map<std::string, shared_ptr < ResHandle  > > ResHandleMap;		// maps indentifiers to resource data
typedef std::list< shared_ptr < IResourceLoader > > ResourceLoaders;

class ResCache
{
	friend class ResHandle;

	ResHandleList m_lru;							// lru list
	ResHandleMap m_resources;
	ResourceLoaders m_resourceLoaders;

	IResourceFile *m_file;

	unsigned int			m_cacheSize;			// total memory size
	unsigned int			m_allocated;			// total memory allocated

protected:

	bool MakeRoom(unsigned int size);
	char *Allocate(unsigned int size);
	void Free(shared_ptr<ResHandle> gonner);

	shared_ptr<ResHandle> Load(Resource * r);
	shared_ptr<ResHandle> Find(Resource * r);
	void Update(shared_ptr<ResHandle> handle);

	void FreeOneResource();
	void MemoryHasBeenFreed(unsigned int size);

public:
	ResCache(const unsigned int sizeInMb, IResourceFile *file);
	virtual ~ResCache();

	bool Init(); 
	
	void RegisterLoader( shared_ptr<IResourceLoader> loader );

	shared_ptr<ResHandle> GetHandle(Resource * r);

	int Preload(const std::string pattern, void (*progressCallback)(int, bool &));
	std::vector<std::string> Match(const std::string pattern);

	void Flush(void);

    bool IsUsingDevelopmentDirectories(void) const { GCC_ASSERT(m_file); return m_file->VIsUsingDevelopmentDirectories(); }

};



