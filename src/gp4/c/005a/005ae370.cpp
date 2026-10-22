#include <gp4/gp4.h>
#include <gp4/dsound.h>

#pragma pack(push, 1)
struct WaveFormat_005ae370 {     // WAVEFORMATEX
    uint16_t FormatTag;
    uint16_t Channels;
    uint32_t SamplesPerSec;
    uint32_t AvgBytesPerSec;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    uint16_t Size;
};
struct BufferDesc_005ae370 {     // DSBUFFERDESC (0x24 bytes)
    uint32_t Size;
    uint32_t Flags;
    uint32_t BufferBytes;
    uint32_t Reserved;
    WaveFormat_005ae370* Format;
    uint32_t Algorithm3D[4];     // guid3DAlgorithm
};
struct BufferCaps_005ae370 {     // DSBCAPS (0x14 bytes)
    uint32_t Size;
    uint32_t Flags;
    uint32_t BufferBytes;
    uint32_t UnlockTransferRate;
    uint32_t PlayCpuOverhead;
};
#pragma pack(pop)

// IKsPropertySet (dsound.h): QueryInterface, AddRef, Release, Get, Set, QuerySupport (+0x14).
struct IKsPropertySet_005ae370 : IUnknown_ {
    virtual HRESULT_ __stdcall Get(const GUID_* set, uint32_t id, void* instance, uint32_t instanceLength, void* data,
                                   uint32_t dataLength, uint32_t* returned) = 0;
    virtual HRESULT_ __stdcall Set(const GUID_* set, uint32_t id, void* instance, uint32_t instanceLength, void* data,
                                   uint32_t dataLength) = 0;
    virtual HRESULT_ __stdcall QuerySupport(const GUID_* set, uint32_t id, uint32_t* support) = 0;
};

#pragma pack(push, 1)
struct Sound_005ae370 {
    uint8_t Unknown000[0x1a4];
    IDirectSound8* DirectSound1a4;
    uint8_t Unknown1a8[0x27c - 0x1a8];
    IDirectSoundBuffer8* Buffer27c;
    IDirectSound3DBuffer* Buffer3D280;
    IKsPropertySet_005ae370* PropertySet284;
    uint8_t Flag288;
    uint8_t PropertySupported289;
};
#pragma pack(pop)

// Creates a 10-byte 11025 Hz mono 8-bit 3D sound buffer, queries its 3D-buffer interface (IID 0x005f62c0) and
// the property set (IID 0x005f62b0), and checks that property id 1 of the set 0x005fbb20 supports get and set
// (+0x289 = 1, returns 0). Failures return 0xbbc / 0xbbb / 0xc80 / 1 and release what was obtained.
// Rerun after the COM model stopped hashing DSBUFFERDESC.lpwfxFormat (a stack address).
// FUNCTION: GP4 0x005ae370
int32_t __fastcall FUN_005ae370(Sound_005ae370* self, void* /*edx*/) {
    BufferDesc_005ae370 desc = {};
    WaveFormat_005ae370 format;
    desc.Algorithm3D[0] = GP4_GLOBAL(uint32_t, 0x005f62a0);
    desc.Algorithm3D[1] = GP4_GLOBAL(uint32_t, 0x005f62a4);
    desc.Algorithm3D[3] = GP4_GLOBAL(uint32_t, 0x005f62ac);
    format.SamplesPerSec = 0x2b11;
    format.AvgBytesPerSec = 0x2b11;
    format.FormatTag = 1;
    format.Channels = 1;
    format.BlockAlign = 1;
    desc.Format = &format;
    desc.Size = 0x24;
    desc.Flags = 0x200b6;
    desc.Algorithm3D[2] = GP4_GLOBAL(uint32_t, 0x005f62a8);
    desc.BufferBytes = 10;
    format.BitsPerSample = 8;
    format.Size = 0;
    if (self->DirectSound1a4->CreateSoundBuffer(&desc, &self->Buffer27c, nullptr) < 0) return 0xbbc;
    if (self->Buffer27c->QueryInterface(GP4_ARRAY(const GUID_, 0x005f62c0), (void**)&self->Buffer3D280) < 0) {
        self->Buffer27c->Release();
        self->Buffer27c = nullptr;
        return 0xbbb;
    }
    BufferCaps_005ae370 caps;
    caps.Size = 0x14;
    self->Buffer27c->GetCaps(&caps);
    if (self->Buffer3D280->QueryInterface(GP4_ARRAY(const GUID_, 0x005f62b0), (void**)&self->PropertySet284) < 0) {
        self->Flag288 = 0;
        self->Buffer27c->Release();
        self->Buffer27c = nullptr;
        self->Buffer3D280->Release();
        self->Buffer3D280 = nullptr;
        return 0xc80;
    }
    uint32_t support = 0;
    if (self->PropertySet284->QuerySupport(GP4_ARRAY(const GUID_, 0x005fbb20), 1, &support) >= 0 &&
        (uint8_t)(support & 3) == 3) {
        self->PropertySupported289 = 1;
        return 0;
    }
    self->Buffer27c->Release();
    self->Buffer27c = nullptr;
    self->Buffer3D280->Release();
    self->Buffer3D280 = nullptr;
    self->PropertySet284->Release();
    self->PropertySet284 = nullptr;
    self->Flag288 = 0;
    return 1;
}
GP4_IMPL(0x005ae370, FUN_005ae370, "thiscall(ptr[0x290]:{*bytes, 0x1a4:ptr[16]:com=IDirectSound8}) -> i32")
