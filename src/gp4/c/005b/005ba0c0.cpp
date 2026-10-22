#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ba0c0 { int32_t Field00; int32_t Field04; int32_t Field08; int32_t Field0c; uint8_t Unknown10[0xf0]; };
#pragma pack(pop)

// Clears the four dwords at +0, +4, +8 and +0xc and returns the object.
// FUNCTION: GP4 0x005ba0c0
Unknown_005ba0c0* __fastcall FUN_005ba0c0(Unknown_005ba0c0* self, void* /*edx*/) {
    self->Field08 = 0;
    self->Field0c = 0;
    self->Field00 = 0;
    self->Field04 = 0;
    return self;
}
GP4_IMPL(0x005ba0c0, FUN_005ba0c0, "thiscall(ptr[0x100]:bytes) -> i32")
