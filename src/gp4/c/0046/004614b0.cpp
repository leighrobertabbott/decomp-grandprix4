#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Object_004614b0 { uint8_t Unknown00[0x58]; int32_t Value58; };
#pragma pack(pop)

// Sets the dword at +0x58 to 1; takes one ignored stack argument (`ret 4`)
// and leaves eax = 1.
// FUNCTION: GP4 0x004614b0
int32_t __fastcall FUN_004614b0(Object_004614b0* self, void* /*edx*/, int32_t /*unused*/) {
    int32_t one = 1;
    self->Value58 = one;
    return one;
}
GP4_IMPL(0x004614b0, FUN_004614b0, "thiscall(ptr[0x70]:bytes, i32) -> i32")
