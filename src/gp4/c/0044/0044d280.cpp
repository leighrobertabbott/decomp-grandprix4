#include <gp4/gp4.h>
#include <gp4/d3d8.h>

#pragma pack(push, 1)
struct Renderer_0044d280 { uint8_t Unknown00[8]; IDirect3DDevice8* Device08; };
#pragma pack(pop)

// Asks the renderer's Direct3D 8 device (at +8) to discard all managed-resource bytes:
// device->ResourceManagerDiscardBytes(0).
// FUNCTION: GP4 0x0044d280
void __fastcall FUN_0044d280(Renderer_0044d280* self, void* /*edx*/) {
    self->Device08->ResourceManagerDiscardBytes(0);
}
GP4_IMPL(0x0044d280, FUN_0044d280, "thiscall(ptr[0x10]:layout=rend_0044d280) -> void")
