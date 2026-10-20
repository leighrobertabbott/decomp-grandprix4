#include <gp4/gp4.h>

// Calls the three global getters 0x0051f991, 0x0051f997 and 0x0051f99d in order; only the last
// result (the dword at 0x00a95b9c) stays in eax.
// FUNCTION: GP4 0x0051fab6
uint32_t __cdecl FUN_0051fab6() {
    gp4::cdecl_<0x0051f991, uint32_t>();
    gp4::cdecl_<0x0051f997, uint32_t>();
    return gp4::cdecl_<0x0051f99d, uint32_t>();
}
GP4_IMPL(0x0051fab6, FUN_0051fab6, "cdecl() -> u32; globals=fuzz:i32+layout=get_0051fab6")
