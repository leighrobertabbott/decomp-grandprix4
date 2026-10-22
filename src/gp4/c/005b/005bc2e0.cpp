#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bc2e0 {
    int32_t Field00;
    uint8_t Unknown04[0x1c];
    int32_t Field20;
    int32_t Field24;
    int32_t Field28;
    uint8_t Unknown2c[0xd4];
};
#pragma pack(pop)

// Clears the dwords at +0, +0x20, +0x24 and +0x28 and returns the object.
// FUNCTION: GP4 0x005bc2e0
Unknown_005bc2e0* __fastcall FUN_005bc2e0(Unknown_005bc2e0* self, void* /*edx*/) {
    self->Field20 = 0;
    self->Field00 = 0;
    self->Field24 = 0;
    self->Field28 = 0;
    return self;
}
GP4_IMPL(0x005bc2e0, FUN_005bc2e0, "thiscall(ptr[0x100]:bytes) -> i32")
