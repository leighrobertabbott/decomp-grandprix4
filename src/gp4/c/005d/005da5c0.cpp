#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005da5c0 { uint8_t Unknown00[0x14]; int32_t Value14; int32_t Value18; uint8_t Unknown1c[0xe4]; };
#pragma pack(pop)

// Stores the two stack arguments at +0x14 and +0x18 of the object (`ret 8`).
// FUNCTION: GP4 0x005da5c0
void __fastcall FUN_005da5c0(Unknown_005da5c0* self, void* /*edx*/, int32_t first, int32_t second) {
    self->Value14 = first;
    self->Value18 = second;
}
GP4_IMPL(0x005da5c0, FUN_005da5c0, "thiscall(ptr[0x100]:bytes, i32, i32) -> void")
