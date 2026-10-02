#include <gp4/gp4.h>

// FUNCTION: GP4 0x004082ee
void __cdecl FUN_004082ee(gp4::Regs* r) {
    r->eax = 0u;
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004082ee, FUN_004082ee, "regs() -> (eax, cf, pf, af, zf, sf, of)")
