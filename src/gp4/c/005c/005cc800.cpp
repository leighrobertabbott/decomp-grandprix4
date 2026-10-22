#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cc800 { uint8_t Unknown00[0x10]; uint32_t Value10; uint32_t Value14; uint32_t Value18; uint32_t Value1c; uint8_t Unknown20[0xe0]; };
#pragma pack(pop)

// Stores 1.0f (0x3f800000) at +0x10 and zeroes +0x14, +0x18 and +0x1c; returns the object.
// FUNCTION: GP4 0x005cc800
Unknown_005cc800* __fastcall FUN_005cc800(Unknown_005cc800* self, void* /*edx*/) {
    self->Value10 = 0x3f800000u;
    self->Value14 = 0;
    self->Value18 = 0;
    self->Value1c = 0;
    return self;
}
GP4_IMPL(0x005cc800, FUN_005cc800, "thiscall(ptr[0x100]:bytes) -> i32")
