#include <gp4/gp4.h>

#pragma pack(push, 1)
struct Unknown_005c7830 { uint8_t Unknown00[0x380]; uint8_t Flag380; };
#pragma pack(pop)

// Clears the global dword at 0x00b4764c and the byte at +0x380 of the object; returns 0 and
// ignores its stack argument (`ret 4`).
// FUNCTION: GP4 0x005c7830
int32_t __fastcall FUN_005c7830(Unknown_005c7830* self, void* /*edx*/, int32_t /*unused*/) {
    GP4_GLOBAL(uint32_t, 0x00b4764c) = 0;
    self->Flag380 = 0;
    return 0;
}
GP4_IMPL(0x005c7830, FUN_005c7830, "thiscall(ptr[0x388]:bytes, i32) -> i32; globals=fuzz:i32")
