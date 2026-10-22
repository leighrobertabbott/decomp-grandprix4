#include <gp4/gp4.h>
#include <gp4/d3d8.h>

#pragma pack(push, 1)
struct Renderer_00495ee0 { uint8_t Unknown00[8]; IDirect3DDevice8* Device08; };
#pragma pack(pop)

// Takes the renderer object in 0xa4dc28, then calls AddRef followed by Release on its Direct3D 8 device
// (at +8); returns Release's result.
// FUNCTION: GP4 0x00495ee0
uint32_t __cdecl FUN_00495ee0() {
    IDirect3DDevice8* device = GP4_GLOBAL(Renderer_00495ee0*, 0x00a4dc28)->Device08;
    device->AddRef();
    return device->Release();
}
GP4_IMPL(0x00495ee0, FUN_00495ee0, "cdecl() -> u32; gfields={0x00a4dc28:ptr[16]:{8:ptr[16]:com=IDirect3DDevice8}}")
