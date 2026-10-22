#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c0230 { uint8_t Unknown00[0x134]; int32_t Value134; uint8_t Unknown138[0x28]; int32_t Value160; uint8_t Unknown164[0x20]; };
#pragma pack(pop)

// Stores its stack argument into both the dword at +0x160 and the dword at +0x134.
// FUNCTION: GP4 0x005c0230
void __fastcall FUN_005c0230(Unknown_005c0230* self, void* /*edx*/, int32_t value) {
    self->Value160 = value;
    self->Value134 = value;
}
GP4_IMPL(0x005c0230, FUN_005c0230, "thiscall(ptr[0x190]:bytes, i32) -> void")
