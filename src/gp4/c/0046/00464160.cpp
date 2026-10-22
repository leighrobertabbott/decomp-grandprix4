#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00464160 { int32_t Field00; int32_t Field04; int32_t Field08; int32_t Field0c; };
#pragma pack(pop)

// Initialiser: clears +0, stores its stack argument at +4, clears +8 and +0xc, returns this.
// FUNCTION: GP4 0x00464160
Unknown_00464160* __fastcall FUN_00464160(Unknown_00464160* self, void* /*edx*/, int32_t a) {
    self->Field00 = 0;
    self->Field04 = a;
    self->Field08 = 0;
    self->Field0c = 0;
    return self;
}
GP4_IMPL(0x00464160, FUN_00464160, "thiscall(ptr[0x10]:bytes, i32) -> u32")
