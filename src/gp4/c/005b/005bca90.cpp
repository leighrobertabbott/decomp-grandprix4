#include <gp4/gp4.h>
#include <gp4/d3d8.h>

typedef void(__stdcall* OutputDebugStringA_t)(const char* text);

#pragma pack(push, 1)
struct Unknown_005bca90 { uint32_t Field00; uint8_t Unknown04[0x1c]; IDirect3DVertexBuffer8* Buffer20; };
#pragma pack(pop)

// Clears the dword at +0, unbinds stream 0 of the Direct3D device (SetStreamSource(0, NULL, 0)), releases the
// vertex buffer at +0x20 (logging "Failed To Release Line VB!" when Release leaves references) and nulls it.
// FUNCTION: GP4 0x005bca90
void __fastcall FUN_005bca90(Unknown_005bca90* self, void* /*edx*/) {
    self->Field00 = 0;
    GP4_D3D_DEVICE->SetStreamSource(0, nullptr, 0);
    if (self->Buffer20 != nullptr) {
        if (self->Buffer20->Release() != 0) {
            GP4_IAT(OutputDebugStringA_t, 0x005f0290)(GP4_ARRAY(const char, 0x009ed73c));
        }
    }
    self->Buffer20 = nullptr;
}
GP4_IMPL(0x005bca90, FUN_005bca90, "thiscall(ptr[0x30]:{0:u32, 0x20:nptr[16]:com=IDirect3DVertexBuffer8}) -> void; globals=fuzz:i32; gfields={0x00b2e808:ptr[16]:com=IDirect3DDevice8}")
