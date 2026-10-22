#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4c90 { uint8_t Unknown00[0x20]; int32_t Value20; int32_t Value24; int32_t Value28; };
#pragma pack(pop)

// Stores its three stack arguments at +0x20, +0x24 and +0x28, calls the empty routine
// 0x005ae990, returns 0.
// FUNCTION: GP4 0x004d4c90
int __fastcall FUN_004d4c90(Unknown_004d4c90* self, void* /*edx*/, int32_t a, int32_t b, int32_t c) {
    self->Value20 = a;
    self->Value24 = b;
    self->Value28 = c;
    gp4::cdecl_<0x005ae990, void>();
    return 0;
}
GP4_IMPL(0x004d4c90, FUN_004d4c90, "thiscall(ptr[0x38]:bytes, i32, i32, i32) -> i32")
