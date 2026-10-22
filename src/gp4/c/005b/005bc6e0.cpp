#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005bc6e0 { uint8_t Unknown00[4]; int32_t Value04; int32_t Value08; uint8_t Unknown0c[0xf4]; };
#pragma pack(pop)

// Stores the two stack arguments at +4 and +8 of the object (`ret 8`).
// FUNCTION: GP4 0x005bc6e0
void __fastcall FUN_005bc6e0(Unknown_005bc6e0* self, void* /*edx*/, int32_t first, int32_t second) {
    self->Value04 = first;
    self->Value08 = second;
}
GP4_IMPL(0x005bc6e0, FUN_005bc6e0, "thiscall(ptr[0x100]:bytes, i32, i32) -> void")
