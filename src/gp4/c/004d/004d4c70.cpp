#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4c70 { uint8_t Unknown00[0x14]; int32_t Value14; int32_t Value18; int32_t Value1c; };
#pragma pack(pop)

// Stores its three stack arguments at +0x14, +0x18 and +0x1c, calls the empty routine
// 0x005ae990, returns 0.
// FUNCTION: GP4 0x004d4c70
int __fastcall FUN_004d4c70(Unknown_004d4c70* self, void* /*edx*/, int32_t a, int32_t b, int32_t c) {
    self->Value14 = a;
    self->Value18 = b;
    self->Value1c = c;
    gp4::cdecl_<0x005ae990, void>();
    return 0;
}
GP4_IMPL(0x004d4c70, FUN_004d4c70, "thiscall(ptr[0x30]:bytes, i32, i32, i32) -> i32")
