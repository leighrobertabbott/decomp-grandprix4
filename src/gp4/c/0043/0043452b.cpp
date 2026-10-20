#include <gp4/gp4.h>

// Address of a 0x0063b968 block, or the value of the getter at 0x0051f997 when the byte at
// 0x006f29a1 is zero.
// FUNCTION: GP4 0x0043452b
uint32_t __cdecl FUN_0043452b() {
    if (GP4_GLOBAL(uint8_t, 0x006f29a1) == 0u) {
        return gp4::cdecl_<0x0051f997, uint32_t>();
    }
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x0063b968)));
}
GP4_IMPL(0x0043452b, FUN_0043452b, "cdecl() -> u32; globals=fuzz:i32")
