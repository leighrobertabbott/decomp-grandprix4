#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: esi = [0x00a69878] (current record), calls 0x0040837c (which leaves CF = 1 when
// the record's state byte +0x7a has neither bit 7 nor bit 6 set), stores 1 or 0 in 0x00a6987c
// according to that carry. Every GPR is restored by popal; the flags are those 0x0040837c leaves.
// FUNCTION: GP4 0x0051f95b
void __cdecl FUN_0051f95b(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00a69878);

    gp4::call_regs(0x0040837c, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = (c.eflags & 1u) != 0u ? 1u : 0u;

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f95b, FUN_0051f95b, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ptr_00a69878")
