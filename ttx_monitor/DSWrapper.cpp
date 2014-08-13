#include <windows.h>
#include "stdafx.h"
#define COM_NO_WINDOWS_H
#include <objbase.h>
#include <dsound.h>
#include <DXErr.h>
#include "DSWrapper.h"


#if 0
#define LOG_API()	logmsg("%s\n", __FUNCTION__)
#else
#define LOG_API()
#endif


LPDirectSoundCreate8 __DirectSoundCreate8;

static LPDIRECTSOUND8 pDS;
static LPDIRECTSOUNDBUFFER8 ppDSB; // Primary Sound Buffer

static HookIDirectSoundBuffer8 DSBWrapper, *pDSBWrapper = &DSBWrapper;
static HookIDirectSound8 DSWrapper, *pDSWrapper = &DSWrapper;

HRESULT HookIDirectSound8::QueryInterface(LPVOID _this, REFIID riid, LPVOID *ppvObj)
{
	LOG_API();
	return pDS->QueryInterface(riid, ppvObj);
}

ULONG HookIDirectSound8::AddRef(LPVOID _this)
{
	LOG_API();
	return pDS->AddRef();
}

ULONG HookIDirectSound8::Release(LPVOID _this)
{
	LOG_API();
	return pDS->Release();
}

HRESULT HookIDirectSound8::CreateSoundBuffer(LPVOID _this, LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter)
{
	LOG_API();
#if 0
	logmsg("Descrição do buffer:\n");
	logmsg("dwSize = %d\n", pcDSBufferDesc->dwSize);
	logmsg("dwFlags = %X\n", pcDSBufferDesc->dwFlags);
	logmsg("dwBufferBytes = %d\n", pcDSBufferDesc->dwBufferBytes);
	logmsg("dwReserved = %d\n", pcDSBufferDesc->dwReserved);
#endif
	if (!(pcDSBufferDesc->dwFlags & DSBCAPS_PRIMARYBUFFER)) {
		WAVEFORMATEX *wfx = pcDSBufferDesc->lpwfxFormat;
		//logmsg("Channels = %d\n", wfx->nChannels);

	} else {
		// primary buffer
		HRESULT result = pDS->CreateSoundBuffer(pcDSBufferDesc, (LPDIRECTSOUNDBUFFER*) &ppDSB, pUnkOuter);
		*ppDSBuffer = (LPDIRECTSOUNDBUFFER) pDSBWrapper;
		return result;
	}
	return pDS->CreateSoundBuffer(pcDSBufferDesc, ppDSBuffer, pUnkOuter);
}

HRESULT HookIDirectSound8::GetCaps(LPVOID _this, LPDSCAPS pDSCaps)
{
	LOG_API();
	return pDS->GetCaps(pDSCaps);
}

HRESULT HookIDirectSound8::DuplicateSoundBuffer(LPVOID _this, LPDIRECTSOUNDBUFFER pDSBufferOriginal,  LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate)
{
	LOG_API();
	return pDS->DuplicateSoundBuffer(pDSBufferOriginal, ppDSBufferDuplicate);
}

HRESULT HookIDirectSound8::SetCooperativeLevel(LPVOID _this, HWND hwnd, DWORD dwLevel)
{
	LOG_API();
	return pDS->SetCooperativeLevel(hwnd, dwLevel);
}

HRESULT HookIDirectSound8::Compact(LPVOID _this)
{
	LOG_API();
	return pDS->Compact();
}

HRESULT HookIDirectSound8::GetSpeakerConfig(LPVOID _this, LPDWORD pdwSpeakerConfig)
{
	LOG_API();
	if (pdwSpeakerConfig) {
		*pdwSpeakerConfig = DSSPEAKER_COMBINED(DSSPEAKER_7POINT1_SURROUND, DSSPEAKER_GEOMETRY_MAX);
	}
	return DS_OK;
	//return pDS->GetSpeakerConfig(pdwSpeakerConfig);
}

HRESULT HookIDirectSound8::SetSpeakerConfig(LPVOID _this, DWORD dwSpeakerConfig)
{
	LOG_API();
	return pDS->SetSpeakerConfig(dwSpeakerConfig);
}

HRESULT HookIDirectSound8::Initialize(LPVOID _this, LPCGUID pcGuidDevice)
{
	LOG_API();
	return pDS->Initialize(pcGuidDevice);
}

HRESULT HookIDirectSound8::VerifyCertification(LPVOID _this, LPDWORD pdwCertified)
{
	LOG_API();
	return pDS->VerifyCertification(pdwCertified);
}


HRESULT __stdcall Hook_DirectSoundCreate8(
         LPCGUID lpcGuidDevice,
         LPDIRECTSOUND8 * ppDS8,
         LPUNKNOWN pUnkOuter)
{
	LOG_API();
	HRESULT result = __DirectSoundCreate8(lpcGuidDevice, &pDS, pUnkOuter);

	*ppDS8 = (LPDIRECTSOUND8) pDSWrapper;
	pUnkOuter = NULL;
	return result;
}


HRESULT HookIDirectSoundBuffer8::QueryInterface(LPVOID _this, REFIID riid,LPVOID *ppvObj)
{
	LOG_API();
	return ppDSB->QueryInterface(riid, ppvObj);
}

ULONG HookIDirectSoundBuffer8::AddRef(LPVOID _this)
{
	LOG_API();
	return ppDSB->AddRef();
}

ULONG HookIDirectSoundBuffer8::Release(LPVOID _this)
{
	return ppDSB->Release();
}

HRESULT HookIDirectSoundBuffer8::GetCaps(LPVOID _this,  LPDSBCAPS pDSBufferCaps)
{
	LOG_API();
	return ppDSB->GetCaps(pDSBufferCaps);
}

HRESULT HookIDirectSoundBuffer8::GetCurrentPosition(LPVOID _this,  LPDWORD pdwCurrentPlayCursor,  LPDWORD pdwCurrentWriteCursor)
{
	LOG_API();
	return ppDSB->GetCurrentPosition(pdwCurrentPlayCursor, pdwCurrentWriteCursor);
}

HRESULT HookIDirectSoundBuffer8::GetFormat(LPVOID _this,  LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated,  LPDWORD pdwSizeWritten)
{
	LOG_API();
	return ppDSB->GetFormat(pwfxFormat, dwSizeAllocated, pdwSizeWritten);
}

HRESULT HookIDirectSoundBuffer8::GetVolume(LPVOID _this,  LPLONG plVolume)
{
	LOG_API();
	return ppDSB->GetVolume(plVolume);
}

HRESULT HookIDirectSoundBuffer8::GetPan(LPVOID _this,  LPLONG plPan)
{
	LOG_API();
	return ppDSB->GetPan(plPan);
}

HRESULT HookIDirectSoundBuffer8::GetFrequency(LPVOID _this,  LPDWORD pdwFrequency)
{
	LOG_API();
	return ppDSB->GetFrequency(pdwFrequency);
}

HRESULT HookIDirectSoundBuffer8::GetStatus(LPVOID _this,  LPDWORD pdwStatus)
{
	LOG_API();
	return ppDSB->GetStatus(pdwStatus);
}

HRESULT HookIDirectSoundBuffer8::Initialize(LPVOID _this,  LPDIRECTSOUND pDirectSound,  LPCDSBUFFERDESC pcDSBufferDesc)
{
	LOG_API();
	return ppDSB->Initialize(pDirectSound, pcDSBufferDesc);
}

HRESULT HookIDirectSoundBuffer8::Lock(LPVOID _this, DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1,  LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2,  LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	LOG_API();
	return ppDSB->Lock(dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
}

HRESULT HookIDirectSoundBuffer8::Play(LPVOID _this, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
	LOG_API();
	return ppDSB->Play(dwReserved1, dwPriority, dwFlags);
}

HRESULT HookIDirectSoundBuffer8::SetCurrentPosition(LPVOID _this, DWORD dwNewPosition)
{
	LOG_API();
	return ppDSB->SetCurrentPosition(dwNewPosition);
}

HRESULT HookIDirectSoundBuffer8::SetFormat(LPVOID _this, LPCWAVEFORMATEX pcfxFormat)
{
	LOG_API();

	WAVEFORMATEX wfx;
	CopyMemory(&wfx, pcfxFormat, sizeof(wfx));
	wfx.wFormatTag = 0;
/*
	logmsg("Tag = %x\n", pcfxFormat->wFormatTag);
	logmsg("Channels = %d\n", pcfxFormat->nChannels);
	logmsg("SPS = %d\n", pcfxFormat->nSamplesPerSec);
	logmsg("BPS = %d\n", pcfxFormat->wBitsPerSample);
	*/
	wfx.nChannels = 2;

	return ppDSB->SetFormat(&wfx);
}

HRESULT HookIDirectSoundBuffer8::SetVolume(LPVOID _this, LONG lVolume)
{
	LOG_API();
	return ppDSB->SetVolume(lVolume);
}

HRESULT HookIDirectSoundBuffer8::SetPan(LPVOID _this, LONG lPan)
{
	LOG_API();
	return ppDSB->SetPan(lPan);
}

HRESULT HookIDirectSoundBuffer8::SetFrequency(LPVOID _this, DWORD dwFrequency)
{
	LOG_API();
	return ppDSB->SetFrequency(dwFrequency);
}

HRESULT HookIDirectSoundBuffer8::Stop(LPVOID _this)
{
	LOG_API();
	return ppDSB->Stop();
}

HRESULT HookIDirectSoundBuffer8::Unlock(LPVOID _this, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	LOG_API();
	return ppDSB->Unlock(pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
}

HRESULT HookIDirectSoundBuffer8::Restore(LPVOID _this)
{
	LOG_API();
	return ppDSB->Restore();
}

HRESULT HookIDirectSoundBuffer8::SetFX(LPVOID _this, DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes)
{
	LOG_API();
	return ppDSB->SetFX(dwEffectsCount, pDSFXDesc, pdwResultCodes);
}

HRESULT HookIDirectSoundBuffer8::AcquireResources(LPVOID _this, DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes)
{
	LOG_API();
	return ppDSB->AcquireResources(dwFlags, dwEffectsCount, pdwResultCodes);
}

HRESULT HookIDirectSoundBuffer8::GetObjectInPath(LPVOID _this,  REFGUID rguidObject, DWORD dwIndex,  REFGUID rguidInterface, LPVOID *ppObject)
{
	LOG_API();
	return ppDSB->GetObjectInPath(rguidObject, dwIndex, rguidInterface, ppObject);
}
