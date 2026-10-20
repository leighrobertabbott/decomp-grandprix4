#include <gp4/gp4.h>

// Stores the fixed-point reciprocal 0x004126e2 (eax = (0x10000 + d/2) / d, d = max(1, [0x7ad810]))
// at 0x007ad8b8. eax keeps the quotient on return and the flags are those 0x004126e2 leaves
// (its final `add eax, edx`); callers follow with `clc; pushfd`, so the other flags are read.
// FUNCTION: GP4 0x00530012
void __cdecl FUN_00530012(gp4::Regs* r) {
    gp4::Regs c = {};
    c.eax = GP4_GLOBAL(uint32_t, 0x007ad810);
    gp4::call_regs(0x004126e2, c);
    GP4_GLOBAL(uint32_t, 0x007ad8b8) = c.eax;

    r->eax = c.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (c.eflags & 0x8d5u);
}
GP4_IMPL(0x00530012, FUN_00530012, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
