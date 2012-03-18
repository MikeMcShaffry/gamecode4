//========================================================================
// DirectSoundAudio.cpp : Defines a simple sound system that uses DirectSound
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

#include "../GameCode4/GameCode.h"
#include "SoundResource.h"
#include "DirectSoundAudio.h"

#include <cguid.h>

//////////////////////////////////////////////////////////////////////
// 
// DirectSoundAudio Implementation
//
//////////////////////////////////////////////////////////////////////



//
// DirectSoundAudio::VInitialize			- Chapter 13, page 415
//
bool DirectSoundAudio::VInitialize(HWND hWnd)
{
	if(m_Initialized)
		return true;

	m_Initialized=false;
	m_AllSamples.clear();

	SAFE_RELEASE( m_pDS );

	HRESULT hr;

	// Create IDirectSound using the primary sound device
	if( FAILED( hr = DirectSoundCreate8( NULL, &m_pDS, NULL ) ) )
		return false;

	// Set DirectSound coop level 
	if( FAILED( hr = m_pDS->SetCooperativeLevel( hWnd, DSSCL_PRIORITY) ) )
		return false;

	if( FAILED( hr = SetPrimaryBufferFormat( 8, 44100, 16 ) ) )
		return false;

	m_Initialized = true;

	return true;
}

//
// DirectSoundAudio::SetPrimaryBufferFormat		- Chapter 13, page 416
//
HRESULT DirectSoundAudio::SetPrimaryBufferFormat( 
	DWORD dwPrimaryChannels, 
    DWORD dwPrimaryFreq, 
    DWORD dwPrimaryBitRate )
{
	// !WARNING! - Setting the primary buffer format and then using this
	// it for DirectMusic messes up DirectMusic! 
	//
	// If you want your primary buffer format to be 22kHz stereo, 16-bit
	// call with these parameters:	SetPrimaryBufferFormat(2, 22050, 16);

	HRESULT             hr;
	LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

	if( ! m_pDS )
		return CO_E_NOTINITIALIZED;

	// Get the primary buffer 
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize        = sizeof(DSBUFFERDESC);
	dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat   = NULL;

	if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) )
		return DXUT_ERR( L"CreateSoundBuffer", hr );

	WAVEFORMATEX wfx;
	ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
	wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM; 
	wfx.nChannels       = (WORD) dwPrimaryChannels; 
	wfx.nSamplesPerSec  = (DWORD) dwPrimaryFreq; 
	wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
	wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);

	if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) )
		return DXUT_ERR( L"SetFormat", hr );

	SAFE_RELEASE( pDSBPrimary );

	return S_OK;
}


//
// DirectSoundAudio::VShutdown					- Chapter 13, page 417
//
void DirectSoundAudio::VShutdown()
{
	if(m_Initialized)
	{
		Audio::VShutdown();
		SAFE_RELEASE(m_pDS);
		m_Initialized = false;
	}
}


//
// DirectSoundAudio::VInitAudioBuffer			- Chapter 13, page 418
//   Allocate a sample handle for the newborn sound (used by SoundResource) and tell you it's length
//
IAudioBuffer *DirectSoundAudio::VInitAudioBuffer(shared_ptr<ResHandle> resHandle)//const
{
	shared_ptr<SoundResourceExtraData> extra = static_pointer_cast<SoundResourceExtraData>(resHandle->GetExtra());

    if( ! m_pDS )
        return NULL;

	switch(extra->GetSoundType())
	{
		case SOUND_TYPE_OGG:
		case SOUND_TYPE_WAVE:
			// We support WAVs and OGGs
			break;

		case SOUND_TYPE_MP3:
		case SOUND_TYPE_MIDI:	//If it's a midi file, then do nothin at this time... maybe we will support this in the future
			GCC_ASSERT(false && "MP3s and MIDI are not supported");
			return NULL;
			break;

		default:
			GCC_ASSERT(false && "Unknown sound type");
			return NULL;
	}//end switch

    LPDIRECTSOUNDBUFFER sampleHandle;

    // Create the direct sound buffer, and only request the flags needed
    // since each requires some overhead and limits if the buffer can 
    // be hardware accelerated
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = DSBCAPS_CTRLVOLUME;
    dsbd.dwBufferBytes   = resHandle->Size();
    dsbd.guid3DAlgorithm = GUID_NULL;
    dsbd.lpwfxFormat     = const_cast<WAVEFORMATEX *>(extra->GetFormat());

	HRESULT hr;
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &sampleHandle, NULL ) ) )
    {
        return NULL;
    }

	// Add handle to the list
	IAudioBuffer *audioBuffer = GCC_NEW DirectSoundAudioBuffer(sampleHandle, resHandle);
	m_AllSamples.push_front( audioBuffer);
	
	return audioBuffer;

}

//
// DirectSoundAudio::VReleaseAudioBuffer			- Chapter 13, page 419
//    Allocate a sample handle for the newborn sound (used by SoundResource)
//
void DirectSoundAudio::VReleaseAudioBuffer(IAudioBuffer *sampleHandle)//const
{
	sampleHandle->VStop();
	m_AllSamples.remove(sampleHandle);
}

//
// DirectSoundAudioBuffer::DirectSoundAudioBuffer	- Chapter 13, page 420
//
DirectSoundAudioBuffer::DirectSoundAudioBuffer(
	LPDIRECTSOUNDBUFFER sample, 
	shared_ptr<ResHandle> resource)
 : AudioBuffer(resource) 
{ 
	m_Sample = sample; 
	FillBufferWithSound();
}

//
// DirectSoundAudioBuffer::VGet						- Chapter 13, page 420
//
void *DirectSoundAudioBuffer::VGet()
{
 	if (!VOnRestore())
		return NULL;

	return m_Sample;
}


//
// DirectSoundAudioBuffer::VPlay					- Chapter 13, page 421
//    Play a sound
//
bool DirectSoundAudioBuffer::VPlay(int volume, bool looping)
{
   if(!g_pAudio->VActive())
      return false;

	VStop();

	m_Volume = volume;
	m_isLooping = looping;

	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)VGet();
	if (!pDSB)
		return false;

    pDSB->SetVolume( volume );
    
    DWORD dwFlags = looping ? DSBPLAY_LOOPING : 0L;

    return (S_OK==pDSB->Play( 0, 0, dwFlags ) );

}//end Play


//
// DirectSoundAudioBuffer::VStop					- Chapter 13, page 421
//    Stop a sound and rewind play position to the beginning.
//
bool DirectSoundAudioBuffer::VStop()
{
	if(!g_pAudio->VActive())
		return false;

	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)VGet();

   if( ! pDSB )
		return false;

	m_isPaused=true;
    pDSB->Stop();
	return true;
}


//
// DirectSoundAudioBuffer::VPause					- Chapter X, page Y
//    Pause a sound 
//
bool DirectSoundAudioBuffer::VPause()
{
	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)VGet();

	if(!g_pAudio->VActive())
		return false;

   if( pDSB )
		return false;

	m_isPaused=true;
    pDSB->Stop();
	pDSB->SetCurrentPosition(0);	// rewinds buffer to beginning.
	return true;
}

//
// DirectSoundAudioBuffer::VResume					- Chapter 13, page 421
//    Resume a sound
bool DirectSoundAudioBuffer::VResume()
{
	m_isPaused=false;
	return VPlay(VGetVolume(), VIsLooping());
}

//
// DirectSoundAudioBuffer::VTogglePause				- Chapter 13, page 421
//    Pause a sound or Resume a Paused sound
//
bool DirectSoundAudioBuffer::VTogglePause()
{
	if(!g_pAudio->VActive())
		return false;

	if(m_isPaused)
	{
		VResume();
	}
	else
	{
		VPause();				// note that the book code calls VStop().
								//   It's better to call VPause() instead.
	}

	return true;
}//end TogglePause





//
// DirectSoundAudioBuffer::VIsPlaying				- Chapter 13, page 422
//
bool DirectSoundAudioBuffer::VIsPlaying() 
{
	if(!g_pAudio->VActive())
		return false;

	DWORD dwStatus = 0;
	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)VGet();
	pDSB->GetStatus( &dwStatus );
    bool bIsPlaying = ( ( dwStatus & DSBSTATUS_PLAYING ) != 0 );

	return bIsPlaying;
}

//
// DirectSoundAudioBuffer::VSetVolume				- Chapter 13, page 422
//
void DirectSoundAudioBuffer::VSetVolume(int volume)
{
	// DSBVOLUME_MIN, defined in dsound.h is set to as -10000, which is just way too silent for a 
	// lower bound and many programmers find -5000 to be a better minimum bound for the volume 
	// range to avoid an annoying silence for the lower 50% of a volume slider that uses a logarithmic scale.
	// This was contributed by BystanderKain!
	int gccDSBVolumeMin = DSBVOLUME_MIN;

	if(!g_pAudio->VActive())
		return;

	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)VGet();

	GCC_ASSERT(volume>=0 && volume<=100 && "Volume must be a number between 0 and 100");

	// convert volume from 0-100 into range for DirectX - don't forget to use a logarithmic scale!

	float coeff = (float)volume / 100.0f;
	float logarithmicProportion = coeff >0.1f  ? 1+log10(coeff)  : 0;
	float range = float(DSBVOLUME_MAX - gccDSBVolumeMin);
	float fvolume = ( range * logarithmicProportion ) + gccDSBVolumeMin;

	GCC_ASSERT(fvolume>=gccDSBVolumeMin && fvolume<=DSBVOLUME_MAX);
	HRESULT hr = pDSB->SetVolume( LONG(fvolume) );
	GCC_ASSERT(hr==S_OK);

}

void DirectSoundAudioBuffer::VSetPosition(unsigned long newPosition)
{
    m_Sample->SetCurrentPosition(newPosition);
}


//
// DirectSoundAudioBuffer::VOnRestore		- Chapter 13, page 423
//    NOTE: Renamed from DirectSoundAudioBuffer::VRestore in the book
bool DirectSoundAudioBuffer::VOnRestore()
{
   HRESULT hr;
   BOOL    bRestored;

    // Restore the buffer if it was lost
    if( FAILED( hr = RestoreBuffer( &bRestored ) ) )
        return NULL;

    if( bRestored )
    {
        // The buffer was restored, so we need to fill it with new data
        if( FAILED( hr = FillBufferWithSound( ) ) )
            return NULL;
    }

	return true;
}

//
// DirectSoundAudioBuffer::RestoreBuffer			- Chapter 13, page 423
//
//    Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was 
//    restored.  It can also NULL if the information is not needed.
//
HRESULT DirectSoundAudioBuffer::RestoreBuffer( BOOL* pbWasRestored )
{
    HRESULT hr;

    if( ! m_Sample )
        return CO_E_NOTINITIALIZED;
    if( pbWasRestored )
        *pbWasRestored = FALSE;

    DWORD dwStatus;
    if( FAILED( hr = m_Sample->GetStatus( &dwStatus ) ) )
        return DXUT_ERR( L"GetStatus", hr );

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control but fail if
        // if it goes on for more than 1 second
        int count = 0;
		do 
        {
            hr = m_Sample->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
        }
        while( ( hr = m_Sample->Restore() ) == DSERR_BUFFERLOST && ++count < 100 );

        if( pbWasRestored != NULL )
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//
// DirectSoundAudioBuffer::FillBufferWithSound				- Chapter 13, page 425
//
HRESULT DirectSoundAudioBuffer::FillBufferWithSound( void )
{
    HRESULT hr; 
	VOID	*pDSLockedBuffer = NULL;	 // a pointer to the DirectSound buffer
    DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
    DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file 

    if( ! m_Sample )
        return CO_E_NOTINITIALIZED;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
    if( FAILED( hr = RestoreBuffer( NULL ) ) ) 
        return DXUT_ERR( L"RestoreBuffer", hr );

	int pcmBufferSize = m_Resource->Size();
	shared_ptr<SoundResourceExtraData> extra = static_pointer_cast<SoundResourceExtraData>(m_Resource->GetExtra());

	
    // Lock the buffer down
    if( FAILED( hr = m_Sample->Lock( 0, pcmBufferSize, 
                                 &pDSLockedBuffer, &dwDSLockedBufferSize, 
                                 NULL, NULL, 0L ) ) )
        return DXUT_ERR( L"Lock", hr );

    if( pcmBufferSize == 0 )
    {
        // Wav is blank, so just fill with silence
        FillMemory( (BYTE*) pDSLockedBuffer, 
                    dwDSLockedBufferSize, 
                    (BYTE)(extra->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else 
	{
		CopyMemory(pDSLockedBuffer, m_Resource->Buffer(), pcmBufferSize);
		if( pcmBufferSize < (int)dwDSLockedBufferSize )
		{
            // If the buffer sizes are different fill in the rest with silence 
            FillMemory( (BYTE*) pDSLockedBuffer + pcmBufferSize, 
                        dwDSLockedBufferSize - pcmBufferSize, 
                        (BYTE)(extra->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
        }
    }

    // Unlock the buffer, we don't need it anymore.
    m_Sample->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return S_OK;
}




//
// DirectSoundAudioBuffer::VGetProgress				- Chapter 13, page 426
//
float DirectSoundAudioBuffer::VGetProgress()
{
	LPDIRECTSOUNDBUFFER pDSB = (LPDIRECTSOUNDBUFFER)VGet();	
	DWORD progress = 0;

	pDSB->GetCurrentPosition(&progress, NULL);

	float length = (float)m_Resource->Size();

	return (float)progress / length;
}


