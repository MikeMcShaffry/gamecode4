#pragma once
/////////////////////////////////////////////////////////////////////////////
// File CSoundResource.h
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>

#include "../ResourceCache/ResCache.h"

//
// class SoundResourceExtraData				- Chapter 13, page 399
//
class SoundResourceExtraData : public IResourceExtraData
{
	friend class WaveResourceLoader;
	friend class OggResourceLoader;

public: 	
	SoundResourceExtraData();
	virtual ~SoundResourceExtraData() { }
	virtual std::string VToString() { return "SoundResourceExtraData"; }
	enum SoundType GetSoundType() { return m_SoundType; }
	WAVEFORMATEX const *GetFormat() { return &m_WavFormatEx; }
	int GetLengthMilli() const { return m_LengthMilli; }

protected:
	enum SoundType m_SoundType;			// is this an Ogg, WAV, etc.?
	bool m_bInitialized;				// has the sound been initialized
	WAVEFORMATEX m_WavFormatEx;			// description of the PCM format
	int m_LengthMilli;					// how long the sound is in milliseconds
};


//
// class WaveResourceLoader						- Chapter 13, page 399
//
class WaveResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.wav"; }

protected:
	bool ParseWave(char *wavStream, size_t length, shared_ptr<ResHandle> handle);
};


//
// class OggResourceLoader						- Chapter 13, page 399
//
class OggResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return true; }
	virtual unsigned int VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.ogg"; }


protected:
	bool ParseOgg(char *oggStream, size_t length, shared_ptr<ResHandle> handle);
};

