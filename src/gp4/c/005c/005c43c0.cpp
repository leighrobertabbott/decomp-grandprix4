#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c43c0 { uint8_t Unknown00[0x2c]; int32_t Value2c; int32_t Value30; uint8_t Unknown34[0xcc]; };
#pragma pack(pop)

// Constructor-like clear: runs the base clear FUN_005bc2e0, zeroes the dwords at +0x2c and +0x30,
// returns this.
// FUNCTION: GP4 0x005c43c0
Unknown_005c43c0* __fastcall FUN_005c43c0(Unknown_005c43c0* self, void* /*edx*/) {
    gp4::thiscall_<0x005bc2e0, void>(self);
    self->Value2c = 0;
    self->Value30 = 0;
    return self;
}
GP4_IMPL(0x005c43c0, FUN_005c43c0, "thiscall(ptr[0x100]:bytes) -> i32")
