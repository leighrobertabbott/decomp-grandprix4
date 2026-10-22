// DirectInput 8 (ANSI) interfaces as GP4 calls them (Windows SDK dinput.h, DIRECTINPUT_VERSION
// 0x0800): method order and argument counts from the SDK's DECLARE_INTERFACE_ blocks. Pointer
// arguments are void* (cast at the call); modelled by the oracle in gp4re/com_dx.py (layout fill
// com=IDirectInputDevice8A etc.).
#pragma once
#include <gp4/com.h>

struct IDirectInput8A : IUnknown_ {
    virtual HRESULT_ __stdcall CreateDevice(const GUID_* a0, void* a1, void* a2) = 0;
    virtual HRESULT_ __stdcall EnumDevices(uint32_t a0, void* a1, void* a2, uint32_t a3) = 0;
    virtual HRESULT_ __stdcall GetDeviceStatus(const GUID_* a0) = 0;
    virtual HRESULT_ __stdcall RunControlPanel(void* a0, uint32_t a1) = 0;
    virtual HRESULT_ __stdcall Initialize(void* a0, uint32_t a1) = 0;
    virtual HRESULT_ __stdcall FindDevice(const GUID_* a0, void* a1, const GUID_* a2) = 0;
    virtual HRESULT_ __stdcall EnumDevicesBySemantics(void* a0, void* a1, void* a2, void* a3, uint32_t a4) = 0;
    virtual HRESULT_ __stdcall ConfigureDevices(void* a0, void* a1, uint32_t a2, void* a3) = 0;
};

struct IDirectInputDevice8A : IUnknown_ {
    virtual HRESULT_ __stdcall GetCapabilities(void* a0) = 0;
    virtual HRESULT_ __stdcall EnumObjects(void* a0, void* a1, uint32_t a2) = 0;
    virtual HRESULT_ __stdcall GetProperty(const GUID_* a0, void* a1) = 0;
    virtual HRESULT_ __stdcall SetProperty(const GUID_* a0, void* a1) = 0;
    virtual HRESULT_ __stdcall Acquire() = 0;
    virtual HRESULT_ __stdcall Unacquire() = 0;
    virtual HRESULT_ __stdcall GetDeviceState(uint32_t a0, void* a1) = 0;
    virtual HRESULT_ __stdcall GetDeviceData(uint32_t a0, void* a1, void* a2, uint32_t a3) = 0;
    virtual HRESULT_ __stdcall SetDataFormat(void* a0) = 0;
    virtual HRESULT_ __stdcall SetEventNotification(void* a0) = 0;
    virtual HRESULT_ __stdcall SetCooperativeLevel(void* a0, uint32_t a1) = 0;
    virtual HRESULT_ __stdcall GetObjectInfo(void* a0, uint32_t a1, uint32_t a2) = 0;
    virtual HRESULT_ __stdcall GetDeviceInfo(void* a0) = 0;
    virtual HRESULT_ __stdcall RunControlPanel(void* a0, uint32_t a1) = 0;
    virtual HRESULT_ __stdcall Initialize(void* a0, uint32_t a1, const GUID_* a2) = 0;
    virtual HRESULT_ __stdcall CreateEffect(const GUID_* a0, void* a1, void* a2, void* a3) = 0;
    virtual HRESULT_ __stdcall EnumEffects(void* a0, void* a1, uint32_t a2) = 0;
    virtual HRESULT_ __stdcall GetEffectInfo(void* a0, const GUID_* a1) = 0;
    virtual HRESULT_ __stdcall GetForceFeedbackState(void* a0) = 0;
    virtual HRESULT_ __stdcall SendForceFeedbackCommand(uint32_t a0) = 0;
    virtual HRESULT_ __stdcall EnumCreatedEffectObjects(void* a0, void* a1, uint32_t a2) = 0;
    virtual HRESULT_ __stdcall Escape(void* a0) = 0;
    virtual HRESULT_ __stdcall Poll() = 0;
    virtual HRESULT_ __stdcall SendDeviceData(uint32_t a0, void* a1, void* a2, uint32_t a3) = 0;
    virtual HRESULT_ __stdcall EnumEffectsInFile(void* a0, void* a1, void* a2, uint32_t a3) = 0;
    virtual HRESULT_ __stdcall WriteEffectToFile(void* a0, uint32_t a1, void* a2, uint32_t a3) = 0;
    virtual HRESULT_ __stdcall BuildActionMap(void* a0, void* a1, uint32_t a2) = 0;
    virtual HRESULT_ __stdcall SetActionMap(void* a0, void* a1, uint32_t a2) = 0;
    virtual HRESULT_ __stdcall GetImageInfo(void* a0) = 0;
};

struct IDirectInputEffect : IUnknown_ {
    virtual HRESULT_ __stdcall Initialize(void* a0, uint32_t a1, const GUID_* a2) = 0;
    virtual HRESULT_ __stdcall GetEffectGuid(GUID_* a0) = 0;
    virtual HRESULT_ __stdcall GetParameters(void* a0, uint32_t a1) = 0;
    virtual HRESULT_ __stdcall SetParameters(void* a0, uint32_t a1) = 0;
    virtual HRESULT_ __stdcall Start(uint32_t a0, uint32_t a1) = 0;
    virtual HRESULT_ __stdcall Stop() = 0;
    virtual HRESULT_ __stdcall GetEffectStatus(void* a0) = 0;
    virtual HRESULT_ __stdcall Download() = 0;
    virtual HRESULT_ __stdcall Unload() = 0;
    virtual HRESULT_ __stdcall Escape(void* a0) = 0;
};
