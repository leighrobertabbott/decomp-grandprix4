#include <gp4/gp4.h>

// `mov eax, 3 ; call 0x0051f293 ; ret`, called from C code (0x004425e0), whose next instructions
// do not read the flags: modelled as cdecl, since the flags the callee chain
// leaves come from stack-pointer arithmetic in 0x004a96a0 and no caller reads them.
// FUNCTION: GP4 0x00442cea
uint32_t __cdecl FUN_00442cea() {
    gp4::Regs c = {};
    c.eax = 3u;
    gp4::call_regs(0x0051f293, c);
    return c.eax;
}
GP4_IMPL(0x00442cea, FUN_00442cea, "cdecl() -> u32; globals=fuzz:i32")
