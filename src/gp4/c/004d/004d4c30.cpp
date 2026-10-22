#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_004d4c30 { uint8_t Unknown00[0x10]; uint8_t Value10; };
#pragma pack(pop)

// Stores the low byte of its stack argument at +0x10, calls the empty routine 0x005ae990, returns 0.
// FUNCTION: GP4 0x004d4c30
int __fastcall FUN_004d4c30(Unknown_004d4c30* self, void* /*edx*/, uint8_t value) {
    self->Value10 = value;
    gp4::cdecl_<0x005ae990, void>();
    return 0;
}
GP4_IMPL(0x004d4c30, FUN_004d4c30, "thiscall(ptr[0x18]:bytes, i32) -> i32")
