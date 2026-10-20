#include <gp4/gp4.h>

// `mov eax, 1 ; call 0x0051f293 ; ret`, called from C code (0x004425e0), whose next instructions
// do not read the flags: modelled as cdecl, since the flags the callee chain
// leaves come from stack-pointer arithmetic in 0x004a96a0 and no caller reads them.
// FUNCTION: GP4 0x00442cdf
uint32_t __cdecl FUN_00442cdf() {
    gp4::Regs c = {};
    c.eax = 1u;
    gp4::call_regs(0x0051f293, c);
    return c.eax;
}
GP4_IMPL(0x00442cdf, FUN_00442cdf, "cdecl() -> u32; globals=fuzz:i32")
