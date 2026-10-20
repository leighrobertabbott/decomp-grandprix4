#include <gp4/gp4.h>

// `mov al, 0x14 ; call 0x0051d494 ; mov al, 0 ; call 0x0051d48e ; ret`: the two byte setters store
// al at 0x0088fd9f (0x14) and 0x0088fda0 (0). Only al changes: it is 0 on return.
// FUNCTION: GP4 0x005142b4
void __cdecl FUN_005142b4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.eax = (c.eax & 0xffffff00u) | 0x14u;
    gp4::call_regs(0x0051d494, c);
    c.eax = c.eax & 0xffffff00u;
    gp4::call_regs(0x0051d48e, c);
    r->eax = c.eax;
}
GP4_IMPL(0x005142b4, FUN_005142b4, "regs(eax:u32) -> (eax)")
