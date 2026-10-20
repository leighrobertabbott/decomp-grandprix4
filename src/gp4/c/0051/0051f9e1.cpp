#include <gp4/gp4.h>

// pushal / call 0x00433fec / mov [0xa6987c], eax / popal / ret: stores the result of 0x00433fec
// into the global at 0x00a6987c; every register and flag is preserved.
// FUNCTION: GP4 0x0051f9e1
void __cdecl FUN_0051f9e1(gp4::Regs* /*r*/) {
    const uint32_t value = gp4::cdecl_<0x00433fec, uint32_t>();
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value;
}
GP4_IMPL(0x0051f9e1, FUN_0051f9e1, "regs() -> (); globals=fuzz:i32")
