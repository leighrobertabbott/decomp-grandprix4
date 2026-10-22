#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005caea0 { uint8_t Unknown00[0x18]; int32_t Value18; int32_t Max1c; uint8_t Unknown20[0xe0]; };
#pragma pack(pop)

// Stores the argument at +0x18, lowered to the upper bound at +0x1c when it is above it.
// FUNCTION: GP4 0x005caea0
void __fastcall FUN_005caea0(Unknown_005caea0* self, void* /*edx*/, int32_t value) {
    self->Value18 = value;
    if (self->Max1c < value) {
        self->Value18 = self->Max1c;
    }
}
GP4_IMPL(0x005caea0, FUN_005caea0, "thiscall(ptr[0x100]:{0x18:i32, 0x1c:i32[-8..8]}, i32[-8..8]) -> void")
