//========================================================================
// ResCache.cpp : Defines a simple resource cache
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

#include "GameCodeStd.h"
#include <list>
#include <map>
#include <cctype>			// for std::tolower

#include "ResCache.h"

#include "../Utilities/String.h"

//
//  Resource::Resource
//
Resource::Resource(const std::string &name) 
{ 
	m_name=name;
	std::transform(m_name.begin(), m_name.end(), m_name.begin(), (int(*)(int)) std::tolower);
}


//========================================================================
// class ResourceZipFile					- not discussed in the book
//
//    This class implements the IResourceFile interface with a ZipFile.
//
ResourceZipFile::~ResourceZipFile() 
{ 
	SAFE_DELETE(m_pZipFile); 
}


bool ResourceZipFile::VOpen()
{
	m_pZipFile = GCC_NEW ZipFile;
    if (m_pZipFile)
    {
		return m_pZipFile->Init(m_resFileName.c_str());
	}
	return false;	
}



int ResourceZipFile::VGetRawResourceSize(const Resource &r)
{
	int resourceNum = m_pZipFile->Find(r.m_name.c_str());
	if (resourceNum == -1)
		return -1;

	return m_pZipFile->GetFileLen(resourceNum);
}



int ResourceZipFile::VGetRawResource(const Resource &r, char *buffer)
{
	int size = 0;
	optional<int> resourceNum = m_pZipFile->Find(r.m_name.c_str());
	if (resourceNum.valid())
	{
		size = m_pZipFile->GetFileLen(*resourceNum);
		m_pZipFile->ReadFile(*resourceNum, buffer);
	}
	return size;	
}



int ResourceZipFile::VGetNumResources() const 
{ 
	return (m_pZipFile==NULL) ? 0 : m_pZipFile->GetNumFiles(); 
}




std::string ResourceZipFile::VGetResourceName(int num) const 
{ 
	std::string resName = "";
	if (m_pZipFile!=NULL && num>=0 && num<m_pZipFile->GetNumFiles())
	{
		resName = m_pZipFile->GetFilename(num); 
	}
	return resName;
}


//========================================================================
// class DevelopmentResourceZipFile					- not discussed in the book
//
//    This class fakes a ZIP file from a normal directory, and is used in the 
//    editor.
//

DevelopmentResourceZipFile::DevelopmentResourceZipFile(const std::wstring resFileName, const Mode mode)
	: ResourceZipFile(resFileName) 
{ 
	m_mode=mode; 

	TCHAR dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, dir);

	m_AssetsDir = dir;
	int lastSlash = m_AssetsDir.find_last_of(L"\\");
	m_AssetsDir = m_AssetsDir.substr(0, lastSlash);
	m_AssetsDir +=  L"\\Assets\\";
}

int DevelopmentResourceZipFile::Find(const std::string &name)
{
	std::string lowerCase = name;
	std::transform(lowerCase.begin(), lowerCase.end(), lowerCase.begin(), (int(*)(int)) std::tolower);
	ZipContentsMap::const_iterator i = m_DirectoryContentsMap.find(lowerCase);
	if (i==m_DirectoryContentsMap.end())
		return -1;

	return i->second;
}



bool DevelopmentResourceZipFile::VOpen()
{
	if (m_mode != Editor)
	{
		ResourceZipFile::VOpen();
	}

	// open the asset directory and read in the non-hidden contents
	if (m_mode == Editor)
	{
		ReadAssetsDirectory(L"*");
	}
	else
	{
		// FUTURE WORK - iterate through the ZipFile contents and go grab WIN32_FIND_DATA 
		//   elements for each one. Then it would be possible to compare the dates/times of the
		//   asset in the Zip file with the source asset.
		GCC_ASSERT(0 && "Not implemented yet");
	}

	return true;
}


int DevelopmentResourceZipFile::VGetRawResourceSize(const Resource &r)
{
	int size = 0;

	if (m_mode == Editor)
	{
		int num = Find(r.m_name.c_str());
		if (num == -1)
			return -1;

		return m_AssetFileInfo[num].nFileSizeLow;
	}

	return ResourceZipFile::VGetRawResourceSize(r);
}

int DevelopmentResourceZipFile::VGetRawResource(const Resource &r, char *buffer)
{
	if (m_mode == Editor)
	{
		int num = Find(r.m_name.c_str());
		if (num == -1)
			return -1;

		std::string fullFileSpec = ws2s(m_AssetsDir) + r.m_name.c_str(); 
		FILE *f = fopen(fullFileSpec.c_str(), "rb");
		size_t bytes = fread(buffer, 1, m_AssetFileInfo[num].nFileSizeLow, f);
		fclose(f);
		return bytes;
	}

	return ResourceZipFile::VGetRawResource(r, buffer);
}

int DevelopmentResourceZipFile::VGetNumResources() const 
{
	return (m_mode == Editor) ? m_AssetFileInfo.size() : ResourceZipFile::VGetNumResources();
}

std::string DevelopmentResourceZipFile::VGetResourceName(int num) const 
{
	if (m_mode == Editor)
	{
		std::wstring wideName = m_AssetFileInfo[num].cFileName;
		return ws2s(wideName);
	}

	return ResourceZipFile::VGetResourceName(num);
}


void DevelopmentResourceZipFile::ReadAssetsDirectory(std::wstring fileSpec)
{
      HANDLE fileHandle;
      WIN32_FIND_DATA findData;
 
      // get first file
	  std::wstring pathSpec = m_AssetsDir + fileSpec;
      fileHandle = FindFirstFile( pathSpec.c_str(), &findData );
      if( fileHandle != INVALID_HANDLE_VALUE )
      {
           // loop on all remeaining entries in dir
            while( FindNextFile( fileHandle,&findData ) )
            {
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
					continue;

				std::wstring fileName = findData.cFileName; 
				if( findData.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY )
				{
					if (fileName != L".." && fileName != L".")
					{
						fileName = fileSpec.substr(0, fileSpec.length()-1) + fileName + L"\\*";
						ReadAssetsDirectory(fileName);
					}
				}
				else
				{
					fileName = fileSpec.substr(0, fileSpec.length()-1) + fileName;
					std::wstring lower = fileName;
					std::transform(lower.begin(), lower.end(), lower.begin(), (int(*)(int)) std::tolower);
					wcscpy_s(&findData.cFileName[0], MAX_PATH, lower.c_str());
					m_DirectoryContentsMap[ws2s(lower)] = m_AssetFileInfo.size();
					m_AssetFileInfo.push_back(findData);
				} 
            }
      } 

      FindClose( fileHandle );
}





//
// ResHandle::ResHandle							- Chapter 8, page 223
//
ResHandle::ResHandle(Resource & resource, char *buffer, unsigned int size, ResCache *pResCache)
: m_resource(resource)
{
	m_buffer = buffer;
	m_size = size;
	m_extra = NULL;
	m_pResCache = pResCache;
}

//
// ResHandle::ResHandle							- Chapter 8, page 223
//
ResHandle::~ResHandle()
{
	SAFE_DELETE_ARRAY(m_buffer);
	m_pResCache->MemoryHasBeenFreed(m_size);
}


//
// ResCache::ResCache							- Chapter 8, page 227
//
ResCache::ResCache(const unsigned int sizeInMb, IResourceFile *resFile )
{
	m_cacheSize = sizeInMb * 1024 * 1024;				// total memory size
	m_allocated = 0;									// total memory allocated
	m_file = resFile;
}

//
// ResCache::~ResCache							- Chapter 8, page 227
//
ResCache::~ResCache()
{
	while (!m_lru.empty())
	{
		FreeOneResource();
	}
	SAFE_DELETE(m_file);
}

//
// ResCache::Init								- Chapter 8, page 227
//
bool ResCache::Init()
{ 
	bool retValue = false;
	if ( m_file->VOpen() )
	{
		RegisterLoader(shared_ptr<IResourceLoader>(GCC_NEW DefaultResourceLoader()));
		retValue = true;
	}
	return retValue;
}

//
// ResCache::RegisterLoader						- Chapter 8, page 225
// 
//    The loaders are discussed on the page refereced above - this method simply adds the loader
//    to the resource cache.
//
void ResCache::RegisterLoader(shared_ptr<IResourceLoader> loader )
{
	m_resourceLoaders.push_front(loader);
}


//
// ResCache::GetHandle							- Chapter 8, page 227
//
shared_ptr<ResHandle> ResCache::GetHandle(Resource * r)
{
	shared_ptr<ResHandle> handle(Find(r));
	if (handle==NULL)
	{
		handle = Load(r);
		GCC_ASSERT(handle);
	}
	else
	{
		Update(handle);
	}
	return handle;
}

//
// ResCache::Load								- Chapter 8, page 228-230
//
shared_ptr<ResHandle> ResCache::Load(Resource *r)
{
	// Create a new resource and add it to the lru list and map

	shared_ptr<IResourceLoader> loader;
	shared_ptr<ResHandle> handle;

	for (ResourceLoaders::iterator it = m_resourceLoaders.begin(); it != m_resourceLoaders.end(); ++it)
	{
		shared_ptr<IResourceLoader> testLoader = *it;

		if (WildcardMatch(testLoader->VGetPattern().c_str(), r->m_name.c_str()))
		{
			loader = testLoader;
			break;
		}
	}

	if (!loader)
	{
		GCC_ASSERT(loader && _T("Default resource loader not found!"));
		return handle;		// Resource not loaded!
	}

	int rawSize = m_file->VGetRawResourceSize(*r);
	if (rawSize < 0)
	{
		GCC_ASSERT(rawSize > 0 && "Resource size returned -1 - Resource not found");
		return shared_ptr<ResHandle>();
	}

    int allocSize = rawSize + ((loader->VAddNullZero()) ? (1) : (0));
	char *rawBuffer = loader->VUseRawFile() ? Allocate(allocSize) : GCC_NEW char[allocSize];
    memset(rawBuffer, 0, allocSize);

	if (rawBuffer==NULL || m_file->VGetRawResource(*r, rawBuffer)==0)
	{
		// resource cache out of memory
		return shared_ptr<ResHandle>();
	}
	
	char *buffer = NULL;
	unsigned int size = 0;

	if (loader->VUseRawFile())
	{
		buffer = rawBuffer;
		handle = shared_ptr<ResHandle>(GCC_NEW ResHandle(*r, buffer, rawSize, this));
	}
	else
	{
		size = loader->VGetLoadedResourceSize(rawBuffer, rawSize);
        buffer = Allocate(size);
		if (rawBuffer==NULL || buffer==NULL)
		{
			// resource cache out of memory
			return shared_ptr<ResHandle>();
		}
		handle = shared_ptr<ResHandle>(GCC_NEW ResHandle(*r, buffer, size, this));
		bool success = loader->VLoadResource(rawBuffer, rawSize, handle);
		
		// [mrmike] - This was added after the chapter went to copy edit. It is used for those
		//            resoruces that are converted to a useable format upon load, such as a compressed
		//            file. If the raw buffer from the resource file isn't needed, it shouldn't take up
		//            any additional memory, so we release it.
		//
		if (loader->VDiscardRawBufferAfterLoad())
		{
			SAFE_DELETE_ARRAY(rawBuffer);
		}

		if (!success)
		{
			// resource cache out of memory
			return shared_ptr<ResHandle>();
		}
	}

	if (handle)
	{
		m_lru.push_front(handle);
		m_resources[r->m_name] = handle;
	}

	GCC_ASSERT(loader && _T("Default resource loader not found!"));
	return handle;		// ResCache is out of memory!
}

//
// ResCache::Find									- Chapter 8, page 228
//
shared_ptr<ResHandle> ResCache::Find(Resource * r)
{
	ResHandleMap::iterator i = m_resources.find(r->m_name);
	if (i==m_resources.end())
		return shared_ptr<ResHandle>();

	return i->second;
}

//
// ResCache::Update									- Chapter 8, page 228
//
void ResCache::Update(shared_ptr<ResHandle> handle)
{
	m_lru.remove(handle);
	m_lru.push_front(handle);
}



//
// ResCache::Allocate								- Chapter 8, page 230
//
char *ResCache::Allocate(unsigned int size)
{
	if (!MakeRoom(size))
		return NULL;

	char *mem = GCC_NEW char[size];
	if (mem)
	{
		m_allocated += size;
	}

	return mem;
}


//
// ResCache::FreeOneResource						- Chapter 8, page 231
//
void ResCache::FreeOneResource()
{
	ResHandleList::iterator gonner = m_lru.end();
	gonner--;

	shared_ptr<ResHandle> handle = *gonner;

	m_lru.pop_back();							
	m_resources.erase(handle->m_resource.m_name);
	// Note - you can't change the resource cache size yet - the resource bits could still actually be
	// used by some sybsystem holding onto the ResHandle. Only when it goes out of scope can the memory
	// be actually free again.
}



//
// ResCache::Flush									- not described in the book
//
//    Frees every handle in the cache - this would be good to call if you are loading a new
//    level, or if you wanted to force a refresh of all the data in the cache - which might be 
//    good in a development environment.
//
void ResCache::Flush()
{
	while (!m_lru.empty())
	{
		shared_ptr<ResHandle> handle = *(m_lru.begin());
		Free(handle);
		m_lru.pop_front();
	}
}


//
// ResCache::MakeRoom									- Chapter 8, page 231
//
bool ResCache::MakeRoom(unsigned int size)
{
	if (size > m_cacheSize)
	{
		return false;
	}

	// return null if there's no possible way to allocate the memory
	while (size > (m_cacheSize - m_allocated))
	{
		// The cache is empty, and there's still not enough room.
		if (m_lru.empty())
			return false;

		FreeOneResource();
	}

	return true;
}

//
//	ResCache::Free									- Chapter 8, page 228
//
void ResCache::Free(shared_ptr<ResHandle> gonner)
{
	m_lru.remove(gonner);
	m_resources.erase(gonner->m_resource.m_name);
	// Note - the resource might still be in use by something,
	// so the cache can't actually count the memory freed until the
	// ResHandle pointing to it is destroyed.

	//m_allocated -= gonner->m_resource.m_size;
	//delete gonner;
}

//
//  ResCache::MemoryHasBeenFreed					- not described in the book
//
//     This is called whenever the memory associated with a resource is actually freed
//
void ResCache::MemoryHasBeenFreed(unsigned int size)
{
	m_allocated -= size;
}

//
// ResCache::Match									- not described in the book
//
//   Searches the resource cache assets for files matching the pattern. Useful for providing a 
//   a list of levels for a main menu screen, for example.
//
std::vector<std::string> ResCache::Match(const std::string pattern)
{
	std::vector<std::string> matchingNames;
	if (m_file==NULL)
		return matchingNames;

	int numFiles = m_file->VGetNumResources();
	for (int i=0; i<numFiles; ++i)
	{
		std::string name = m_file->VGetResourceName(i);
		std::transform(name.begin(), name.end(), name.begin(), (int(*)(int)) std::tolower);
		if (WildcardMatch(pattern.c_str(), name.c_str()))
		{
			matchingNames.push_back(name);
		}
	}
	return matchingNames;
}


//
// ResCache::Preload								- Chapter 8, page 236
//
int ResCache::Preload(const std::string pattern, void (*progressCallback)(int, bool &))
{
	if (m_file==NULL)
		return 0;

	int numFiles = m_file->VGetNumResources();
	int loaded = 0;
	bool cancel = false;
	for (int i=0; i<numFiles; ++i)
	{
		Resource resource(m_file->VGetResourceName(i));

		if (WildcardMatch(pattern.c_str(), resource.m_name.c_str()))
		{
			shared_ptr<ResHandle> handle = g_pApp->m_ResCache->GetHandle(&resource);
			++loaded;
		}

		if (progressCallback != NULL)
		{
			progressCallback(i * 100/numFiles, cancel);
		}
	}
	return loaded;
}
