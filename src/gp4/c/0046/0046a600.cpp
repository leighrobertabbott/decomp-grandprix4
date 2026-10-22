#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_0046a600 { uint8_t Unknown00[0x58]; int32_t Value58; int32_t State5c; uint8_t Unknown60[0xa0]; };
#pragma pack(pop)

// Returns the dword at +0x58 and sets the state dword at +0x5c to 2; the one stack argument is ignored.
// FUNCTION: GP4 0x0046a600
int32_t __fastcall FUN_0046a600(Unknown_0046a600* self, void* /*edx*/, int /*unused*/) {
    int32_t v = self->Value58;
    self->State5c = 2;
    return v;
}
GP4_IMPL(0x0046a600, FUN_0046a600, "thiscall(ptr[0x100]:bytes, i32) -> i32")
