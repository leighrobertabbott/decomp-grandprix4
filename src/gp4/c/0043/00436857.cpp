#include <gp4/gp4.h>

// pushal; esi = 0x00629e0c, edi = 0x00629e34; call 0x0043679c (carry = a collision was found);
// when none, swap esi/edi and call it again. popal restores every register; the routine returns
// stc when either call collided, else clc. 0x0043679c's carry is exactly the outcome, so the flags
// are those of the last call made (stc/clc only restate its carry).
// FUNCTION: GP4 0x00436857
void __cdecl FUN_00436857(gp4::Regs* r) {
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    callee.esi = 0x00629e0cu;
    callee.edi = 0x00629e34u;
    gp4::call_regs(0x0043679c, callee);

    if ((callee.eflags & 1u) == 0u) {
        callee.esi = 0x00629e34u;
        callee.edi = 0x00629e0cu;
        gp4::call_regs(0x0043679c, callee);
    }

    const uint32_t carry = callee.eflags & 1u;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d4u) | carry;
}
GP4_IMPL(0x00436857, FUN_00436857, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
