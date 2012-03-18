/////////////////////////////////////////////////////////////////////////////
// File SoundResource.cpp
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

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include    <vorbis/codec.h>            // from the vorbis sdk
#include    <vorbis/vorbisfile.h>       // also :)

#include "../GameCode4/GameCode.h"

#include "SoundResource.h"
#include "Audio.h"

	
//
// SoundResource::SoundResource			- Chapter X, page 362
//


SoundResourceExtraData::SoundResourceExtraData()
:	m_SoundType(SOUND_TYPE_UNKNOWN),
	m_bInitialized(false),
	m_LengthMilli(0)
{	
	// don't do anything yet - timing sound Initialization is important!
}

unsigned int WaveResourceLoader::VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize)
{
	DWORD		file = 0; 
	DWORD		fileEnd = 0; 
	
	DWORD		length = 0;     
	DWORD		type = 0;									

	DWORD pos = 0;

	// mmioFOURCC -- converts four chars into a 4 byte integer code.
	// The first 4 bytes of a valid .wav file is 'R','I','F','F'

	type = *((DWORD *)(rawBuffer+pos));		pos+=sizeof(DWORD);
	if(type != mmioFOURCC('R', 'I', 'F', 'F'))
		return false;	
	
	length = *((DWORD *)(rawBuffer+pos));	pos+=sizeof(DWORD);
	type = *((DWORD *)(rawBuffer+pos));		pos+=sizeof(DWORD);

	// 'W','A','V','E' for a legal .wav file
	if(type != mmioFOURCC('W', 'A', 'V', 'E'))
		return false;		//not a WAV

	// Find the end of the file
	fileEnd = length - 4;
	
	bool copiedBuffer = false;

	// Load the .wav format and the .wav data
	// Note that these blocks can be in either order.
	while(file < fileEnd)
	{
		type = *((DWORD *)(rawBuffer+pos));		pos+=sizeof(DWORD);
		file += sizeof(DWORD);

		length = *((DWORD *)(rawBuffer+pos));	pos+=sizeof(DWORD);
		file += sizeof(DWORD);

		switch(type)
		{
			case mmioFOURCC('f', 'a', 'c', 't'):
			{
				GCC_ASSERT(false && "This wav file is compressed.  We don't handle compressed wav at this time");
				break;
			}

			case mmioFOURCC('f', 'm', 't', ' '):
			{
				pos+=length;   
				break;
			}

			case mmioFOURCC('d', 'a', 't', 'a'):
			{
				return length;
			}
		} 

		file += length;

		// Increment the pointer past the block we just read,
		// and make sure the pointer is word aliged.
		if (length & 1)
		{
			++pos;
			++file;
		}
	} 

	// If we get to here, the .wav file didn't contain all the right pieces.
	return false; 
}

bool WaveResourceLoader::VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle)
{
	shared_ptr<SoundResourceExtraData> extra = shared_ptr<SoundResourceExtraData>(GCC_NEW SoundResourceExtraData());
	extra->m_SoundType = SOUND_TYPE_WAVE;
	handle->SetExtra(shared_ptr<SoundResourceExtraData>(extra));
	if (!ParseWave(rawBuffer, rawSize, handle))
	{
		return false;
	}
	return true;
}

//
// WaveResourceLoader::ParseWave				- Chapter 13, page 401
//
bool WaveResourceLoader::ParseWave(char *wavStream, size_t bufferLength, shared_ptr<ResHandle> handle)
{
	shared_ptr<SoundResourceExtraData> extra = static_pointer_cast<SoundResourceExtraData>(handle->GetExtra());
	DWORD		file = 0; 
	DWORD		fileEnd = 0; 
	
	DWORD		length = 0;     
	DWORD		type = 0;									

	DWORD pos = 0;

	// mmioFOURCC -- converts four chars into a 4 byte integer code.
	// The first 4 bytes of a valid .wav file is 'R','I','F','F'

	type = *((DWORD *)(wavStream+pos));		pos+=sizeof(DWORD);
	if(type != mmioFOURCC('R', 'I', 'F', 'F'))
		return false;	
	
	length = *((DWORD *)(wavStream+pos));	pos+=sizeof(DWORD);
	type = *((DWORD *)(wavStream+pos));		pos+=sizeof(DWORD);

	// 'W','A','V','E' for a legal .wav file
	if(type != mmioFOURCC('W', 'A', 'V', 'E'))
		return false;		//not a WAV

	// Find the end of the file
	fileEnd = length - 4;
	
	memset(&extra->m_WavFormatEx, 0, sizeof(WAVEFORMATEX));

	bool copiedBuffer = false;

	// Load the .wav format and the .wav data
	// Note that these blocks can be in either order.
	while(file < fileEnd)
	{
		type = *((DWORD *)(wavStream+pos));		pos+=sizeof(DWORD);
		file += sizeof(DWORD);

		length = *((DWORD *)(wavStream+pos));	pos+=sizeof(DWORD);
		file += sizeof(DWORD);

		switch(type)
		{
			case mmioFOURCC('f', 'a', 'c', 't'):
			{
				GCC_ASSERT(false && "This wav file is compressed.  We don't handle compressed wav at this time");
				break;
			}

			case mmioFOURCC('f', 'm', 't', ' '):
			{
				memcpy(&extra->m_WavFormatEx, wavStream+pos, length);		pos+=length;   
				extra->m_WavFormatEx.cbSize = (WORD)length;
				break;
			}

			case mmioFOURCC('d', 'a', 't', 'a'):
			{
				copiedBuffer = true;
				if (length != handle->Size())
				{
					GCC_ASSERT(0 && _T("Wav resource size does not equal the buffer size"));
					return 0;
				}
				memcpy(handle->WritableBuffer(), wavStream+pos, length);			pos+=length;
				break;
			}
		} 

		file += length;

		// If both blocks have been seen, we can return true.
		if( copiedBuffer )
		{
			extra->m_LengthMilli = ( handle->Size() * 1000 ) / extra->GetFormat()->nAvgBytesPerSec;
			return true;
		}

		// Increment the pointer past the block we just read,
		// and make sure the pointer is word aliged.
		if (length & 1)
		{
			++pos;
			++file;
		}
	} 

	// If we get to here, the .wav file didn't contain all the right pieces.
	return false; 
}


//
// struct OggMemoryFile							- Chapter 13, page 403
//
struct OggMemoryFile
{
    unsigned char*  dataPtr;// Pointer to the data in memory
    size_t    dataSize;     // Size of the data
    size_t    dataRead;     // Bytes read so far

    OggMemoryFile(void)
    {
        dataPtr = NULL;
        dataSize = 0;
        dataRead = 0;
    }
}; 

//
// VorbisRead									- Chapter 13, page 404
//
size_t VorbisRead(void* data_ptr, size_t byteSize, size_t sizeToRead, void* data_src)                               
{
   OggMemoryFile *pVorbisData = static_cast<OggMemoryFile *>(data_src);
   if (NULL == pVorbisData) 
   {
	   return -1;
   }

   size_t actualSizeToRead, spaceToEOF = 
       pVorbisData->dataSize - pVorbisData->dataRead;
   if ((sizeToRead*byteSize) < spaceToEOF)
   {
       actualSizeToRead = (sizeToRead*byteSize);
   }
   else
   {
       actualSizeToRead = spaceToEOF;  
   }
  
   if (actualSizeToRead)
   {
     memcpy(data_ptr, 
		 (char*)pVorbisData->dataPtr + pVorbisData->dataRead, actualSizeToRead);
     pVorbisData->dataRead += actualSizeToRead;
   }

   return actualSizeToRead;
}

//
// VorbisSeek									- Chapter 13, page 404
//
int VorbisSeek(void* data_src, ogg_int64_t offset, int origin)            
{
	OggMemoryFile *pVorbisData = static_cast<OggMemoryFile *>(data_src);
	if (NULL == pVorbisData) 
	{
		return -1;
	}

	switch (origin)
	{
		case SEEK_SET: 
		{ 
		  ogg_int64_t actualOffset; 
		  actualOffset = (pVorbisData->dataSize >= offset) ? offset : pVorbisData->dataSize;
		  pVorbisData->dataRead = static_cast<size_t>(actualOffset);
		  break;
		}

		case SEEK_CUR: 
		{
		  size_t spaceToEOF = 
			 pVorbisData->dataSize - pVorbisData->dataRead;

		  ogg_int64_t actualOffset; 
		  actualOffset = (offset < spaceToEOF) ? offset : spaceToEOF;  

		  pVorbisData->dataRead += static_cast<LONG>(actualOffset);
		  break;
		}

    case SEEK_END: 
		pVorbisData->dataRead = pVorbisData->dataSize+1;
		break;

    default:
      GCC_ASSERT(false && "Bad parameter for 'origin', requires same as fseek.");
      break;
  };

  return 0;
}

//
// VorbisClose									- Chapter 13, page 405
//
int VorbisClose(void *src)
{
	// Do nothing - we assume someone else is managing the raw buffer
    return 0;
}

//
// VorbisTell									- Chapter 13, page 405
//
long VorbisTell(void *data_src) 
{
	OggMemoryFile *pVorbisData = static_cast<OggMemoryFile *>(data_src);
	if (NULL == pVorbisData) 
	{
		return -1L;
	}

	return static_cast<long>(pVorbisData->dataRead);
}

shared_ptr<IResourceLoader> CreateWAVResourceLoader()
{
	return shared_ptr<IResourceLoader>(GCC_NEW WaveResourceLoader());
}

shared_ptr<IResourceLoader> CreateOGGResourceLoader()
{
	return shared_ptr<IResourceLoader>(GCC_NEW OggResourceLoader());
}


unsigned int OggResourceLoader::VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize)
{
	OggVorbis_File vf;                     // for the vorbisfile interface

	ov_callbacks oggCallbacks;

	OggMemoryFile *vorbisMemoryFile = GCC_NEW OggMemoryFile; 
	vorbisMemoryFile->dataRead = 0;
	vorbisMemoryFile->dataSize = rawSize; 
	vorbisMemoryFile->dataPtr = (unsigned char *)rawBuffer;

	oggCallbacks.read_func = VorbisRead;
	oggCallbacks.close_func = VorbisClose;
	oggCallbacks.seek_func = VorbisSeek;
	oggCallbacks.tell_func = VorbisTell;

	int ov_ret = ov_open_callbacks(vorbisMemoryFile, &vf, NULL, 0, oggCallbacks);
	GCC_ASSERT(ov_ret>=0);

    // ok now the tricky part
    // the vorbis_info struct keeps the most of the interesting format info
    vorbis_info *vi = ov_info(&vf,-1);

	DWORD   size = 4096 * 16;
    DWORD   pos = 0;
    int     sec = 0;
    int     ret = 1;
    
	//Thanks to Kain for fixing this bug!
	DWORD bytes = (DWORD)ov_pcm_total(&vf, -1);    
	bytes *= 2 * vi->channels;

	ov_clear(&vf);

	SAFE_DELETE(vorbisMemoryFile);

	return bytes;
}

bool OggResourceLoader::VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle)
{
	shared_ptr<SoundResourceExtraData> extra = shared_ptr<SoundResourceExtraData>(GCC_NEW SoundResourceExtraData());
	extra->m_SoundType = SOUND_TYPE_OGG;
	handle->SetExtra(shared_ptr<SoundResourceExtraData>(extra));
	if (!ParseOgg(rawBuffer, rawSize, handle))
	{
		return false;
	}
	return true;
}

//
// OggResourceLoader::ParseOgg					- Chapter 13, page 405
//
bool OggResourceLoader::ParseOgg(char *oggStream, size_t length, shared_ptr<ResHandle> handle)
{
	shared_ptr<SoundResourceExtraData> extra = static_pointer_cast<SoundResourceExtraData>(handle->GetExtra());

	OggVorbis_File vf;                     // for the vorbisfile interface

	ov_callbacks oggCallbacks;

	OggMemoryFile *vorbisMemoryFile = GCC_NEW OggMemoryFile; 
	vorbisMemoryFile->dataRead = 0;
	vorbisMemoryFile->dataSize = length; 
	vorbisMemoryFile->dataPtr = (unsigned char *)oggStream;

	oggCallbacks.read_func = VorbisRead;
	oggCallbacks.close_func = VorbisClose;
	oggCallbacks.seek_func = VorbisSeek;
	oggCallbacks.tell_func = VorbisTell;

	int ov_ret = ov_open_callbacks(vorbisMemoryFile, &vf, NULL, 0, oggCallbacks);
	GCC_ASSERT(ov_ret>=0);

    // ok now the tricky part
    // the vorbis_info struct keeps the most of the interesting format info
    vorbis_info *vi = ov_info(&vf,-1);

    memset(&(extra->m_WavFormatEx), 0, sizeof(extra->m_WavFormatEx));

    extra->m_WavFormatEx.cbSize          = sizeof(extra->m_WavFormatEx);
    extra->m_WavFormatEx.nChannels       = vi->channels;
    extra->m_WavFormatEx.wBitsPerSample  = 16;                    // ogg vorbis is always 16 bit
    extra->m_WavFormatEx.nSamplesPerSec  = vi->rate;
    extra->m_WavFormatEx.nAvgBytesPerSec = extra->m_WavFormatEx.nSamplesPerSec*extra->m_WavFormatEx.nChannels*2;
    extra->m_WavFormatEx.nBlockAlign     = 2*extra->m_WavFormatEx.nChannels;
    extra->m_WavFormatEx.wFormatTag      = 1;

	DWORD   size = 4096 * 16;
    DWORD   pos = 0;
    int     sec = 0;
    int     ret = 1;
    
	DWORD bytes = (DWORD)ov_pcm_total(&vf, -1);    
	bytes *= 2 * vi->channels;
	
	if (handle->Size() != bytes)
	{
		GCC_ASSERT(0 && _T("The Ogg size does not match the memory buffer size!"));
		ov_clear(&vf);
		SAFE_DELETE(vorbisMemoryFile);
		return false;
	}

    // now read in the bits
    while(ret && pos<bytes)
    {
        ret = ov_read(&vf, handle->WritableBuffer()+pos, size, 0, 2, 1, &sec);
        pos += ret;
		if (bytes - pos < size)
		{
			size = bytes - pos;
		}
    }

	extra->m_LengthMilli = (int)(1000.f * ov_time_total(&vf, -1));
	
	ov_clear(&vf);

	SAFE_DELETE(vorbisMemoryFile);

	return true;
}








