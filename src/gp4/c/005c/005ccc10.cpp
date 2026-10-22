#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005ccc10 { int32_t Field00; int32_t Field04; int32_t Field08; int32_t Field0c; int32_t Field10; uint8_t Unknown14[0xec]; };
#pragma pack(pop)

// Clears the five dwords at +0..+0x10 and returns the object.
// FUNCTION: GP4 0x005ccc10
Unknown_005ccc10* __fastcall FUN_005ccc10(Unknown_005ccc10* self, void* /*edx*/) {
    self->Field00 = 0;
    self->Field04 = 0;
    self->Field08 = 0;
    self->Field0c = 0;
    self->Field10 = 0;
    return self;
}
GP4_IMPL(0x005ccc10, FUN_005ccc10, "thiscall(ptr[0x100]:bytes) -> i32")
