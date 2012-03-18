#pragma once
/////////////////////////////////////////////////////////////////////////////
// File SoundProcess.h
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



#include "..\MainLoop\Process.h"

#include "Audio.h"
#include "SoundResource.h"

/////////////////////////////////////////////////////////////////////////////
// class SoundProcess							- Chapter 13, page 426
//
//    A Sound Process, not to be confused with a Sound Resource (SoundResource)
//	  manages a sound as it is being played. You can use this class to manage
//    timing between sounds & animations.
//
/////////////////////////////////////////////////////////////////////////////

class SoundProcess : public Process
{
protected:
    shared_ptr<ResHandle> m_handle;			// this is the raw sound data
    shared_ptr<IAudioBuffer> m_AudioBuffer;		// handle to the implementation dependent audio buffer (DSound, Miles) 

    int m_Volume;						// these hold the initial setting until the sound is actually launched.
    bool m_isLooping;

public:
	SoundProcess(shared_ptr<ResHandle> soundResource, int volume=100, bool looping=false);
	virtual ~SoundProcess();

    void Play(const int volume, const bool looping);
    void Stop();

    void SetVolume(int volume);
    int GetVolume();
    int GetLengthMilli();
    bool IsSoundValid() { return m_handle != NULL; }
    bool IsPlaying();
    bool IsLooping() { return m_AudioBuffer && m_AudioBuffer->VIsLooping(); }
    float GetProgress();
    void PauseSound(void);

protected:
	virtual void VOnInit();
    virtual void VOnUpdate(unsigned long deltaMs);

    void InitializeVolume();

protected:
	SoundProcess();	 // Disable Default Construction

};



/////////////////////////////////////////////////////////////////////////////
// c;ass ExplosionProcess						- Chapter 13, page 433
//
// This is an example of a process that uses a simple state machine
// to control itself. 
//
/////////////////////////////////////////////////////////////////////////////

class ExplosionProcess : public Process
{
protected:
    int m_Stage;
    shared_ptr<SoundProcess> m_Sound;

  public:
  	ExplosionProcess() { m_Stage=0; }

protected:   
    virtual void VOnInit();
  	virtual void VOnUpdate(unsigned long deltaMs);
};

/////////////////////////////////////////////////////////////////////////////
// class FadeProcess							- Chapter 13, page 435
//
// Fades sound volume in or out over time and then kills itself.
// This should be useful for groups of sound effects, too - such as when
//   an AI barks and it must be heard above the other effects like too much freaking thunder.
//
/////////////////////////////////////////////////////////////////////////////

class FadeProcess : public Process
{
protected:
	shared_ptr<SoundProcess> m_Sound;
	
	int m_TotalFadeTime;
	int m_ElapsedTime;
	int m_StartVolume;
	int m_EndVolume;

public:
	FadeProcess(shared_ptr<SoundProcess> sound, int fadeTime, int endVolume);
	virtual void VOnUpdate(unsigned long deltaMs);	
};




