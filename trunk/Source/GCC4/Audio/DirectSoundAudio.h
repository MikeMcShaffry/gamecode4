#pragma once
//========================================================================
// DirectSoundAudio.h : Implements audio interfaces for DirectSound
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

#include "Audio.h"

// DirectSound includes
#include <dsound.h>
#include <mmsystem.h>


//////////////////////////////////////////////////////////////////////
// DirectSoundAudioBuffer						- Chapter 13, page 420
//
// Implements the rest of the IAudioBuffer interface left out by AudioBuffer.  
// If you are interested in implementing a sound system using OpenAL 
// you'd create a class OpenALAudioBuffer from AudioBuffer.
// 
//////////////////////////////////////////////////////////////////////

class DirectSoundAudioBuffer : public AudioBuffer
{
protected:
	LPDIRECTSOUNDBUFFER m_Sample;

public:
	DirectSoundAudioBuffer(LPDIRECTSOUNDBUFFER sample, shared_ptr<ResHandle> resource);
	virtual void *VGet();
	virtual bool VOnRestore();

	virtual bool VPlay(int volume, bool looping);
	virtual bool VPause();
	virtual bool VStop();
	virtual bool VResume();

	virtual bool VTogglePause();
	virtual bool VIsPlaying();
	virtual void VSetVolume(int volume);
    virtual void VSetPosition(unsigned long newPosition);

	virtual float VGetProgress();

private:
	HRESULT FillBufferWithSound( );
	HRESULT RestoreBuffer( BOOL* pbWasRestored );
};



//////////////////////////////////////////////////////////////////////
// class DirectSoundAudio						- Chapter 13, page 414
//
// Implements the rest of the IAudio interface left out by Audio.  
// If you are interested in implementing a sound system using OpenAL 
// you'd create a class OpenALAudioBuffer from AudioBuffer.
// 
//////////////////////////////////////////////////////////////////////

class DirectSoundAudio : public Audio
{
public:
	DirectSoundAudio() { m_pDS = NULL; }
	virtual bool VActive() { return m_pDS!=NULL; }

	virtual IAudioBuffer *VInitAudioBuffer(shared_ptr<ResHandle> handle);
	virtual void VReleaseAudioBuffer(IAudioBuffer* audioBuffer);

	virtual void VShutdown();
	virtual bool VInitialize(HWND hWnd);

protected:

    IDirectSound8* m_pDS;

	HRESULT SetPrimaryBufferFormat( DWORD dwPrimaryChannels, 
									DWORD dwPrimaryFreq, 
									DWORD dwPrimaryBitRate );
};
