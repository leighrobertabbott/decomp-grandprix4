#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c48f0 { int32_t Value00; int32_t Value04; int32_t Value08; uint8_t Unknown0c[0xf4]; };
#pragma pack(pop)

// Zeroes the three dwords at +0, +4 and +8 of the object and returns the object (eax = ecx).
// FUNCTION: GP4 0x005c48f0
Unknown_005c48f0* __fastcall FUN_005c48f0(Unknown_005c48f0* self, void* /*edx*/) {
    self->Value08 = 0;
    self->Value00 = 0;
    self->Value04 = 0;
    return self;
}
GP4_IMPL(0x005c48f0, FUN_005c48f0, "thiscall(ptr[0x100]:bytes) -> i32")
