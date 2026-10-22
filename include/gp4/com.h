// Shared COM basics for the DirectX interface headers (d3d8.h, dinput8.h, dsound.h), declared
// without <windows.h>. Interfaces are abstract structs whose __stdcall virtual methods follow the
// SDK vtable order, so calls compile to the original's `mov ecx,[eax]; push ...; push eax;
// call [ecx+N]`. The oracle models them in gp4re/com.py and gp4re/com_dx.py.
#pragma once
#include <stdint.h>

typedef int32_t HRESULT_;   // HRESULT (no <windows.h>)

struct GUID_ { uint32_t d1; uint16_t d2, d3; uint8_t d4[8]; };

struct IUnknown_ {
    virtual HRESULT_ __stdcall QueryInterface(const GUID_* riid, void** ppvObj) = 0;
    virtual uint32_t __stdcall AddRef() = 0;
    virtual uint32_t __stdcall Release() = 0;
};
