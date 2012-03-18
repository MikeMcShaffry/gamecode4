#pragma once
//========================================================================
// Audio.cpp : Defines a simple sound system.
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


class SoundResourceExtraData;


//////////////////////////////////////////////////////////////////////
// SoundType Description
//
// This is an enum that represents the different kinds of sound data
// streams the sound system can handle.
//
//////////////////////////////////////////////////////////////////////

enum SoundType
{
	SOUND_TYPE_FIRST,
	SOUND_TYPE_MP3 = SOUND_TYPE_FIRST,
	SOUND_TYPE_WAVE,
	SOUND_TYPE_MIDI,
	SOUND_TYPE_OGG,

	// This needs to be the last sound type
	SOUND_TYPE_COUNT,
	SOUND_TYPE_UNKNOWN,
};

extern char *gSoundExtentions[];

//////////////////////////////////////////////////////////////////////
// class IAudioBuffer							- Chapter 13, page 409
//
// The interface class that defines the public API for audio buffers.
// An audio buffer maps to one instance of a sound being played, 
// which ISNT the sound data. Two different sounds can be played from
// the same source data - such as two explosions in two different places.
//////////////////////////////////////////////////////////////////////

class IAudioBuffer
{
public:
	virtual ~IAudioBuffer() { }

	virtual void *VGet()=0;
	virtual shared_ptr<ResHandle> VGetResource()=0;
	virtual bool VOnRestore()=0;

	virtual bool VPlay(int volume, bool looping)=0;
	virtual bool VPause()=0;
	virtual bool VStop()=0;
	virtual bool VResume()=0;

	virtual bool VTogglePause()=0;
	virtual bool VIsPlaying()=0;
	virtual bool VIsLooping() const=0;
	virtual void VSetVolume(int volume)=0;
    virtual void VSetPosition(unsigned long newPosition)=0;
	virtual int VGetVolume() const=0;
	virtual float VGetProgress()=0;
};

//////////////////////////////////////////////////////////////////////
// class AudioBuffer							- Chapter 13, page 411 
//
// Implements IAudiobuffer interface using a smart pointer to SoundResource.
//
//////////////////////////////////////////////////////////////////////

class AudioBuffer : public IAudioBuffer
{
public: 
	virtual shared_ptr<ResHandle> VGetResource() { return m_Resource; }
	virtual bool VIsLooping() const { return m_isLooping; }
	virtual int VGetVolume() const { return m_Volume; }
protected:
	AudioBuffer(shared_ptr<ResHandle >resource) 
		{ m_Resource = resource; 
		  m_isPaused = false;
		  m_isLooping = false;
		  m_Volume = 0;
		}	// disable public construction

	shared_ptr<ResHandle> m_Resource;

	// Is the sound paused
	bool m_isPaused;

	// Is the sound looping
	bool m_isLooping;

	//the volume
	int m_Volume;
};


//////////////////////////////////////////////////////////////////////
// class IAudio									- Chapter 13, page 411
//
// This interface class describes the public interface for 
// a game's audio system. 
//////////////////////////////////////////////////////////////////////

class IAudio
{
public:
	virtual bool VActive()=0;

	virtual IAudioBuffer *VInitAudioBuffer(shared_ptr<ResHandle> handle)=0;
	virtual void VReleaseAudioBuffer(IAudioBuffer* audioBuffer)=0;

	virtual void VStopAllSounds()=0;
	virtual void VPauseAllSounds()=0;
	virtual void VResumeAllSounds()=0;

	virtual bool VInitialize(HWND hWnd)=0;
	virtual void VShutdown()=0;
};

//////////////////////////////////////////////////////////////////////
// class Audio									- Chapter 13, page 412
//
// Implements IAudio interface - but not all the way - this is 
// still a base class. See class DirectSoundAudio.
// 
//////////////////////////////////////////////////////////////////////

class Audio : public IAudio
{
public:
	Audio();
	~Audio() { VShutdown(); }
	virtual void VStopAllSounds();
	virtual void VPauseAllSounds();
	virtual void VResumeAllSounds();

	virtual void VShutdown();
	static bool HasSoundCard(void);
	bool IsPaused() { return m_AllPaused; }

protected:

	typedef std::list<IAudioBuffer *> AudioBufferList;

	AudioBufferList m_AllSamples;	// List of all currently allocated audio buffers
	bool m_AllPaused;				// Has the sound system been paused?
	bool m_Initialized;				// Has the sound system been initialized?
};

extern Audio *g_pAudio;


