#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0045a1c0 { uint8_t Unknown00[8]; int32_t Value08; };
#pragma pack(pop)

// Adds the argument to the dword at +8 of the object.
// FUNCTION: GP4 0x0045a1c0
void __fastcall FUN_0045a1c0(Unknown_0045a1c0* self, void* /*edx*/, int32_t delta) {
    self->Value08 = self->Value08 + delta;
}
GP4_IMPL(0x0045a1c0, FUN_0045a1c0, "thiscall(ptr[0x10]:bytes, i32) -> void")
