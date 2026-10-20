#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the pending-amount flag [0x00838e88] is set, adds the amount [0x00838e8c] to the running total
// with 0x00538a24 and clears the flag and the byte [0x00838e90]. eax is pushed and popped, so every
// register is kept; the flags are those of `cmp [0x838e88], 0` when idle, else those 0x00538a24
// leaves (the stores touch none).
// FUNCTION: GP4 0x0054ec64
void __cdecl FUN_0054ec64(gp4::Regs* r) {
    const uint32_t pending = GP4_GLOBAL(uint32_t, 0x00838e88);
    const uint32_t arith = gp4::flags::sub32(pending, 0u);
    if (pending == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, arith);
        return;
    }

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eflags = gp4::flags::merge(c.eflags, arith);
    c.eax = GP4_GLOBAL(uint32_t, 0x00838e8c);
    gp4::call_regs(0x00538a24, c);
    GP4_GLOBAL(uint32_t, 0x00838e88) = 0u;
    GP4_GLOBAL(uint8_t, 0x00838e90) = 0u;

    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0054ec64, FUN_0054ec64, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
