#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005cf340 { uint8_t Unknown00[0x10]; int32_t Value10; int32_t Value14; uint8_t Unknown18[0xe8]; };
#pragma pack(pop)

// Stores the two stack arguments at +0x10 and +0x14 of the object (`ret 8`).
// FUNCTION: GP4 0x005cf340
void __fastcall FUN_005cf340(Unknown_005cf340* self, void* /*edx*/, int32_t first, int32_t second) {
    self->Value10 = first;
    self->Value14 = second;
}
GP4_IMPL(0x005cf340, FUN_005cf340, "thiscall(ptr[0x100]:bytes, i32, i32) -> void")
