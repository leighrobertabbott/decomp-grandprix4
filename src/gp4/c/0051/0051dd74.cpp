#include <gp4/gp4.h>
#include <gp4/flags.h>

// Asks 0x0051ddda (flag-bit test through a byte table, key = bl) about the byte [0x007ab569]. A
// zero key answers at once: CF = 0 (clc) with the other flags of `cmp bl, 0`. Otherwise the
// callee's flags are kept, CF is forced by the epilogue: callee CF set -> clc, eax = the callee's
// eax; callee CF clear -> stc, eax = 1 unless the callee's ZF is set (then 0). All registers but
// eax are saved and restored.
// FUNCTION: GP4 0x0051dd74
void __cdecl FUN_0051dd74(gp4::Regs* r) {
    const uint8_t key = GP4_GLOBAL(uint8_t, 0x007ab569);
    if (key == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(key, 0u) & ~gp4::flags::CF);
        return;
    }
    gp4::Regs c = *r;
    c.ebx = (c.ebx & 0xffffff00u) | key;
    gp4::call_regs(0x0051ddda, c);
    if ((c.eflags & gp4::flags::CF) != 0u) {
        r->eax = c.eax;
        r->eflags = gp4::flags::merge(r->eflags, c.eflags & ~gp4::flags::CF);
        return;
    }
    r->eax = (c.eflags & gp4::flags::ZF) != 0u ? 0u : 1u;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags | gp4::flags::CF);
}
GP4_IMPL(0x0051dd74, FUN_0051dd74, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=bits_006f2d90+codes_0051dd08")
