#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns in eax the index (0..3) of the first of the bits 0x10, 0x20, 0x40, 0x80 that is set in the
// state dword [0x006f3adc]; eax is unchanged when none is set. The flags are those of the last
// `test dword [0x006f3adc], mask` executed.
// FUNCTION: GP4 0x005259ce
void __cdecl FUN_005259ce(gp4::Regs* r) {
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x006f3adc);
    uint32_t arith;
    if ((state & 0x10u) != 0u) {
        arith = gp4::flags::logic32(state & 0x10u);
        r->eax = 0u;
    } else if ((state & 0x20u) != 0u) {
        arith = gp4::flags::logic32(state & 0x20u);
        r->eax = 1u;
    } else if ((state & 0x40u) != 0u) {
        arith = gp4::flags::logic32(state & 0x40u);
        r->eax = 2u;
    } else {
        arith = gp4::flags::logic32(state & 0x80u);
        if ((state & 0x80u) != 0u) {
            r->eax = 3u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005259ce, FUN_005259ce, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
