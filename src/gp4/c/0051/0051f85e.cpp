#include <gp4/gp4.h>

// pushal / call 0x00433d7b / mov [0xa6987c], eax / popal / ret: stores the result of 0x00433d7b
// into the global at 0x00a6987c; every register and flag is preserved.
// FUNCTION: GP4 0x0051f85e
void __cdecl FUN_0051f85e(gp4::Regs* /*r*/) {
    const uint32_t value = gp4::cdecl_<0x00433d7b, uint32_t>();
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value;
}
GP4_IMPL(0x0051f85e, FUN_0051f85e, "regs() -> (); globals=fuzz:i32")
