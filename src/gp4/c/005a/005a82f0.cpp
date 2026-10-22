#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005a82f0 { uint8_t Unknown00[0xf4]; int32_t Valuef4; uint8_t Unknownf8[0x10]; };
#pragma pack(pop)

// Stores the stack argument at +0xf4 of the object and returns 0.
// FUNCTION: GP4 0x005a82f0
int __fastcall FUN_005a82f0(Unknown_005a82f0* self, void* /*edx*/, int32_t value) {
    self->Valuef4 = value;
    return 0;
}
GP4_IMPL(0x005a82f0, FUN_005a82f0, "thiscall(ptr[0x108]:bytes, i32) -> i32")
