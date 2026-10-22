#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cae80 { uint8_t Unknown00[0x18]; int32_t Min18; int32_t Value1c; uint8_t Unknown20[0xe0]; };
#pragma pack(pop)

// Stores the argument at +0x1c, raised to the lower bound at +0x18 when it is below it.
// FUNCTION: GP4 0x005cae80
void __fastcall FUN_005cae80(Unknown_005cae80* self, void* /*edx*/, int32_t value) {
    self->Value1c = value;
    if (value < self->Min18) {
        self->Value1c = self->Min18;
    }
}
GP4_IMPL(0x005cae80, FUN_005cae80, "thiscall(ptr[0x100]:{0x18:i32[-8..8], 0x1c:i32}, i32[-8..8]) -> void")
