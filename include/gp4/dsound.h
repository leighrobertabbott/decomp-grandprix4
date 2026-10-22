// DirectSound interfaces as GP4 calls them (Windows SDK dsound.h): method order and argument
// counts from the SDK's DECLARE_INTERFACE_ blocks. IDirectSoundBuffer is IDirectSoundBuffer8
// without its last three methods. Pointer arguments are void* (cast at the call); D3DVALUE is
// float. Modelled by the oracle in gp4re/com_dx.py (layout fill com=IDirectSoundBuffer etc.).
#pragma once
#include <gp4/com.h>

struct IDirectSound8 : IUnknown_ {
    virtual HRESULT_ __stdcall CreateSoundBuffer(void* pcDSBufferDesc, void* ppDSBuffer, void* pUnkOuter) = 0;
    virtual HRESULT_ __stdcall GetCaps(void* pDSCaps) = 0;
    virtual HRESULT_ __stdcall DuplicateSoundBuffer(void* pDSBufferOriginal, void* ppDSBufferDuplicate) = 0;
    virtual HRESULT_ __stdcall SetCooperativeLevel(void* hwnd, uint32_t dwLevel) = 0;
    virtual HRESULT_ __stdcall Compact() = 0;
    virtual HRESULT_ __stdcall GetSpeakerConfig(void* pdwSpeakerConfig) = 0;
    virtual HRESULT_ __stdcall SetSpeakerConfig(uint32_t dwSpeakerConfig) = 0;
    virtual HRESULT_ __stdcall Initialize(const GUID_* pcGuidDevice) = 0;
    virtual HRESULT_ __stdcall VerifyCertification(void* pdwCertified) = 0;
};

struct IDirectSoundBuffer8 : IUnknown_ {
    virtual HRESULT_ __stdcall GetCaps(void* pDSBufferCaps) = 0;
    virtual HRESULT_ __stdcall GetCurrentPosition(void* pdwCurrentPlayCursor, void* pdwCurrentWriteCursor) = 0;
    virtual HRESULT_ __stdcall GetFormat(void* pwfxFormat, uint32_t dwSizeAllocated, void* pdwSizeWritten) = 0;
    virtual HRESULT_ __stdcall GetVolume(void* plVolume) = 0;
    virtual HRESULT_ __stdcall GetPan(void* plPan) = 0;
    virtual HRESULT_ __stdcall GetFrequency(void* pdwFrequency) = 0;
    virtual HRESULT_ __stdcall GetStatus(void* pdwStatus) = 0;
    virtual HRESULT_ __stdcall Initialize(void* pDirectSound, void* pcDSBufferDesc) = 0;
    virtual HRESULT_ __stdcall Lock(uint32_t dwOffset, uint32_t dwBytes, void* ppvAudioPtr1, void* pdwAudioBytes1, void* ppvAudioPtr2, void* pdwAudioBytes2, uint32_t dwFlags) = 0;
    virtual HRESULT_ __stdcall Play(uint32_t dwReserved1, uint32_t dwPriority, uint32_t dwFlags) = 0;
    virtual HRESULT_ __stdcall SetCurrentPosition(uint32_t dwNewPosition) = 0;
    virtual HRESULT_ __stdcall SetFormat(void* pcfxFormat) = 0;
    virtual HRESULT_ __stdcall SetVolume(int32_t lVolume) = 0;
    virtual HRESULT_ __stdcall SetPan(int32_t lPan) = 0;
    virtual HRESULT_ __stdcall SetFrequency(uint32_t dwFrequency) = 0;
    virtual HRESULT_ __stdcall Stop() = 0;
    virtual HRESULT_ __stdcall Unlock(void* pvAudioPtr1, uint32_t dwAudioBytes1, void* pvAudioPtr2, uint32_t dwAudioBytes2) = 0;
    virtual HRESULT_ __stdcall Restore() = 0;
    virtual HRESULT_ __stdcall SetFX(uint32_t dwEffectsCount, void* pDSFXDesc, void* pdwResultCodes) = 0;
    virtual HRESULT_ __stdcall AcquireResources(uint32_t dwFlags, uint32_t dwEffectsCount, void* pdwResultCodes) = 0;
    virtual HRESULT_ __stdcall GetObjectInPath(const GUID_* rguidObject, uint32_t dwIndex, const GUID_* rguidInterface, void* ppObject) = 0;
};

struct IDirectSound3DBuffer : IUnknown_ {
    virtual HRESULT_ __stdcall GetAllParameters(void* pDs3dBuffer) = 0;
    virtual HRESULT_ __stdcall GetConeAngles(void* pdwInsideConeAngle, void* pdwOutsideConeAngle) = 0;
    virtual HRESULT_ __stdcall GetConeOrientation(void* pvOrientation) = 0;
    virtual HRESULT_ __stdcall GetConeOutsideVolume(void* plConeOutsideVolume) = 0;
    virtual HRESULT_ __stdcall GetMaxDistance(void* pflMaxDistance) = 0;
    virtual HRESULT_ __stdcall GetMinDistance(void* pflMinDistance) = 0;
    virtual HRESULT_ __stdcall GetMode(void* pdwMode) = 0;
    virtual HRESULT_ __stdcall GetPosition(void* pvPosition) = 0;
    virtual HRESULT_ __stdcall GetVelocity(void* pvVelocity) = 0;
    virtual HRESULT_ __stdcall SetAllParameters(void* pcDs3dBuffer, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetConeAngles(uint32_t dwInsideConeAngle, uint32_t dwOutsideConeAngle, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetConeOrientation(float x, float y, float z, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetConeOutsideVolume(int32_t lConeOutsideVolume, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetMaxDistance(float flMaxDistance, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetMinDistance(float flMinDistance, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetMode(uint32_t dwMode, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetPosition(float x, float y, float z, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetVelocity(float x, float y, float z, uint32_t dwApply) = 0;
};

struct IDirectSound3DListener : IUnknown_ {
    virtual HRESULT_ __stdcall GetAllParameters(void* pListener) = 0;
    virtual HRESULT_ __stdcall GetDistanceFactor(void* pflDistanceFactor) = 0;
    virtual HRESULT_ __stdcall GetDopplerFactor(void* pflDopplerFactor) = 0;
    virtual HRESULT_ __stdcall GetOrientation(void* pvOrientFront, void* pvOrientTop) = 0;
    virtual HRESULT_ __stdcall GetPosition(void* pvPosition) = 0;
    virtual HRESULT_ __stdcall GetRolloffFactor(void* pflRolloffFactor) = 0;
    virtual HRESULT_ __stdcall GetVelocity(void* pvVelocity) = 0;
    virtual HRESULT_ __stdcall SetAllParameters(void* pcListener, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetDistanceFactor(float flDistanceFactor, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetDopplerFactor(float flDopplerFactor, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetOrientation(float xFront, float yFront, float zFront, float xTop, float yTop, float zTop, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetPosition(float x, float y, float z, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetRolloffFactor(float flRolloffFactor, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall SetVelocity(float x, float y, float z, uint32_t dwApply) = 0;
    virtual HRESULT_ __stdcall CommitDeferredSettings() = 0;
};

struct IDirectSoundNotify : IUnknown_ {
    virtual HRESULT_ __stdcall SetNotificationPositions(uint32_t dwPositionNotifies, void* pcPositionNotifies) = 0;
};
typedef IDirectSoundBuffer8 IDirectSoundBuffer;   // same vtable prefix: call only the first 21 methods
