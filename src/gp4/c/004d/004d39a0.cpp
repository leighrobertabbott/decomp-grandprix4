#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d39a0 { uint32_t Vtable00; uint8_t Unknown04[0xfc]; };
#pragma pack(pop)

// Scalar deleting destructor of the class whose vtable is 0x005f0efc: stores the
// vtable pointer; when bit 0 of flags is set the object itself is freed with
// 0x00446a60. Returns this.
// FUNCTION: GP4 0x004d39a0
Unknown_004d39a0* __fastcall FUN_004d39a0(Unknown_004d39a0* self, void* /*edx*/, uint8_t flags) {
    self->Vtable00 = 0x005f0efc;
    if (flags & 1) {
        gp4::cdecl_<0x00446a60, int>(self);
    }
    return self;
}
GP4_IMPL(0x004d39a0, FUN_004d39a0, "thiscall(ptr[0x100]:bytes, u32) -> i32")
