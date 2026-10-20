#include <gp4/gp4.h>

// `mov eax, 0 ; call 0x0051f293 ; ret`, called from C code (0x00442d50), whose next instruction
// (test byte [0x7aafd4]) overwrites the flags: modelled as cdecl, since the flags the callee chain
// leaves come from stack-pointer arithmetic in 0x004a96a0 and no caller reads them.
// FUNCTION: GP4 0x00442cd4
uint32_t __cdecl FUN_00442cd4() {
    gp4::Regs c = {};
    c.eax = 0u;
    gp4::call_regs(0x0051f293, c);
    return c.eax;
}
GP4_IMPL(0x00442cd4, FUN_00442cd4, "cdecl() -> u32; globals=fuzz:i32")
