#include <gp4/gp4.h>
#include <gp4/flags.h>

// Spin-wait: `cmp byte [0x7a9469], 0 / jne self`. Each pass leaves the flags of the compare in
// the register block, so the loop is observable and the compiler keeps the read and the branch;
// the exit (byte == 0) leaves a zero-result compare: ZF/PF set, the rest clear. The byte (the
// mouse-button state written by the window procedure) is zero in the image, so the oracle never
// spins.
// FUNCTION: GP4 0x0053ede2
void __cdecl FUN_0053ede2(gp4::Regs* r) {
    uint8_t value;
    do {
        value = GP4_GLOBAL(uint8_t, 0x007a9469);
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(value, 0u));
    } while (value != 0u);
}
GP4_IMPL(0x0053ede2, FUN_0053ede2, "regs() -> (cf, pf, af, zf, sf, of)")
