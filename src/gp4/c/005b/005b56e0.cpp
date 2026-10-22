#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005b56e0 { uint8_t Unknown00[0x2c]; int32_t Value2c; uint8_t Flag30; uint8_t Unknown31[0xcf]; };
#pragma pack(pop)

// Clears the dword at +0x2c, runs the line-vertex-buffer release routine 0x005bca90 on the
// object, then clears the byte at +0x30.
// FUNCTION: GP4 0x005b56e0
void __fastcall FUN_005b56e0(Unknown_005b56e0* self, void* /*edx*/) {
    self->Value2c = 0;
    gp4::thiscall_<0x005bca90, void>(self);
    self->Flag30 = 0;
}
GP4_IMPL(0x005b56e0, FUN_005b56e0, "thiscall(ptr[0x40]:{0x20:nptr[16]:com=IDirect3DVertexBuffer8}) -> void; globals=fuzz:i32; gfields={0x00b2e808:ptr[16]:com=IDirect3DDevice8}")
