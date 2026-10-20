#include <gp4/gp4.h>
#include <gp4/flags.h>

// Peeks the key queue head with 0x0052d7c8 (ax = next word or 0, not consumed), then reports through
// the carry flag whether ah is zero: `cmp ah, 0` / `stc` when ah == 0, else `clc`. The other flags
// are those of the compare; only eax comes back from the callee (it preserves the rest).
// FUNCTION: GP4 0x0052d71a
void __cdecl FUN_0052d71a(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x0052d7c8, c);

    const uint32_t ah = (c.eax >> 8) & 0xffu;
    const uint32_t carry = (ah == 0u) ? gp4::flags::CF : 0u;
    const uint32_t flags = (gp4::flags::sub8(ah, 0u) & ~gp4::flags::CF) | carry;
    r->eax = c.eax;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0052d71a, FUN_0052d71a, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keyq_008245c0")
