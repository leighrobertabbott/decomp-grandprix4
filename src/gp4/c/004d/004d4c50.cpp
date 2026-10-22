#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4c50 { uint8_t Unknown00[0x38]; int32_t Value38; int32_t Value3c; };
#pragma pack(pop)

// Stores its two stack arguments at +0x38 and +0x3c, calls the empty routine 0x005ae990, returns 0.
// FUNCTION: GP4 0x004d4c50
int __fastcall FUN_004d4c50(Unknown_004d4c50* self, void* /*edx*/, int32_t a, int32_t b) {
    self->Value38 = a;
    self->Value3c = b;
    gp4::cdecl_<0x005ae990, void>();
    return 0;
}
GP4_IMPL(0x004d4c50, FUN_004d4c50, "thiscall(ptr[0x48]:bytes, i32, i32) -> i32")
