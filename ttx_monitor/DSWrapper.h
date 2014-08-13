#pragma once

#include <windows.h>
#include "stdafx.h"
#define COM_NO_WINDOWS_H
#include <objbase.h>
#include <dsound.h>
#include <DXErr.h>

struct FAR HookIDirectSound8
{
    /*** IUnknown methods ***/
    virtual HRESULT QueryInterface(LPVOID _this, REFIID riid,LPVOID *ppvObj);
    virtual ULONG AddRef(LPVOID _this);
    virtual ULONG Release(LPVOID _this);

    // IDirectSound methods
    virtual HRESULT CreateSoundBuffer    (LPVOID _this, LPCDSBUFFERDESC pcDSBufferDesc,  LPDIRECTSOUNDBUFFER *ppDSBuffer, __null LPUNKNOWN pUnkOuter) ;
    virtual HRESULT GetCaps              (LPVOID _this, LPDSCAPS pDSCaps) ;
    virtual HRESULT DuplicateSoundBuffer (LPVOID _this, LPDIRECTSOUNDBUFFER pDSBufferOriginal,  LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate) ;
    virtual HRESULT SetCooperativeLevel  (LPVOID _this, HWND hwnd, DWORD dwLevel) ;
    virtual HRESULT Compact              (LPVOID _this);
    virtual HRESULT GetSpeakerConfig     (LPVOID _this, LPDWORD pdwSpeakerConfig) ;
    virtual HRESULT SetSpeakerConfig     (LPVOID _this, DWORD dwSpeakerConfig) ;
    virtual HRESULT Initialize           (LPVOID _this, LPCGUID pcGuidDevice) ;

    // IDirectSound8 methods
    virtual HRESULT VerifyCertification  (LPVOID _this, LPDWORD pdwCertified) ;
};

struct FAR HookIDirectSoundBuffer8
{
    /*** IUnknown methods ***/
    virtual HRESULT QueryInterface(LPVOID _this, REFIID riid,LPVOID *ppvObj);
    virtual ULONG AddRef(LPVOID _this);
    virtual ULONG Release(LPVOID _this);

    // IDirectSoundBuffer methods
    virtual HRESULT GetCaps              (LPVOID _this,  LPDSBCAPS pDSBufferCaps) ;
    virtual HRESULT GetCurrentPosition   (LPVOID _this,  LPDWORD pdwCurrentPlayCursor,  LPDWORD pdwCurrentWriteCursor) ;
    virtual HRESULT GetFormat            (LPVOID _this,  LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated,  LPDWORD pdwSizeWritten) ;
    virtual HRESULT GetVolume            (LPVOID _this,  LPLONG plVolume) ;
    virtual HRESULT GetPan               (LPVOID _this,  LPLONG plPan) ;
    virtual HRESULT GetFrequency         (LPVOID _this,  LPDWORD pdwFrequency) ;
    virtual HRESULT GetStatus            (LPVOID _this,  LPDWORD pdwStatus) ;
    virtual HRESULT Initialize           (LPVOID _this,  LPDIRECTSOUND pDirectSound,  LPCDSBUFFERDESC pcDSBufferDesc) ;
    virtual HRESULT Lock                 (LPVOID _this, DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1,  LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2,  LPDWORD pdwAudioBytes2, DWORD dwFlags) ;
    virtual HRESULT Play                 (LPVOID _this, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags) ;
    virtual HRESULT SetCurrentPosition   (LPVOID _this, DWORD dwNewPosition) ;
    virtual HRESULT SetFormat            (LPVOID _this, LPCWAVEFORMATEX pcfxFormat) ;
    virtual HRESULT SetVolume            (LPVOID _this, LONG lVolume) ;
    virtual HRESULT SetPan               (LPVOID _this, LONG lPan) ;
    virtual HRESULT SetFrequency         (LPVOID _this, DWORD dwFrequency) ;
    virtual HRESULT Stop                 (LPVOID _this) ;
    virtual HRESULT Unlock               (LPVOID _this, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2) ;
    virtual HRESULT Restore              (LPVOID _this) ;
	virtual HRESULT SetFX                (LPVOID _this, DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes);
    virtual HRESULT AcquireResources     (LPVOID _this, DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes);
    virtual HRESULT GetObjectInPath      (LPVOID _this,  REFGUID rguidObject, DWORD dwIndex,  REFGUID rguidInterface, LPVOID *ppObject);
};
