#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs the track-ring marking walk 0x005523f2 over the records [0x008943a0, [0x00866ecc])
// and then over [[0x00866ed0], [0x00866ed4]). Every GPR is restored; the flags are those the second
// walk leaves (its final `cmp edi, ebp`).
// FUNCTION: GP4 0x00552499
void __cdecl FUN_00552499(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edi = 0x008943a0u;
    c.ebp = GP4_GLOBAL(uint32_t, 0x00866ecc);
    gp4::call_regs(0x005523f2, c);
    c.edi = GP4_GLOBAL(uint32_t, 0x00866ed0);
    c.ebp = GP4_GLOBAL(uint32_t, 0x00866ed4);
    gp4::call_regs(0x005523f2, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00552499, FUN_00552499, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ring_008943a0+words_005523f2+bounds_00406a57")
