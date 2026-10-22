#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_00448070 { uint8_t Unknown00[0x58]; int32_t Value58; int32_t Value5c; };
#pragma pack(pop)

// Resets the object: +0x58 = 0, +0x5c = 3; ignores its stack argument and returns 0.
// FUNCTION: GP4 0x00448070
int __fastcall FUN_00448070(Unknown_00448070* self, void* /*edx*/, int32_t /*unused*/) {
    self->Value58 = 0;
    self->Value5c = 3;
    return 0;
}
GP4_IMPL(0x00448070, FUN_00448070, "thiscall(ptr[0x70]:bytes, i32) -> i32")
